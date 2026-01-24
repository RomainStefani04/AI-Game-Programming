#ifndef AI_COMMON_H
#define AI_COMMON_H

#include "game.h"
#include <stdint.h>

#define MAX_DEPTH 50
#define WIN_SCORE 100000
#define TIME_LIMIT_MS 2000
#define HASH_SIZE 1048576

#define TT_EXACT 0
#define TT_LOWER 1
#define TT_UPPER 2

#define SEEDS_TO_WIN 49
#define MAX_TURNS 400

typedef struct {
    uint64_t hash;
    int depth;
    int score;
    int flag;
    Move best_move;
    int valid;
} TTEntry;

uint64_t compute_hash(const GameState *state);
int base_evaluate(const GameState *state, PlayerIndex maximizing_player);

#endif
