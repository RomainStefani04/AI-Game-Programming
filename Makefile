CC = gcc
CFLAGS = -Wall -Wextra -std=c11
IFLAGS = -Iinclude

SRC_DIR = src
PLAYER_DIR = player
MAIN_DIR = main
TARGET_DIR = target

SRCS_COMMON = $(SRC_DIR)/game.c $(SRC_DIR)/engine.c $(PLAYER_DIR)/player.c $(PLAYER_DIR)/ai_random.c $(PLAYER_DIR)/ai_minimax.c $(PLAYER_DIR)/ai_alpha_beta.c $(PLAYER_DIR)/ai_alpha_beta_nul.c

all: main simulation external

main: $(SRCS_COMMON) $(MAIN_DIR)/main.c
	$(CC) $(CFLAGS) $(IFLAGS) -o $(TARGET_DIR)/main $(SRCS_COMMON) $(MAIN_DIR)/main.c

simulation: $(SRCS_COMMON) $(MAIN_DIR)/simulation.c
	$(CC) $(CFLAGS) $(IFLAGS) -o $(TARGET_DIR)/simulation $(SRCS_COMMON) $(MAIN_DIR)/simulation.c

external: $(SRCS_COMMON) $(MAIN_DIR)/external_player.c
	$(CC) $(CFLAGS) $(IFLAGS) -o $(TARGET_DIR)/external_player $(SRCS_COMMON) $(MAIN_DIR)/external_player.c

clean:
	rm -f $(TARGET_DIR)/*

.PHONY: all main simulation external clean