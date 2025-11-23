/**
 * @file game.h
 * @brief Main game controller for the Civilization simulation
 */

#ifndef CIVILIZATION_GAME_H
#define CIVILIZATION_GAME_H

#include "../common.h"
#include "../types.h"
#include "../utils/cache.h"
#include "../utils/config.h"
#include "../utils/memory_pool.h"
#include "abstracts/soft_metrics.h"
#include "ai/ai_system.h"
#include "culture/culture.h"
#include "culture/ideology_system.h"
#include "diplomacy/international_organizations.h"
#include "diplomacy/relations.h"
#include "economy/currency_system.h"
#include "economy/market.h"
#include "economy/trade_system.h"
#include "environment/disaster_system.h"
#include "environment/geography.h"
#include "events/event_manager.h"
#include "governance/custom_governance.h"
#include "governance/government.h"
#include "governance/legislative_system.h"
#include "military/combat.h"
#include "military/conquest.h"
#include "military/units.h"
#include "politics/politics.h"
#include "population/population_manager.h"
#include "simulation_engine/performance_optimizer.h"
#include "simulation_engine/system_orchestrator.h"
#include "simulation_engine/time_manager.h"
#include "subunits/subunit.h"
#include "technology/innovation_system.h"
#include "visualization/cultural_display.h"
#include "world/dynamic_borders.h"
#include "world/map_generator.h"
#include "world/map_view.h"
#include "world/settlement_manager.h"
#include "world/territory.h"

/* Game state enumeration */
typedef enum {
  CIV_GAME_STATE_INITIALIZING = 0,
  CIV_GAME_STATE_RUNNING = 1,
  CIV_GAME_STATE_PAUSED = 2,
  CIV_GAME_STATE_SAVING = 3,
  CIV_GAME_STATE_LOADING = 4,
  CIV_GAME_STATE_SHUTTING_DOWN = 5
} civ_game_state_t;

/* Game configuration structure */
typedef struct {
  char name[STRING_MAX_LEN];
  char version[STRING_SHORT_LEN];
  int32_t start_year;
  int32_t difficulty;
  bool enable_fuzzy_logic;
  bool enable_dependency_tracking;
  int32_t autosave_interval;
  int32_t max_event_log;
} civ_game_config_t;

/* Performance metrics */
typedef struct {
  uint64_t update_count;
  civ_float_t avg_update_time;
  civ_float_t last_update_time;
} civ_performance_metrics_t;

/* Game event structure */
/* Game event structure */
/* Defined in events/event_manager.h, but we need it here if not included
   properly or if circular dependency. Actually, event_manager.h usually defines
   civ_event_t, not civ_game_event_t. Let's check if civ_game_event_t is used or
   if it should be civ_event_t. Looking at game.c, it uses civ_game_event_t. If
   it's redefined, it means it's defined elsewhere. Let's assume it's NOT
   defined elsewhere for now, or if it is, we should use that. However, the
   error said "Typedef redefinition". Let's check event_manager.h to see if it's
   defined there. For now, I will comment it out if it's likely defined in
   event_manager.h, BUT I haven't checked event_manager.h yet. Wait, the error
   said "Typedef redefinition with different types". This implies it IS defined
   elsewhere. I will remove this definition and rely on the include, OR rename
   it if it's a specific game event struct. Let's rename it to avoid collision
   if it's local to game.h, but game.c uses it. Let's try to find where else it
   is defined. Actually, I'll just rename it to `civ_game_log_entry_t` to be
   safe and distinct.
*/
typedef struct {
  char type[STRING_SHORT_LEN];
  civ_timestamp_t timestamp;
  char description[STRING_MAX_LEN];
  civ_float_t importance;
} civ_game_log_entry_t;

/* Game structure */
typedef struct civ_game {
  civ_game_state_t state;
  civ_time_manager_t *time_manager;
  civ_game_config_t config;
  civ_performance_metrics_t performance;

  /* Event log */
  /* Event log */
  civ_game_log_entry_t *event_log;
  size_t event_log_size;
  size_t event_log_capacity;

  /* Core systems */
  civ_population_manager_t *population_manager;
  civ_market_dynamics_t *market_economy;
  civ_innovation_system_t *technology_tree;
  civ_combat_system_t *military_system;
  civ_unit_manager_t *unit_manager;
  civ_diplomacy_system_t *diplomacy_system;
  civ_soft_metrics_manager_t *soft_metrics;
  civ_event_manager_t *event_manager;
  civ_dynamic_borders_t *dynamic_borders;
  civ_government_t *government;
  civ_geography_t *geography;
  civ_culture_system_t *culture_system;
  civ_ai_system_t *ai_system;
  civ_map_t *world_map;
  civ_map_view_manager_t *map_view_manager;
  civ_territory_manager_t *territory_manager;
  civ_custom_governance_manager_t *custom_governance_manager;
  civ_conquest_system_t *conquest_system;
  civ_cultural_display_t *cultural_display;
  civ_politics_system_t *politics_system;
  civ_subunit_manager_t *subunit_manager;
  civ_ideology_system_t *ideology_system;
  civ_org_manager_t *international_orgs;
  civ_settlement_manager_t *settlement_manager;
  civ_legislative_manager_t *legislative_manager;
  civ_currency_manager_t *currency_manager;
  civ_trade_manager_t *trade_manager;
  civ_disaster_manager_t *disaster_manager;

  /* Performance and modularity systems */
  civ_system_orchestrator_t *system_orchestrator;
  civ_performance_optimizer_t *performance_optimizer;
  civ_config_manager_t *config_manager;
  civ_cache_t *cache;
  civ_memory_pool_manager_t *memory_pool;

  /* Nations */
  char **nations;
  size_t nation_count;
  size_t nation_capacity;

  bool is_running;
  bool is_paused;
} civ_game_t;

/* Function declarations */

/**
 * Create a new game instance
 */
civ_game_t *civ_game_create(void);

/**
 * Destroy a game instance
 */
void civ_game_destroy(civ_game_t *game);

/**
 * Initialize the game with configuration
 */
civ_result_t civ_game_initialize(civ_game_t *game,
                                 const civ_game_config_t *config);

/**
 * Run the main game loop
 */
void civ_game_run(civ_game_t *game);

/**
 * Update the game (call each frame)
 */
void civ_game_update(civ_game_t *game);

/**
 * Pause the game
 */
void civ_game_pause(civ_game_t *game);

/**
 * Resume the game
 */
void civ_game_resume(civ_game_t *game);

/**
 * Shutdown the game
 */
void civ_game_shutdown(civ_game_t *game);

/**
 * Save game to file
 */
civ_result_t civ_game_save(civ_game_t *game, const char *filename);

/**
 * Load game from file
 */
civ_result_t civ_game_load(civ_game_t *game, const char *filename);

/**
 * Get default game configuration
 */
void civ_game_get_default_config(civ_game_config_t *config);

/**
 * Add event to event log
 */
void civ_game_add_event(civ_game_t *game, const char *type,
                        const char *description, civ_float_t importance);

#endif /* CIVILIZATION_GAME_H */
