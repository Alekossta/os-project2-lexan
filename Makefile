BIN_DIR = bin
DATA_DIR = data
INCLUDE_DIR = include
SRC_DIR = src
OBJ_DIR = obj
CC = gcc
CFLAGS = -Wall -I$(INCLUDE_DIR)
MAIN_FILES = lexan.c splitter.c builder.c
COMMON_SRCS = $(filter-out $(addprefix $(SRC_DIR)/, $(MAIN_FILES)), $(wildcard $(SRC_DIR)/*.c))
LEXAN_SRCS = $(SRC_DIR)/lexan.c $(COMMON_SRCS)
SPLITTER_SRCS = $(SRC_DIR)/splitter.c $(COMMON_SRCS)
BUILDER_SRCS = $(SRC_DIR)/builder.c $(COMMON_SRCS)
LEXAN_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LEXAN_SRCS))
SPLITTER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SPLITTER_SRCS))
BUILDER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(BUILDER_SRCS))

all: $(BIN_DIR)/lexan $(BIN_DIR)/splitter $(BIN_DIR)/builder

$(BIN_DIR)/lexan: $(LEXAN_OBJS) | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/splitter: $(SPLITTER_OBJS) | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/builder: $(BUILDER_OBJS) | $(BIN_DIR)
	$(CC) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

.PHONY: all clean

clean:
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/*

run: $(BIN_DIR)/lexan
	$(BIN_DIR)/lexan -i $(DATA_DIR)/texts/WilliamShakespeareWorks_a.txt -l 50 -m 50 -t 5 -e $(DATA_DIR)/exclusionLists/ExclusionList1_a.txt -o $(DATA_DIR)/output/output.txt
valgrind: $(BIN_DIR)/lexan
	valgrind --leak-check=full --track-origins=yes $(BIN_DIR)/lexan -i $(DATA_DIR)/texts/WilliamShakespeareWorks_a.txt -l 50 -m 50 -t 5 -e $(DATA_DIR)/exclusionLists/ExclusionList1_a.txt -o $(DATA_DIR)/output/output.txt