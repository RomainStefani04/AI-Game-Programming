#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

typedef void (*PlayFunction)(const GameState *state, Move *selected_move);

typedef struct {
    PlayFunction play;
    const char *name;
} Player;

Player create_human_player(void);
Player create_ai_random_player(void);
Player create_ai_minimax_player(void);
Player create_ai_alpha_beta_player(void);
Player create_ai_alphabeta_player(void);
Player create_ai_pvs_player(void);
Player create_ai_pvs_v2_player(void);
Player create_ai_mtdf_player(void);
Player create_ai_aspiration_player(void);

void human_play(const GameState *state, Move *selected_move);

#endif
