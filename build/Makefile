# Compiler
CC = gcc

# Directories
SRC_DIR = ../src
OBJ_DIR = ../obj
INCLUDE_DIR = ../include
BUILD_DIR = .

# Flags
CFLAGS = -I$(INCLUDE_DIR) -Wall
LDFLAGS = -lSDL2 -lSDL2_ttf -lm  # Added -lSDL2_ttf for text rendering

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Object files
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Target executable
TARGET = $(BUILD_DIR)/ti84_emulator

# Rule to build the target
all: directories $(TARGET)

# Rule to link the final executable
$(TARGET): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(TARGET) $(LDFLAGS)

# Rule to compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to ensure the obj directory exists
directories:
	mkdir -p $(OBJ_DIR)

# Clean rule to remove object files and the target executable
clean:
	rm -rf $(OBJ_DIR)/*.o $(TARGET)
