#include "../include/game.h"
#include "../include/player.h"
#include "../include/engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define NUM_AIS 2
#define GAMES_PER_MATCH 4

typedef struct {
    Player player;
    int wins, losses, draws, points;
} AIEntry;

void run_match(AIEntry *a1, AIEntry *a2, int games, int verbose) {
    int w1 = 0, w2 = 0, d = 0;
    printf("\n%s vs %s (%d games)\n", a1->player.name, a2->player.name, games);

    for (int i = 0; i < games; i++) {
        int r;
        if (i % 2 == 0) {
            r = play_game(a1->player, a2->player, verbose);
            if (r == PLAYER_1) w1++; else if (r == PLAYER_2) w2++; else d++;
        } else {
            r = play_game(a2->player, a1->player, verbose);
            if (r == PLAYER_1) w2++; else if (r == PLAYER_2) w1++; else d++;
        }
        if (!verbose) printf("  Game %d: %s\n", i + 1,
            ((i % 2 == 0 && r == PLAYER_1) || (i % 2 == 1 && r == PLAYER_2)) ? a1->player.name :
            ((i % 2 == 0 && r == PLAYER_2) || (i % 2 == 1 && r == PLAYER_1)) ? a2->player.name : "Draw");
    }

    printf("Result: %s %d - %d %s (draws: %d)\n", a1->player.name, w1, w2, a2->player.name, d);

    a1->wins += w1; a1->losses += w2; a1->draws += d; a1->points += w1 * 3 + d;
    a2->wins += w2; a2->losses += w1; a2->draws += d; a2->points += w2 * 3 + d;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int verbose = 0, games = GAMES_PER_MATCH;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) verbose = 1;
        else if (strcmp(argv[i], "-q") == 0) games = 2;
        else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) games = atoi(argv[++i]);
    }

    printf("\n=== TOURNAMENT === (%d games/match)\n", games);

    AIEntry ais[NUM_AIS] = {
        { create_ai_pvs_player(), 0, 0, 0, 0 },
        { create_ai_pvs_v2_player(), 0, 0, 0, 0 }
    };

    int total = (NUM_AIS * (NUM_AIS - 1)) / 2, match = 0;
    for (int i = 0; i < NUM_AIS; i++)
        for (int j = i + 1; j < NUM_AIS; j++)
            { printf("\n[Match %d/%d]", ++match, total); run_match(&ais[i], &ais[j], games, verbose); }

    // Sort by points
    for (int i = 0; i < NUM_AIS - 1; i++)
        for (int j = i + 1; j < NUM_AIS; j++)
            if (ais[j].points > ais[i].points) { AIEntry t = ais[i]; ais[i] = ais[j]; ais[j] = t; }

    printf("\n=== STANDINGS ===\n");
    printf("%-12s  W    L    D   Pts\n", "AI");
    for (int i = 0; i < NUM_AIS; i++)
        printf("%-12s %3d  %3d  %3d  %3d\n", ais[i].player.name, ais[i].wins, ais[i].losses, ais[i].draws, ais[i].points);

    return 0;
}
