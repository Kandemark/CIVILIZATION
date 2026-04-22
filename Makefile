# Makefile for DOMINION SDL3 (Linux) - FULL GAME BUILD

# Compiler and flags
CC = gcc
SDL3_CFLAGS = $(shell pkg-config --cflags sdl3 sdl3-ttf 2>/dev/null || echo "-I/usr/include/SDL3 -I/usr/include/SDL3_ttf")
SDL3_LIBS = $(shell pkg-config --libs sdl3 sdl3-ttf 2>/dev/null || echo "-lSDL3 -lSDL3_ttf")
CFLAGS = -std=c11 -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L -Iinclude $(SDL3_CFLAGS) -Wall -Wno-unused-variable -Wno-unused-function
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

# Display engine sources
DISPLAY_SRCS = \
	src/display/layer.c \
	src/display/camera.c \
	src/display/theme.c \
	src/display/animation.c \
	src/display/draw_list.c \
	src/display/debug_overlay.c

# Widget library sources
WIDGET_SRCS = \
	src/ui/widget/widget_base.c \
	src/ui/widget/button.c \
	src/ui/widget/panel.c \
	src/ui/widget/progress_bar.c \
	src/ui/window_mgr.c \
	src/ui/widget/drawer.c \
	src/ui/widget/tooltip.c \
	src/ui/widget/scroll_area.c \
	src/ui/widget/label.c \
	src/ui/widget/toggle.c \
	src/ui/widget/slider.c \
	src/ui/widget/dropdown.c \
	src/ui/widget/tab_bar.c \
	src/ui/widget/modal.c \
	src/ui/confirm_dialog.c \
	src/ui/nuklear_impl.c

# Layout engine sources
LAYOUT_SRCS = \
	src/ui/layout/layout.c

# Graph rendering sources
GRAPH_SRCS = \
	src/ui/graph/graph.c

# Icon system sources
ICON_SRCS = \
	src/ui/icon/icon_atlas.c

# Screen sources
SCREEN_SRCS = \
	src/ui/screens/screen_dashboard.c \
	src/ui/screens/screen_work.c \
	src/ui/screens/screen_finance.c \
	src/ui/screens/screen_housing.c \
	src/ui/screens/screen_education.c \
	src/ui/screens/screen_network.c \
	src/ui/screens/screen_politics.c \
	src/ui/screens/screen_health.c \
	src/ui/screens/screen_constitution.c \
	src/ui/screens/screen_economy.c

# Panel sources
PANEL_SRCS = \
	src/ui/panel/diplomacy_panel.c \
	src/ui/panel/research_panel.c \
	src/ui/panel/governance_panel.c \
	src/ui/panel/wonders_panel.c \
	src/ui/panel/rulebook_panel.c \
	src/ui/panel/unit_sidebar.c \
	src/ui/panel/settlement_sidebar.c

# UI sources
UI_SRCS = \
	src/ui/sdl3_main.c \
	src/ui/ui_common.c \
	src/ui/app_controller.c \
	src/ui/scene_manager.c \
	src/ui/scenes/scene_splash.c \
	src/ui/scenes/scene_profile_select.c \
	src/ui/scenes/scene_profile_create.c \
	src/ui/scenes/scene_main_menu.c \
	src/ui/scenes/scene_setup.c \
	src/ui/scenes/scene_life_origin.c \
	src/ui/scenes/scene_identity.c \
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
	src/core/economy/macro_economy.c \
	src/core/economy/currency.c \
	src/core/economy/banking.c \
	src/core/economy/taxation.c \
	src/core/economy/budget.c \
	src/core/economy/economic_policy.c \
	src/core/economy/agriculture.c \
	src/core/economy/extraction.c \
	src/core/economy/manufacturing.c \
	src/core/economy/energy.c \
	src/core/economy/financial_markets.c \
	src/core/economy/commodity_markets.c \
	src/core/economy/domestic_trade.c \
	src/core/economy/international_trade.c \
	src/core/economy/labor_market.c \
	src/core/economy/infrastructure.c \
	src/core/economy/housing.c \
	src/core/economy/land_use.c \
	src/core/economy/capital_assets.c \
	src/core/economy/war_economy.c \
	src/core/economy/black_market.c \
	src/core/economy/innovation_economy.c \
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
	src/core/world/real_world_map.c \
	src/core/world/map_view.c \
	src/core/world/territory.c \
	src/core/world/settlement_manager.c \
	src/core/world/wonders.c \
	src/core/world/nation.c \
	src/core/world/nations_data.c \
	src/core/world/political_borders.c \
	src/core/world/resource_map.c \
	src/core/world/cities_data.c \
	src/core/world/flag_system.c \
	src/core/governance/government.c \
	src/core/governance/custom_governance.c \
	src/core/governance/branches/council.c \
	src/core/governance/branches/executive.c \
	src/core/governance/branches/judiciary.c \
	src/core/governance/branches/legislative.c \
	src/core/governance/branches/religious_body.c \
	src/core/governance/legal/constitution.c \
	src/core/governance/legal/legal_status.c \
	src/core/governance/legal/rights.c \
	src/core/governance/institutions/civil_service.c \
	src/core/governance/institutions/institution.c \
	src/core/governance/institutions/ministry.c \
	src/core/governance/territorial/subdivision.c \
	src/core/governance/political/corruption.c \
	src/core/governance/political/elections.c \
	src/core/governance/political/political_violence.c \
	src/core/governance/evolution/governance_evolution.c \
	src/core/governance/metrics/societal_metrics.c \
	src/core/governance/interaction/conversation.c \
	src/core/governance/interaction/interaction.c \
	src/core/governance/interaction/notebook.c \
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
	src/core/culture/ideology_system.c \
	src/core/visualization/cultural_display.c \
	src/core/ai/base_ai.c \
	src/core/ai/strategic_ai.c \
	src/core/ai/tactical_ai.c \
	src/core/ai/ai_system.c \
	src/core/politics/faction_system.c \
	src/core/politics/politics.c \
	src/core/politics/political_rivalry.c \
	src/core/subunits/subunit.c \
	src/core/faction.c \
	src/core/character.c \
	src/core/npc_engine.c \
	src/core/role.c \
	src/core/constitution.c \
	src/core/time_engine.c \
	src/core/knowledge_system.c

# Utils sources
UTILS_SRCS = \
	src/utils/common.c \
	src/utils/types.c \
	src/utils/memory_pool.c \
	src/utils/config.c \
	src/utils/cache.c \
	src/utils/noise.c \
	src/utils/paths.c

# Visuals
VISUAL_SRCS = \
	src/core/visuals/vexillology.c

# All sources
SRCS = $(ENGINE_SRCS) $(DISPLAY_SRCS) $(UI_SRCS) $(WIDGET_SRCS) $(LAYOUT_SRCS) $(GRAPH_SRCS) $(ICON_SRCS) $(SCREEN_SRCS) $(PANEL_SRCS) $(CORE_SRCS) $(UTILS_SRCS) $(VISUAL_SRCS)

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
