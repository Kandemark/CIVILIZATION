# Makefile for Civilization C project

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g
LDFLAGS = 
INCLUDES = -Iinclude
SRCDIR = src
OBJDIR = build/obj
BINDIR = build/bin

# Source files
CORE_SOURCES = src/core/game.c \
               src/core/simulation_engine/time_manager.c \
               src/core/simulation_engine/system_orchestrator.c \
               src/core/simulation_engine/performance_optimizer.c \
               src/core/simulation_engine/event_dispatcher.c \
               src/core/simulation_engine/state_persistence.c \
               src/core/population/demographics.c \
               src/core/population/population_manager.c \
               src/core/economy/market.c \
               src/core/technology/innovation_system.c \
               src/core/military/units.c \
               src/core/military/combat.c \
               src/core/diplomacy/relations.c \
               src/core/events/event_manager.c \
               src/core/world/dynamic_borders.c \
               src/core/governance/government.c \
               src/core/environment/geography.c \
               src/core/abstracts/soft_metrics.c \
               src/core/culture/cultural_identity.c \
               src/core/culture/cultural_diffusion.c \
               src/core/culture/cultural_assimilation.c \
               src/core/culture/language_evolution.c \
               src/core/culture/culture.c \
               src/core/world/map_generator.c \
               src/core/world/map_view.c \
               src/core/world/territory.c \
               src/core/culture/writing_system.c \
               src/core/governance/custom_governance.c \
               src/core/military/conquest.c \
               src/core/visualization/cultural_display.c \
               src/core/ai/base_ai.c \
               src/core/ai/strategic_ai.c \
               src/core/ai/tactical_ai.c \
               src/core/ai/ai_system.c \
               src/core/politics/faction_system.c \
               src/core/politics/politics.c \
               src/core/subunits/subunit.c
UTILS_SOURCES = src/utils/common.c \
               src/utils/types.c \
               src/utils/memory_pool.c \
               src/utils/config.c \
               src/utils/cache.c
MAIN_SOURCES = src/main.c

ALL_SOURCES = $(CORE_SOURCES) $(UTILS_SOURCES) $(MAIN_SOURCES)
OBJECTS = $(ALL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Target executable
TARGET = $(BINDIR)/civilization

# Default target
all: $(TARGET)

# Create directories
$(OBJDIR):
	mkdir -p $(OBJDIR)/core/simulation_engine
	mkdir -p $(OBJDIR)/core/economy
	mkdir -p $(OBJDIR)/core/population
	mkdir -p $(OBJDIR)/core/technology
	mkdir -p $(OBJDIR)/core/military
	mkdir -p $(OBJDIR)/core/diplomacy
	mkdir -p $(OBJDIR)/core/abstracts
	mkdir -p $(OBJDIR)/core/events
	mkdir -p $(OBJDIR)/core/world
	mkdir -p $(OBJDIR)/core/governance
	mkdir -p $(OBJDIR)/core/environment
	mkdir -p $(OBJDIR)/core
	mkdir -p $(OBJDIR)/utils
	mkdir -p $(BINDIR)

# Build object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ -lm
	@echo "Build complete: $(TARGET)"

# Clean build artifacts
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Run the game
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CFLAGS += -DDEBUG -g3
debug: $(TARGET)

# Release build
release: CFLAGS += -O3 -DNDEBUG -flto -march=native -ffast-math -funroll-loops
release: LDFLAGS += -flto
release: clean $(TARGET)

# Install (Unix-like systems)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	rm -f /usr/local/bin/civilization

.PHONY: all clean run debug release install uninstall
