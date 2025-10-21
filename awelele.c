//
// Created by romai on 21/10/2025.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Structure pour un trou
typedef struct {
    int red;
    int blue;
    int transparent;
} Hole;

// Structure du plateau de jeu
typedef struct {
    Hole holes[16];
    int captured_player1;
    int captured_player2;
    int current_player;  // 1 ou 2
} GameBoard;

// Initialiser le plateau de jeu
void init_game(GameBoard *board) {
    for (int i = 0; i < 16; i++) {
        board->holes[i].red = 2;
        board->holes[i].blue = 2;
        board->holes[i].transparent = 2;
    }
    board->captured_player1 = 0;
    board->captured_player2 = 0;
    board->current_player = 1;
}

// Compter le total de graines dans un trou
int count_hole_seeds(Hole *hole) {
    return hole->red + hole->blue + hole->transparent;
}

// Compter le total de graines sur le plateau
int count_seeds_on_board(GameBoard *board) {
    int total = 0;
    for (int i = 0; i < 16; i++) {
        total += count_hole_seeds(&board->holes[i]);
    }
    return total;
}

// Analyser un coup (format: "3R", "14B", "4TR")
int parse_move(char *move, int *hole, char *color, char *transparent_as) {
    int len = strlen(move);
    if (len < 2 || len > 4) return 0;

    // Parser le numéro du trou
    char hole_str[3] = {0};
    int i = 0;
    while (i < len && isdigit(move[i])) {
        hole_str[i] = move[i];
        i++;
    }

    if (i == 0) return 0;
    *hole = atoi(hole_str);

    if (*hole < 1 || *hole > 16) return 0;

    // Parser la couleur
    if (i >= len) return 0;

    if (move[i] == 'T' && i + 1 < len) {
        *color = 'T';
        *transparent_as = move[i + 1];
        return (*transparent_as == 'R' || *transparent_as == 'B');
    } else if (move[i] == 'R' || move[i] == 'B') {
        *color = move[i];
        *transparent_as = '\0';
        return 1;
    }

    return 0;
}

// Vérifier si un coup est valide
int is_valid_move(GameBoard *board, int hole, char color, int player) {
    // Vérifier que le trou appartient au joueur
    if (player == 1 && hole % 2 == 0) return 0;
    if (player == 2 && hole % 2 == 1) return 0;

    // Vérifier qu'il y a des graines de cette couleur
    Hole *h = &board->holes[hole - 1];
    if (color == 'R' && h->red == 0) return 0;
    if (color == 'B' && h->blue == 0) return 0;
    if (color == 'T' && h->transparent == 0) return 0;

    return 1;
}

// Exécuter le semis et retourner le dernier trou
int execute_move(GameBoard *board, int hole, char color, char transparent_as) {
    Hole *start_hole = &board->holes[hole - 1];
    int player = board->current_player;

    // Récupérer les graines selon la couleur
    int transparent_seeds = 0;
    int other_seeds = 0;

    if (color == 'T') {
        transparent_seeds = start_hole->transparent;
        start_hole->transparent = 0;

        if (transparent_as == 'R') {
            other_seeds = start_hole->red;
            start_hole->red = 0;
        } else {
            other_seeds = start_hole->blue;
            start_hole->blue = 0;
        }
    } else if (color == 'R') {
        other_seeds = start_hole->red;
        start_hole->red = 0;
    } else {
        other_seeds = start_hole->blue;
        start_hole->blue = 0;
    }

    int current_pos = hole; // Position de 1 à 16
    int last_pos = hole;

    // Distribuer les transparentes EN PREMIER
    for (int i = 0; i < transparent_seeds; i++) {
        // Avancer à la position suivante
        do {
            current_pos = (current_pos % 16) + 1;
        } while (current_pos == hole); // Sauter le trou de départ

        // Si bleu, trouver le prochain trou adverse
        if (color == 'T' && transparent_as == 'B') {
            while (!((player == 1 && current_pos % 2 == 0) ||
                     (player == 2 && current_pos % 2 == 1))) {
                current_pos = (current_pos % 16) + 1;
                if (current_pos == hole) {
                    current_pos = (current_pos % 16) + 1;
                }
            }
        }

        board->holes[current_pos - 1].transparent++;
        last_pos = current_pos;
    }

    // Distribuer les autres graines
    for (int i = 0; i < other_seeds; i++) {
        // Avancer à la position suivante
        do {
            current_pos = (current_pos % 16) + 1;
        } while (current_pos == hole); // Sauter le trou de départ

        // Si bleu, trouver le prochain trou adverse
        if (color == 'B' || (color == 'T' && transparent_as == 'B')) {
            while (!((player == 1 && current_pos % 2 == 0) ||
                     (player == 2 && current_pos % 2 == 1))) {
                current_pos = (current_pos % 16) + 1;
                if (current_pos == hole) {
                    current_pos = (current_pos % 16) + 1;
                }
            }
        }

        if (color == 'R' || (color == 'T' && transparent_as == 'R')) {
            board->holes[current_pos - 1].red++;
        } else {
            board->holes[current_pos - 1].blue++;
        }
        last_pos = current_pos;
    }

    return last_pos;
}

