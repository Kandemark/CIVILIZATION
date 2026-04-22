#include "core/game.h"
#include "core/ai/ai_system.h"
#include "core/character.h"
#include "core/economy/financial_markets.h"
#include "core/npc_engine.h"
#include "core/culture/culture.h"
#include "core/data/history_db.h"
#include "core/diplomacy/relations.h"
#include "core/military/combat.h"
#include "core/profile.h"
#include "core/time_engine.h"
#include "core/world/cities_data.h"
#include "core/world/flag_system.h"
#include "core/world/map_view.h"
#include "core/world/nation.h"
#include "core/world/nations_data.h"
#include "core/world/political_borders.h"
#include "core/world/real_world_map.h"
#include "core/world/resource_map.h"
#include "core/technology/innovation_system.h"
#include "utils/config.h"
#include "utils/memory_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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

  // Initialize world map — try Earth data first, fall back to procedural atlas
  uint32_t seed = CIV_GLOBAL_MAP_SEED;
  game->world_map =
      civ_map_create(CIV_DEFAULT_MAP_WIDTH, CIV_DEFAULT_MAP_HEIGHT, seed);
  if (game->world_map) {
    if (civ_earth_map_is_valid(CIV_EARTH_MAP_DEFAULT_PATH)) {
      civ_result_t earth_res =
          civ_earth_map_load(CIV_EARTH_MAP_DEFAULT_PATH, game->world_map);
      if (earth_res.error == CIV_OK) {
        printf("[GAME] Earth map loaded from %s\n",
               CIV_EARTH_MAP_DEFAULT_PATH);
      } else {
        printf("[GAME] Earth map load failed: %s — using procedural atlas\n",
               earth_res.message);
        civ_map_generate_terrain(game->world_map);
      }
    } else {
      printf("[GAME] No Earth map found — using procedural atlas\n");
      civ_map_generate_terrain(game->world_map);
    }
  }

  // Initialize Systems
  game->population_manager = civ_population_manager_create();
  game->market_economy = civ_market_dynamics_create();
  game->technology_tree = civ_innovation_system_create();
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
  game->custom_governance_manager = civ_custom_governance_manager_create();

  /* Initialize system orchestrator */
  game->system_orchestrator = civ_system_orchestrator_create();

  /* Load real political borders from Natural Earth data */
  if (civ_political_borders_load("data/earth_borders.bin",
                                  game->world_map->width,
                                  game->world_map->height)) {
    civ_political_borders_apply(game->world_map);
  }

  /* Load master nation index from Natural Earth */
  game->nations_data = civ_nations_data_create();
  if (game->nations_data) {
    civ_nations_data_load(game->nations_data, "data/nations.bin");
    printf("[GAME] Nations data: %u countries loaded\n",
           game->nations_data->count);
  }

  /* Load resource map */
  game->resource_map = civ_resource_map_create(
      game->world_map->width, game->world_map->height);
  if (game->resource_map) {
    civ_resource_map_load(game->resource_map, "data/resources.bin");
    printf("[GAME] Resource map loaded\n");
  }

  /* Initialize nations from borders data + nations_data */
  {
    civ_nation_manager_t *nm = civ_nation_manager_create();
    if (nm) {
      civ_nation_manager_init_from_data(nm, game->nations_data,
          game->world_map->width, game->world_map->height,
          CIV_NATION_DEFAULT_COUNT);
      printf("[GAME] %d nations initialized\n", nm->count);
      game->nation_manager = nm;

      /* Claim territory from borders for pixel-accurate ownership */
      for (int ni = 0; ni < nm->count; ni++) {
        civ_nation_claim_from_borders(&nm->nations[ni], game->world_map,
            ni, game->world_map->width, game->world_map->height);
      }
      printf("[GAME] Territory claimed for %d nations\n", nm->count);
    }
  }

  /* Load flag metadata — textures loaded later when renderer is available */
  game->flag_system = civ_flag_system_create(NULL);
  if (game->flag_system) {
    civ_flag_system_load(game->flag_system, "data/flags/index.bin", "data/flags");
  }

  /* Load cities data */
  game->cities_data = civ_cities_data_create(
      game->world_map->width, game->world_map->height);
  if (game->cities_data) {
    civ_cities_data_load(game->cities_data, "data/cities.bin");
    printf("[GAME] Cities data: %u cities loaded\n",
           game->cities_data->count);
  }

  /* Initialize time engine — global baseline 00 BC, custom calendars */
  {
    civ_time_engine_t *te = civ_time_engine_create();
    if (te) {
      civ_time_engine_init_default_calendars(te);
      game->time_engine = te;
      printf("[GAME] Time engine initialized: Year %d, %d calendars\n",
             te->global.global_year, te->calendar_count);
    }
  }

  /* Initialize NPC engine */
  {
    civ_npc_engine_t *ne = civ_npc_engine_create();
    if (ne) {
      civ_npc_engine_add(ne, "Aleksandr Volkov", "President", "Russia");
      civ_npc_engine_add(ne, "Maria Chen", "Trade Minister", "China");
      civ_npc_engine_add(ne, "James Okafor", "Finance Director", "Nigeria");
      civ_npc_engine_add(ne, "Isabel Rojas", "Foreign Secretary", "Brazil");
      civ_npc_engine_add(ne, "Klaus Weber", "Chancellor", "Germany");
      civ_npc_engine_add(ne, "Yuki Tanaka", "Defense Minister", "Japan");
      civ_npc_engine_add(ne, "Amina Hassan", "Culture Minister", "Egypt");
      civ_npc_engine_add(ne, "Raj Patel", "Prime Minister", "India");
      game->npc_engine = ne;
    }
  }

  /* Initialize player as Private Citizen */
  civ_role_init(&game->player_role);
  civ_role_set(&game->player_role, &civ_role_private_citizen, "none");

  /* Initialize market with all real-world currencies */
  game->market = civ_market_create();
  printf("[GAME] %d global currencies active\n",
         ((civ_market_engine_t *)game->market)->currency_count);
  civ_market_generate_companies((civ_market_engine_t *)game->market, "Kenya");
  civ_market_generate_companies((civ_market_engine_t *)game->market, "Global");
  civ_wallet_init(&game->wallet);

  /* --- Initialize economy modules --- */
  game->banking            = civ_banking_create();
  game->taxation           = civ_taxation_create();
  game->budget             = civ_budget_create();
  game->economic_policy    = civ_economic_policy_create();
  game->agriculture        = civ_agriculture_create();
  game->extraction         = civ_extraction_create();
  game->manufacturing      = civ_manufacturing_create();
  game->energy             = civ_energy_create();
  game->financial_market   = civ_market_create();
  game->commodity_market   = civ_resource_market_create();
  game->domestic_trade     = civ_domestic_trade_create();
  game->labor_market       = civ_labor_market_create();
  game->infrastructure     = civ_infrastructure_create();
  game->housing            = civ_housing_create();
  game->land_use           = civ_land_use_create(10000.0);
  game->capital_assets     = civ_capital_assets_create();
  game->war_economy        = civ_war_economy_create();
  game->black_market       = civ_black_market_create();
  game->innovation_economy = civ_innovation_economy_create();
  printf("[GAME] 22 economy modules initialized\n");

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
  if (game->time_engine)
    civ_time_engine_advance_turn((civ_time_engine_t *)game->time_engine);
  /* NPC decisions */
  if (game->npc_engine && game->time_engine) {
    civ_time_engine_t *te = (civ_time_engine_t *)game->time_engine;
    civ_npc_engine_process_turn((civ_npc_engine_t *)game->npc_engine,
                                game->nation_manager,
                                te->global.global_year, te->global.global_day);
  }
  /* Market fluctuation */
  if (game->market)
    civ_market_update((civ_market_engine_t *)game->market);
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
    /* End-of-turn governance tick */
    int pop = game->population_manager && game->population_manager->demographics
      ? (int)game->population_manager->demographics->total_population : 10000;
    float cult = game->culture_system ? 0.50f : 0.30f;
    float gbudget = game->budget ? game->budget->total_expenditure : 100000.0f;
    float edu   = game->population_manager ? game->population_manager->education_quality : 0.50f;
    float econ  = game->economic_policy ? game->economic_policy->consumer_confidence : 0.50f;
    civ_government_update(game->government, 1.0f, pop, cult, gbudget, edu, econ, edu,
                          game->politics_system ? 0.55f : 0.50f,
                          game->politics_system ? 3 : 2);
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

  /* Phase 0: Time — produces delta for all downstream systems */
  civ_float_t dt = 1.0f;
  if (game->time_manager) {
    dt = civ_time_manager_update(game->time_manager);
  }

  /* Phase 1: Demographics & Economy
   * ================================================================
   * Dependencies flow top-down. Each module reads values produced by
   * modules above it. Circular deps use previous-cycle (lagged) values.
   * ================================================================ */

  /* --- Extract baseline data from core systems --- */
  int64_t   total_pop  = 0;
  civ_float_t tech_lev  = 1.0;
  civ_float_t education = 0.50;
  civ_float_t health    = 0.50;

  if (game->population_manager) {
    civ_population_manager_update(game->population_manager, dt, NULL);
    if (game->population_manager->demographics)
      total_pop = game->population_manager->demographics->total_population;
    education = game->population_manager->education_quality;
    health    = game->population_manager->health_index;
  }
  if (total_pop < 100) total_pop = 100;

  if (game->technology_tree)
    tech_lev = (civ_float_t)game->technology_tree->aggregate_index / 100.0;
  if (tech_lev < 0.1) tech_lev = 0.1;

  /* Governance signals — single source of truth */
  civ_float_t gov_efficiency  = game->government ? game->government->efficiency : 0.50;
  civ_float_t corruption      = game->government
    ? civ_government_get_corruption(game->government) : 0.10f;
  civ_float_t gov_stability   = game->government ? game->government->stability : 0.60f;
  civ_float_t gov_legitimacy  = game->government ? game->government->legitimacy : 0.60f;
  civ_float_t regulation_level = 0.35;
  if (game->economic_policy)
    regulation_level = (civ_float_t)game->economic_policy->regulation * 0.25;

  /* Geography */
  civ_float_t arable_area    = game->geography ? civ_geography_get_agricultural_area(game->geography) : 2000.0;
  civ_float_t geography_size = 10000.0;
  if (game->geography && game->geography->patch_count > 0) {
    geography_size = 0.0;
    for (size_t pi = 0; pi < game->geography->patch_count; pi++)
      geography_size += game->geography->land_patches[pi].area;
  }
  if (geography_size < 100.0) geography_size = 100.0;

  /* Governance cross-module bonuses — read once, applied throughout economy */
  float gov_econ_bonus    = 1.0f;
  float gov_trade_bonus   = 1.0f;
  float gov_research_bonus = 1.0f;
  float gov_military_bonus = 1.0f;
  float gov_cohesion       = 0.50f;
  if (game->government) {
    civ_governance_state_t *ev = &game->government->evolution_state;
    gov_econ_bonus     = (float)civ_governance_economic_bonus(ev);
    gov_trade_bonus    = (float)civ_governance_trade_bonus(ev);
    gov_research_bonus = (float)civ_governance_research_bonus(ev);
    gov_military_bonus = (float)civ_governance_military_bonus(ev);
    gov_cohesion       = (float)civ_governance_cohesion_bonus(ev);
  }

  /* =================================================================
   * Phase 1a: macro_economy (produces GDP, inflation, growth, unemployment)
   * ================================================================= */
  civ_float_t gdp            = 500000.0;
  civ_float_t gdp_per_capita = 50000.0;
  civ_float_t gdp_growth     = 0.02;
  civ_float_t inflation      = 0.02;
  civ_float_t unemployment   = 0.05;

  if (game->market_economy) {
    civ_market_dynamics_update(game->market_economy, dt, game->population_manager,
                               game->geography, tech_lev * gov_econ_bonus);
    civ_economic_report_t rep = civ_market_dynamics_get_report(game->market_economy);
    gdp            = rep.gdp * gov_econ_bonus;
    gdp_per_capita = rep.gdp_per_capita * gov_econ_bonus;
    gdp_growth     = rep.growth_rate * gov_econ_bonus;
    inflation      = rep.inflation_rate;
    unemployment   = rep.unemployment_rate;
  }

  /* =================================================================
   * Phase 1b: labor_market (needs population + GDP + education + confidence)
   * ================================================================= */
  civ_float_t business_conf = game->economic_policy ? game->economic_policy->business_confidence : 0.60;
  civ_float_t avg_wage      = 30000.0;
  int         labor_avail   = (int)(total_pop * 0.4);
  int         labor_employed = 0;

  if (game->labor_market) {
    civ_labor_market_update(game->labor_market, dt, (int)total_pop, gdp, tech_lev,
                            education, business_conf);
    unemployment  = game->labor_market->overall_unemployment;
    avg_wage      = game->labor_market->avg_wage_national;
    labor_avail   = game->labor_market->total_workforce - game->labor_market->total_employed;
    if (labor_avail < 0) labor_avail = 0;
    labor_employed = game->labor_market->total_employed;
  }

  /* =================================================================
   * Phase 1c: economic_policy (needs macro indicators)
   * ================================================================= */
  if (game->economic_policy) {
    civ_economic_policy_update(game->economic_policy, dt, inflation,
                               unemployment, gdp_growth, corruption);
    business_conf = game->economic_policy->business_confidence;
    regulation_level = (civ_float_t)game->economic_policy->regulation * 0.25;
  }

  /* =================================================================
   * Phase 1d: taxation (needs GDP + population + governance)
   * ================================================================= */
  civ_float_t tax_revenue = 50000.0;

  if (game->taxation) {
    civ_taxation_update(game->taxation, dt, gdp, (civ_float_t)total_pop,
                        gov_efficiency, corruption);
    tax_revenue = game->taxation->total_revenue;
  }

  /* =================================================================
   * Phase 1e: budget (needs tax revenue + GDP + population + inflation)
   * ================================================================= */
  civ_float_t budget_infra_allocation = 30000.0;
  civ_float_t gov_spending_ratio      = 0.20;
  civ_float_t debt_interest_rate      = 0.03;
  civ_float_t savings_rate            = 0.15;

  if (game->budget) {
    civ_budget_update(game->budget, dt, tax_revenue, gdp,
                      (civ_float_t)total_pop, inflation);
    gov_spending_ratio   = (gdp > 0) ? game->budget->total_expenditure / gdp : 0.20;
    budget_infra_allocation = civ_budget_spending_for(game->budget, CIV_BUDGET_INFRASTRUCTURE);
    debt_interest_rate   = game->budget->debt_interest_rate;
    savings_rate         = 1.0 - gov_spending_ratio;
    if (savings_rate < 0.05) savings_rate = 0.05;
  }

  /* =================================================================
   * Phase 1f: banking (needs macro + gov spending)
   * ================================================================= */
  if (game->banking) {
    civ_banking_update(game->banking, dt, inflation, gdp_growth,
                       unemployment, gov_spending_ratio);
    debt_interest_rate = game->banking->base_interest_rate; /* banking rate overrides budget rate */
  }

  /* =================================================================
   * Phase 1g: agriculture (needs population + arable land + tech + climate)
   * ================================================================= */
  civ_float_t food_surplus = 0.0;
  if (game->agriculture) {
    civ_agriculture_update(game->agriculture, dt, (civ_float_t)total_pop,
                           arable_area, tech_lev, 1.0);
    food_surplus = game->agriculture->food_surplus;
  }

  /* =================================================================
   * Phase 1h: extraction (needs tech + labor + geography)
   * ================================================================= */
  civ_float_t raw_materials = 100.0;
  if (game->extraction) {
    civ_float_t mineral_richness = 0.70; /* proxy — no direct geography field yet */
    civ_extraction_update(game->extraction, dt, tech_lev,
                          (civ_float_t)labor_avail, mineral_richness);
    raw_materials = game->extraction->total_output;
    if (raw_materials < 1.0) raw_materials = 1.0;
  }

  /* =================================================================
   * Phase 1i: infrastructure (needs budget + population + geography)
   * ================================================================= */
  civ_float_t infra_quality = 0.50;
  civ_float_t infra_supply_chain = 0.50;
  if (game->infrastructure) {
    civ_infrastructure_update(game->infrastructure, dt, budget_infra_allocation,
                              (civ_float_t)total_pop, geography_size);
    infra_quality       = game->infrastructure->overall_quality;
    infra_supply_chain  = game->infrastructure->supply_chain_efficiency;
  }

  /* =================================================================
   * Phase 1j: manufacturing (needs tech + labor + raw materials + infra)
   * ================================================================= */
  civ_float_t industrial_output = 500.0;
  if (game->manufacturing) {
    civ_manufacturing_update(game->manufacturing, dt, tech_lev,
                             (civ_float_t)labor_avail, raw_materials, infra_quality);
    industrial_output = game->manufacturing->total_industrial_output;
    if (industrial_output < 1.0) industrial_output = 1.0;
  }

  /* =================================================================
   * Phase 1k: energy (needs population + industrial output + tech)
   * ================================================================= */
  civ_float_t energy_price = 50.0;
  if (game->energy) {
    civ_energy_update(game->energy, dt, (civ_float_t)total_pop,
                      industrial_output, tech_lev, 0.80);
    energy_price = game->energy->energy_price;
  }

  /* =================================================================
   * Phase 1l: housing (needs population + wages + interest + costs)
   * ================================================================= */
  civ_float_t construction_cost_index = (energy_price / 50.0 + infra_quality > 0)
                                         ? (1.0 + (1.0 - infra_quality)) : 1.5;
  if (game->housing) {
    civ_housing_update(game->housing, dt, (int)total_pop, avg_wage,
                       debt_interest_rate, construction_cost_index);
  }

  /* =================================================================
   * Phase 1m: land_use (needs population + GDP/capita + urbanization)
   * ================================================================= */
  civ_float_t urban_rate = game->housing ? game->housing->urbanization : 0.55;
  if (game->land_use) {
    civ_land_use_update(game->land_use, dt, (int)total_pop, gdp_per_capita, urban_rate);
  }

  /* =================================================================
   * Phase 1n: capital_assets (needs GDP + savings + interest + confidence)
   * ================================================================= */
  if (game->capital_assets) {
    civ_capital_assets_update(game->capital_assets, dt, gdp, savings_rate,
                              debt_interest_rate, business_conf);
  }

  /* =================================================================
   * Phase 1o: domestic_trade (needs infrastructure + population + geography)
   * ================================================================= */
  if (game->domestic_trade) {
    civ_domestic_trade_update(game->domestic_trade, dt, infra_quality,
                              (civ_float_t)total_pop, geography_size);
  }

  /* =================================================================
   * Phase 1p: international_trade
   * ================================================================= */
  if (game->trade_manager)
    civ_trade_update(game->trade_manager, dt);

  /* =================================================================
   * Phase 1q: innovation_economy (needs GDP + education + confidence + capital)
   * ================================================================= */
  civ_float_t capital_avail = game->capital_assets
    ? game->capital_assets->total_capital_stock / (gdp + 1.0) : 0.50;
  if (game->innovation_economy) {
    civ_innovation_economy_update(game->innovation_economy, dt, gdp, education,
                                  business_conf, regulation_level, capital_avail);
  }

  /* =================================================================
   * Phase 1r: black_market (needs GDP + corruption + unemployment + regulation)
   * ================================================================= */
  if (game->black_market) {
    civ_black_market_update(game->black_market, dt, gdp, corruption,
                            unemployment, regulation_level, 10000.0);
  }

  /* =================================================================
   * Phase 1s: war_economy (needs GDP + industrial output + population)
   * ================================================================= */
  bool actively_fighting = false;
  if (game->war_economy) {
    civ_war_economy_update(game->war_economy, dt, gdp, industrial_output,
                           (int)total_pop, actively_fighting);
  }

  /* =================================================================
   * Phase 2: Diplomacy & Governance
   * ================================================================= */
  if (game->diplomacy_system)
    civ_diplomacy_system_update_relations(game->diplomacy_system, 0);

  /* Feed budget allocations into government from economic budget module */
  if (game->government && game->budget) {
    civ_government_set_budget(game->government, CIV_BUDGET_MILITARY,
      civ_budget_spending_for(game->budget, CIV_BUDGET_MILITARY));
    civ_government_set_budget(game->government, CIV_BUDGET_INFRASTRUCTURE,
      civ_budget_spending_for(game->budget, CIV_BUDGET_INFRASTRUCTURE));
    civ_government_set_budget(game->government, CIV_BUDGET_EDUCATION,
      civ_budget_spending_for(game->budget, CIV_BUDGET_EDUCATION));
    civ_government_set_budget(game->government, CIV_BUDGET_HEALTHCARE,
      civ_budget_spending_for(game->budget, CIV_BUDGET_HEALTHCARE));
    civ_government_set_budget(game->government, CIV_BUDGET_WELFARE,
      civ_budget_spending_for(game->budget, CIV_BUDGET_WELFARE));
    civ_government_set_budget(game->government, CIV_BUDGET_RESEARCH,
      civ_budget_spending_for(game->budget, CIV_BUDGET_RESEARCH));
    civ_government_set_budget(game->government, CIV_BUDGET_ADMINISTRATION,
      civ_budget_spending_for(game->budget, CIV_BUDGET_ADMINISTRATION));
    civ_government_set_budget(game->government, CIV_BUDGET_DEBT_SERVICE,
      civ_budget_spending_for(game->budget, CIV_BUDGET_DEBT_SERVICE));
  }

  /* Main governance tick — propagates to ALL 14 subsystems */
  civ_float_t total_gov_budget = game->budget ? game->budget->total_expenditure : 100000.0f;
  civ_float_t culture_level    = game->culture_system ? 0.50f : 0.30f;

  if (game->government) {
    float edu_lvl  = game->population_manager ? game->population_manager->education_quality : 0.50f;
    float econ_conf = game->economic_policy ? game->economic_policy->consumer_confidence : 0.50f;
    float lit_rate  = edu_lvl;
    float fac_sup   = 0.55f;
    int   fac_cnt   = 3;
    civ_government_update(game->government, dt, (int)total_pop,
                          culture_level, total_gov_budget, edu_lvl, econ_conf, lit_rate,
                          fac_sup, fac_cnt);

    /* Refresh corruption/efficiency from governance (updated this frame) */
    corruption     = civ_government_get_corruption(game->government);
    gov_efficiency = game->government->efficiency;
    gov_stability  = game->government->stability;
    gov_legitimacy = game->government->legitimacy;
  }

  /* Custom governance: parallel system — update if nations have custom govs */
  if (game->custom_governance_manager) {
    for (size_t i = 0; i < game->custom_governance_manager->government_count; i++) {
      civ_custom_governance_t *cg = game->custom_governance_manager->governments + i;
      civ_custom_governance_update(cg, dt);
    }
  }

  /* ── Tick ALL nation governments autonomously ── */
  if (game->nation_manager) {
    civ_nation_manager_t *nm = (civ_nation_manager_t *)game->nation_manager;
    for (int ni = 0; ni < nm->count; ni++) {
      civ_nation_t *nation = &nm->nations[ni];
      if (!nation->government) continue;
      /* Skip player nation — already ticked above */
      if (nation->government == game->government) continue;

      /* Each nation gets its own governance tick with autonomous trait evolution */
      civ_government_update(nation->government, dt, (int)total_pop,
                            culture_level, total_gov_budget / (float)(nm->count + 1),
                            education, business_conf, education, 0.55f, 3);
    }
  }

  /* Phase 3: Settlements & Production */
  if (game->settlement_manager)
    civ_settlement_manager_update(game->settlement_manager, game->world_map,
                                  game->government, dt);

  /* Phase 4: Technology */
  if (game->technology_tree)
    civ_innovation_system_update(game->technology_tree, dt * gov_research_bonus);

  /* Phase 5: Culture & Society */
  if (game->culture_system)
    civ_culture_system_update(game->culture_system, dt);
  if (game->politics_system)
    civ_politics_system_update(game->politics_system, dt);

  /* Phase 6: Military */
  if (game->conquest_system) {
    civ_conquest_update(game->conquest_system, dt);
    int transferred = civ_conquest_transfer_territory(
        game->conquest_system, game->world_map, game->nation_manager);
    if (transferred > 0)
      printf("[GAME] %d border tiles transferred via conquest\n", transferred);
  }

  /* Phase 7: Territory & Borders */
  if (game->dynamic_borders)
    civ_dynamic_borders_update(game->dynamic_borders, dt);
  if (game->territory_manager)
    civ_territory_manager_update(game->territory_manager, dt);

  /* Phase 8: AI reacts to current world state */
  if (game->ai_system)
    civ_ai_system_update(game->ai_system, dt);
  if (game->subunit_manager)
    civ_subunit_manager_update(game->subunit_manager, dt);

  /* Phase 9: Events & Disasters */
  if (game->event_manager)
    civ_event_manager_update(game->event_manager, dt);
  if (game->disaster_manager)
    civ_disaster_update(game->disaster_manager, dt);

  /* Phase 10: Stature rankings (computed from all above) */
  civ_game_update_stature_rankings(game);

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

  /* Economy systems */
  if (game->banking)            civ_banking_destroy(game->banking);
  if (game->taxation)           civ_taxation_destroy(game->taxation);
  if (game->budget)             civ_budget_destroy(game->budget);
  if (game->economic_policy)    civ_economic_policy_destroy(game->economic_policy);
  if (game->agriculture)        civ_agriculture_destroy(game->agriculture);
  if (game->extraction)         civ_extraction_destroy(game->extraction);
  if (game->manufacturing)      civ_manufacturing_destroy(game->manufacturing);
  if (game->energy)             civ_energy_destroy(game->energy);
  if (game->financial_market)   civ_market_destroy(game->financial_market);
  if (game->commodity_market)   civ_resource_market_destroy(game->commodity_market);
  if (game->domestic_trade)     civ_domestic_trade_destroy(game->domestic_trade);
  if (game->labor_market)       civ_labor_market_destroy(game->labor_market);
  if (game->infrastructure)     civ_infrastructure_destroy(game->infrastructure);
  if (game->housing)            civ_housing_destroy(game->housing);
  if (game->land_use)           civ_land_use_destroy(game->land_use);
  if (game->capital_assets)     civ_capital_assets_destroy(game->capital_assets);
  if (game->war_economy)        civ_war_economy_destroy(game->war_economy);
  if (game->black_market)       civ_black_market_destroy(game->black_market);
  if (game->innovation_economy) civ_innovation_economy_destroy(game->innovation_economy);

  /* Governance */
  if (game->nation_manager) {
    civ_nation_manager_t *nm = (civ_nation_manager_t *)game->nation_manager;
    for (int ni = 0; ni < nm->count; ni++) {
      civ_nation_t *nation = &nm->nations[ni];
      if (nation->government && nation->government != game->government)
        civ_government_destroy(nation->government);
    }
  }
  if (game->government)               civ_government_destroy(game->government);
  if (game->custom_governance_manager) civ_custom_governance_manager_destroy(game->custom_governance_manager);

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
  if (game->system_orchestrator)
    civ_system_orchestrator_destroy(game->system_orchestrator);
  // ... destroy others ...
}

