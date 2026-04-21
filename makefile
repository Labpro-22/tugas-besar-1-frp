# Makefile for C++ OOP Project (Optimized & Recursive)

# Compiler settings
CXX      := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -I include
SFML_LIBS := -lsfml-graphics -lsfml-window -lsfml-system

# Directories
SRC_DIR     := src
OBJ_DIR     := build
BIN_DIR     := bin
INCLUDE_DIR := include
DATA_DIR    := data
CONFIG_DIR  := config

# Target executable
TARGET := $(BIN_DIR)/game
GUI_TARGET := $(BIN_DIR)/game_gui

# 1. Recursive Source Finding
# Secara otomatis mencari semua file .cpp di dalam src/ dan semua sub-foldernya.
# Target game CLI tidak menyertakan test, GUI, dan main_gui.cpp.
SRCS := $(shell find $(SRC_DIR) -name '*.cpp' ! -name 'test_*.cpp' ! -path '$(SRC_DIR)/viewsGUI/*' ! -name 'main_gui.cpp')
GUI_VIEW_SRCS := $(shell find $(SRC_DIR)/viewsGUI -name '*.cpp' 2>/dev/null)
GUI_SRCS := $(filter-out $(SRC_DIR)/main.cpp, $(SRCS)) $(GUI_VIEW_SRCS) $(SRC_DIR)/main_gui.cpp
TEST_SRCS := $(shell find $(SRC_DIR) -name 'test_*.cpp')

# 2. Dynamic Object Mapping
# Mengubah path src/xxx/yyy.cpp menjadi build/xxx/yyy.o
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
GUI_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(GUI_SRCS))
TEST_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(TEST_SRCS))
GAME_OBJS_NO_MAIN := $(filter-out $(OBJ_DIR)/main.o, $(OBJS))
TEST_TARGET := $(BIN_DIR)/tests

# Main targets
all: directories $(TARGET)
gui: directories $(GUI_TARGET)

# Create necessary root directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(DATA_DIR) $(CONFIG_DIR)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
	@echo "Build successful! Executable is at $(TARGET)"

$(GUI_TARGET): $(GUI_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(SFML_LIBS)
	@echo "GUI build successful! Executable is at $(GUI_TARGET)"

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the game
run: all
	./$(TARGET)

run-gui: gui
	./$(GUI_TARGET)

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

.PHONY: all gui clean rebuild run run-gui test directories