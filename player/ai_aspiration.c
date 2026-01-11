//
// ai_aspiration.c - Alpha-Beta avec Aspiration Windows
// Fenêtre étroite basée sur le score précédent
//
#include "../include/ai_aspiration.h"
#include "../include/ai_common.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define ASPIRATION_WINDOW 50

static TTEntry tt[HASH_SIZE];
static Move killers[MAX_DEPTH][2];
static clock_t start_time;
static int time_exceeded, nodes, tt_hits, cutoffs, window_fails;

static int is_time_up(void) {
    if (nodes++ % 1024 == 0)
        return ((clock() - start_time) * 1000 / CLOCKS_PER_SEC) >= TIME_LIMIT_MS;
    return 0;
}

static void store_killer(int ply, const Move *m) {
    if (ply >= MAX_DEPTH) return;
    if (killers[ply][0].hole_number != m->hole_number || killers[ply][0].color != m->color) {
        killers[ply][1] = killers[ply][0];
        killers[ply][0] = *m;
    }
}

static int is_killer(int ply, const Move *m) {
    if (ply >= MAX_DEPTH) return 0;
    return (killers[ply][0].hole_number == m->hole_number && killers[ply][0].color == m->color) ||
           (killers[ply][1].hole_number == m->hole_number && killers[ply][1].color == m->color);
}

static void order_moves(const GameState *state, Move *moves, int n, int *scores, int ply, const Move *tt_move) {
    for (int i = 0; i < n; i++) {
        if (tt_move && tt_move->hole_number == moves[i].hole_number && tt_move->color == moves[i].color)
            scores[i] = 1000000;
        else if (is_killer(ply, &moves[i]))
            scores[i] = 500000;
        else {
            GameState copy = *state;
            scores[i] = execute_move(&copy, &moves[i]) * 1000;
        }
    }
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (scores[j] > scores[i]) {
                int ts = scores[i]; scores[i] = scores[j]; scores[j] = ts;
                Move tm = moves[i]; moves[i] = moves[j]; moves[j] = tm;
            }
        }
    }
}

static int alphabeta(GameState *state, int depth, int alpha, int beta, int maximizing, PlayerIndex max_player, int ply) {
    if (is_time_up()) { time_exceeded = 1; return 0; }
    if (depth == 0 || is_game_over(state)) return base_evaluate(state, max_player);

    uint64_t hash = compute_hash(state);
    int idx = hash % HASH_SIZE;
    TTEntry *e = &tt[idx];
    Move *tt_move = NULL;

    if (e->valid && e->hash == hash && e->depth >= depth) {
        tt_hits++;
        if (e->flag == TT_EXACT) return e->score;
        if (e->flag == TT_LOWER && e->score > alpha) alpha = e->score;
        if (e->flag == TT_UPPER && e->score < beta) beta = e->score;
        if (alpha >= beta) return e->score;
        tt_move = &e->best_move;
    }

    Move moves[128];
    int n = generate_legal_moves(state, moves);
    if (n == 0) return base_evaluate(state, max_player);

    int scores[128];
    order_moves(state, moves, n, scores, ply, tt_move);

    Move best = moves[0];
    int orig_alpha = alpha;

    if (maximizing) {
        int max_eval = INT_MIN;
        for (int i = 0; i < n && !time_exceeded; i++) {
            GameState copy = *state;
            int cap = execute_move(&copy, &moves[i]);
            copy.captures[copy.current_player] += cap;
            copy.current_player = 1 - copy.current_player;
            copy.turn_number++;

            int eval = alphabeta(&copy, depth - 1, alpha, beta, 0, max_player, ply + 1);
            if (eval > max_eval) { max_eval = eval; best = moves[i]; }
            if (eval > alpha) alpha = eval;
            if (beta <= alpha) { store_killer(ply, &moves[i]); cutoffs++; break; }
        }
        if (!time_exceeded) {
            e->hash = hash; e->depth = depth; e->score = max_eval; e->best_move = best; e->valid = 1;
            e->flag = (max_eval <= orig_alpha) ? TT_UPPER : (max_eval >= beta) ? TT_LOWER : TT_EXACT;
        }
        return max_eval;
    } else {
        int min_eval = INT_MAX;
        for (int i = 0; i < n && !time_exceeded; i++) {
            GameState copy = *state;
            int cap = execute_move(&copy, &moves[i]);
            copy.captures[copy.current_player] += cap;
            copy.current_player = 1 - copy.current_player;
            copy.turn_number++;

            int eval = alphabeta(&copy, depth - 1, alpha, beta, 1, max_player, ply + 1);
            if (eval < min_eval) { min_eval = eval; best = moves[i]; }
            if (eval < beta) beta = eval;
            if (beta <= alpha) { store_killer(ply, &moves[i]); cutoffs++; break; }
        }
        if (!time_exceeded) {
            e->hash = hash; e->depth = depth; e->score = min_eval; e->best_move = best; e->valid = 1;
            e->flag = (min_eval <= orig_alpha) ? TT_UPPER : (min_eval >= beta) ? TT_LOWER : TT_EXACT;
        }
        return min_eval;
    }
}

static int search_root(GameState *state, Move *moves, int n, int depth, int alpha, int beta,
                       PlayerIndex max_player, Move *best_out) {
    int best_score = INT_MIN;
    Move best = moves[0];

    for (int i = 0; i < n && !time_exceeded; i++) {
        GameState copy = *state;
        int cap = execute_move(&copy, &moves[i]);
        copy.captures[copy.current_player] += cap;
        copy.current_player = 1 - copy.current_player;
        copy.turn_number++;

        int score = alphabeta(&copy, depth - 1, alpha, beta, 0, max_player, 1);
        if (!time_exceeded && score > best_score) { best_score = score; best = moves[i]; }
        if (score > alpha) alpha = score;
    }

    *best_out = best;
    return best_score;
}

void ai_aspiration_move(const GameState *state, Move *selected_move) {
    Move moves[128];
    int n = generate_legal_moves(state, moves);
    if (n == 0) return;

    start_time = clock();
    time_exceeded = 0; nodes = 0; tt_hits = 0; cutoffs = 0; window_fails = 0;
    memset(killers, 0, sizeof(killers));

    Move best = moves[0];
    int best_score = 0, completed = 0;

    int scores[128];
    order_moves(state, moves, n, scores, 0, NULL);

    for (int depth = 1; depth <= MAX_DEPTH && !time_exceeded; depth++) {
        Move curr_best = best;
        int score;

        if (depth <= 2) {
            // Fenêtre complète pour les premières itérations
            score = search_root((GameState *)state, moves, n, depth, INT_MIN, INT_MAX,
                               state->current_player, &curr_best);
        } else {
            // Aspiration window
            int alpha = best_score - ASPIRATION_WINDOW;
            int beta = best_score + ASPIRATION_WINDOW;

            score = search_root((GameState *)state, moves, n, depth, alpha, beta,
                               state->current_player, &curr_best);

            // Re-search si hors fenêtre
            if (!time_exceeded && (score <= alpha || score >= beta)) {
                window_fails++;
                score = search_root((GameState *)state, moves, n, depth, INT_MIN, INT_MAX,
                                   state->current_player, &curr_best);
            }
        }

        if (!time_exceeded) {
            best_score = score;
            best = curr_best;
            completed = depth;
        }
    }

    printf("[Aspiration] depth=%d score=%d nodes=%d tt=%d cuts=%d w-fail=%d time=%ldms\n",
           completed, best_score, nodes, tt_hits, cutoffs, window_fails,
           (clock() - start_time) * 1000 / CLOCKS_PER_SEC);

    *selected_move = best;
}
