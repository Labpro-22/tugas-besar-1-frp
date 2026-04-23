# Makefile untuk proyek Nimonspoli
# Default build/run: CLI dari src/main.cpp
# GUI tetap tersedia lewat target terpisah.

CXX := g++
.DEFAULT_GOAL := all

SRC_DIR := src
OBJ_DIR := build
BIN_DIR := bin
DATA_DIR := data
CONFIG_DIR := config
SFML_DIR := C:/Users/juan oloando/SFML-3.0.0

ifeq ($(OS),Windows_NT)
SHELL := powershell.exe
.SHELLFLAGS := -NoProfile -ExecutionPolicy Bypass -Command
EXE_EXT := .exe

define make-dir
New-Item -ItemType Directory -Force -Path '$(1)' | Out-Null
endef

define remove-build-dir
if (Test-Path '$(1)') { Get-ChildItem -Path '$(1)' -Recurse -File -Include '*.o','*.d' -ErrorAction SilentlyContinue | ForEach-Object { Remove-Item -LiteralPath $$_.FullName -Force -ErrorAction SilentlyContinue }; Get-ChildItem -Path '$(1)' -Recurse -Directory -ErrorAction SilentlyContinue | Sort-Object FullName -Descending | ForEach-Object { if (Test-Path $$_.FullName) { Remove-Item -LiteralPath $$_.FullName -Force -ErrorAction SilentlyContinue } }; if ((Test-Path '$(1)') -and (Get-ChildItem -Path '$(1)' -Recurse -File -Include '*.o','*.d' -ErrorAction SilentlyContinue)) { Write-Host 'Peringatan: sebagian artefak build masih dipakai proses lain.' } }
endef

define remove-bin-targets
if (Test-Path '$(1)') { $$targets = @('game','game.exe','game_gui','game_gui.exe','tests','tests.exe'); foreach ($$name in $$targets) { $$path = Join-Path '$(1)' $$name; if (Test-Path $$path) { Remove-Item -Path $$path -Force -ErrorAction SilentlyContinue } } $$remaining = @(); foreach ($$name in $$targets) { $$path = Join-Path '$(1)' $$name; if (Test-Path $$path) { $$remaining += $$name } } if ($$remaining.Count -gt 0) { Write-Host ('Peringatan: file output belum terhapus (mungkin masih dipakai): ' + ($$remaining -join ', ')) } }
endef

define copy-sfml-dlls
if (Test-Path '$(SFML_DIR)/bin') { Copy-Item -Path '$(SFML_DIR)/bin/*.dll' -Destination '$(1)' -Force -ErrorAction SilentlyContinue }
endef

RUN_GAME = & './$(TARGET)'
RUN_GUI = & './$(GUI_TARGET)'
RUN_TESTS = & './$(TEST_TARGET)'
NOOP := Write-Host ''
else
EXE_EXT :=

define make-dir
mkdir -p '$(1)'
endef

define remove-build-dir
rm -rf '$(1)'
endef

