//
// Created by romai on 24/11/2025.
//
#include "../include/game.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * ============================================================================
 * FONCTIONS UTILITAIRES
 * ============================================================================
 */

int hole_display_number(int internal_index) {
    return internal_index + 1;
}

bool is_player_hole(int hole_index, PlayerIndex playerIndex) {
    if (hole_index < 0 || hole_index >= NUM_HOLES) {
        return false;
    }

    int display_num = hole_display_number(hole_index);

    if (playerIndex == PLAYER_1) {
        return display_num % 2 == 1;  // Impair
    } else {
        return display_num % 2 == 0;  // Pair
    }
}

int get_total_seeds_in_hole(const Hole *hole) {
    return hole->seeds[RED] + hole->seeds[BLUE] + hole->seeds[TRANSPARENT];
}

int get_total_seeds_on_board(const GameState *state) {
    int total = 0;
    for (int i = 0; i < NUM_HOLES; i++) {
        total += get_total_seeds_in_hole(&state->board[i]);
    }
    return total;
}

int is_valid_move(GameState *state, Move *move) {
    if (move->hole_number < 1 || move->hole_number > 16) return 0;
    if (move->hole_number % 2 == (int)state->current_player) return 0;
    if (state->board[move->hole_number - 1].seeds[move->color] == 0) return 0;
    return 1;
}

int generate_legal_moves(const GameState *state, Move *moves) {
    int count = 0;

    for (int i = 0; i < NUM_HOLES; i++) {
        if (!is_player_hole(i, state->current_player)) continue;

        // Rouge
        if (state->board[i].seeds[RED] > 0) {
            moves[count].hole_number = i + 1;  // Numéro d'affichage
            moves[count].color = RED;
            count++;
        }

        // Bleu
        if (state->board[i].seeds[BLUE] > 0) {
            moves[count].hole_number = i + 1;
            moves[count].color = BLUE;
            count++;
        }

        // Transparent comme Rouge
        if (state->board[i].seeds[TRANSPARENT] > 0) {
            moves[count].hole_number = i + 1;
            moves[count].color = TRANSPARENT;
            moves[count].transparent_color = RED;
            count++;

            // Transparent comme Bleu
            moves[count].hole_number = i + 1;
            moves[count].color = TRANSPARENT;
            moves[count].transparent_color = BLUE;
            count++;
        }
    }

    return count;
}

int is_game_over(const GameState *state) {
    // Vérifie si un joueur a capturé au moins 49 graines (victoire)
    if (state->captures[PLAYER_1] >= 49 || state->captures[PLAYER_2] >= 49) {
        return 1;
    }

    // Vérifie s'il reste moins de 10 graines sur le plateau
    if (get_total_seeds_on_board(state) < 10) {
        return 1;
    }

    if (state->turn_number > 400) {
        return 1;
    }

    return 0;
}

/*
 * ============================================================================
 * INITIALISATION
 * ============================================================================
 */

void init_game_state(GameState *state) {
    memset(state, 0, sizeof(GameState));

    for (int i = 0; i < NUM_HOLES; i++) {
        state->board[i].seeds[RED] = 2;
        state->board[i].seeds[BLUE] = 2;
        state->board[i].seeds[TRANSPARENT] = 2;
    }

    state->captures[PLAYER_1] = 0;
    state->captures[PLAYER_2] = 0;
    state->current_player = PLAYER_1;
    state->turn_number = 1;
}

void copy_game_state(const GameState *source, GameState *dest) {
    memcpy(dest, source, sizeof(GameState));
}

int parse_move(char *move_str, Move *move) {
    int len = strlen(move_str);
    if (len < 2 || len > 4) return 0;

    // Parser le numero du trou
    char hole_str[3] = {0};
    int i = 0;
    while (i < len && isdigit(move_str[i])) {
        hole_str[i] = move_str[i];
        i++;
    }

    if (i == 0) return 0;
    move->hole_number = atoi(hole_str);

    if (move->hole_number < 1 || move->hole_number > 16) return 0;

    // Parser la couleur
    if (i >= len) return 0;

    if (move_str[i] == 'T' && i + 1 < len) {
        move->color = TRANSPARENT;
        if (move_str[i + 1] == 'R') {
            move->transparent_color = RED;
        } else if (move_str[i + 1] == 'B') {
            move->transparent_color = BLUE;
        } else {
            return 0;
        }
        return 1;
    } else if (move_str[i] == 'R' || move_str[i] == 'B') {
        move->color = (move_str[i] == 'R') ? RED : BLUE;
        return 1;
    }

    return 0;
}

