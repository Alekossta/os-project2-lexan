# Directories
BIN_DIR = bin
DATA_DIR = data
INCLUDE_DIR = include
SRC_DIR = src
OBJ_DIR = obj

# Compiler and Flags
CC = gcc
CFLAGS = -Wall -I$(INCLUDE_DIR)

# Source Files
MAIN_FILES = lexan.c splitter.c builder.c
COMMON_SRCS = $(filter-out $(addprefix $(SRC_DIR)/, $(MAIN_FILES)), $(wildcard $(SRC_DIR)/*.c))

LEXAN_SRCS = $(SRC_DIR)/lexan.c $(COMMON_SRCS)
SPLITTER_SRCS = $(SRC_DIR)/splitter.c $(COMMON_SRCS)
BUILDER_SRCS = $(SRC_DIR)/builder.c $(COMMON_SRCS)

# Object Files
LEXAN_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LEXAN_SRCS))
SPLITTER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SPLITTER_SRCS))
BUILDER_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(BUILDER_SRCS))

# Default Target
all: $(BIN_DIR)/lexan $(BIN_DIR)/splitter $(BIN_DIR)/builder

# Build Executables
$(BIN_DIR)/lexan: $(LEXAN_OBJS) | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/splitter: $(SPLITTER_OBJS) | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/builder: $(BUILDER_OBJS) | $(BIN_DIR)
	$(CC) -o $@ $^

# Compile Object Files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure Directories Exist
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Phony Targets
.PHONY: all clean

# Clean Build Files
clean:
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/*

run: $(BIN_DIR)/lexan
	$(BIN_DIR)/lexan -i $(DATA_DIR)/texts/GreatExpectations_a.txt -l 50 -m 50 -t 5 -e $(DATA_DIR)/exclusionLists/ExclusionList1_a.txt -o $(DATA_DIR)/output/output.txt