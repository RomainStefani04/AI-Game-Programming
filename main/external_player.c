/*
 * external_player.c
 * Programme pour jouer via stdin/stdout avec l'arbitre Java
 */

#include "../include/game.h"
#include "../include/player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <process.h>
    #define getpid _getpid
#else
    #include <unistd.h>
#endif

void send_move(const Move* move) {
    if (move->color == TRANSPARENT) {
        printf("%d%s%s\n",
               move->hole_number,
               "T",
               (move->transparent_color == RED) ? "R" : "B");
    } else {
        printf("%d%s\n",
               move->hole_number,
               (move->color == RED) ? "R" : "B");
    }
    fflush(stdout);
}

void send_result(const Move* move, const GameState* state) {
    if (move->color == TRANSPARENT) {
        printf("RESULT %d%s%s %d %d\n",
               move->hole_number,
               "T",
               (move->transparent_color == RED) ? "R" : "B",
               state->captures[PLAYER_1],
               state->captures[PLAYER_2]);
    } else {
        printf("RESULT %d%s %d %d\n",
               move->hole_number,
               (move->color == RED) ? "R" : "B",
               state->captures[PLAYER_1],
               state->captures[PLAYER_2]);
    }
    fflush(stdout);
}

int main(int argc, char* argv[]) {
    unsigned int seed = (unsigned int)(time(NULL) + getpid());
    srand(seed);

    PlayerIndex our_player = PLAYER_1;
    if (argc > 1) {
        if (strstr(argv[1], "B") != NULL || strstr(argv[1], "2") != NULL) {
            our_player = PLAYER_2;
        }
    }

    GameState state;
    init_game_state(&state);

    Player our_ai = create_ai_pvs_player();

    char input_line[256];

    while (fgets(input_line, sizeof(input_line), stdin) != NULL) {
        input_line[strcspn(input_line, "\n")] = 0;

        if (strcmp(input_line, "START") == 0) {
            if (our_player == PLAYER_1) {
                Move our_move;
                our_ai.play(&state, &our_move);

                state.captures[state.current_player] += execute_move(&state, &our_move);
                state.current_player = (state.current_player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
                state.turn_number++;

                if (is_game_over(&state)) {
                    send_result(&our_move, &state);
                    break;
                }

                send_move(&our_move);
            }
            continue;
        }

        if (strcmp(input_line, "END") == 0) {
            break;
        }

        Move opponent_move;
        if (parse_move(input_line, &opponent_move)) {
            if (state.current_player != our_player) {
                state.captures[state.current_player] += execute_move(&state, &opponent_move);
                state.current_player = (state.current_player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
                state.turn_number++;
            }

            Move our_move;
            our_ai.play(&state, &our_move);

            state.captures[state.current_player] += execute_move(&state, &our_move);
            state.current_player = (state.current_player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
            state.turn_number++;

            if (is_game_over(&state)) {
                send_result(&our_move, &state);
                break;
            }

            send_move(&our_move);
        }
    }

    return 0;
}