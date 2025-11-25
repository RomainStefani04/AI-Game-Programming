//
// Created by romai on 24/11/2025.
//
#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#define NUM_HOLES 16
#define HOLES_PER_PLAYER 8
#define NUM_COLORS 3

typedef enum {
    RED = 0,
    BLUE = 1,
    TRANSPARENT = 2
} Color;

typedef enum {
    PLAYER_1 = 0,    // Trous impairs (1,3,5,7,9,11,13,15)
    PLAYER_2 = 1     // Trous pairs (2,4,6,8,10,12,14,16)
} PlayerIndex;

typedef struct {
    int seeds[NUM_COLORS];
} Hole;

typedef struct {
    Hole board[NUM_HOLES];
    int captures[2];
    PlayerIndex current_player;
    int turn_number;
} GameState;

typedef struct {
    int hole_number;          // 1-16 (numéro d'affichage)
    Color color;              // RED, BLUE, ou TRANSPARENT
    Color transparent_color;  // Si color==TRANSPARENT : RED ou BLUE (couleur réelle pour la distribution)
} Move;

// Fonctions utilitaires
int hole_display_number(int internal_index);
bool is_player_hole(int hole_index, PlayerIndex playerIndex);
int get_total_seeds_in_hole(const Hole *hole);
int get_total_seeds_on_board(const GameState *state);
int is_valid_move(GameState *state, Move *move);
int generate_legal_moves(const GameState *state, Move *moves);

// Initialisation
void init_game_state(GameState *state);
void copy_game_state(const GameState *source, GameState *dest);
int parse_move(char *move_str, Move *move);

// Affichage
const char* color_to_string(Color color);
void display_game_state(const GameState *state);
void display_move(const Move *move);

// Moteur de jeu
int execute_move(GameState *state, Move *move);

#endif // GAME_H