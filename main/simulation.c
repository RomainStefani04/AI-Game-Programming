//
// Created by romai on 25/11/2025.
//
#include "../include/game.h"
#include "../include/player.h"
#include "../include/engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));

    const int NUM_GAMES = 1000;
    int wins_player1 = 0;
    int wins_player2 = 0;
    int draws = 0;

    Player player1 = create_ai_random_player();
    Player player2 = create_ai_random_player();

    printf("=== SIMULATION: %d parties Random vs Random ===\n\n", NUM_GAMES);

    for (int i = 0; i < NUM_GAMES; i++) {
        int winner = play_game(player1, player2, false);

        if (winner == PLAYER_1) {
            wins_player1++;
        } else if (winner == PLAYER_2) {
            wins_player2++;
        } else {
            draws++;
        }

        if ((i + 1) % 10 == 0) {
            printf("Progression: %d/%d parties completees\n", i + 1, NUM_GAMES);
        }
    }

    printf("\n=== RESULTATS ===\n");
    printf("Victoires Joueur 1: %d (%.1f%%)\n", wins_player1, (wins_player1 * 100.0) / NUM_GAMES);
    printf("Victoires Joueur 2: %d (%.1f%%)\n", wins_player2, (wins_player2 * 100.0) / NUM_GAMES);
    printf("Matchs nuls: %d (%.1f%%)\n", draws, (draws * 100.0) / NUM_GAMES);

    return 0;
}