#define CIV_SAVE_MAGIC   0x43495653 /* "CIVS" */
#define CIV_SAVE_VERSION 3

typedef struct {
  /* v1 */
  uint32_t          magic, version;
  civ_game_config_t config;
  uint32_t          map_width, map_height, map_seed;
  float             sea_level;
  /* v2 */
  uint32_t          turn;
  uint64_t          timestamp;
  char              faction_id[32];
  char              profile_name[64];
  char              save_label[64];
  uint32_t          settlement_count;
  int64_t           total_population;
  /* v3 — time engine + character */
  int32_t           global_year;
  int32_t           global_day;
  uint32_t          turn_number;
  int32_t           char_bg;         /* character background */
  float             char_wealth;
  float             char_reputation;
  float             char_influence;
  int32_t           char_skills[10];
  uint32_t          reserved[8];
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
  /* v2 metadata */
  header.turn = game->current_turn;
  header.timestamp = (uint64_t)time(NULL);
  snprintf(header.faction_id, sizeof(header.faction_id), "%s",
           game->faction_id[0] ? game->faction_id : "none");
  snprintf(header.profile_name, sizeof(header.profile_name), "%s",
           game->current_profile ? game->current_profile->name : "unknown");
  snprintf(header.save_label, sizeof(header.save_label), "Turn %d",
           game->current_turn);
  if (game->settlement_manager)
    header.settlement_count = (uint32_t)game->settlement_manager->settlement_count;
  if (game->settlement_manager) {
    for (size_t i = 0; i < game->settlement_manager->settlement_count; i++)
      header.total_population +=
          game->settlement_manager->settlements[i].population;
  }
  /* v3: time engine state */
  if (game->time_engine) {
    civ_time_engine_t *te = (civ_time_engine_t *)game->time_engine;
    header.global_year = te->global.global_year;
    header.global_day = te->global.global_day;
    header.turn_number = te->global.turn_number;
  }
  /* v3: player character */
  if (game->player_character) {
    civ_character_t *pc = (civ_character_t *)game->player_character;
    header.char_bg = (int32_t)pc->background;
    header.char_wealth = pc->personal_wealth;
    header.char_reputation = pc->reputation;
    header.char_influence = pc->political_influence;
    memcpy(header.char_skills, pc->skills, sizeof(pc->skills));
  }

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

  /* v3: Restore time engine */
  if (header->version >= 3 && game->time_engine) {
    civ_time_engine_t *te = (civ_time_engine_t *)game->time_engine;
    te->global.global_year = header->global_year;
    te->global.global_day = header->global_day;
    te->global.turn_number = header->turn_number;
  }
  /* v3: Restore player character */
  if (header->version >= 3 && game->player_character) {
    civ_character_t *pc = (civ_character_t *)game->player_character;
    pc->background = (civ_background_t)header->char_bg;
    pc->personal_wealth = header->char_wealth;
    pc->reputation = header->char_reputation;
    pc->political_influence = header->char_influence;
    memcpy(pc->skills, header->char_skills, sizeof(pc->skills));
  }
  /* Restore turn */
  game->current_turn = header->turn;

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
