/**
 * @file governance_evolution.h
 * @brief Government systems that evolve based on culture, population, and
 * player decisions
 *
 * NO PREDEFINED PATHS - Government type emerges from your choices and
 * circumstances
 */

#ifndef CIVILIZATION_GOVERNANCE_EVOLUTION_H
#define CIVILIZATION_GOVERNANCE_EVOLUTION_H

#include "../common.h"
#include "../types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Governance traits (continuous values, not discrete types)
 */
typedef struct {
  double centralization;      // 0.0 (distributed) → 1.0 (autocratic)
  double representation;      // 0.0 (none) → 1.0 (full democracy)
  double militarization;      // 0.0 (peaceful) → 1.0 (martial law)
  double religious_authority; // 0.0 (secular) → 1.0 (theocracy)
  double meritocracy;         // 0.0 (hereditary) → 1.0 (merit-based)
} civ_governance_traits_t;

/**
 * @brief Governance state evolves over time based on decisions
 */
typedef struct {
  civ_governance_traits_t traits;
  double stability;  // 0.0 → 1.0+ (can exceed 1.0 with strong institutions)
  double legitimacy; // People's acceptance of government
  double corruption; // 0.0 (none) → high values increase
  int32_t reform_cooldown; // Turns since last major change

  /* Evolution & Preferences */
  char common_title_pattern[STRING_SHORT_LEN]; /* e.g. "Lord of {Region}" */
  double
      local_autonomy_preference; /* Preference for local vs central control */
} civ_governance_state_t;

/**
 * @brief Decision that affects governance evolution
 */
typedef struct {
  const char *prompt;
  const char *option_a_text;
  const char *option_b_text;
  const char *option_c_text;

  /* Effects on traits */
  double a_centralization;
  double a_representation;
  double a_meritocracy;
  double a_stability;

  double b_centralization;
  double b_representation;
  double b_meritocracy;
  double b_stability;

  double c_centralization;
  double c_representation;
  double c_meritocracy;
  double c_stability;

  /* Requirements */
  double min_culture;
  double min_population;
} civ_governance_decision_t;

/**
 * @brief Calculate governance efficiency (affects production, research)
 */
double civ_governance_efficiency(const civ_governance_state_t *gov,
                                 double population, double culture_level);

/**
 * @brief Calculate happiness modifier from government type
 */
double civ_governance_happiness(const civ_governance_state_t *gov,
                                double population, double culture_level);

/**
 * @brief Update governance based on population pressure and culture
 * May trigger decision events
 */
civ_result_t civ_governance_update(civ_governance_state_t *gov,
                                   double population, double culture_level);

/**
 * @brief Apply a governance decision choice
 * @param choice 0=A, 1=B, 2=C
 */
void civ_governance_apply_decision(civ_governance_state_t *gov,
                                   const civ_governance_decision_t *decision,
                                   int32_t choice);

/**
 * @brief Check if a governance evolution event should trigger
 */
bool civ_governance_should_decide(const civ_governance_state_t *gov,
                                  double population, double culture_level);

/**
 * @brief Generate a context-appropriate decision based on current state
 */
civ_governance_decision_t
civ_governance_generate_decision(const civ_governance_state_t *gov,
                                 double population, double culture_level);

/**
 * @brief Initialize governance with starting conditions
 */
void civ_governance_init(civ_governance_state_t *gov);

/**
 * @brief Get governance description (e.g., "Democratic Federation", "Merchant
 * Republic")
 */
const char *civ_governance_describe(const civ_governance_state_t *gov,
                                    char *buffer, size_t buffer_size);

/**
 * @brief Generate a title for a leader based on governance and language
 */
char *civ_governance_generate_title(const civ_governance_state_t *gov,
                                    const char *language_id);

/**
 * @brief Add a local preference shift (e.g. community prefers "Lords")
 */
void civ_governance_add_local_preference(civ_governance_state_t *gov,
                                         const char *pref_type,
                                         double strength);

#ifdef __cplusplus
}
#endif

#endif /* CIVILIZATION_GOVERNANCE_EVOLUTION_H */
