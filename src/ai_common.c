#include "../include/ai_common.h"

uint64_t compute_hash(const GameState *state) {
    uint64_t hash = 0;
    for (int i = 0; i < NUM_HOLES; i++) {
        hash ^= (uint64_t)(state->board[i].seeds[RED]) << (i * 3);
        hash ^= (uint64_t)(state->board[i].seeds[BLUE]) << (i * 3 + 16);
        hash ^= (uint64_t)(state->board[i].seeds[TRANSPARENT]) << (i * 3 + 32);
    }
    hash ^= (uint64_t)state->captures[0] << 48;
    hash ^= (uint64_t)state->captures[1] << 54;
    hash ^= (uint64_t)state->current_player << 63;
    return hash;
}

int base_evaluate(const GameState *state, PlayerIndex maximizing_player) {
    int my_captures = state->captures[maximizing_player];
    int opp_captures = state->captures[1 - maximizing_player];

    if (my_captures >= SEEDS_TO_WIN) return WIN_SCORE - state->turn_number;
    if (opp_captures >= SEEDS_TO_WIN) return -WIN_SCORE + state->turn_number;

    int score = (my_captures - opp_captures) * 100;
    int my_seeds = 0, opp_seeds = 0;

    for (int i = 0; i < NUM_HOLES; i++) {
        int total = get_total_seeds_in_hole(&state->board[i]);
        if (is_player_hole(i, maximizing_player)) {
            my_seeds += total;
            if (total == 1) score -= 2;
        } else {
            opp_seeds += total;
            if (total == 1) score += 5;
            if (total == 2 || total == 3) score += 3;
        }
    }
    score += (my_seeds - opp_seeds);

    int turns_remaining = MAX_TURNS - state->turn_number;
    if (turns_remaining < 50 && my_captures > opp_captures) score += 10;

    return score;
}
