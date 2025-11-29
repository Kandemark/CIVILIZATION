/**
 * @file game.c
 * @brief Implementation of the main game controller
 */

#include "../../include/core/game.h"
#include "../../include/common.h"
#include "../../include/core/military/conquest.h"
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/* Forward declarations */
static void initialize_systems(civ_game_t *game,
                               const civ_game_config_t *config);
static void update_systems(civ_game_t *game, civ_float_t time_delta);

civ_game_t *civ_game_create(void) {
  civ_game_t *game = (civ_game_t *)CIV_MALLOC(sizeof(civ_game_t));
  if (!game) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate game");
    return NULL;
  }

  memset(game, 0, sizeof(civ_game_t));
  game->state = CIV_GAME_STATE_INITIALIZING;
  game->event_log_capacity = 1000;
  game->event_log = (civ_game_log_entry_t *)CIV_CALLOC(
      game->event_log_capacity, sizeof(civ_game_log_entry_t));

  if (!game->event_log) {
    CIV_FREE(game);
    return NULL;
  }

  return game;
}

void civ_game_destroy(civ_game_t *game) {
  if (!game)
    return;

  if (game->currency_manager)
    civ_currency_manager_destroy(game->currency_manager);
  if (game->trade_manager)
    civ_trade_manager_destroy(game->trade_manager);
  if (game->disaster_manager)
    civ_disaster_manager_destroy(game->disaster_manager);
  if (game->population_manager)
    civ_population_manager_destroy(game->population_manager);
  if (game->market_economy)
    civ_market_dynamics_destroy(game->market_economy);
  if (game->technology_tree)
    civ_innovation_system_destroy(game->technology_tree);
  if (game->military_system)
    civ_combat_system_destroy(game->military_system);
  if (game->unit_manager)
    civ_unit_manager_destroy(game->unit_manager);
  if (game->diplomacy_system)
    civ_diplomacy_system_destroy(game->diplomacy_system);
  if (game->soft_metrics)
    civ_soft_metrics_manager_destroy(game->soft_metrics);
  if (game->event_manager)
    civ_event_manager_destroy(game->event_manager);
  if (game->dynamic_borders)
    civ_dynamic_borders_destroy(game->dynamic_borders);
  if (game->government)
    civ_government_destroy(game->government);
  if (game->geography)
    civ_geography_destroy(game->geography);
  if (game->culture_system)
    civ_culture_system_destroy(game->culture_system);
  if (game->ai_system)
    civ_ai_system_destroy(game->ai_system);
  if (game->world_map)
    civ_map_destroy(game->world_map);
  if (game->map_view_manager)
    civ_map_view_manager_destroy(game->map_view_manager);
  if (game->territory_manager)
    civ_territory_manager_destroy(game->territory_manager);
  if (game->custom_governance_manager)
    civ_custom_governance_manager_destroy(game->custom_governance_manager);
  if (game->conquest_system)
    civ_conquest_system_destroy(game->conquest_system);
  if (game->cultural_display)
    civ_cultural_display_destroy(game->cultural_display);
  if (game->politics_system)
    civ_politics_system_destroy(game->politics_system);
  if (game->subunit_manager)
    civ_subunit_manager_destroy(game->subunit_manager);
  if (game->ideology_system)
    civ_ideology_system_destroy(game->ideology_system);
  if (game->international_orgs)
    civ_org_manager_destroy(game->international_orgs);
  if (game->settlement_manager)
    civ_settlement_manager_destroy(game->settlement_manager);
  if (game->legislative_manager)
    civ_legislative_manager_destroy(game->legislative_manager);
  if (game->system_orchestrator)
    civ_system_orchestrator_destroy(game->system_orchestrator);
  if (game->performance_optimizer)
    civ_performance_optimizer_destroy(game->performance_optimizer);
  if (game->config_manager)
    civ_config_manager_destroy(game->config_manager);
  if (game->cache)
    civ_cache_destroy(game->cache);
  if (game->memory_pool)
    civ_memory_pool_manager_destroy(game->memory_pool);
  if (game->time_manager)
    civ_time_manager_destroy(game->time_manager);

  if (game->nations) {
    for (size_t i = 0; i < game->nation_count; i++) {
      CIV_FREE(game->nations[i]);
    }
    CIV_FREE(game->nations);
  }

  if (game->event_log)
    CIV_FREE(game->event_log);
  CIV_FREE(game);
}

civ_result_t civ_game_initialize(civ_game_t *game,
                                 const civ_game_config_t *config) {
  if (!game || !config)
    return (civ_result_t){.error = CIV_ERROR_INVALID_ARGUMENT,
                          .message = "Invalid arguments"};

  game->config = *config;
  initialize_systems(game, config);
  game->state = CIV_GAME_STATE_RUNNING;
  game->is_running = true;
  game->is_paused = false;

  return (civ_result_t){.error = CIV_OK, .message = "Initialized"};
}

void civ_game_run(civ_game_t *game) {
  if (!game)
    return;
  while (game->is_running) {
    civ_game_update(game);
#ifdef _WIN32
    Sleep(100);
#else
    usleep(100000);
#endif
  }
}

void civ_game_update(civ_game_t *game) {
  if (!game || game->is_paused)
    return;

  civ_float_t time_delta = 1.0f; // Simplified
  if (game->time_manager) {
    time_delta = civ_time_manager_update(game->time_manager);
  }
  update_systems(game, time_delta);

  game->performance.update_count++;
}

void civ_game_pause(civ_game_t *game) {
  if (game)
    game->is_paused = true;
}

void civ_game_resume(civ_game_t *game) {
  if (game)
    game->is_paused = false;
}

void civ_game_shutdown(civ_game_t *game) {
  if (game)
    game->is_running = false;
}

void civ_game_get_default_config(civ_game_config_t *config) {
  if (!config)
    return;
  memset(config, 0, sizeof(civ_game_config_t));
  strcpy(config->name, "Civilization");
  config->start_year = -4000;
  config->difficulty = 1;
}

void civ_game_add_event(civ_game_t *game, const char *type,
                        const char *description, civ_float_t importance) {
  if (!game || !game->event_log)
    return;

  if (game->event_log_size < game->event_log_capacity) {
    civ_game_log_entry_t *entry = &game->event_log[game->event_log_size++];
    strncpy(entry->type, type, sizeof(entry->type) - 1);
    strncpy(entry->description, description, sizeof(entry->description) - 1);
    entry->importance = importance;
    entry->timestamp = (civ_timestamp_t){0}; // Placeholder
  }
}

/* Internal helpers */
static void initialize_systems(civ_game_t *game,
                               const civ_game_config_t *config) {
  game->time_manager = civ_time_manager_create();
  game->currency_manager = civ_currency_manager_create();
  game->trade_manager = civ_trade_manager_create(game->currency_manager);

  civ_coordinate_t sw = {0, 0}, ne = {100, 100};
  game->geography = civ_geography_create("World", sw, ne);
  game->disaster_manager = civ_disaster_manager_create(game->geography);

  // Initialize other systems as needed (simplified for GUI demo)
  game->population_manager = civ_population_manager_create();
}

static void update_systems(civ_game_t *game, civ_float_t time_delta) {
  if (game->currency_manager)
    civ_currency_update_rates(game->currency_manager, time_delta);
  if (game->trade_manager)
    civ_trade_update(game->trade_manager, time_delta);
  if (game->disaster_manager)
    civ_disaster_update(game->disaster_manager, time_delta);
}
