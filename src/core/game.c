#include "../../include/core/game.h"
#include "../../include/core/ai/ai_system.h"
#include "../../include/core/culture/culture.h"
#include "../../include/core/data/history_db.h"
#include "../../include/core/diplomacy/relations.h"
#include "../../include/core/military/combat.h"
#include "../../include/core/technology/innovation_system.h"
#include "../../include/utils/config.h"
#include "../../include/utils/memory_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_journal_t *g_journal = NULL;

/* Helper helper for result */
static civ_result_t ok_result(void) {
  civ_result_t res = {CIV_OK, "Success"};
  return res;
}

static civ_result_t error_result(civ_error_t code, const char *msg) {
  civ_result_t res;
  res.error = code;
  res.message = msg;
  return res;
}

/* Helper macros for safe system destruction */
#define SAFE_DESTROY(obj, destroy_fn)                                          \
  if (obj) {                                                                   \
    destroy_fn(obj);                                                           \
    obj = NULL;                                                                \
  }

civ_game_t *civ_game_create(void) {
  civ_game_t *game = (civ_game_t *)malloc(sizeof(civ_game_t));
  if (!game) {
    return NULL;
  }
  memset(game, 0, sizeof(civ_game_t));
  game->state = CIV_GAME_STATE_INITIALIZING;
  return game;
}

void civ_game_destroy(civ_game_t *game) {
  if (!game)
    return;

  civ_game_shutdown(game);

  free(game);
}

civ_result_t civ_game_initialize(civ_game_t *game,
                                 const civ_game_config_t *config) {
  if (!game || !config)
    return error_result(CIV_ERROR_INVALID_ARGUMENT, "Invalid arguments");

  // Copy config
  memcpy(&game->config, config, sizeof(civ_game_config_t));

  // Initialize Memory Pool
  game->memory_pool = civ_memory_pool_manager_create(1024, 100);

  // Initialize Time Manager
  game->time_manager = civ_time_manager_create();
  if (game->config.start_year != 0) {
    // Assume time manager has a way to set year, or defaults
  }

  // Initialize State Persistence
  game->persistence = civ_state_persistence_create("saves");

  // Initialize Event Manager
  game->event_manager = civ_event_manager_create();

  // Initialize World Map with random seed
  uint32_t seed = (uint32_t)time(NULL);
  game->world_map =
      civ_map_create(CIV_DEFAULT_MAP_WIDTH, CIV_DEFAULT_MAP_HEIGHT, seed);
  if (game->world_map) {
    civ_map_generate_terrain(game->world_map);
  }

  // Initialize Systems
  game->population_manager = civ_population_manager_create();
  game->market_economy = civ_market_dynamics_create();
  game->technology_tree = civ_innovation_system_create();
  if (game->technology_tree) {
    civ_innovation_system_populate_default_tree(game->technology_tree);
  }
  game->military_system = civ_combat_system_create();
  game->unit_manager = civ_unit_manager_create();
  game->diplomacy_system = civ_diplomacy_system_create();
  game->culture_system = civ_culture_system_create();
  game->ai_system = civ_ai_system_create();
  if (game->ai_system) {
    game->ai_system->game_ptr = game;
  }
  game->settlement_manager = civ_settlement_manager_create();
  game->wonder_manager = civ_wonder_manager_create();
  game->government = civ_government_create("Initial Government");

  /* Initialize Nations and Diplomacy */
  const char *initial_nations[] = {"PLAYER", "RIVAL"};
  civ_diplomacy_system_initialize_relations(game->diplomacy_system,
                                            initial_nations, 2);

  /* Spawn Rival Base */
  civ_settlement_t rival_base;
  memset(&rival_base, 0, sizeof(civ_settlement_t));
  strcpy(rival_base.id, "rival_capital");
  strcpy(rival_base.name, "Rival Kingdom");
  rival_base.x = 30.0f;
  rival_base.y = 30.0f;
  rival_base.population = 800;
  rival_base.attractiveness = 0.8f;
  rival_base.tier = CIV_SETTLEMENT_HAMLET;
  rival_base.culture_yield = 1.0f;
  rival_base.accumulated_culture = 0.0f;
  rival_base.territory_radius = 2;
  civ_settlement_manager_add(game->settlement_manager, &rival_base);

  /* Add Strategic AI for Rival */
  civ_strategic_ai_t *rival_ai = civ_strategic_ai_create("RIVAL", "Rival King");
  civ_ai_system_add_strategic(game->ai_system, rival_ai);

  game->state = CIV_GAME_STATE_RUNNING;
  game->is_running = true;
  game->is_paused = false;
  game->current_turn = 1;

  printf("[GAME] Initialized at turn %d\n", game->current_turn);

  return ok_result();
}

