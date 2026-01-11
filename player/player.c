//
// Created by romai on 25/11/2025.
//
#include "../include/player.h"
#include "../include/game.h"
#include "../include/ai_random.h"
#include "../include/ai_minimax.h"
#include "../include/ai_alpha_beta.h"
#include "../include/ai_pvs_v2.h"
#include "../include/ai_alphabeta.h"
#include "../include/ai_pvs.h"
#include "../include/ai_aspiration.h"
#include "../include/ai_mtdf.h"
#include <stdio.h>
#include <string.h>

void human_play(const GameState *state, Move *selected_move) {
    char move_str[10];

    while(1) {
        printf("Joueur %d, votre coup: ", state->current_player + 1);
        if (scanf("%9s", move_str) != 1) return;

        if (strcmp(move_str, "quit") == 0) {
            selected_move->hole_number = -1; // Indicateur de quit
            return;
        }

        if (!parse_move(move_str, selected_move)) {
            printf("Coup invalide! Format: NC (ex: 3R, 14B, 4TR)\n");
            continue;
        }

        if (!is_valid_move((GameState*)state, selected_move)) {
            printf("Coup illegal! Verifiez le trou et la couleur.\n");
            continue;
        }

        break;
    }
}

Player create_human_player(void) {
    Player p = {
        .play = human_play,
        .name = "Humain"
    };
    return p;
}

Player create_ai_random_player(void) {
    Player p = {
        .play = ai_random_move,
        .name = "IA Random"
    };
    return p;
}

Player create_ai_minimax_player(void) {
    Player p = {
        .play = ai_minimax_move,
        .name = "IA Minimax"
    };
    return p;
}

Player create_ai_alpha_beta_player(void) {
    Player p = {
        .play = ai_alpha_beta_move,
        .name = "IA Alphabeta"
    };
    return p;
}

Player create_ai_alphabeta_player(void) {
    Player p = {
        .play = ai_alphabeta_move,
        .name = "IA Alphabeta claude"
    };
    return p;
}

Player create_ai_pvs_player(void) {
    Player p = {
        .play = ai_pvs_move,
        .name = "IA PVS"
    };
    return p;
}

Player create_ai_pvs_v2_player(void) {
    Player p = {
        .play = ai_pvs_v2_move,
        .name = "IA PVS V2"
    };
    return p;
}

Player create_ai_mtdf_player(void) {
    Player p = {
        .play = ai_mtdf_move,
        .name = "IA MTDF"
    };
    return p;
}

Player create_ai_aspiration_player(void) {
    Player p = {
        .play = ai_aspiration_move,
        .name = "IA Aspiration"
    };
    return p;
}

