//
// Created by romai on 24/11/2025.
//
#include "../include/ai_random.h"
#include <stdlib.h>
#include <time.h>

/*
 * IA Random - Choisit un coup aléatoire parmi les coups légaux
 */

void ai_random_move(const GameState *state, Move *selected_move) {
    Move legal_moves[128];  // Buffer pour stocker tous les coups légaux possibles
    int num_legal_moves = generate_legal_moves(state, legal_moves);

    if (num_legal_moves > 0) {
        int random_index = rand() % num_legal_moves;
        *selected_move = legal_moves[random_index];
    }
}