civ_result_t civ_game_end_turn(civ_game_t *game) {
  if (!game)
    return error_result(CIV_ERROR_INVALID_ARGUMENT, "Invalid game");

  game->current_turn++;
  printf("[GAME] Advanced to turn %d\n", game->current_turn);

  // Reset unit movement
  if (game->unit_manager && game->unit_manager->units) {
    for (size_t i = 0; i < game->unit_manager->unit_count; i++) {
      civ_unit_t *u = &game->unit_manager->units[i];
      u->has_moved = false;

      /* Simple Barbarian AI: Move randomly if it's a Barbarian unit */
      if (strstr(u->name, "Barbarians")) {
        int dx = (rand() % 3) - 1; /* -1, 0, 1 */
        int dy = (rand() % 3) - 1;

        int32_t nx =
            (u->x + dx + game->world_map->width) % game->world_map->width;
        int32_t ny = u->y + dy;

        if (ny >= 0 && ny < game->world_map->height) {
          /* Check if tile is occupied */
          bool occupied = false;
          for (size_t j = 0; j < game->unit_manager->unit_count; j++) {
            if (i != j && game->unit_manager->units[j].x == nx &&
                game->unit_manager->units[j].y == ny) {
              occupied = true;
              break;
            }
          }
          if (!occupied) {
            u->x = nx;
            u->y = ny;
          }
        }
      }
    }
  }

  // Trigger settlement growth and calculate science
  if (game->settlement_manager) {
    civ_settlement_manager_update(game->settlement_manager, game->world_map,
                                  game->government, 1.0f);
  }

  if (game->government) {
    civ_government_update(game->government, 1.0f);
  }

  /* Global Science Production: 1 base + 1 per 1000 population */
  int64_t total_pop = 0;
  for (size_t i = 0; i < game->settlement_manager->settlement_count; i++) {
    total_pop += game->settlement_manager->settlements[i].population;
  }

  /* Calculate Trade Bonuses */
  float trade_bonus = 0.0f;
  if (game->diplomacy_system) {
    for (size_t t = 0; t < game->diplomacy_system->treaty_count; t++) {
      civ_treaty_t *treaty = &game->diplomacy_system->treaties[t];
      if (treaty->active &&
          treaty->treaty_type == CIV_TREATY_TYPE_TRADE_AGREEMENT) {
        trade_bonus += 1.0f; /* +1 Science per active agreement */
      }
    }
  }

  /* Calculate Wonder Bonuses */
  civ_wonder_effects_t wonder_bonuses = {0};
  if (game->wonder_manager) {
    wonder_bonuses =
        civ_wonder_calculate_global_bonuses(game->wonder_manager, "PLAYER");
  }

  float base_science = 1.0f + (float)(total_pop / 1000);
  float science_per_turn =
      (base_science + trade_bonus) * (1.0f + wonder_bonuses.science_mult);

  if (game->technology_tree) {
    civ_innovation_system_set_research_budget(game->technology_tree,
                                              science_per_turn);
    civ_innovation_system_update(game->technology_tree, 1.0f);
  }

  /* Process Production Completion */
  for (size_t i = 0; i < game->settlement_manager->settlement_count; i++) {
    civ_settlement_t *s = &game->settlement_manager->settlements[i];

    /* If city is not producing, and is a Rival city, start producing
     * something */
    if (!s->is_producing && strstr(s->id, "rival")) {
      s->is_producing = true;
      if (s->population > 500 && (rand() % 100 < 30)) {
        s->production_type = 7; /* Settler */
        s->production_target = 80.0f;
        printf("[AI] %s started training Settlers\n", s->name);
      } else {
        s->production_type = 0; /* Infantry */
        s->production_target = 30.0f;
        printf("[AI] %s started training Infantry\n", s->name);
      }
      s->production_progress = 0.0f;
    }

    if (s->is_producing && s->production_progress >= s->production_target) {
      printf("[GAME] PRODUCTION COMPLETE IN %s: %d\n", s->name,
             s->production_type);

      /* Spawn the unit */
      if (game->unit_manager) {
        char unit_name[64];
        snprintf(unit_name, sizeof(unit_name), "City Garrison (%s)", s->name);
        civ_unit_manager_spawn_unit(game->unit_manager, s->production_type,
                                    unit_name, 100, (int32_t)s->x,
                                    (int32_t)s->y);
      }

      /* Reset production */
      s->is_producing = false;
      s->production_progress = 0.0f;
      s->production_type = 0; /* CIV_UNIT_TYPE_NONE */
    }

    /* Phase 9: Revolts */
    if (strcmp(s->region_id, "REBELS") == 0 && (rand() % 100 < 20)) {
      if (game->unit_manager) {
        civ_unit_manager_spawn_unit(game->unit_manager, CIV_UNIT_TYPE_INFANTRY,
                                    "Rebel Insurgents", 80, (int32_t)s->x,
                                    (int32_t)s->y);
      }
    }
  }

  // AI System Update
  if (game->ai_system) {
    civ_ai_system_update(game->ai_system, 1.0f);

    /* Trigger Proactive AI Expansion */
    for (size_t i = 0; i < game->ai_system->strategic_count; i++) {
      civ_strategic_ai_process_expansion(game->ai_system->strategic_ais[i],
                                         game);
    }
  }

  return ok_result();
}

