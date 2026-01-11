//
// Created by Guillaume170604
//

#include "../include/ai_alpha_beta.h"
#include "../include/game.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_DEPTH 10
#define WIN_SCORE 100000
#define SEEDS_TO_WIN 49
#define TIME_LIMIT_MS 3000

static clock_t start_time;
static int time_exceeded;

// Vérifie si le temps est écoulé
static int is_time_up(void) {
    clock_t elapsed = (clock() - start_time) * 1000 / CLOCKS_PER_SEC;
    return elapsed >= TIME_LIMIT_MS;
}

// Fonction d'évaluation
static int evaluate(const GameState *state, PlayerIndex maximizing_player) {
    int my_captures = state->captures[maximizing_player];
    int opp_captures = state->captures[1 - maximizing_player];

    if (my_captures >= SEEDS_TO_WIN) return WIN_SCORE;
    if (opp_captures >= SEEDS_TO_WIN) return -WIN_SCORE;

    int score = (my_captures - opp_captures) * 10;

    int my_seeds = 0, opp_seeds = 0;
    for (int i = 0; i < NUM_HOLES; i++) {
        int total = get_total_seeds_in_hole(&state->board[i]);
        if (is_player_hole(i, maximizing_player)) {
            my_seeds += total;
            if (total == 2 || total == 3) score += 2;
        } else {
            opp_seeds += total;
        }
    }

    score += (my_seeds - opp_seeds) / 2;
    return score;
}

// Tri des coups par potentiel de capture
static void order_moves(const GameState *state, Move *moves, int num_moves, int *scores) {
    for (int i = 0; i < num_moves; i++) {
        GameState copy = *state;
        scores[i] = execute_move(&copy, &moves[i]);
    }

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

// Minimax avec Alpha-Beta et vérification du temps
static int alphabeta(GameState *state, int depth, int alpha, int beta,
                     int is_maximizing, PlayerIndex maximizing_player) {

    // Vérification périodique du temps
    if (is_time_up()) {
        time_exceeded = 1;
        return 0;
    }

    if (depth == 0 || is_game_over(state)) {
        return evaluate(state, maximizing_player);
    }

    Move legal_moves[128];
    int num_moves = generate_legal_moves(state, legal_moves);

    if (num_moves == 0) {
        return evaluate(state, maximizing_player);
    }

    int scores[128];
    order_moves(state, legal_moves, num_moves, scores);

    if (is_maximizing) {
        int max_eval = INT_MIN;
        for (int i = 0; i < num_moves; i++) {
            if (time_exceeded) break;

            GameState copy = *state;
            execute_move(&copy, &legal_moves[i]);
            copy.current_player = 1 - copy.current_player;

            int eval = alphabeta(&copy, depth - 1, alpha, beta, 0, maximizing_player);

            if (eval > max_eval) max_eval = eval;
            if (eval > alpha) alpha = eval;
            if (beta <= alpha) break;
        }
        return max_eval;
    } else {
        int min_eval = INT_MAX;
        for (int i = 0; i < num_moves; i++) {
            if (time_exceeded) break;

            GameState copy = *state;
            execute_move(&copy, &legal_moves[i]);
            copy.current_player = 1 - copy.current_player;

            int eval = alphabeta(&copy, depth - 1, alpha, beta, 1, maximizing_player);

            if (eval < min_eval) min_eval = eval;
            if (eval < beta) beta = eval;
            if (beta <= alpha) break;
        }
        return min_eval;
    }
}

// Fonction principale avec Iterative Deepening
void ai_alphabeta_move(const GameState *state, Move *selected_move) {
    Move legal_moves[128];
    int num_moves = generate_legal_moves(state, legal_moves);

    if (num_moves == 0) return;

    start_time = clock();
    time_exceeded = 0;

    Move best_move = legal_moves[0];
    int best_score = INT_MIN;
    PlayerIndex maximizing_player = state->current_player;
    int completed_depth = 0;  // Profondeur complétée avec succès

    int scores[128];
    order_moves(state, legal_moves, num_moves, scores);

    // Iterative Deepening: augmente la profondeur jusqu'à timeout
    for (int depth = 1; depth <= MAX_DEPTH && !time_exceeded; depth++) {
        int current_best_score = INT_MIN;
        Move current_best_move = legal_moves[0];

        for (int i = 0; i < num_moves && !time_exceeded; i++) {
            GameState copy = *state;
            execute_move(&copy, &legal_moves[i]);
            copy.current_player = 1 - copy.current_player;

            int score = alphabeta(&copy, depth - 1, INT_MIN, INT_MAX, 0, maximizing_player);

            if (!time_exceeded && score > current_best_score) {
                current_best_score = score;
                current_best_move = legal_moves[i];
            }
        }

        // Sauvegarde uniquement si la profondeur est complète
        if (!time_exceeded) {
            best_score = current_best_score;
            best_move = current_best_move;
            completed_depth = depth;
        }
    }

    // Affichage des statistiques
    clock_t elapsed = (clock() - start_time) * 1000 / CLOCKS_PER_SEC;
    printf("[Alpha-Beta] Profondeur: %d | Score: %d | Temps: %ld ms\n", completed_depth, best_score, (long)elapsed);

    *selected_move = best_move;
}