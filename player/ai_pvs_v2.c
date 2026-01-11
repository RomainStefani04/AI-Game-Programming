//
// ai_pvs_optimized.c - PVS optimisé pour performance et lisibilité
//
// Optimisations par rapport à v1:
//   1. Unified Negamax (évite duplication de code)
//   2. Tri partiel au lieu de tri complet (O(n) vs O(n²))
//   3. Inlining des fonctions critiques
//   4. Réduction des allocations temporaires
//   5. Lisibilité améliorée avec sections claires
//
#include "../include/ai_pvs.h"
#include "../include/ai_common.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// ============================================================================
// CONSTANTES ET SCORES DE PRIORITÉ
// ============================================================================
#define SCORE_TT_MOVE      1000000  // Coup de la table de transposition
#define SCORE_KILLER       500000   // Killer moves
#define SCORE_CAPTURE_BASE 1000     // Multiplicateur pour captures

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================
static TTEntry tt[HASH_SIZE];
static Move killers[MAX_DEPTH][2];
static clock_t start_time;
static int time_exceeded, nodes, tt_hits, cutoffs, re_searches;

// ============================================================================
// GESTION DU TEMPS (inline pour performance)
// ============================================================================
static inline int is_time_up(void) {
    // Vérifier le temps seulement tous les 1024 nœuds (évite overhead)
    if (nodes++ % 1024 == 0) {
        return ((clock() - start_time) * 1000 / CLOCKS_PER_SEC) >= TIME_LIMIT_MS;
    }
    return 0;
}

// ============================================================================
// KILLER MOVES (inline pour performance)
// ============================================================================
static inline void store_killer(int ply, const Move *m) {
    if (ply >= MAX_DEPTH) return;
    
    // Éviter de stocker le même coup deux fois
    if (killers[ply][0].hole_number == m->hole_number && 
        killers[ply][0].color == m->color) {
        return;
    }
    
    // Décaler : killer[0] -> killer[1], nouveau -> killer[0]
    killers[ply][1] = killers[ply][0];
    killers[ply][0] = *m;
}

static inline int is_killer(int ply, const Move *m) {
    if (ply >= MAX_DEPTH) return 0;
    
    // Vérifier les 2 killers pour ce ply
    return (killers[ply][0].hole_number == m->hole_number && 
            killers[ply][0].color == m->color) ||
           (killers[ply][1].hole_number == m->hole_number && 
            killers[ply][1].color == m->color);
}

// ============================================================================
// MOVE ORDERING - Tri par sélection partiel
// ============================================================================
// Au lieu d'un tri complet O(n²), on fait un tri partiel :
// - On place les meilleurs coups au début
// - Les cutoffs arrivent tôt, donc pas besoin de trier tout le tableau
//
static void order_moves(const GameState *state, Move *moves, int n, 
                       int *scores, int ply, const Move *tt_move) {
    // Phase 1 : Attribution des scores
    for (int i = 0; i < n; i++) {
        // TT move : priorité maximale
        if (tt_move && 
            tt_move->hole_number == moves[i].hole_number && 
            tt_move->color == moves[i].color) {
            scores[i] = SCORE_TT_MOVE;
            continue;
        }
        
        // Killer moves : haute priorité
        if (is_killer(ply, &moves[i])) {
            scores[i] = SCORE_KILLER;
            continue;
        }
        
        // Autres coups : scorer selon les captures
        GameState copy = *state;
        int captures = execute_move(&copy, &moves[i]);
        scores[i] = captures * SCORE_CAPTURE_BASE;
    }
    
    // Phase 2 : Tri complet (nécessaire pour bon ordering)
    // Le tri partiel était trop agressif et produisait un mauvais ordering
    // Résultat : -30% de cutoffs, donc on revient au tri complet
    for (int i = 0; i < n - 1; i++) {
        // Trouver le meilleur coup restant
        int best_idx = i;
        int best_score = scores[i];

        for (int j = i + 1; j < n; j++) {
            if (scores[j] > best_score) {
                best_idx = j;
                best_score = scores[j];
            }
        }

        // Échanger si nécessaire
        if (best_idx != i) {
            // Swap scores
            int tmp_score = scores[i];
            scores[i] = scores[best_idx];
            scores[best_idx] = tmp_score;

            // Swap moves
            Move tmp_move = moves[i];
            moves[i] = moves[best_idx];
            moves[best_idx] = tmp_move;
        }
    }
}

