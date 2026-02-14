/**
 * @file knowledge_system.h
 * @brief Continuous knowledge advancement system (replaces fixed tech tree)
 *
 * NO CAPS - Knowledge grows infinitely with exponential costs
 */

#ifndef CIVILIZATION_KNOWLEDGE_SYSTEM_H
#define CIVILIZATION_KNOWLEDGE_SYSTEM_H

#include "../common.h"
#include "../types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Continuous knowledge metrics (no maximum values)
 * Each domain progresses independently with exponential costs
 */
typedef struct {
  double science;     // Pure research: physics, chemistry, biology (0 → ∞)
  double engineering; // Applied technology: tools, machines, automation (0 → ∞)
  double culture;     // Arts, philosophy, social sciences (0 → ∞)
  double medicine;    // Healthcare, longevity, disease control (0 → ∞)
  double mathematics; // Foundation that amplifies other fields (0 → ∞)
  double agriculture; // Food production efficiency (0 → ∞)
  double metallurgy;  // Material science, stronger equipment (0 → ∞)
  double navigation;  // Exploration, trade efficiency (0 → ∞)
} civ_knowledge_t;

/**
 * @brief Research progress tracking
 */
typedef struct {
  civ_knowledge_t current;
  double research_points_pool; // Accumulated but not yet applied
  double research_rate;        // Points per turn
} civ_research_state_t;

/**
 * @brief Calculate cost to advance knowledge from current to next level
 * Uses exponential scaling: cost = base × (1 + level)^exponent
 */
double civ_knowledge_cost(double current_level, double base_cost,
                          double exponent);

/**
 * @brief Apply research points to a knowledge domain
 * Returns actual advancement achieved
 */
double civ_knowledge_advance(double *knowledge, double research_points,
                             double base_cost, double exponent);

/**
 * @brief Calculate production multiplier from knowledge
 * Returns logarithmic bonus: 1.0 + log(1 + knowledge/scale)
 */
double civ_knowledge_multiplier(double knowledge, double scale);

/**
 * @brief Get mathematics bonus (amplifies other research)
 * Higher math makes other domains cheaper to research
 */
double civ_knowledge_math_bonus(double math_level);

/**
 * @brief Calculate research points generated per turn
 * Based on population, buildings, and knowledge synergies
 */
double civ_knowledge_research_rate(const civ_research_state_t *state,
                                   int32_t population, int32_t libraries);

/**
 * @brief Update research system for one turn
 */
civ_result_t civ_knowledge_update(civ_research_state_t *state,
                                  int32_t population, int32_t libraries);

/**
 * @brief Initialize knowledge system
 */
void civ_knowledge_init(civ_research_state_t *state);

/**
 * @brief Format knowledge value for display (handles large numbers)
 * Example: 1523.7 → "1.5K", 2500000 → "2.5M"
 */
const char *civ_knowledge_format(double value, char *buffer,
                                 size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* CIVILIZATION_KNOWLEDGE_SYSTEM_H */
