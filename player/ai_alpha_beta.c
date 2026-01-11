//
// Created by Guillaume170604
//

#include "../include/ai_alpha_beta.h"
#include "../include/game.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define MAX_DEPTH 50
#define WIN_SCORE 100000
#define SEEDS_TO_WIN 49
#define TIME_LIMIT_MS 3000
#define HASH_SIZE 1048576
#define ASPIRATION_WINDOW 50

#define TT_EXACT 0
#define TT_LOWER 1
#define TT_UPPER 2

typedef struct {
    uint64_t hash;
    int depth;
    int score;
    int flag;
    Move best_move;
    int valid;
} TTEntry;

static TTEntry transposition_table[HASH_SIZE];
static Move killer_moves[MAX_DEPTH][2];
static int history_table[NUM_HOLES][3];  // History heuristic [hole][color]

static clock_t start_time;
static int time_exceeded;
static int nodes_searched;

// Vérifie si le temps est écoulé - version corrigée
static int is_time_up(void) {
    nodes_searched++;
    if (nodes_searched % 1024 == 0) {
        clock_t now = clock();
        long elapsed = (now - start_time) * 1000 / CLOCKS_PER_SEC;
        if (elapsed >= TIME_LIMIT_MS) {
            time_exceeded = 1;
            return 1;
        }
    }
    return time_exceeded;
}

static uint64_t compute_hash(const GameState *state) {
    uint64_t hash = 0;
    for (int i = 0; i < NUM_HOLES; i++) {
        hash ^= (uint64_t)(state->board[i].seeds[RED]) << (i * 3);
        hash ^= (uint64_t)(state->board[i].seeds[BLUE]) << (i * 3 + 16);
        hash ^= (uint64_t)(state->board[i].seeds[TRANSPARENT]) << (i * 3 + 32);
    }
    hash ^= (uint64_t)state->captures[0] << 48;
    hash ^= (uint64_t)state->captures[1] << 54;
    hash ^= (uint64_t)state->current_player << 63;
    return hash;
}

static void store_killer(int depth, const Move *move) {
    if (depth >= MAX_DEPTH) return;
    if (killer_moves[depth][0].hole_number != move->hole_number ||
        killer_moves[depth][0].color != move->color) {
        killer_moves[depth][1] = killer_moves[depth][0];
        killer_moves[depth][0] = *move;
    }
}

static int is_killer(int depth, const Move *move) {
    if (depth >= MAX_DEPTH) return 0;
    return (killer_moves[depth][0].hole_number == move->hole_number &&
            killer_moves[depth][0].color == move->color) ||
           (killer_moves[depth][1].hole_number == move->hole_number &&
            killer_moves[depth][1].color == move->color);
}

// Met à jour l'historique pour les coups qui causent des coupures
static void update_history(const Move *move, int depth) {
    int idx = move->hole_number - 1;
    if (idx >= 0 && idx < NUM_HOLES) {
        history_table[idx][move->color] += depth * depth;
        if (history_table[idx][move->color] > 1000000) {
            for (int h = 0; h < NUM_HOLES; h++) {
                for (int c = 0; c < 3; c++) {
                    history_table[h][c] /= 2;
                }
            }
        }
    }
}

static int get_history_score(const Move *move) {
    int idx = move->hole_number - 1;
    if (idx >= 0 && idx < NUM_HOLES) {
        return history_table[idx][move->color];
    }
    return 0;
}

static int evaluate(const GameState *state, PlayerIndex maximizing_player) {
    int my_captures = state->captures[maximizing_player];
    int opp_captures = state->captures[1 - maximizing_player];

    if (my_captures >= SEEDS_TO_WIN) return WIN_SCORE - state->turn_number;
    if (opp_captures >= SEEDS_TO_WIN) return -WIN_SCORE + state->turn_number;

    int score = (my_captures - opp_captures) * 100;

    int my_seeds = 0, opp_seeds = 0;
    int my_threats = 0, opp_threats = 0;
    int my_vulnerable = 0;

    for (int i = 0; i < NUM_HOLES; i++) {
        int total = get_total_seeds_in_hole(&state->board[i]);

        if (is_player_hole(i, maximizing_player)) {
            my_seeds += total;
            if (total >= 1 && total <= 3) my_threats += 3;
            if (total == 1) my_vulnerable += 2;
        } else {
            opp_seeds += total;
            if (total == 1) score += 5;
            if (total == 2 || total == 3) {
                opp_threats += 3;
                score += 3;
            }
        }
    }

    score += (my_seeds - opp_seeds);
    score += (my_threats - opp_threats);
    score -= my_vulnerable;

    return score;
}

