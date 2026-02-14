/**
 * @file knowledge_system.c
 * @brief Implementation of uncapped knowledge advancement
 */

#include "../../include/core/knowledge_system.h"
#include "../../include/common.h"
#include <math.h>
#include <stdio.h>

/* Default cost parameters */
#define BASE_RESEARCH_COST 100.0
#define COST_EXPONENT 1.8
#define MULTIPLIER_SCALE 100.0

double civ_knowledge_cost(double current_level, double base_cost,
                          double exponent) {
  if (current_level < 0.0)
    current_level = 0.0;
  return base_cost * pow(1.0 + current_level, exponent);
}

double civ_knowledge_advance(double *knowledge, double research_points,
                             double base_cost, double exponent) {
  if (!knowledge || research_points <= 0.0)
    return 0.0;

  double remaining = research_points;
  double total_advanced = 0.0;

  /* Apply research points incrementally */
  while (remaining > 0.0) {
    double cost = civ_knowledge_cost(*knowledge, base_cost, exponent);

    if (remaining >= cost) {
      /* Full level advancement */
      *knowledge += 1.0;
      remaining -= cost;
      total_advanced += 1.0;
    } else {
      /* Partial advancement */
      double fraction = remaining / cost;
      *knowledge += fraction;
      total_advanced += fraction;
      remaining = 0.0;
    }

    /* Safety check for extremely slow progress */
    if (total_advanced > 0.0 && total_advanced < 0.0001)
      break;
  }

  return total_advanced;
}

double civ_knowledge_multiplier(double knowledge, double scale) {
  if (knowledge < 0.0)
    knowledge = 0.0;
  if (scale <= 0.0)
    scale = 1.0;

  /* Logarithmic bonus: diminishing returns */
  return 1.0 + log(1.0 + knowledge / scale);
}

double civ_knowledge_math_bonus(double math_level) {
  /* Mathematics reduces research costs for other domains */
  /* At math 100: 10% cost reduction */
  /* At math 1000: 25% cost reduction */
  if (math_level <= 0.0)
    return 1.0;

  double reduction = log(1.0 + math_level / 200.0) * 0.15;
  return 1.0 - CLAMP(reduction, 0.0, 0.4); /* Max 40% reduction */
}

double civ_knowledge_research_rate(const civ_research_state_t *state,
                                   int32_t population, int32_t libraries) {
  if (!state)
    return 0.0;

  /* Base research from population */
  double pop_research = (double)population * 0.01; /* 1% of pop contributes */

  /* Library bonus (multiplicative) */
  double library_mult = 1.0 + (double)libraries * 0.2;

  /* Mathematics synergy */
  double math_bonus = civ_knowledge_math_bonus(state->current.mathematics);

  /* Culture increases research efficiency */
  double culture_bonus =
      civ_knowledge_multiplier(state->current.culture, 200.0);

  return pop_research * library_mult * math_bonus * culture_bonus;
}

civ_result_t civ_knowledge_update(civ_research_state_t *state,
                                  int32_t population, int32_t libraries) {
  if (!state) {
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null research state"};
  }

  /* Generate research points this turn */
  state->research_rate =
      civ_knowledge_research_rate(state, population, libraries);
  state->research_points_pool += state->research_rate;

  /* Auto-advance science (can be made player-directed later) */
  if (state->research_points_pool > 0.0) {
    double math_cost_reduction =
        civ_knowledge_math_bonus(state->current.mathematics);
    double effective_cost = BASE_RESEARCH_COST * math_cost_reduction;

    /* Distribute research across domains (simplified - can be made strategic)
     */
    double per_domain = state->research_points_pool / 8.0;

    civ_knowledge_advance(&state->current.science, per_domain, effective_cost,
                          COST_EXPONENT);
    civ_knowledge_advance(&state->current.engineering, per_domain,
                          effective_cost, COST_EXPONENT);
    civ_knowledge_advance(&state->current.culture, per_domain, effective_cost,
                          COST_EXPONENT);
    civ_knowledge_advance(&state->current.medicine, per_domain, effective_cost,
                          COST_EXPONENT);
    civ_knowledge_advance(&state->current.mathematics, per_domain,
                          effective_cost, COST_EXPONENT);
    civ_knowledge_advance(&state->current.agriculture, per_domain,
                          effective_cost, COST_EXPONENT);
    civ_knowledge_advance(&state->current.metallurgy, per_domain,
                          effective_cost, COST_EXPONENT);
    civ_knowledge_advance(&state->current.navigation, per_domain,
                          effective_cost, COST_EXPONENT);

    state->research_points_pool = 0.0;
  }

  return (civ_result_t){CIV_OK, NULL};
}

void civ_knowledge_init(civ_research_state_t *state) {
  if (!state)
    return;

  memset(state, 0, sizeof(civ_research_state_t));

  /* Start with minimal knowledge */
  state->current.science = 1.0;
  state->current.engineering = 1.0;
  state->current.culture = 1.0;
  state->current.mathematics = 1.0;
  state->current.agriculture = 5.0; /* Civilizations start knowing farming */
}

const char *civ_knowledge_format(double value, char *buffer,
                                 size_t buffer_size) {
  if (!buffer || buffer_size < 16)
    return "ERROR";

  if (value < 1000.0) {
    snprintf(buffer, buffer_size, "%.1f", value);
  } else if (value < 1e6) {
    snprintf(buffer, buffer_size, "%.1fK", value / 1e3);
  } else if (value < 1e9) {
    snprintf(buffer, buffer_size, "%.1fM", value / 1e6);
  } else if (value < 1e12) {
    snprintf(buffer, buffer_size, "%.1fB", value / 1e9);
  } else {
    snprintf(buffer, buffer_size, "%.1fT", value / 1e12);
  }

  return buffer;
}
