//
// Created by romai on 24/11/2025.
//
#include "game.h"
#include "ai_random.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum {
    HUMAN,
    AI_RANDOM
} PlayerType;

int play_game(PlayerType player1_type, PlayerType player2_type) {
    GameState state;
    init_game_state(&state);

    char move_str[10];
    Move move;

    while(1) {
        if (get_total_seeds_on_board(&state) < 10) {
            break;
        }

        display_game_state(&state);

        PlayerType current_type = (state.current_player == PLAYER_1) ? player1_type : player2_type;

        if (current_type == HUMAN) {
            // Joueur humain
            printf("Joueur %d, votre coup: ", state.current_player + 1);
            if (scanf("%9s", move_str) != 1) break;

            if (strcmp(move_str, "quit") == 0) break;

            if (!parse_move(move_str, &move)) {
                printf("Coup invalide! Format: NC (ex: 3R, 14B, 4TR)\n");
                continue;
            }

            if (!is_valid_move(&state, &move)) {
                printf("Coup illegal! Verifiez le trou et la couleur.\n");
                continue;
            }
        } else if (current_type == AI_RANDOM) {
            // IA Random
            ai_random_move(&state, &move);
            printf("IA Joueur %d joue: ", state.current_player + 1);
            display_move(&move);
        }

        state.captures[state.current_player] += execute_move(&state, &move);

        if (state.captures[state.current_player] >= 49) {
            break;
        }

        state.current_player = (state.current_player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
        state.turn_number++;
    }

    display_game_state(&state);

    int winner = (state.captures[PLAYER_1] == state.captures[PLAYER_2]) ? -1 :
                 (state.captures[PLAYER_1] > state.captures[PLAYER_2]) ? PLAYER_1 : PLAYER_2;

    if (winner == -1) {
        printf("Match nul!\n");
    } else {
        printf("Le joueur %d gagne avec %d captures!\n",
               winner + 1, state.captures[winner]);
    }

    return winner;
}

int main() {
    srand(time(NULL));  // Initialiser le générateur aléatoire

    int choice;
    printf("=== MANCALA IA ===\n\n");
    printf("Choisissez le mode de jeu:\n");
    printf("1. Humain vs Humain\n");
    printf("2. Humain vs IA Random\n");
    printf("3. IA Random vs Humain\n");
    printf("4. IA Random vs IA Random\n");
    printf("\nVotre choix: ");

    if (scanf("%d", &choice) != 1) {
        printf("Choix invalide!\n");
        return 1;
    }

    PlayerType player1, player2;

    switch(choice) {
        case 1:
            player1 = HUMAN;
            player2 = HUMAN;
            break;
        case 2:
            player1 = HUMAN;
            player2 = AI_RANDOM;
            break;
        case 3:
            player1 = AI_RANDOM;
            player2 = HUMAN;
            break;
        case 4:
            player1 = AI_RANDOM;
            player2 = AI_RANDOM;
            break;
        default:
            printf("Choix invalide!\n");
            return 1;
    }

    play_game(player1, player2);

    return 0;
}