//
// Created by romai on 25/11/2025.
//

#include "../include/engine.h"
#include <stdio.h>

int play_game(Player player1, Player player2, bool display) {
    GameState state;
    init_game_state(&state);

    Move move;

    while(!is_game_over(&state)) {
        if (display) {
            display_game_state(&state);
        }

        Player *current_player = (state.current_player == PLAYER_1) ? &player1 : &player2;

        current_player->play(&state, &move);

        if (move.hole_number == -1) break; // Quit

        if (display && current_player->play != human_play) {
            printf("%s Joueur %d joue: ", current_player->name, state.current_player + 1);
            display_move(&move);
        }

        state.captures[state.current_player] += execute_move(&state, &move);

        state.current_player = (state.current_player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
        state.turn_number++;
    }

    if (display) {
        display_game_state(&state);
    }

    int winner = (state.captures[PLAYER_1] == state.captures[PLAYER_2]) ? -1 :
                 (state.captures[PLAYER_1] > state.captures[PLAYER_2]) ? PLAYER_1 : PLAYER_2;

    if (winner == -1) {
        printf("Match nul!\n");
    } else {
        if (display) {
            printf("Le joueur %d gagne avec %d captures!\n",
                   winner + 1, state.captures[winner]);
        }
    }

    return winner;
}
