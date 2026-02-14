/**
 * @file strategic_ai.c
 * @brief Implementation of strategic AI
 */

#include "../../../include/core/ai/strategic_ai.h"
#include "../../../include/core/game.h"
#include "../../../include/core/world/map_generator.h"
#include "../../../include/core/world/settlement_manager.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_strategic_ai_t *civ_strategic_ai_create(const char *id, const char *name) {
  civ_strategic_ai_t *ai =
      (civ_strategic_ai_t *)CIV_MALLOC(sizeof(civ_strategic_ai_t));
  if (!ai) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate strategic AI");
    return NULL;
  }

  civ_strategic_ai_init(ai, id, name);
  return ai;
}

void civ_strategic_ai_destroy(civ_strategic_ai_t *ai) {
  if (!ai)
    return;

  if (ai->base_ai) {
    civ_base_ai_destroy(ai->base_ai);
  }
  CIV_FREE(ai->goals);
  CIV_FREE(ai);
}

void civ_strategic_ai_init(civ_strategic_ai_t *ai, const char *id,
                           const char *name) {
  if (!ai || !id || !name)
    return;

  memset(ai, 0, sizeof(civ_strategic_ai_t));
  ai->base_ai = civ_base_ai_create(id, name);
  ai->planning_horizon = 10.0f; /* 10 years */
  ai->risk_tolerance = 0.5f;
  ai->goal_capacity = 16;
  ai->goals = (civ_strategic_goal_t *)CIV_CALLOC(ai->goal_capacity,
                                                 sizeof(civ_strategic_goal_t));

  ai->personality = (civ_personality_type_t)(rand() % 4);
  ai->last_expansion_turn = 0;
  ai->expansion_frequency = (ai->personality == CIV_PERSONALITY_EXPANSIONIST ||
                             ai->personality == CIV_PERSONALITY_CULTURAL)
                                ? 12
                                : 25;

  civ_log(CIV_LOG_INFO, "Strategic AI initialized for %s with personality %d",
          name, ai->personality);
}

civ_result_t civ_strategic_ai_plan(civ_strategic_ai_t *ai,
                                   civ_float_t time_delta) {
  civ_result_t result = {CIV_OK, NULL};

  if (!ai) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  /* Update base AI thinking */
  if (ai->base_ai) {
    civ_base_ai_think(ai->base_ai, time_delta);
  }

  /* Phase 10: Evaluate threats and update stance */
  civ_strategic_ai_evaluate_threats(ai, ai->game_ptr);

  /* Adjust risk tolerance by personality */
  if (ai->personality == CIV_PERSONALITY_AGGRESSIVE) {
    ai->risk_tolerance = 0.8f;
  } else if (ai->personality == CIV_PERSONALITY_BALANCED) {
    ai->risk_tolerance = 0.5f;
  } else {
    ai->risk_tolerance = 0.3f;
  }

  /* Personality-specific goal planning, modified by stance */
  if (ai->goal_count < 3) {
    civ_diplomatic_relation_t *rel = civ_diplomacy_system_get_relation(
        ((civ_game_t *)ai->game_ptr)->diplomacy_system, ai->base_ai->id,
        "PLAYER");

    if (rel && (rel->current_stance == CIV_STANCE_HOSTILE ||
                rel->current_stance == CIV_STANCE_WARY)) {
      civ_strategic_ai_add_goal(ai, "Military", "Prepare for conflict", 0.95f);
    } else if (ai->personality == CIV_PERSONALITY_EXPANSIONIST) {
      civ_strategic_ai_add_goal(ai, "Expansion", "Found new settlements", 0.9f);
    } else if (ai->personality == CIV_PERSONALITY_AGGRESSIVE) {
      civ_strategic_ai_add_goal(ai, "Military", "Build up forces", 0.8f);
    } else if (ai->personality == CIV_PERSONALITY_MERCANTILE) {
      civ_strategic_ai_add_goal(ai, "Trade", "Establish trade routes", 0.7f);
    } else if (ai->personality == CIV_PERSONALITY_CULTURAL) {
      civ_strategic_ai_add_goal(ai, "Culture", "Achieve Cultural Hegemony",
                                0.95f);
    }
  }

  /* Update goal progress */
  time_t now = time(NULL);
  for (size_t i = 0; i < ai->goal_count; i++) {
    civ_strategic_goal_t *goal = &ai->goals[i];

    /* Remove completed or expired goals */
    if (goal->progress >= 1.0f ||
        (goal->deadline > 0 && now > goal->deadline)) {
      /* Remove goal */
      memmove(&ai->goals[i], &ai->goals[i + 1],
              (ai->goal_count - i - 1) * sizeof(civ_strategic_goal_t));
      ai->goal_count--;
      i--;
    }
  }

  return result;
}