// Vérifier et exécuter les captures
int check_captures(GameBoard *board, int hole, int player) {
    int captured = 0;
    int current_pos = hole;

    while (1) {
        Hole *h = &board->holes[current_pos - 1];
        int total = count_hole_seeds(h);

        if (total == 2 || total == 3) {
            captured += total;
            h->red = 0;
            h->blue = 0;
            h->transparent = 0;

            // Continuer en sens inverse (rétrograde)
            current_pos--;
            if (current_pos < 1) current_pos = 16;
        } else {
            break;
        }
    }

    if (player == 1) {
        board->captured_player1 += captured;
    } else {
        board->captured_player2 += captured;
    }

    return captured;
}

// Vérifier les conditions de fin
int check_game_end(GameBoard *board) {
    // Victoire si 49+ graines
    if (board->captured_player1 >= 49) return 1;
    if (board->captured_player2 >= 49) return 2;

    // Égalité si 48 chacun
    if (board->captured_player1 >= 48 && board->captured_player2 >= 48) return 3;

    // Fin si < 10 graines sur le plateau
    if (count_seeds_on_board(board) < 10) {
        if (board->captured_player1 > board->captured_player2) return 1;
        if (board->captured_player2 > board->captured_player1) return 2;
        return 3;
    }

    return 0; // Continue
}

// Afficher le plateau
void display_board(GameBoard *board) {
    printf("\n=== État du plateau ===\n");
    printf("Joueur 1 (impair) - Capturées: %d\n", board->captured_player1);
    printf("Joueur 2 (pair) - Capturées: %d\n", board->captured_player2);
    printf("\n");

    // Afficher en deux lignes pour meilleure visualisation
    printf("Trous du Joueur 1 (impair):\n");
    for (int i = 0; i < 16; i += 2) {
        Hole *h = &board->holes[i];
        printf("Trou %2d: %dR %dB %dT (total:%d)  ",
               i + 1, h->red, h->blue, h->transparent, count_hole_seeds(h));
        if ((i + 2) % 8 == 0) printf("\n");
    }

    printf("\nTrous du Joueur 2 (pair):\n");
    for (int i = 1; i < 16; i += 2) {
        Hole *h = &board->holes[i];
        printf("Trou %2d: %dR %dB %dT (total:%d)  ",
               i + 1, h->red, h->blue, h->transparent, count_hole_seeds(h));
        if ((i + 1) % 8 == 0) printf("\n");
    }

    printf("\nTrous du plateau:\n");
    for (int i = 0; i < 16; i += 1) {
        Hole *h = &board->holes[i];
        printf("Trou %2d: %dR %dB %dT (total:%d)  ",
               i + 1, h->red, h->blue, h->transparent, count_hole_seeds(h));
        if ((i + 1) % 16 == 0) printf("\n");
    }

    printf("\nGraines totales sur le plateau: %d\n", count_seeds_on_board(board));
    printf("========================\n\n");
}

// Fonction principale du jeu
void play_game() {
    GameBoard board;
    init_game(&board);

    char move_str[10];
    int hole;
    char color, transparent_as;

    printf("=== Jeu de Semis avec Graines Colorées ===\n");
    printf("Format des coups: NC (ex: 3R, 14B, 4TR)\n");
    printf("Tapez 'quit' pour quitter\n\n");

    while (1) {
        display_board(&board);

        // Vérifier fin de jeu
        int result = check_game_end(&board);
        if (result != 0) {
            if (result == 1) printf(">>> JOUEUR 1 GAGNE! <<<\n");
            else if (result == 2) printf(">>> JOUEUR 2 GAGNE! <<<\n");
            else printf(">>> ÉGALITÉ! <<<\n");
            break;
        }

        // Demander le coup
        printf("Joueur %d, votre coup: ", board.current_player);
        if (scanf("%9s", move_str) != 1) break;

        if (strcmp(move_str, "quit") == 0) break;

        // Parser et valider
        if (!parse_move(move_str, &hole, &color, &transparent_as)) {
            printf("Coup invalide! Format: NC (ex: 3R, 14B, 4TR)\n");
            continue;
        }

        if (!is_valid_move(&board, hole, color, board.current_player)) {
            printf("Coup illégal! Vérifiez le trou et la couleur.\n");
            continue;
        }

        // Exécuter le coup et récupérer le dernier trou
        int last_hole = execute_move(&board, hole, color, transparent_as);

        // Vérifier les captures à partir du dernier trou
        int captured = check_captures(&board, last_hole, board.current_player);

        if (captured > 0) {
            printf(">>> %d graine(s) capturée(s)!\n", captured);
        }

        // Changer de joueur
        board.current_player = (board.current_player == 1) ? 2 : 1;
    }
}

