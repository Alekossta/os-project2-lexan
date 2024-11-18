# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

# Binaries
LEXAN = $(BIN_DIR)/lexan
BUILDER = $(BIN_DIR)/builder
SPLITTER = $(BIN_DIR)/splitter

# Default target
all: $(LEXAN) $(BUILDER) $(SPLITTER)

# Build lexan binary
$(LEXAN): $(OBJ_DIR)/lexan.o $(OBJ_DIR)/ConsoleReader.o
	$(CC) $(CFLAGS) $^ -o $@

# Build builder binary
$(BUILDER): $(OBJ_DIR)/builder.o
	$(CC) $(CFLAGS) $^ -o $@

# Build splitter binary
$(SPLITTER): $(OBJ_DIR)/splitter.o
	$(CC) $(CFLAGS) $^ -o $@

# Compile object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Create bin directory
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Run lexan binary
run-lexan: $(LEXAN)
	@$(LEXAN) -i ./data/texts/GreatExpectations_a.txt -l 50 -m 50 -t 5 -e ./data/exclusionLists/ExclusionList1_a.txt -o output.txt

.PHONY: all clean run-lexan