civ_result_t civ_strategic_ai_add_goal(civ_strategic_ai_t *ai,
                                       const char *goal_type,
                                       const char *description,
                                       civ_float_t priority) {
  civ_result_t result = {CIV_OK, NULL};

  if (!ai || !goal_type) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  priority = CLAMP(priority, 0.0f, 1.0f);

  /* Expand if needed */
  if (ai->goal_count >= ai->goal_capacity) {
    ai->goal_capacity *= 2;
    ai->goals = (civ_strategic_goal_t *)CIV_REALLOC(
        ai->goals, ai->goal_capacity * sizeof(civ_strategic_goal_t));
  }

  if (ai->goals) {
    civ_strategic_goal_t *goal = &ai->goals[ai->goal_count++];
    strncpy(goal->goal_type, goal_type, sizeof(goal->goal_type) - 1);
    if (description) {
      strncpy(goal->description, description, sizeof(goal->description) - 1);
    }
    goal->priority = priority;
    goal->progress = 0.0f;
    goal->deadline = 0; /* No deadline by default */
    goal->created = time(NULL);
  } else {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

civ_strategic_goal_t *
civ_strategic_ai_get_priority_goal(const civ_strategic_ai_t *ai) {
  if (!ai || ai->goal_count == 0)
    return NULL;

  civ_strategic_goal_t *best = &ai->goals[0];
  for (size_t i = 1; i < ai->goal_count; i++) {
    if (ai->goals[i].priority > best->priority) {
      best = &ai->goals[i];
    }
  }

  return best;
}

civ_result_t civ_strategic_ai_update_goal(civ_strategic_ai_t *ai,
                                          size_t goal_index,
                                          civ_float_t progress) {
  civ_result_t result = {CIV_OK, NULL};

  if (!ai) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  if (goal_index >= ai->goal_count) {
    result.error = CIV_ERROR_INVALID_ARGUMENT;
    return result;
  }

  ai->goals[goal_index].progress = CLAMP(progress, 0.0f, 1.0f);
  return result;
}

civ_result_t civ_strategic_ai_process_expansion(civ_strategic_ai_t *ai,
                                                void *game_ptr) {
  civ_game_t *game = (civ_game_t *)game_ptr;
  if (!ai || !game)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null args"};

  /* Check turn frequency */
  if (game->current_turn - ai->last_expansion_turn < ai->expansion_frequency) {
    return (civ_result_t){CIV_OK, "Cooldown"};
  }

  /* Strategy: Find a suitable site near existing Rival cities or units */
  /* For simplicity, scan around (30, 30) or any settlement belonging to Rival
   */
  civ_float_t search_x = 30.0f;
  civ_float_t search_y = 30.0f;

  /* Find a random spot within 10 units */
  for (int attempts = 0; attempts < 10; attempts++) {
    float ox = (float)(rand() % 20 - 10);
    float oy = (float)(rand() % 20 - 10);
    float tx = search_x + ox;
    float ty = search_y + oy;

    /* Check suitability - Cultural AI is slightly less picky but values culture
     */
    float fitness = civ_calculate_site_suitability(tx, ty);
    float threshold =
        (ai->personality == CIV_PERSONALITY_CULTURAL) ? 0.65f : 0.75f;

    if (fitness > threshold) {
      civ_result_t res =
          civ_attempt_settlement_spawn(game->settlement_manager, tx, ty);
      if (res.error == CIV_OK) {
        printf(
            "[AI] %s founded a new settlement at %.1f, %.1f (Fitness: %.2f)\n",
            ai->base_ai->name, tx, ty, fitness);
        ai->last_expansion_turn = game->current_turn;

        /* Update goal progress if expansion or culture was a goal */
        for (size_t i = 0; i < ai->goal_count; i++) {
          if (strcmp(ai->goals[i].goal_type, "Expansion") == 0 ||
              strcmp(ai->goals[i].goal_type, "Culture") == 0) {
            ai->goals[i].progress += 0.34f; /* 3 settlements to complete goal */
          }
        }
        return res;
      }
    }
  }

  return (civ_result_t){CIV_OK, "No suitable site found"};
}

civ_result_t civ_strategic_ai_evaluate_threats(civ_strategic_ai_t *ai,
                                               void *game_ptr) {
  civ_game_t *game = (civ_game_t *)game_ptr;
  if (!ai || !game || !game->diplomacy_system)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Missing components"};

  /* Get relation with PLAYER */
  civ_diplomatic_relation_t *rel = civ_diplomacy_system_get_relation(
      game->diplomacy_system, ai->base_ai->id, "PLAYER");

  if (!rel)
    return (civ_result_t){CIV_OK, "No relation with player"};

  /* 1. Calculate Border Friction */
  float min_dist = 1000.0f;
  if (game->settlement_manager) {
    for (size_t i = 0; i < game->settlement_manager->settlement_count; i++) {
      civ_settlement_t *s1 = &game->settlement_manager->settlements[i];
      if (strcmp(s1->region_id, ai->base_ai->id) != 0)
        continue;

      for (size_t j = 0; j < game->settlement_manager->settlement_count; j++) {
        civ_settlement_t *s2 = &game->settlement_manager->settlements[j];
        if (strcmp(s2->region_id, "PLAYER") != 0)
          continue;

        float dx = s1->x - s2->x;
        float dy = s1->y - s2->y;
        float d = sqrtf(dx * dx + dy * dy);
        if (d < min_dist)
          min_dist = d;
      }
    }
  }

  /* Opinion modifiers */
  float border_penalty = (min_dist < 15.0f) ? (15.0f - min_dist) * 2.0f : 0.0f;

  /* 2. Military Balance */
  /* Placeholder: Assume AI likes being stronger if aggressive */
  float power_factor = 0.0f;
  if (ai->personality == CIV_PERSONALITY_AGGRESSIVE) {
    power_factor = -10.0f; /* Naturally more hostile */
  }

  /* 3. Update Opinion Score */
  /* Drift toward 0, then apply penalties */
  rel->opinion_score = rel->opinion_score * 0.95f;
  rel->opinion_score -= border_penalty;
  rel->opinion_score += power_factor;

  /* Clamp */
  rel->opinion_score = CLAMP(rel->opinion_score, -100.0f, 100.0f);

  /* 4. Update Stance based on Opinion */
  if (rel->opinion_score < -50.0f) {
    rel->current_stance = CIV_STANCE_HOSTILE;
  } else if (rel->opinion_score < -10.0f) {
    rel->current_stance = CIV_STANCE_WARY;
  } else if (rel->opinion_score > 40.0f) {
    rel->current_stance = CIV_STANCE_FRIENDLY;
  } else {
    rel->current_stance = CIV_STANCE_NEUTRAL;
  }

  return (civ_result_t){CIV_OK, "Threats evaluated"};
}

bool civ_strategic_ai_should_declare_war(civ_strategic_ai_t *ai, void *game_ptr,
                                         const char *target_id) {
  civ_game_t *game = (civ_game_t *)game_ptr;
  if (!ai || !game || !game->diplomacy_system)
    return false;

  civ_diplomatic_relation_t *rel = civ_diplomacy_system_get_relation(
      game->diplomacy_system, ai->base_ai->id, target_id);

  if (!rel || rel->relation_level == CIV_RELATION_LEVEL_WAR)
    return false;

  /* Aggressive AIs declare war on sight if opinion is low */
  if (ai->personality == CIV_PERSONALITY_AGGRESSIVE &&
      rel->opinion_score < -40.0f) {
    return true;
  }

  /* Hostile stance + very low opinion */
  if (rel->current_stance == CIV_STANCE_HOSTILE &&
      rel->opinion_score < -70.0f) {
    return true;
  }

  return false;
}

bool civ_strategic_ai_should_offer_peace(civ_strategic_ai_t *ai, void *game_ptr,
                                         const char *target_id) {
  civ_game_t *game = (civ_game_t *)game_ptr;
  if (!ai || !game || !game->diplomacy_system)
    return false;

  civ_diplomatic_relation_t *rel = civ_diplomacy_system_get_relation(
      game->diplomacy_system, ai->base_ai->id, target_id);

  if (!rel || rel->relation_level != CIV_RELATION_LEVEL_WAR)
    return false;

  /* Offer peace if opinion has recovered or AI is risk-averse */
  if (rel->opinion_score > -20.0f)
    return true;

  if (ai->risk_tolerance < 0.3f && (rand() % 100 < 5))
    return true;

  return false;
}