// Tri des coups amélioré avec history heuristic
static void order_moves(const GameState *state, Move *moves, int num_moves,
                        int *scores, int depth, const Move *tt_move) {
    for (int i = 0; i < num_moves; i++) {
        // Priorité maximale au coup de la table de transposition
        if (tt_move && tt_move->hole_number == moves[i].hole_number &&
            tt_move->color == moves[i].color) {
            scores[i] = 10000000;
            continue;
        }

        // Priorité aux killer moves
        if (is_killer(depth, &moves[i])) {
            scores[i] = 5000000;
            continue;
        }

        // Score basé sur captures + history heuristic
        GameState copy = *state;
        int captured = execute_move(&copy, &moves[i]);
        scores[i] = captured * 100000 + get_history_score(&moves[i]);
    }

    // Tri décroissant par score
    for (int i = 0; i < num_moves - 1; i++) {
        for (int j = i + 1; j < num_moves; j++) {
            if (scores[j] > scores[i]) {
                int tmp_score = scores[i];
                scores[i] = scores[j];
                scores[j] = tmp_score;

                Move tmp_move = moves[i];
                moves[i] = moves[j];
                moves[j] = tmp_move;
            }
        }
    }
}

static int alphabeta(GameState *state, int depth, int alpha, int beta,
                     int is_maximizing, PlayerIndex maximizing_player,
                     int null_move_allowed, int ply) {

    if (time_exceeded || is_time_up()) return 0;

    if (depth == 0 || is_game_over(state)) {
        return evaluate(state, maximizing_player);
    }

    uint64_t hash = compute_hash(state);
    int tt_index = hash % HASH_SIZE;
    TTEntry *tt_entry = &transposition_table[tt_index];
    Move *tt_move = NULL;

    if (tt_entry->valid && tt_entry->hash == hash && tt_entry->depth >= depth) {
        if (tt_entry->flag == TT_EXACT) return tt_entry->score;
        if (tt_entry->flag == TT_LOWER && tt_entry->score > alpha) alpha = tt_entry->score;
        if (tt_entry->flag == TT_UPPER && tt_entry->score < beta) beta = tt_entry->score;
        if (alpha >= beta) return tt_entry->score;
        tt_move = &tt_entry->best_move;
    }

    if (null_move_allowed && depth >= 3 && !is_maximizing) {
        GameState null_state = *state;
        null_state.current_player = 1 - null_state.current_player;
        int null_score = alphabeta(&null_state, depth - 3, alpha, beta, 1, maximizing_player, 0, ply + 1);
        if (!time_exceeded && null_score >= beta) return beta;
    }

    Move legal_moves[128];
    int num_moves = generate_legal_moves(state, legal_moves);
    if (num_moves == 0) return evaluate(state, maximizing_player);

    int scores[128];
    order_moves(state, legal_moves, num_moves, scores, ply, tt_move);

    Move best_move = legal_moves[0];
    int original_alpha = alpha;

    if (is_maximizing) {
        int max_eval = INT_MIN;

        for (int i = 0; i < num_moves; i++) {
            if (time_exceeded) break;

            GameState copy = *state;
            int captured = execute_move(&copy, &legal_moves[i]);
            copy.captures[copy.current_player] += captured;
            copy.current_player = 1 - copy.current_player;

            int eval;
            if (i >= 4 && depth >= 3 && captured == 0) {
                eval = alphabeta(&copy, depth - 2, alpha, beta, 0, maximizing_player, 1, ply + 1);
                if (!time_exceeded && eval > alpha) {
                    eval = alphabeta(&copy, depth - 1, alpha, beta, 0, maximizing_player, 1, ply + 1);
                }
            } else {
                eval = alphabeta(&copy, depth - 1, alpha, beta, 0, maximizing_player, 1, ply + 1);
            }

            if (time_exceeded) break;

            if (eval > max_eval) { max_eval = eval; best_move = legal_moves[i]; }
            if (eval > alpha) alpha = eval;
            if (beta <= alpha) {
                store_killer(ply, &legal_moves[i]);
                update_history(&legal_moves[i], depth);
                break;
            }
        }

        if (!time_exceeded) {
            tt_entry->hash = hash;
            tt_entry->depth = depth;
            tt_entry->score = max_eval;
            tt_entry->best_move = best_move;
            tt_entry->valid = 1;
            tt_entry->flag = (max_eval <= original_alpha) ? TT_UPPER :
                             (max_eval >= beta) ? TT_LOWER : TT_EXACT;
        }
        return max_eval;
    } else {
        int min_eval = INT_MAX;

        for (int i = 0; i < num_moves; i++) {
            if (time_exceeded) break;

            GameState copy = *state;
            int captured = execute_move(&copy, &legal_moves[i]);
            copy.captures[copy.current_player] += captured;
            copy.current_player = 1 - copy.current_player;

            int eval;
            if (i >= 4 && depth >= 3 && captured == 0) {
                eval = alphabeta(&copy, depth - 2, alpha, beta, 1, maximizing_player, 1, ply + 1);
                if (!time_exceeded && eval < beta) {
                    eval = alphabeta(&copy, depth - 1, alpha, beta, 1, maximizing_player, 1, ply + 1);
                }
            } else {
                eval = alphabeta(&copy, depth - 1, alpha, beta, 1, maximizing_player, 1, ply + 1);
            }

            if (time_exceeded) break;

            if (eval < min_eval) { min_eval = eval; best_move = legal_moves[i]; }
            if (eval < beta) beta = eval;
            if (beta <= alpha) {
                store_killer(ply, &legal_moves[i]);
                update_history(&legal_moves[i], depth);
                break;
            }
        }

        if (!time_exceeded) {
            tt_entry->hash = hash;
            tt_entry->depth = depth;
            tt_entry->score = min_eval;
            tt_entry->best_move = best_move;
            tt_entry->valid = 1;
            tt_entry->flag = (min_eval <= original_alpha) ? TT_UPPER :
                             (min_eval >= beta) ? TT_LOWER : TT_EXACT;
        }
        return min_eval;
    }
}

