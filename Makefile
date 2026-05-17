# Makefile for DOMINION SDL3 (Linux) - FULL GAME BUILD

# Compiler and flags
CC = gcc
SDL3_CFLAGS = $(shell pkg-config --cflags sdl3 sdl3-ttf 2>/dev/null || echo "-I/usr/include/SDL3 -I/usr/include/SDL3_ttf")
SDL3_LIBS = $(shell pkg-config --libs sdl3 sdl3-ttf 2>/dev/null || echo "-lSDL3 -lSDL3_ttf")
CFLAGS = -std=c11 -Iinclude $(SDL3_CFLAGS) -Wall -Wno-unused-variable -Wno-unused-function
LDFLAGS =
LIBS = $(SDL3_LIBS) -lm

# Build type
DEBUG_FLAGS = -g -O0 -DDEBUG
RELEASE_FLAGS = -O3 -DNDEBUG -flto -march=native -mtune=native -fomit-frame-pointer -ffast-math -funroll-loops

# Directories
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Target
TARGET = $(BUILD_DIR)/dominion

# Engine sources
ENGINE_SRCS = \
	src/engine/window.c \
	src/engine/renderer.c \
	src/engine/input.c \
	src/engine/font.c

# UI sources
UI_SRCS = \
	src/ui/sdl3_main.c \
	src/ui/button.c \
	src/ui/ui_common.c \
	src/ui/app_controller.c \
	src/ui/scene_manager.c \
	src/ui/scenes/scene_splash.c \
	src/ui/scenes/scene_profile_select.c \
	src/ui/scenes/scene_profile_create.c \
	src/ui/scenes/scene_main_menu.c \
	src/ui/scenes/scene_setup.c \
	src/ui/scenes/scene_spawn_select.c \
	src/ui/scenes/scene_game.c

# Core game sources - ALL SYSTEMS
CORE_SRCS = \
	src/core/game.c \
	src/core/profile.c \
	src/core/simulation_engine/time_manager.c \
	src/core/simulation_engine/system_orchestrator.c \
	src/core/simulation_engine/performance_optimizer.c \
	src/core/simulation_engine/event_dispatcher.c \
	src/core/simulation_engine/state_persistence.c \
	src/core/data/history_db.c \
	src/core/population/demographics.c \
	src/core/population/population_manager.c \
	src/core/population/population_vitality.c \
	src/core/population/race_system.c \
	src/core/economy/market.c \
	src/core/economy/currency_system.c \
	src/core/economy/trade_system.c \
	src/core/economy/resource_market.c \
	src/core/technology/innovation_system.c \
	src/core/military/units.c \
	src/core/military/combat.c \
	src/core/military/conquest.c \
	src/core/diplomacy/relations.c \
	src/core/diplomacy/international_organizations.c \
	src/core/diplomacy/unification_engine.c \
	src/core/events/event_manager.c \
	src/core/events/story_events.c \
	src/core/events/game_events.c \
	src/core/world/dynamic_borders.c \
	src/core/world/map_generator.c \
	src/core/world/map_view.c \
	src/core/world/territory.c \
	src/core/world/settlement_manager.c \
	src/core/world/wonders.c \
	src/core/governance/government.c \
	src/core/governance/custom_governance.c \
	src/core/governance/interaction.c \
	src/core/governance/conversation.c \
	src/core/governance/ministry.c \
	src/core/governance/notebook.c \
	src/core/governance/corruption.c \
	src/core/governance/societal_metrics.c \
	src/core/governance/rule_system.c \
	src/core/governance/governance_evolution.c \
	src/core/governance/legal_status.c \
	src/core/governance/legislative_system.c \
	src/core/governance/institution.c \
	src/core/governance/subdivision.c \
	src/core/environment/geography.c \
	src/core/environment/disaster_system.c \
	src/core/abstracts/soft_metrics.c \
	src/core/culture/cultural_identity.c \
	src/core/culture/writing_system.c \
	src/core/culture/cultural_diffusion.c \
	src/core/culture/cultural_assimilation.c \
	src/core/culture/language_evolution.c \
	src/core/culture/culture.c \
	src/core/culture/religion_system.c \
	src/core/visualization/cultural_display.c \
	src/core/ai/base_ai.c \
	src/core/ai/strategic_ai.c \
	src/core/ai/tactical_ai.c \
	src/core/ai/ai_system.c \
	src/core/politics/faction_system.c \
	src/core/politics/politics.c \
	src/core/politics/political_rivalry.c \
	src/core/subunits/subunit.c \
	src/core/knowledge_system.c

# Utils sources
UTILS_SRCS = \
	src/utils/common.c \
	src/utils/types.c \
	src/utils/memory_pool.c \
	src/utils/config.c \
	src/utils/cache.c \
	src/utils/noise.c

# Visuals
VISUAL_SRCS = \
	src/core/visuals/vexillology.c

# All sources
SRCS = $(ENGINE_SRCS) $(UI_SRCS) $(CORE_SRCS) $(UTILS_SRCS) $(VISUAL_SRCS)

# Object files
OBJS = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Default target: release build
.PHONY: all release debug clean help

all: release

release: CFLAGS += $(RELEASE_FLAGS)
release: $(TARGET)
	@echo ""
	@echo "========================================"
	@echo "  DOMINION - SDL3 Build Complete!"
	@echo "========================================"
	@echo "  Run: ./build/dominion"
	@echo "========================================"
	@echo ""

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)
	@echo ""
	@echo "========================================"
	@echo "  DOMINION - Debug Build Complete!"
	@echo "========================================"
	@echo "  Run: ./build/dominion"
	@echo "========================================"
	@echo ""

# Link
$(TARGET): $(OBJS)
	@echo ""
	@echo "=== Linking executable ==="
	@mkdir -p "$(BUILD_DIR)"
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
	@echo ""

# Compile source files
$(OBJ_DIR)/%.o: src/%.c
	@echo "Compiling $<..."
	@mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	@echo "Cleaning build files..."
	@rm -rf "$(BUILD_DIR)"
	@echo "Clean complete."

# Display help
help:
	@echo "Makefile for DOMINION SDL3 - Linux"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build release version (default)"
	@echo "  release  - Build optimized release version"
	@echo "  debug    - Build with debug symbols"
	@echo "  clean    - Remove all build files"
	@echo "  help     - Display this help message"
	@echo ""
	@echo "Source files: $(words $(SRCS)) files"
	@echo ""