/* Phase 11: Stature Ranking Logic */
void civ_game_update_stature_rankings(civ_game_t *game) {
  if (!game || !game->government)
    return;

  civ_government_t *gov = game->government;

  /* Calculate National Capability Index (NCI) */
  /* Factors: Institutional Stature, Stability, Efficiency, and Tech Level */
  civ_float_t institutional_stature = 0.0f;
  if (gov->institution_manager) {
    /* Sum of log-scaled stature from all institutions */
    for (size_t i = 0; i < gov->institution_manager->count; i++) {
      if (gov->institution_manager->items[i].is_active) {
        institutional_stature +=
            log2f(1.0f + gov->institution_manager->items[i].stature);
      }
    }
  }

  civ_float_t nci =
      (institutional_stature * 10.0f) * gov->efficiency * gov->stability;

  /* Simple Tier Mapping (In a multi-nation world, this would be relative to
   * others) */
  if (nci > 500.0f)
    gov->stature_tier = CIV_STATURE_HEGEMON;
  else if (nci > 300.0f)
    gov->stature_tier = CIV_STATURE_GREAT_POWER;
  else if (nci > 150.0f)
    gov->stature_tier = CIV_STATURE_REGIONAL_POWER;
  else if (nci > 75.0f)
    gov->stature_tier = CIV_STATURE_STABLE_STATE;
  else if (nci > 30.0f)
    gov->stature_tier = CIV_STATURE_DEVELOPING_STATE;
  else if (nci > 10.0f)
    gov->stature_tier = CIV_STATURE_FRONTIER_NATION;
  else
    gov->stature_tier = CIV_STATURE_FAILED_STATE;
}

void civ_game_run(civ_game_t *game) {
  if (!game)
    return;
  game->is_running = true;
  while (game->is_running) {
    civ_game_update(game);
    // Sleep or limit frame rate handled by caller or platform layer usually
  }
}

void civ_game_update(civ_game_t *game) {
  if (!game || !game->is_running || game->is_paused)
    return;

  // Update Time
  if (game->time_manager) {
    civ_time_manager_update(game->time_manager);
  }

  // Update Systems
  // if (game->system_orchestrator) ...

  // Simple update loop for now
  if (game->population_manager)
    civ_population_manager_update(game->population_manager, 1.0f, NULL);
  if (game->market_economy)
    civ_market_dynamics_update(game->market_economy, 1.0f, NULL, NULL, 0.0f);
  if (game->technology_tree)
    civ_innovation_system_update(game->technology_tree, 1.0f);

  // New Evolutionary Systems
  civ_game_update_stature_rankings(game);

  // Remaining updates
  if (game->diplomacy_system)
    civ_diplomacy_system_update_relations(game->diplomacy_system, 0);
  if (game->culture_system)
    civ_culture_system_update(game->culture_system, 1.0f);

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
  if (!game)
    return;

  game->is_running = false;
  game->state = CIV_GAME_STATE_SHUTTING_DOWN;

  // Destroy systems in reverse order of dependency
  if (game->world_map)
    civ_map_destroy(game->world_map);
  // if (game->event_manager) civ_event_manager_destroy(game->event_manager); //
  // event_manager free might differ
  if (game->time_manager)
    civ_time_manager_destroy(game->time_manager);
  if (game->memory_pool)
    civ_memory_pool_manager_destroy(game->memory_pool);

  if (game->population_manager)
    civ_population_manager_destroy(game->population_manager);
  if (game->market_economy)
    civ_market_dynamics_destroy(game->market_economy);
  if (game->technology_tree)
    civ_innovation_system_destroy(game->technology_tree);
  if (game->military_system)
    civ_combat_system_destroy(game->military_system);
  if (game->diplomacy_system)
    civ_diplomacy_system_destroy(game->diplomacy_system);
  if (game->culture_system)
    civ_culture_system_destroy(game->culture_system);
  if (game->settlement_manager)
    civ_settlement_manager_destroy(game->settlement_manager);
  if (game->wonder_manager)
    civ_wonder_manager_destroy(game->wonder_manager);
  if (game->persistence)
    civ_state_persistence_destroy(game->persistence);
  // ... destroy others ...
}

#define CIV_SAVE_MAGIC 0x43495653 // "CIVS"
#define CIV_SAVE_VERSION 1

