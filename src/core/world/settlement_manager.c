/**
 * @file settlement_manager.c
 * @brief Implementation of Dynamic Settlement Manager
 */

#include "../../../include/core/world/settlement_manager.h"
#include "../../../include/core/governance/government.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

civ_settlement_manager_t *civ_settlement_manager_create(void) {
  civ_settlement_manager_t *manager =
      CIV_MALLOC(sizeof(civ_settlement_manager_t));
  if (manager) {
    manager->settlements = NULL;
    manager->settlement_count = 0;
    manager->settlement_capacity = 0;
    manager->min_distance = 10.0f; // Arbitrary unit distance
  }
  return manager;
}

void civ_settlement_manager_destroy(civ_settlement_manager_t *manager) {
  if (manager) {
    CIV_FREE(manager->settlements);
    CIV_FREE(manager);
  }
}

civ_result_t civ_settlement_manager_add(civ_settlement_manager_t *manager,
                                        civ_settlement_t *settlement) {
  if (!manager || !settlement)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid args"};

  if (manager->settlement_count >= manager->settlement_capacity) {
    size_t new_cap = manager->settlement_capacity == 0
                         ? 16
                         : manager->settlement_capacity * 2;
    civ_settlement_t *new_arr =
        CIV_REALLOC(manager->settlements, new_cap * sizeof(civ_settlement_t));
    if (!new_arr)
      return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
    manager->settlements = new_arr;
    manager->settlement_capacity = new_cap;
  }

  manager->settlements[manager->settlement_count++] = *settlement;
  return (civ_result_t){CIV_OK, "Settlement added"};
}

civ_float_t civ_calculate_site_suitability(civ_float_t x, civ_float_t y) {
  // Placeholder: In real game, query terrain, resources, water access here.
  // For now, return random suitability
  return (civ_float_t)(rand() % 100) / 100.0f;
}

civ_result_t civ_attempt_settlement_spawn(civ_settlement_manager_t *manager,
                                          civ_float_t x, civ_float_t y) {
  if (!manager)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "No manager"};

  // Check distance to existing
  for (size_t i = 0; i < manager->settlement_count; i++) {
    civ_float_t dx = manager->settlements[i].x - x;
    civ_float_t dy = manager->settlements[i].y - y;
    if (sqrt(dx * dx + dy * dy) < manager->min_distance) {
      return (civ_result_t){CIV_ERROR_INVALID_STATE, "Too close to existing"};
    }
  }

  civ_float_t suitability = civ_calculate_site_suitability(x, y);
  if (suitability > 0.7f) {
    civ_settlement_t new_town;
    memset(&new_town, 0, sizeof(civ_settlement_t));
    snprintf(new_town.id, STRING_SHORT_LEN, "settle_%ld", (long)time(NULL));
    snprintf(new_town.name, STRING_MEDIUM_LEN, "New Settlement %zu",
             manager->settlement_count + 1);
    new_town.tier = CIV_SETTLEMENT_HAMLET;
    new_town.x = x;
    new_town.y = y;
    new_town.population = 100; // Starting pop
    new_town.founded_date = time(NULL);
    new_town.attractiveness = suitability;
    new_town.culture_yield = 1.0f;
    new_town.accumulated_culture = 0.0f;
    new_town.territory_radius = 2; /* Start with small radius */

    /* Phase 9/11: Initialize Sovereign Affairs & Identity */
    new_town.loyalty = 1.0f;
    new_town.unrest = 0.0f;
    new_town.primary_ethnicity = 0; /* Default */
    new_town.primary_language = 0;
    new_town.primary_faith = 0;

    /* Initialize demographic populations with the 100 residents */
    memset(&new_town.demographics, 0, sizeof(civ_demographic_stats_t));
    new_town.demographics.race_pop[0] = 100;
    new_town.demographics.language_pop[0] = 100;
    new_town.demographics.faith_pop[0] = 100;

    return civ_settlement_manager_add(manager, &new_town);
  }

  return (civ_result_t){CIV_OK, "Not suitable"};
}

