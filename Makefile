CC = gcc
CFLAGS = -Wall -Wextra -std=c11
IFLAGS = -Iinclude

SRC_DIR = src
PLAYER_DIR = player
MAIN_DIR = main
TARGET_DIR = target

SRCS_COMMON = $(SRC_DIR)/game.c $(SRC_DIR)/engine.c $(SRC_DIR)/ai_common.c \
	$(PLAYER_DIR)/player.c $(PLAYER_DIR)/ai_random.c $(PLAYER_DIR)/ai_minimax.c $(PLAYER_DIR)/ai_alpha_beta.c  \
	$(PLAYER_DIR)/ai_alphabeta.c $(PLAYER_DIR)/ai_aspiration.c $(PLAYER_DIR)/ai_mtdf.c $(PLAYER_DIR)/ai_pvs.c $(PLAYER_DIR)/ai_pvs_v2.c

all: main simulation external

main: $(SRCS_COMMON) $(MAIN_DIR)/main.c
	$(CC) $(CFLAGS) $(IFLAGS) -o $(TARGET_DIR)/main $(SRCS_COMMON) $(MAIN_DIR)/main.c

replay: $(SRCS_COMMON) $(MAIN_DIR)/replay_game.c
	$(CC) $(CFLAGS) $(IFLAGS) -o $(TARGET_DIR)/replay_game $(SRCS_COMMON) $(MAIN_DIR)/replay_game.c


simulation: $(SRCS_COMMON) $(MAIN_DIR)/simulation.c
	$(CC) $(CFLAGS) $(IFLAGS) -o $(TARGET_DIR)/simulation $(SRCS_COMMON) $(MAIN_DIR)/simulation.c

tournament: $(SRCS_COMMON) $(MAIN_DIR)/tournament.c
	$(CC) $(CFLAGS) $(IFLAGS) -o $(TARGET_DIR)/tournament $(SRCS_COMMON) $(MAIN_DIR)/tournament.c

external: $(SRCS_COMMON) $(MAIN_DIR)/external_player.c
	$(CC) $(CFLAGS) $(IFLAGS) -o $(TARGET_DIR)/external_player $(SRCS_COMMON) $(MAIN_DIR)/external_player.c

clean:
	rm -f $(TARGET_DIR)/*

.PHONY: all main simulation external clean