void ai_alpha_beta_move(const GameState *state, Move *selected_move) {
    Move legal_moves[128];
    int num_moves = generate_legal_moves(state, legal_moves);
    if (num_moves == 0) return;

    start_time = clock();
    time_exceeded = 0;
    nodes_searched = 0;

    memset(killer_moves, 0, sizeof(killer_moves));

    Move best_move = legal_moves[0];
    int best_score = INT_MIN;
    PlayerIndex maximizing_player = state->current_player;
    int completed_depth = 0;
    int prev_score = 0;

    int scores[128];
    order_moves(state, legal_moves, num_moves, scores, 0, NULL);

    for (int depth = 1; depth <= MAX_DEPTH; depth++) {
        if (time_exceeded) break;

        int alpha = (depth >= 4) ? prev_score - ASPIRATION_WINDOW : INT_MIN;
        int beta = (depth >= 4) ? prev_score + ASPIRATION_WINDOW : INT_MAX;

        int current_best_score = INT_MIN;
        Move current_best_move = legal_moves[0];

        for (int i = 0; i < num_moves; i++) {
            if (time_exceeded) break;

            GameState copy = *state;
            int captured = execute_move(&copy, &legal_moves[i]);
            copy.captures[copy.current_player] += captured;
            copy.current_player = 1 - copy.current_player;

            int score = alphabeta(&copy, depth - 1, alpha, beta, 0, maximizing_player, 1, 1);

            // Re-recherche si hors fenêtre d'aspiration
            if (!time_exceeded && (score <= alpha || score >= beta)) {
                score = alphabeta(&copy, depth - 1, INT_MIN, INT_MAX, 0, maximizing_player, 1, 1);
            }

            if (!time_exceeded && score > current_best_score) {
                current_best_score = score;
                current_best_move = legal_moves[i];
            }
        }

        if (!time_exceeded) {
            best_score = current_best_score;
            best_move = current_best_move;
            prev_score = best_score;
            completed_depth = depth;
        }
    }

    printf("[Alpha-Beta] Profondeur: %d | Score: %d | Noeuds: %d | Temps: %ld ms\n",
           completed_depth, best_score, nodes_searched,
           (long)((clock() - start_time) * 1000 / CLOCKS_PER_SEC));

    *selected_move = best_move;
}