typedef struct {
  uint32_t magic;
  uint32_t version;
  civ_game_config_t config;
  uint32_t map_width;
  uint32_t map_height;
  uint32_t map_seed;
  float sea_level;
} civ_save_header_t;

civ_result_t civ_game_save_state(civ_game_t *game, const char *filename) {
  if (!game || !game->persistence)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT,
                          "Invalid game or persistence"};

  // 1. Prepare Header
  civ_save_header_t header;
  memset(&header, 0, sizeof(header));
  header.magic = CIV_SAVE_MAGIC;
  header.version = CIV_SAVE_VERSION;
  header.config = game->config;
  header.map_width = game->world_map ? game->world_map->width : 0;
  header.map_height = game->world_map ? game->world_map->height : 0;
  header.map_seed = game->world_map ? game->world_map->seed : 0;
  header.sea_level = game->world_map ? game->world_map->sea_level : 0.0f;

  // 2. Serialize Map Data (Tiles)
  size_t map_size = 0;
  void *map_data = NULL;
  if (game->world_map) {
    map_size = header.map_width * header.map_height * sizeof(civ_map_tile_t);
    map_data = game->world_map->tiles;
  }

  // 3. Combine into one buffer
  size_t total_size = sizeof(header) + map_size;
  uint8_t *buffer = (uint8_t *)CIV_MALLOC(total_size);
  if (!buffer)
    return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "Buffer allocation failed"};

  memcpy(buffer, &header, sizeof(header));
  if (map_data) {
    memcpy(buffer + sizeof(header), map_data, map_size);
  }

  // 4. Save
  civ_result_t res = civ_state_persistence_save(game->persistence, filename,
                                                buffer, total_size);
  CIV_FREE(buffer);
  return res;
}

civ_result_t civ_game_load_state(civ_game_t *game, const char *filename) {
  if (!game || !game->persistence)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT,
                          "Invalid game or persistence"};

  // 1. Load File Data
  // Use a large buffer for the expanded world scale (2048x1024)
  size_t buffer_cap = 256 * 1024 * 1024; // 256MB
  uint8_t *buffer = (uint8_t *)CIV_MALLOC(buffer_cap);
  if (!buffer)
    return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "Buffer allocation failed"};

  size_t data_size = buffer_cap;
  civ_result_t res = civ_state_persistence_load(game->persistence, filename,
                                                buffer, &data_size);
  if (res.error != CIV_OK) {
    CIV_FREE(buffer);
    return res;
  }

  // 2. Parse Header
  if (data_size < sizeof(civ_save_header_t)) {
    CIV_FREE(buffer);
    return (civ_result_t){CIV_ERROR_INVALID_DATA, "File too small"};
  }

  civ_save_header_t *header = (civ_save_header_t *)buffer;
  if (header->magic != CIV_SAVE_MAGIC) {
    CIV_FREE(buffer);
    return (civ_result_t){CIV_ERROR_INVALID_DATA, "Invalid file format"};
  }

  // 3. Restore Config
  game->config = header->config;

  // 4. Restore Map
  if (header->map_width > 0 && header->map_height > 0) {
    if (game->world_map)
      civ_map_destroy(game->world_map);

    game->world_map =
        civ_map_create(header->map_width, header->map_height, header->map_seed);
    if (game->world_map) {
      game->world_map->sea_level = header->sea_level;
      size_t map_byte_size =
          header->map_width * header->map_height * sizeof(civ_map_tile_t);
      if (sizeof(civ_save_header_t) + map_byte_size <= data_size) {
        memcpy(game->world_map->tiles, buffer + sizeof(civ_save_header_t),
               map_byte_size);
      }
    }
  }

  CIV_FREE(buffer);
  return (civ_result_t){CIV_OK, "Game loaded successfully"};
}

/* Wrappers */
civ_result_t civ_game_save(civ_game_t *game, const char *filename) {
  return civ_game_save_state(game, filename);
}

civ_result_t civ_game_load(civ_game_t *game, const char *filename) {
  return civ_game_load_state(game, filename);
}

void civ_game_get_default_config(civ_game_config_t *config) {
  if (!config)
    return;
  memset(config, 0, sizeof(civ_game_config_t));
  snprintf(config->name, sizeof(config->name), "New Civilization");
  snprintf(config->version, sizeof(config->version), "0.1.0");
  config->start_year = -4000;
  config->difficulty = 1;
  config->enable_fuzzy_logic = true;
  config->enable_dependency_tracking = true;
}

void civ_game_add_event(civ_game_t *game, const char *type,
                        const char *description, civ_float_t importance) {
  if (!game || !game->event_manager)
    return;
  // Dispatch to event manager
  // civ_event_manager_emit(...)
}
