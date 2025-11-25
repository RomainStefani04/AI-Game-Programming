CC = gcc
CFLAGS = -Wall -Wextra -std=c11
IFLAGS = -Iinclude

SRC_DIR = src
PLAYER_DIR = player
MAIN_DIR = main
TARGET_DIR = target

SRCS_COMMON = $(SRC_DIR)/game.c $(SRC_DIR)/engine.c $(PLAYER_DIR)/player.c $(PLAYER_DIR)/ai_random.c

all: main simulation

main: $(SRCS_COMMON) $(MAIN_DIR)/main.c
	$(CC) $(CFLAGS) $(IFLAGS) -o $(TARGET_DIR)/main $(SRCS_COMMON) $(MAIN_DIR)/main.c

simulation: $(SRCS_COMMON) $(MAIN_DIR)/simulation.c
	$(CC) $(CFLAGS) $(IFLAGS) -o $(TARGET_DIR)/simulation $(SRCS_COMMON) $(MAIN_DIR)/simulation.c

clean:
	rm -f $(TARGET_DIR)/*

.PHONY: all main simulation clean