/*
 * ============================================================================
 * AFFICHAGE (sans emojis, avec | et -)
 * ============================================================================
 */

const char* color_to_string(Color color) {
    switch(color) {
        case RED: return "R";
        case BLUE: return "B";
        case TRANSPARENT: return "T";
        default: return "?";
    }
}

void display_game_state(const GameState *state) {
    printf("\n");
    printf("|--------------------------------------------------------|\n");
    printf("|               ETAT DU JEU - Tour %d                    |\n", state->turn_number);
    printf("|--------------------------------------------------------|\n");
    printf("| Joueur 1 (trous impairs) : 1,3,5,7,9,11,13,15          |\n");
    printf("| Joueur 2 (trous pairs)   : 2,4,6,8,10,12,14,16         |\n");
    printf("|--------------------------------------------------------|\n");

    // Afficher les trous en ligne (comme dans les exemples)
    for (int i = 0; i < NUM_HOLES; i++) {
        printf("| %2d (%dR %dB %dT) ",
               hole_display_number(i),
               state->board[i].seeds[RED],
               state->board[i].seeds[BLUE],
               state->board[i].seeds[TRANSPARENT]);

        if ((i + 1) % 4 == 0) {
            printf("|\n");
        }
    }

    printf("|--------------------------------------------------------|\n");
    printf("| Captures: J1 = %2d | J2 = %2d                         |\n",
           state->captures[PLAYER_1],
           state->captures[PLAYER_2]);
    printf("| Joueur actuel: %s                                     |\n",
           state->current_player == PLAYER_1 ? "Joueur 1 (impairs)" : "Joueur 2 (pairs)");
    printf("|--------------------------------------------------------|\n");
    printf("\n");
}

void display_move(const Move *move) {
    if (move->color == TRANSPARENT) {
        printf("Coup: %dT%s (transparent comme %s)\n",
               move->hole_number,
               (move->transparent_color == RED) ? "R" : "B",
               (move->transparent_color == RED) ? "rouge" : "bleu");
    } else {
        printf("Coup: %d%s\n", move->hole_number, color_to_string(move->color));
    }
}

/*
 * ============================================================================
 * FONCTION PRINCIPALE DU JEU
 * ============================================================================
*/

int execute_move(GameState *state, Move *move) {
    // Sowing
    int hole_index = move->hole_number - 1;  // Convertir numéro (1-16) en index (0-15)

    bool transparent = (move->color == TRANSPARENT);
    Color color = move->color;
    int nbSeedsTransparent = 0;
    if (transparent) {
        color = move->transparent_color;
        nbSeedsTransparent = state->board[hole_index].seeds[TRANSPARENT];
        state->board[hole_index].seeds[TRANSPARENT] = 0;
    }
    int nbSeeds = state->board[hole_index].seeds[color];
    state->board[hole_index].seeds[color] = 0;

    // Commencer au trou suivant
    int indexHole = (hole_index + 1) % NUM_HOLES;

    while(nbSeedsTransparent + nbSeeds > 0) {
        if (indexHole == hole_index) {
            // Sauter le trou de départ
            indexHole = (indexHole + (color == RED ? 1 : 2)) % NUM_HOLES;
            continue;
        }

        // Déposer une graine
        if (nbSeedsTransparent > 0) {
            state->board[indexHole].seeds[TRANSPARENT]++;
            nbSeedsTransparent--;
        } else {
            state->board[indexHole].seeds[color]++;
            nbSeeds--;
        }

        // Avancer au trou suivant (Rouge: +1, Bleu: +2)
        indexHole = (indexHole + (color == RED ? 1 : 2)) % NUM_HOLES;
    }

    // Revenir au dernier trou où on a déposé (on a avancé une fois de trop)
    indexHole = (indexHole - (color == RED ? 1 : 2) + NUM_HOLES) % NUM_HOLES;

    //Capturing

    int seedsCaptured = 0;
    while (true) {
        int totalSeedsHole = get_total_seeds_in_hole(&state->board[indexHole]);
        if (totalSeedsHole == 2 || totalSeedsHole == 3) {
            seedsCaptured += totalSeedsHole;
            state->board[indexHole].seeds[RED] = 0;
            state->board[indexHole].seeds[BLUE] = 0;
            state->board[indexHole].seeds[TRANSPARENT] = 0;
            indexHole = (indexHole - 1 + NUM_HOLES) % NUM_HOLES;
        } else {
            break;
        }
    }
    return seedsCaptured;
}