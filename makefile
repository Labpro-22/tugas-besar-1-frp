# Makefile untuk proyek Nimonspoli
# Cocok dengan struktur repo saat ini:
# - src/main.cpp                    -> executable game (CLI)
# - src/main_gui.cpp                -> executable game GUI
# - src/core/test_engine.cpp        -> entry point test
# - src/**/test_*.cpp lainnya       -> helper test

CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -I include
LDFLAGS :=
SFML_LIBS := -lsfml-graphics -lsfml-window -lsfml-system

.DEFAULT_GOAL := all

SRC_DIR := src
OBJ_DIR := build
BIN_DIR := bin
DATA_DIR := data
CONFIG_DIR := config

SRC_DIRS := \
	$(SRC_DIR) \
	$(SRC_DIR)/core \
	$(SRC_DIR)/models \
	$(SRC_DIR)/utils \
	$(SRC_DIR)/views \
	$(SRC_DIR)/viewsGUI

ALL_SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
TEST_SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/test_*.cpp))
GUI_SRCS := $(filter-out $(TEST_SRCS) $(SRC_DIR)/main.cpp,$(ALL_SRCS))
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

ifeq ($(OS),Windows_NT)
SHELL := powershell.exe
.SHELLFLAGS := -NoProfile -ExecutionPolicy Bypass -Command
EXE_EXT := .exe

define make-dir
New-Item -ItemType Directory -Force -Path '$(1)' | Out-Null
endef

define remove-build-dir
if (Test-Path '$(1)') { Remove-Item -Path '$(1)' -Recurse -Force -ErrorAction SilentlyContinue; if (Test-Path '$(1)') { Write-Error 'Gagal menghapus folder $(1). Tutup program yang masih memakai file hasil build lalu coba lagi.'; exit 1 } }
endef

define remove-exes
if (Test-Path '$(1)') { Get-ChildItem -Path '$(1)' -Filter '*.exe' -File -ErrorAction SilentlyContinue | ForEach-Object { Remove-Item -Path $$_.FullName -Force -ErrorAction SilentlyContinue }; $$remaining = Get-ChildItem -Path '$(1)' -Filter '*.exe' -File -ErrorAction SilentlyContinue; if ($$remaining) { Write-Host ('Peringatan: exe tidak terhapus (mungkin masih berjalan): ' + ($$remaining.Name -join ', ')) } }
endef

RUN_GAME := & './$(TARGET)'
RUN_GUI := & './$(GUI_TARGET)'
RUN_TESTS := & './$(TEST_TARGET)'
NOOP := Write-Host ''
else
EXE_EXT :=

define make-dir
mkdir -p '$(1)'
endef

define remove-build-dir
rm -rf '$(1)'
endef

define remove-exes
rm -f '$(1)'/*.exe '$(1)'/game '$(1)'/game_gui '$(1)'/tests
endef

RUN_GAME := ./$(TARGET)
RUN_GUI := ./$(GUI_TARGET)
RUN_TESTS := ./$(TEST_TARGET)
NOOP := true
endif

.PHONY: all gui directories run run-gui test clean makeclean rebuild info

all: directories $(TARGET)

gui: directories $(GUI_TARGET)

directories:
	@$(call make-dir,$(OBJ_DIR))
	@$(call make-dir,$(BIN_DIR))
	@$(call make-dir,$(DATA_DIR))
	@$(call make-dir,$(CONFIG_DIR))

$(TARGET): $(GAME_OBJS)
	@$(call make-dir,$(BIN_DIR))
	$(CXX) $(CXXFLAGS) $(GAME_OBJS) $(LDFLAGS) -o $@

$(GUI_TARGET): $(GUI_OBJS)
	@$(call make-dir,$(BIN_DIR))
	$(CXX) $(CXXFLAGS) $(GUI_OBJS) $(LDFLAGS) $(SFML_LIBS) -o $@

$(TEST_TARGET): $(COMMON_GAME_OBJS) $(TEST_MAIN_OBJ) $(TEST_SUPPORT_OBJS)
	@$(call make-dir,$(BIN_DIR))
	$(CXX) $(CXXFLAGS) $(COMMON_GAME_OBJS) $(TEST_MAIN_OBJ) $(TEST_SUPPORT_OBJS) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(call make-dir,$(dir $@))
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

run: $(TARGET)
	$(RUN_GAME)

run-gui: $(GUI_TARGET)
	$(RUN_GUI)

test: $(TEST_TARGET)
	$(RUN_TESTS)

clean:
	@$(call remove-build-dir,$(OBJ_DIR))
	@$(call remove-exes,$(BIN_DIR))

makeclean: clean

rebuild: clean all

info:
	$(info Game sources:)
	$(foreach src,$(GAME_SRCS),$(info $(src)))
	$(info GUI sources:)
	$(foreach src,$(GUI_SRCS),$(info $(src)))
	$(info Test sources:)
	$(foreach src,$(TEST_SRCS),$(info $(src)))
	@$(NOOP)

-include $(DEPS)