// Fonction pour configurer manuellement un plateau (pour les tests)
void setup_test_board(GameBoard *board, int setup_number) {
    // Réinitialiser
    for (int i = 0; i < 16; i++) {
        board->holes[i].red = 0;
        board->holes[i].blue = 0;
        board->holes[i].transparent = 0;
    }
    board->captured_player1 = 0;
    board->captured_player2 = 0;

    if (setup_number == 1) {
        // Case 1
        board->holes[0].red = 2;  // Trou 1
        board->holes[15].red = 2; // Trou 16
        board->holes[14].blue = 2; // Trou 15
        board->holes[13].blue = 2; board->holes[13].red = 2; // Trou 14
        board->holes[12].red = 2; board->holes[12].blue = 2; // Trou 13
        board->current_player = 2;
    } else if (setup_number == 2) {
        // Case 2
        board->holes[0].red = 1;  // Trou 1
        board->holes[1].red = 2;  // Trou 2
        board->holes[2].blue = 1; // Trou 3
        board->holes[3].blue = 2; // Trou 4
        board->holes[4].red = 1;  // Trou 5
        board->holes[15].blue = 3; board->holes[15].red = 1; // Trou 16
        board->holes[14].red = 2; // Trou 15
        board->holes[13].blue = 4; // Trou 14
        board->current_player = 2;
    }
}

// Fonction de test pour Case 1
void test_case1() {
    printf("\n=== TEST CASE 1 ===\n");
    GameBoard board;
    setup_test_board(&board, 1);

    printf("État initial:\n");
    display_board(&board);

    printf("Exécution: 14B\n");
    int last_hole = execute_move(&board, 14, 'B', '\0');
    printf("Dernier trou: %d\n", last_hole);

    int captured = check_captures(&board, last_hole, 2);
    printf("Graines capturées: %d (attendu: 10)\n", captured);

    display_board(&board);
    printf("===================\n\n");
}

// Fonction de test pour Case 2.1
void test_case2_1() {
    printf("\n=== TEST CASE 2.1 ===\n");
    GameBoard board;
    setup_test_board(&board, 2);

    printf("État initial:\n");
    display_board(&board);

    printf("Exécution: 16B\n");
    int last_hole = execute_move(&board, 16, 'B', '\0');
    printf("Dernier trou: %d\n", last_hole);

    int captured = check_captures(&board, last_hole, 2);
    printf("Graines capturées: %d (attendu: 10)\n", captured);

    display_board(&board);
    printf("===================\n\n");
}

// Fonction de test pour Case 2.2
void test_case2_2() {
    printf("\n=== TEST CASE 2.2 ===\n");
    GameBoard board;
    setup_test_board(&board, 2);

    printf("État initial:\n");
    display_board(&board);

    printf("Exécution: 16R\n");
    int last_hole = execute_move(&board, 16, 'R', '\0');
    printf("Dernier trou: %d\n", last_hole);

    int captured = check_captures(&board, last_hole, 2);
    printf("Graines capturées: %d (attendu: 7)\n", captured);

    display_board(&board);
    printf("===================\n\n");
}

// Menu principal
void main_menu() {
    int choice;

    while (1) {
        printf("\n=== JEU DE SEMIS AVEC GRAINES COLORÉES ===\n");
        printf("1. Jouer une partie\n");
        printf("2. Test Case 1 (14B)\n");
        printf("3. Test Case 2.1 (16B)\n");
        printf("4. Test Case 2.2 (16R)\n");
        printf("5. Quitter\n");
        printf("Votre choix: ");

        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1:
                play_game();
                break;
            case 2:
                test_case1();
                break;
            case 3:
                test_case2_1();
                break;
            case 4:
                test_case2_2();
                break;
            case 5:
                printf("Au revoir!\n");
                return;
            default:
                printf("Choix invalide!\n");
        }
    }
}

int main() {
    main_menu();
    return 0;
}