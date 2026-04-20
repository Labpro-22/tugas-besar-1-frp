# Makefile for C++ OOP Project (Optimized & Recursive)

# Compiler settings
CXX      := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -I include

# Directories
SRC_DIR     := src
OBJ_DIR     := build
BIN_DIR     := bin
INCLUDE_DIR := include
DATA_DIR    := data
CONFIG_DIR  := config

# Target executable
TARGET := $(BIN_DIR)/game

# 1. Recursive Source Finding
# Secara otomatis mencari semua file .cpp di dalam src/ dan semua sub-foldernya
# Target game tidak menyertakan file test_*.cpp
SRCS := $(shell find $(SRC_DIR) -name '*.cpp' ! -name 'test_*.cpp')
TEST_SRCS := $(shell find $(SRC_DIR) -name 'test_*.cpp')

# 2. Dynamic Object Mapping
# Mengubah path src/xxx/yyy.cpp menjadi build/xxx/yyy.o
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
TEST_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(TEST_SRCS))
GAME_OBJS_NO_MAIN := $(filter-out $(OBJ_DIR)/main.o, $(OBJS))
TEST_TARGET := $(BIN_DIR)/tests

# Main targets
all: directories $(TARGET)

# Create necessary root directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(DATA_DIR) $(CONFIG_DIR)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Build successful! Executable is at $(TARGET)"

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the game
run: all
	./$(TARGET)

# Build and run tests
test: directories $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(GAME_OBJS_NO_MAIN) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Test build successful! Executable is at $(TEST_TARGET)"

# Clean up generated files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned up $(OBJ_DIR) and $(BIN_DIR)"

# Rebuild everything from scratch
rebuild: clean all

.PHONY: all clean rebuild run test directories