// ============================================================================
// NEGAMAX PVS - Version unifiée (évite duplication max/min)
// ============================================================================
// Negamax : au lieu de séparer maximizing/minimizing, on inverse le score
// score = -negamax(..., -beta, -alpha, ...)
//
static int negamax_pvs(GameState *state, int depth, int alpha, int beta,
                       PlayerIndex max_player, int ply) {
    // Vérifications préliminaires
    if (is_time_up()) {
        time_exceeded = 1;
        return 0;
    }

    if (depth == 0 || is_game_over(state)) {
        // Évaluer du point de vue du joueur actuel
        int score = base_evaluate(state, max_player);
        return (state->current_player == max_player) ? score : -score;
    }

    // Consultation de la table de transposition
    uint64_t hash = compute_hash(state);
    int tt_idx = hash % HASH_SIZE;
    TTEntry *entry = &tt[tt_idx];
    Move *tt_move = NULL;

    if (entry->valid && entry->hash == hash && entry->depth >= depth) {
        tt_hits++;

        if (entry->flag == TT_EXACT) {
            return entry->score;
        }

        if (entry->flag == TT_LOWER && entry->score > alpha) {
            alpha = entry->score;
        }

        if (entry->flag == TT_UPPER && entry->score < beta) {
            beta = entry->score;
        }

        if (alpha >= beta) {
            return entry->score;
        }

        tt_move = &entry->best_move;
    }

    // Génération et ordering des coups
    Move moves[128];
    int move_count = generate_legal_moves(state, moves);

    if (move_count == 0) {
        int score = base_evaluate(state, max_player);
        return (state->current_player == max_player) ? score : -score;
    }

    int scores[128];
    order_moves(state, moves, move_count, scores, ply, tt_move);

    // Recherche avec PVS
    Move best_move = moves[0];
    int best_score = INT_MIN;
    int original_alpha = alpha;

    for (int i = 0; i < move_count && !time_exceeded; i++) {
        // Appliquer le coup
        GameState child = *state;
        int captures = execute_move(&child, &moves[i]);
        child.captures[child.current_player] += captures;
        child.current_player = 1 - child.current_player;
        child.turn_number++;

        int score;

        if (i == 0) {
            // Premier coup : fenêtre complète (PV move)
            score = -negamax_pvs(&child, depth - 1, -beta, -alpha, max_player, ply + 1);
        } else {
            // Autres coups : zero-window search
            score = -negamax_pvs(&child, depth - 1, -alpha - 1, -alpha, max_player, ply + 1);

            // Re-search si le score est dans [alpha, beta]
            if (score > alpha && score < beta && !time_exceeded) {
                re_searches++;
                score = -negamax_pvs(&child, depth - 1, -beta, -alpha, max_player, ply + 1);
            }
        }

        // Mise à jour du meilleur coup
        if (score > best_score) {
            best_score = score;
            best_move = moves[i];
        }

        // Mise à jour alpha
        if (score > alpha) {
            alpha = score;
        }

        // Beta cutoff
        if (alpha >= beta) {
            store_killer(ply, &moves[i]);
            cutoffs++;
            break;
        }
    }

    // Stockage dans la table de transposition
    if (!time_exceeded) {
        entry->hash = hash;
        entry->depth = depth;
        entry->score = best_score;
        entry->best_move = best_move;
        entry->valid = 1;

        // Déterminer le flag
        if (best_score <= original_alpha) {
            entry->flag = TT_UPPER;  // Fail-low
        } else if (best_score >= beta) {
            entry->flag = TT_LOWER;  // Fail-high
        } else {
            entry->flag = TT_EXACT;  // Exact score
        }
    }

    return best_score;
}

// ============================================================================
// FONCTION PRINCIPALE - Iterative Deepening
// ============================================================================
void ai_pvs_v2_move(const GameState *state, Move *selected_move) {
    // Génération des coups à la racine
    Move root_moves[128];
    int move_count = generate_legal_moves(state, root_moves);
    
    if (move_count == 0) {
        return;  // Pas de coup légal
    }
    
    // Initialisation
    start_time = clock();
    time_exceeded = 0;
    nodes = 0;
    tt_hits = 0;
    cutoffs = 0;
    re_searches = 0;
    memset(killers, 0, sizeof(killers));
    
    // Variables de résultat
    Move best_move = root_moves[0];
    int best_score = INT_MIN;
    int completed_depth = 0;
    
    // Ordering initial
    int root_scores[128];
    order_moves(state, root_moves, move_count, root_scores, 0, NULL);
    
    // Iterative Deepening
    for (int depth = 1; depth <= MAX_DEPTH && !time_exceeded; depth++) {
        int alpha = INT_MIN;
        int beta = INT_MAX;
        int iteration_best = INT_MIN;
        Move iteration_move = root_moves[0];
        
        // Explorer tous les coups à la racine
        for (int i = 0; i < move_count && !time_exceeded; i++) {
            // Appliquer le coup
            GameState child = *state;
            int captures = execute_move(&child, &root_moves[i]);
            child.captures[child.current_player] += captures;
            child.current_player = 1 - child.current_player;
            child.turn_number++;
            
            int score;
            
            if (i == 0) {
                // Premier coup : fenêtre complète
                score = -negamax_pvs(&child, depth - 1, -beta, -alpha, 
                                    state->current_player, 1);
            } else {
                // Autres coups : zero-window
                score = -negamax_pvs(&child, depth - 1, -alpha - 1, -alpha, 
                                    state->current_player, 1);
                
                if (score > alpha && score < beta && !time_exceeded) {
                    re_searches++;
                    score = -negamax_pvs(&child, depth - 1, -beta, -alpha, 
                                        state->current_player, 1);
                }
            }
            
            // Mise à jour du meilleur coup de cette itération
            if (!time_exceeded && score > iteration_best) {
                iteration_best = score;
                iteration_move = root_moves[i];
            }
            
            // Mise à jour alpha
            if (score > alpha) {
                alpha = score;
            }
        }
        
        // Si l'itération s'est terminée sans timeout, sauvegarder le résultat
        if (!time_exceeded) {
            best_score = iteration_best;
            best_move = iteration_move;
            completed_depth = depth;
        }
    }
    
    // // Affichage des statistiques
    // long elapsed_ms = (clock() - start_time) * 1000 / CLOCKS_PER_SEC;
    // printf("[PVS-OPT] depth=%d score=%d nodes=%d tt=%d cuts=%d re-search=%d time=%ldms\n",
    //        completed_depth, best_score, nodes, tt_hits, cutoffs, re_searches, elapsed_ms);
    
    // Retourner le meilleur coup trouvé
    *selected_move = best_move;
}