civ_result_t civ_settlement_manager_update(civ_settlement_manager_t *manager,
                                           civ_map_t *map,
                                           struct civ_government *gov,
                                           civ_float_t time_delta) {
  if (!manager)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null manager"};

  for (size_t i = 0; i < manager->settlement_count; i++) {
    civ_settlement_t *s = &manager->settlements[i];

    // Organic growth
    civ_float_t growth = s->population * 0.01f * s->attractiveness * time_delta;
    s->population += (int64_t)growth;

    // Tier upgrade
    if (s->tier == CIV_SETTLEMENT_HAMLET && s->population > 1000)
      s->tier = CIV_SETTLEMENT_VILLAGE;
    else if (s->tier == CIV_SETTLEMENT_VILLAGE && s->population > 5000)
      s->tier = CIV_SETTLEMENT_TOWN;
    else if (s->tier == CIV_SETTLEMENT_TOWN && s->population > 20000)
      s->tier = CIV_SETTLEMENT_CITY;
    else if (s->tier == CIV_SETTLEMENT_CITY && s->population > 100000)
      s->tier = CIV_SETTLEMENT_METROPOLIS;

    // Production processing
    if (s->is_producing) {
      /* Base 2.0 + 1.0 per 2000 population */
      civ_float_t prod_rate = 2.0f + floorf((float)s->population / 2000.0f);
      s->production_progress += prod_rate * time_delta;
    }

    // Culture generation
    s->culture_yield = 1.0f + floorf((float)s->population / 5000.0f);
    s->accumulated_culture += s->culture_yield * time_delta;

    // Phase 9/11: Loyalty and Unrest Updates
    civ_float_t pop_factor = (civ_float_t)s->population / 10000.0f;
    civ_float_t crowding = MAX(0.0f, pop_factor - 1.0f) * 0.05f;

    /* Phase 11: Socio-Political Friction (Identity Mismatches) */
    civ_float_t identity_friction = 0.0f;
    if (s->primary_language != 0)
      identity_friction += 0.05f; /* Language friction */
    if (s->primary_faith != 0)
      identity_friction += 0.03f; /* Religious friction */
    if (s->primary_ethnicity != 0)
      identity_friction += 0.02f; /* Ethnic friction */

    s->unrest = CLAMP(s->unrest + (crowding + identity_friction -
                                   s->infrastructure.sanitation * 0.1f) *
                                      time_delta,
                      0.0f, 1.0f);

    /* Phase 11: Identity Assimilation (Slow drift toward national standard 0)
     */
    if (s->population > 100 && time_delta > 0) {
      civ_float_t assimilation_rate = 0.01f * time_delta;
      if (s->primary_language != 0) {
        int64_t drift = (int64_t)(s->population * assimilation_rate);
        s->demographics.language_pop[0] += drift;
        s->demographics.language_pop[s->primary_language] -= drift;
      }
    }

    /* Apply Institutional Bonuses */
    civ_float_t prod_mult = 1.0f;
    civ_float_t science_mult = 1.0f;
    if (gov && gov->institution_manager) {
      prod_mult += civ_institution_get_total_bonus(
          gov->institution_manager, CIV_INSTITUTION_FOCUS_PRODUCTION);
      science_mult += civ_institution_get_total_bonus(
          gov->institution_manager, CIV_INSTITUTION_FOCUS_RESEARCH);
    }

    /* Loyalty decay based on unrest and national stability */
    civ_float_t stability_bonus =
        gov ? (gov->stability - 0.5f) * 0.05f : -0.01f;
    civ_float_t legitimacy_bonus =
        gov ? (gov->legitimacy - 0.5f) * 0.03f : 0.0f;

    civ_float_t loyalty_drift = (0.01f * s->infrastructure.road_quality) -
                                (0.02f * s->unrest) + stability_bonus +
                                legitimacy_bonus;
    s->loyalty = CLAMP(s->loyalty + loyalty_drift * time_delta, 0.0f, 1.0f);

    // Revolt Trigger
    if (s->loyalty < 0.15f && strcmp(s->id, "player_capital") != 0) {
      strncpy(s->region_id, "REBELS", STRING_SHORT_LEN - 1);
      printf("[SOVEREIGN] %s HAS REVOLTED! Loyalty: %.2f\n", s->name,
             s->loyalty);
      /* In a full implementation, we would change owner_id on tiles too,
         but for now we mark the settlement as rebel-held */
      /* TODO: Spawn rebel units via unit_manager */
    }

    // Expansion thresholds (Automatic radius increase based on total culture)
    if (s->territory_radius == 2 && s->accumulated_culture >= 20.0f) {
      s->territory_radius = 3;
    } else if (s->territory_radius == 3 && s->accumulated_culture >= 100.0f) {
      s->territory_radius = 4;
    } else if (s->territory_radius == 4 && s->accumulated_culture >= 500.0f) {
      s->territory_radius = 5;
    }

    // Tile Influence & Competition
    if (map) {
      int32_t cx = (int32_t)s->x;
      int32_t cy = (int32_t)s->y;
      int32_t r = s->territory_radius;

      for (int32_t dy = -r; dy <= r; dy++) {
        for (int32_t dx = -r; dx <= r; dx++) {
          float dist_sq = (float)(dx * dx + dy * dy);
          if (dist_sq <= (float)(r * r)) {
            int32_t tx = cx + dx;
            int32_t ty = cy + dy;

            /* Handle map wrapping for X */
            tx = (tx + map->width) % map->width;

            if (ty >= 0 && ty < map->height) {
              civ_map_tile_t *tile = civ_map_get_tile(map, tx, ty);
              if (tile) {
                /* Calculate terrain resistance */
                float resistance = 1.0f;
                if (tile->terrain == CIV_TERRAIN_MOUNTAIN)
                  resistance = 5.0f;
                else if (tile->land_use == CIV_LAND_USE_WATER)
                  resistance = 2.0f;

                /* Influence Model: Yield / (effective_dist + 1)^2 */
                float effective_dist = sqrtf(dist_sq) * resistance;
                float influence = s->culture_yield / (effective_dist + 1.0f);

                /* Competition & Flipping Logic */
                if (influence >
                    0.05f) { /* Minimum threshold to exert influence */
                  bool is_unowned = (tile->owner_id[0] == '\0');
                  bool is_owner = (strcmp(tile->owner_id, s->id) == 0);

                  if (is_owner) {
                    /* Refresh our own influence record */
                    tile->cultural_influence = influence;
                  } else if (is_unowned) {
                    /* Claim unowned tiles easily */
                    strncpy(tile->owner_id, s->id, STRING_SHORT_LEN - 1);
                    tile->owner_id[STRING_SHORT_LEN - 1] = '\0';
                    tile->cultural_influence = influence;
                  } else {
                    /* Cultural Flipping: Must have significantly higher
                     * influence */
                    /* Threshold: 1.5x current record */
                    if (influence > tile->cultural_influence * 1.5f) {
                      strncpy(tile->owner_id, s->id, STRING_SHORT_LEN - 1);
                      tile->owner_id[STRING_SHORT_LEN - 1] = '\0';
                      tile->cultural_influence = influence;

                      /* Log the flip */
                      printf("[HEGEMONY] Tile at (%d,%d) flipped to %s (Power: "
                             "%.2f vs %.2f)\n",
                             tx, ty, s->name, influence,
                             tile->cultural_influence);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return (civ_result_t){CIV_OK, "Updated"};
}
