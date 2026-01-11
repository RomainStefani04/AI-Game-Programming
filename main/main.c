#include "../include/game.h"
#include "../include/player.h"
#include "../include/engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));

    int choice;
    printf("=== MANCALA IA ===\n\n");
    printf("Choisissez le mode de jeu:\n");
    printf("1. Humain vs Humain\n");
    printf("2. Humain vs IA Random\n");
    printf("3. IA Random vs Humain\n");
    printf("4. IA vs IA\n");
    printf("\nVotre choix: ");

    if (scanf("%d", &choice) != 1) {
        printf("Choix invalide!\n");
        return 1;
    }

    Player player1, player2;

    switch(choice) {
        case 1:
            player1 = create_human_player();
            player2 = create_human_player();
            break;
        case 2:
            player1 = create_human_player();
            player2 = create_ai_random_player();
            break;
        case 3:
            player1 = create_ai_random_player();
            player2 = create_human_player();
            break;
        case 4:
            player1 = create_ai_pvs_player();
            player2 = create_ai_pvs_v2_player();
            break;
        default:
            printf("Choix invalide!\n");
            return 1;
    }

    play_game(player1, player2, true);

    return 0;
}