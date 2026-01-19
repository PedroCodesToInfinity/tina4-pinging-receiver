# Compiler
CC = gcc

# Project name (executable)
TARGET = httpserver

# Directories
SRC_DIR = src
HDR_DIR = src/header
BIN_DIR = bin
LIB_DIR = lib

# Find all .c files in src/
SRC = $(wildcard $(SRC_DIR)/*.c)

# Convert src/*.c → bin/*.o
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRC))

# Compiler flags (C90 preferred)
CFLAGS = -std=c90 -Wall -Wextra -pedantic -I$(HDR_DIR)

# Linker flags (empty for now)
LDFLAGS =

# Default target
all: $(TARGET)

# Final executable in root
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compile .c → .o into bin/
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(BIN_DIR)/*.o $(TARGET)

# Rebuild from scratch
re: clean all