define remove-bin-targets
rm -f '$(1)'/*.exe '$(1)'/game '$(1)'/game_gui '$(1)'/tests
endef

define copy-sfml-dlls
true
endef

RUN_GAME = ./$(TARGET)
RUN_GUI = ./$(GUI_TARGET)
RUN_TESTS = ./$(TEST_TARGET)
NOOP := true
endif

COMMON_CXXFLAGS := -Wall -Wextra -std=c++17 -I include
CLI_CXXFLAGS := $(COMMON_CXXFLAGS)
GUI_CXXFLAGS := $(COMMON_CXXFLAGS) -I"$(SFML_DIR)/include"
CLI_LDFLAGS :=
GUI_LDFLAGS := -L"$(SFML_DIR)/lib" -lsfml-graphics -lsfml-window -lsfml-system

SRC_DIRS := \
	$(SRC_DIR) \
	$(SRC_DIR)/core \
	$(SRC_DIR)/models \
	$(SRC_DIR)/utils \
	$(SRC_DIR)/views \
	$(SRC_DIR)/viewsGUI

ALL_SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
TEST_SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/test_*.cpp))
GUI_SRCS := $(filter-out $(TEST_SRCS) $(SRC_DIR)/main.cpp $(SRC_DIR)/views/%.cpp,$(ALL_SRCS))
GAME_SRCS := $(filter-out $(TEST_SRCS) $(SRC_DIR)/main_gui.cpp $(SRC_DIR)/viewsGUI/%.cpp,$(ALL_SRCS))

TEST_MAIN := $(SRC_DIR)/core/test_engine.cpp
TEST_SUPPORT_SRCS := $(filter-out $(TEST_MAIN),$(TEST_SRCS))

GAME_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(GAME_SRCS))
GUI_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(GUI_SRCS))
COMMON_GAME_OBJS := $(filter-out $(OBJ_DIR)/main.o,$(GAME_OBJS))
TEST_MAIN_OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(TEST_MAIN))
TEST_SUPPORT_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(TEST_SUPPORT_SRCS))

TARGET := $(BIN_DIR)/game$(EXE_EXT)
GUI_TARGET := $(BIN_DIR)/game_gui$(EXE_EXT)
TEST_TARGET := $(BIN_DIR)/tests$(EXE_EXT)

DEPS := \
	$(GAME_OBJS:.o=.d) \
	$(GUI_OBJS:.o=.d) \
	$(TEST_MAIN_OBJ:.o=.d) \
	$(TEST_SUPPORT_OBJS:.o=.d)

.PHONY: all cli gui directories run run-cli run-gui test clean makeclean rebuild info

all: cli

cli: directories $(TARGET)

gui: directories $(GUI_TARGET)

directories:
	@$(call make-dir,$(OBJ_DIR))
	@$(call make-dir,$(BIN_DIR))
	@$(call make-dir,$(DATA_DIR))
	@$(call make-dir,$(CONFIG_DIR))

$(TARGET): $(GAME_OBJS)
	@$(call make-dir,$(BIN_DIR))
	$(CXX) $(CLI_CXXFLAGS) $(GAME_OBJS) $(CLI_LDFLAGS) -o $@

$(GUI_TARGET): $(GUI_OBJS)
	@$(call make-dir,$(BIN_DIR))
	$(CXX) $(GUI_CXXFLAGS) $(GUI_OBJS) $(GUI_LDFLAGS) -o $@
	@$(call copy-sfml-dlls,$(BIN_DIR))

$(TEST_TARGET): $(COMMON_GAME_OBJS) $(TEST_MAIN_OBJ) $(TEST_SUPPORT_OBJS)
	@$(call make-dir,$(BIN_DIR))
	$(CXX) $(CLI_CXXFLAGS) $(COMMON_GAME_OBJS) $(TEST_MAIN_OBJ) $(TEST_SUPPORT_OBJS) $(CLI_LDFLAGS) -o $@

$(OBJ_DIR)/viewsGUI/%.o: $(SRC_DIR)/viewsGUI/%.cpp
	@$(call make-dir,$(dir $@))
	$(CXX) $(GUI_CXXFLAGS) -MMD -MP -c $< -o $@

$(OBJ_DIR)/main_gui.o: $(SRC_DIR)/main_gui.cpp
	@$(call make-dir,$(dir $@))
	$(CXX) $(GUI_CXXFLAGS) -MMD -MP -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(call make-dir,$(dir $@))
	$(CXX) $(CLI_CXXFLAGS) -MMD -MP -c $< -o $@

run: cli
	$(RUN_GAME)

run-cli: run

run-gui: gui
	$(RUN_GUI)

test: $(TEST_TARGET)
	$(RUN_TESTS)

clean:
	@$(call remove-build-dir,$(OBJ_DIR))
	@$(call remove-bin-targets,$(BIN_DIR))

makeclean: clean

rebuild: clean all

info:
	$(info CLI sources:)
	$(foreach src,$(GAME_SRCS),$(info $(src)))
	$(info GUI sources:)
	$(foreach src,$(GUI_SRCS),$(info $(src)))
	$(info Test sources:)
	$(foreach src,$(TEST_SRCS),$(info $(src)))
	@$(NOOP)

-include $(DEPS)
