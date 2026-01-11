cmake_minimum_required(VERSION 3.29)
project(AI_Game_Programming C)

set(CMAKE_C_STANDARD 11)

add_executable(AI_Game_Programming
        include/game.h
        src/game.c
        player/ai_random.c
        include/ai_random.h
        main/main.c
        player/player.c
        include/player.h
        main/simulation.c
        src/engine.c
        include/engine.h
        player/ai_minimax.c
        include/ai_minimax.h
        main/external_player.c
        player/ai_alpha_beta.c
        include/ai_alpha_beta.h
        player/ai_pvs_v2.c
        include/ai_pvs_v2.h
        player/ai_alphabeta.c
        src/ai_common.c
        player/ai_pvs.c
        player/ai_mtdf.c
        player/ai_aspiration.c
        main/tournament.c
)
