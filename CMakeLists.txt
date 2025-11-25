cmake_minimum_required(VERSION 3.29)
project(AI_Game_Programming C)

set(CMAKE_C_STANDARD 11)

add_executable(AI_Game_Programming
        game.h
        game.c
        ai_random.c
        ai_random.h
        main.c)
