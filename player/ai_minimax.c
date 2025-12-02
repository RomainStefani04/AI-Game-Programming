//
// Created by romai on 02/12/2025.
//

#include "../include/ai_minimax.h"
#include "../include/game.h"
#include <limits.h>
#include <string.h>

#define MAX_DEPTH 4

// Fonction d'évaluation simple : différence de graines capturées
static int evaluate(const GameState *state) {
    return state->captures[state->current_player] - state->captures[1 - state->current_player];
}

// Fonction Minimax récursive
static int minimax(GameState *state, int depth, int is_maximizing) {
    // Condition d'arrêt : profondeur max ou partie terminée
    if (depth == 0 || is_game_over(state)) {
        return evaluate(state);
    }

    Move legal_moves[128];
    int num_legal_moves = generate_legal_moves(state, legal_moves);

    if (num_legal_moves == 0) {
        return evaluate(state);
    }

    if (is_maximizing) {
        int max_eval = INT_MIN;
        for (int i = 0; i < num_legal_moves; i++) {
            GameState state_copy = *state;
            execute_move(&state_copy, &legal_moves[i]);

            int eval = minimax(&state_copy, depth - 1, 0);
            if (eval > max_eval) {
                max_eval = eval;
            }
        }
        return max_eval;
    } else {
        int min_eval = INT_MAX;
        for (int i = 0; i < num_legal_moves; i++) {
            GameState state_copy = *state;
            execute_move(&state_copy, &legal_moves[i]);

            int eval = minimax(&state_copy, depth - 1, 1);
            if (eval < min_eval) {
                min_eval = eval;
            }
        }
        return min_eval;
    }
}

// Fonction principale de l'IA Minimax
void ai_minimax_move(const GameState *state, Move *selected_move) {
    Move legal_moves[128];
    int num_legal_moves = generate_legal_moves(state, legal_moves);

    if (num_legal_moves == 0) {
        return;
    }

    int best_score = INT_MIN;
    Move best_move = legal_moves[0];

    for (int i = 0; i < num_legal_moves; i++) {
        GameState state_copy = *state;
        execute_move(&state_copy, &legal_moves[i]);

        int score = minimax(&state_copy, MAX_DEPTH - 1, 0);

        if (score > best_score) {
            best_score = score;
            best_move = legal_moves[i];
        }
    }

    *selected_move = best_move;
}
