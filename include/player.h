//
// Created by romai on 25/11/2025.
//
#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

typedef void (*PlayFunction)(const GameState *state, Move *selected_move);

typedef struct {
    PlayFunction play;
    const char *name;
} Player;

// Fonction pour créer des joueurs
Player create_human_player(void);
Player create_ai_random_player(void);
Player create_ai_minimax_player(void);
Player create_ai_alphabeta_player(void);
Player create_ai_alphabeta_nul_player(void);

// Fonction de jeu pour humain
void human_play(const GameState *state, Move *selected_move);

#endif //PLAYER_H
