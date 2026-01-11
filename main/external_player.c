/*
 * external_player.c
 * Programme pour jouer via stdin/stdout avec l'arbitre Java
 * 
 * Protocole:
 * - Reçoit "START" -> initialise et joue premier coup
 * - Reçoit coup adverse (ex: "3R", "14B", "4TR") -> applique et répond avec notre coup
 * - Envoie "RESULT [score_J1] [score_J2]" quand partie terminée
 * 
 * IMPORTANT: Aucun affichage debug sur stdout (pollue la communication)
 *            Utiliser stderr pour debug: fprintf(stderr, "debug: ...\n");
 */

#include "../include/game.h"
#include "../include/player.h"
#include "../include/ai_minimax.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

// Pour getpid() - compatible Windows et Unix
#ifdef _WIN32
    #include <process.h>
    #define getpid _getpid
#else
    #include <unistd.h>
#endif

// Fonction pour logger sur stderr (ne pollue pas stdout)
void log_debug(const char* format, ...) {
    #ifdef DEBUG_MODE
    va_list args;
    va_start(args, format);
    fprintf(stderr, "[DEBUG] ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    fflush(stderr);
    va_end(args);
    #endif
}

// Fonction pour envoyer un coup sur stdout
void send_move(const Move* move) {
    if (move->color == TRANSPARENT) {
        printf("%d%s%s\n",
               move->hole_number,
               "T",
               (move->transparent_color == RED) ? "R" : "B");
    } else {
        printf("%d%s\n",
               move->hole_number,
               (move->color == RED) ? "R" : "B");
    }
    fflush(stdout);
}

// Fonction pour envoyer le résultat final
void send_result(const GameState* state) {
    printf("RESULT %d %d\n",
           state->captures[PLAYER_1],
           state->captures[PLAYER_2]);
    fflush(stdout);
}

int main(int argc, char* argv[]) {
    // Initialiser le générateur aléatoire (CRITIQUE pour ai_random)
    unsigned int seed;

    if (argc > 2) {
        // Seed fourni en argument (pour tests reproductibles)
        seed = (unsigned int)atoi(argv[2]);
        log_debug("Mode reproductible - Seed: %u", seed);
    } else {
        // Seed basé sur temps + PID (pour maximum d'aléatoire)
        // Le PID garantit des seeds différents même si lancés à la même seconde
        seed = (unsigned int)(time(NULL) + getpid());
        log_debug("Mode aléatoire - Seed: %u", seed);
    }

    srand(seed);

    // Déterminer quel joueur nous sommes (PLAYER_1 ou PLAYER_2)
    PlayerIndex our_player = PLAYER_1;

    if (argc > 1) {
        // Si argument fourni (ex: "JoueurA" ou "JoueurB")
        if (strstr(argv[1], "B") != NULL || strstr(argv[1], "2") != NULL) {
            our_player = PLAYER_2;
        }
    }

    log_debug("Starting as Player %d", our_player + 1);

    // Initialiser l'état du jeu
    GameState state;
    init_game_state(&state);

    // Créer notre IA
    Player our_ai = create_ai_random_player();

    // Buffer pour lecture
    char input_line[256];

    // Boucle principale de communication
    while (fgets(input_line, sizeof(input_line), stdin) != NULL) {
        // Retirer le '\n' de fin
        input_line[strcspn(input_line, "\n")] = 0;

        log_debug("Received: %s", input_line);

        // Cas 1: START - Début de partie
        if (strcmp(input_line, "START") == 0) {
            log_debug("Game starting, we are player %d", our_player + 1);

            // Si nous sommes le joueur 1, on joue en premier
            if (our_player == PLAYER_1) {
                Move our_move;
                our_ai.play(&state, &our_move);

                // Appliquer notre coup
                state.captures[state.current_player] += execute_move(&state, &our_move);
                state.current_player = (state.current_player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
                state.turn_number++;

                log_debug("Playing first move");
                send_move(&our_move);
            }
            // Sinon on attend le coup du joueur 1
            continue;
        }

        // Cas 2: END - Fin de partie
        if (strcmp(input_line, "END") == 0) {
            log_debug("Game ended by arbiter");
            break;
        }

        // Cas 3: Coup de l'adversaire
        Move opponent_move;
        if (parse_move(input_line, &opponent_move)) {
            log_debug("Opponent played: %s", input_line);

            // Vérifier que c'est bien le tour de l'adversaire
            if (state.current_player != our_player) {
                // Appliquer le coup adverse
                state.captures[state.current_player] += execute_move(&state, &opponent_move);
                state.current_player = (state.current_player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
                state.turn_number++;
            } else {
                log_debug("ERROR: Received opponent move but it's our turn");
            }

            // Vérifier si la partie est terminée
            if (is_game_over(&state)) {
                log_debug("Game over detected");
                send_result(&state);
                break;
            }

            // Jouer notre coup
            Move our_move;
            our_ai.play(&state, &our_move);

            // Appliquer notre coup
            state.captures[state.current_player] += execute_move(&state, &our_move);
            state.current_player = (state.current_player == PLAYER_1) ? PLAYER_2 : PLAYER_1;
            state.turn_number++;

            log_debug("Playing our move");
            send_move(&our_move);

            // Vérifier si la partie est terminée après notre coup
            if (is_game_over(&state)) {
                log_debug("Game over after our move");
                send_result(&state);
                break;
            }
        } else {
            log_debug("ERROR: Could not parse move: %s", input_line);
        }
    }

    log_debug("External player exiting");
    return 0;
}