/*
 * replay_game.c
 * Programme pour rejouer une partie depuis une liste de coups
 * puis continuer en mode humain vs humain
 */

#include "../include/game.h"
#include "../include/player.h"
#include "../include/engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_replay_line(const char* line, Move* move, char* player) {
    int move_num;
    char player_char;
    char move_str[16];

    if (sscanf(line, "%d:%c -> %s", &move_num, &player_char, move_str) != 3) {
        return 0;
    }

    *player = player_char;

    int len = strlen(move_str);
    if (len < 2) return 0;

    if (len >= 3 && move_str[len - 2] == 'T') {
        move->color = TRANSPARENT;
        move->transparent_color = (move_str[len - 1] == 'R') ? RED : BLUE;
        move_str[len - 2] = '\0';
        move->hole_number = atoi(move_str);
    } else {
        move->color = (move_str[len - 1] == 'R') ? RED : BLUE;
        move->transparent_color = RED;
        move_str[len - 1] = '\0';
        move->hole_number = atoi(move_str);
    }

    return move_num;
}

int main(int argc, char* argv[]) {
    int stop_at_move = -1;

    if (argc > 1) {
        stop_at_move = atoi(argv[1]);
        printf("Arrêt après le coup %d\n", stop_at_move);
    }

    GameState state;
    init_game_state(&state);

    printf("=== REPLAY DE PARTIE ===\n");
    printf("État initial:\n");
    display_game_state(&state);

    char input_line[256];
    int move_count = 0;

    printf("\nEntrez les coups (format: 1:A -> 13R), ligne vide pour terminer:\n");

    while (fgets(input_line, sizeof(input_line), stdin) != NULL) {
        input_line[strcspn(input_line, "\n")] = 0;

        if (strlen(input_line) == 0) {
            break;
        }

        Move move;
        char player;
        int move_num = parse_replay_line(input_line, &move, &player);

        if (move_num == 0) {
            printf("Erreur de parsing: %s\n", input_line);
            continue;
        }

        PlayerIndex expected = (player == 'A') ? PLAYER_1 : PLAYER_2;
        if (state.current_player != expected) {
            printf("Attention: joueur attendu %c mais c'est au tour de %c\n",
                   player, state.current_player == PLAYER_1 ? 'A' : 'B');
        }

        int captures = execute_move(&state, &move);
        state.captures[state.current_player] += captures;

        printf("\n");
        printf("|=====================================================================================================================|\n");
        printf("|  COUP %2d : Joueur %c joue ", move_num, player);
        display_move(&move);
        printf("  -->  %d capture(s)  |\n", captures);
        printf("|============================================================|\n");

        state.current_player = (state.current_player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
        state.turn_number++;
        move_count++;

        display_game_state(&state);

        if (stop_at_move == move_num) {
            printf("\n>>> Arrêt demandé au coup %d <<<\n", move_num);
            break;
        }

        if (is_game_over(&state)) {
            printf("\n*** PARTIE TERMINÉE ***\n");
            printf("Score final - A: %d | B: %d\n",
                   state.captures[PLAYER_1], state.captures[PLAYER_2]);
            return 0;
        }
    }

    printf("\n=== Continuation en mode Humain vs Humain ===\n");
    display_game_state(&state);

    while (!is_game_over(&state)) {
        Move move;
        printf("\nJoueur %c, ", state.current_player == PLAYER_1 ? 'A' : 'B');
        human_play(&state, &move);

        if (!is_valid_move(&state, &move)) {
            printf("Coup invalide, réessayez.\n");
            continue;
        }

        int captures = execute_move(&state, &move);
        state.captures[state.current_player] += captures;
        move_count++;

        printf("\n");
        printf("|============================================================|\n");
        printf("|  COUP %2d : Joueur %c joue ", move_count,
               state.current_player == PLAYER_1 ? 'A' : 'B');
        display_move(&move);
        printf("  -->  %d capture(s)  |\n", captures);
        printf("|============================================================|\n");

        state.current_player = (state.current_player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
        state.turn_number++;

        display_game_state(&state);
    }

    printf("\n*** PARTIE TERMINÉE ***\n");
    printf("Score final - A: %d | B: %d\n",
           state.captures[PLAYER_1], state.captures[PLAYER_2]);

    return 0;
}
