/**
 * @file governance_evolution.h
 * @brief Government systems that evolve based on culture, population, and
 * player decisions
 *
 * NO PREDEFINED PATHS — Government type emerges from your choices and
 * circumstances. NO governance configuration is inherently "bad" — each
 * has tradeoffs. Centralization trades speed for representation.
 * Meritocracy trades competence for rigidity. Theocracy trades cohesion
 * for innovation. Militarization trades order for civil liberty.
 *
 * All 7 trait axes feed into actual game mechanics.
 */
#ifndef CIVILIZATION_GOVERNANCE_EVOLUTION_H
#define CIVILIZATION_GOVERNANCE_EVOLUTION_H

#include "../../../common.h"
#include "../../../types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Governance traits — 7 continuous axes, no discrete types
 *
 * Every axis is 0.0–1.0. NO configuration is "bad." Each trades one
 * benefit for another. The optimal governance depends on context
 * (geography, population, culture, external threats, technology).
 */
typedef struct {
  double centralization;        /* 0.0 distributed → 1.0 unitary (speed vs voice) */
  double representation;        /* 0.0 none → 1.0 full (legitimacy vs decisiveness) */
  double militarization;        /* 0.0 civilian → 1.0 martial (order vs liberty) */
  double religious_authority;   /* 0.0 secular → 1.0 theocratic (cohesion vs inquiry) */
  double meritocracy;           /* 0.0 hereditary/connection → 1.0 exam/merit (competence vs flexibility) */
  double economic_freedom;      /* 0.0 state-directed → 1.0 laissez-faire (equality vs growth) */
  double tradition_index;       /* 0.0 reformist → 1.0 traditionalist (innovation vs continuity) */
} civ_governance_traits_t;

/**
 * @brief Governance state — evolves each tick from decisions and pressure
 */
typedef struct {
  civ_governance_traits_t traits;
  double stability;
  double legitimacy;
  double corruption;
  int32_t reform_cooldown;

  /* Succession state */
  int32_t leader_age;             /* years, ticks up each cycle */
  int32_t succession_crisis;      /* 0=none, >0 = crisis severity */
  double  succession_stability;   /* 0.0–1.0, how clear the line of succession is */

  /* Emergency powers state */
  bool    emergency_active;
  int32_t emergency_remaining;    /* turns until auto-expiry */
  double  emergency_power_grab;   /* 0.0–1.0, how much the executive took */

  char    common_title_pattern[STRING_SHORT_LEN];
  double  local_autonomy_preference;
} civ_governance_state_t;

/**
 * @brief Decision that affects governance evolution
 */
typedef struct {
  const char *prompt;
  const char *option_a_text;     /* centralizing / authoritarian direction */
  const char *option_b_text;     /* moderate / status-quo direction */
  const char *option_c_text;     /* decentralizing / liberalizing direction */

  /* Trait deltas per option (can be positive or negative) */
  double a_centralization, a_representation, a_militarization,
         a_religious, a_meritocracy, a_economic, a_tradition;
  double b_centralization, b_representation, b_militarization,
         b_religious, b_meritocracy, b_economic, b_tradition;
  double c_centralization, c_representation, c_militarization,
         c_religious, c_meritocracy, c_economic, c_tradition;

  double a_stability, b_stability, c_stability; /* immediate stability effects */
  double min_culture, min_population;           /* triggers */
} civ_governance_decision_t;

/* ── Core API ───────────────────────────────────────────────────── */

void civ_governance_init(civ_governance_state_t *gov);
civ_result_t civ_governance_update(civ_governance_state_t *gov,
                                   double population, double culture_level);

/* ── Gameplay effect queries — every axis feeds into at least one ─ */

/** Governance efficiency: affects production, research, tax collection.
 *  meritocracy↑, economic_freedom↑, low corruption = high efficiency.
 *  centralization↑ = faster decisions but representation↓ means lower
 *  long-term efficiency from lack of feedback. */
double civ_governance_efficiency(const civ_governance_state_t *gov,
                                 double population, double culture_level);

/** Citizen happiness: representation↑, low militarization↑,
 *  economic_freedom↑ (consumer choice), tradition↑ (cultural comfort).
 *  But too much representation = gridlock frustration; too much
 *  tradition = youth discontent. The curve is an inverted-U. */
double civ_governance_happiness(const civ_governance_state_t *gov,
                                double population, double culture_level);

/** Military effectiveness: militarization↑, centralization↑ (unified command).
 *  meritocracy↑ (competent officers). But excessive militarization
 *  diverts resources and creates coup risk. */
double civ_governance_military_bonus(const civ_governance_state_t *gov);

/** Research/innovation rate: secular↑, economic_freedom↑, meritocracy↑.
 *  tradition↑ provides stability for long-term research but slows adoption. */
double civ_governance_research_bonus(const civ_governance_state_t *gov);

/** Cultural cohesion: tradition↑, religious_authority↑.
 *  But excessive cohesion = xenophobia. representation↑ helps integrate
 *  diverse populations. */
double civ_governance_cohesion_bonus(const civ_governance_state_t *gov);

/** Economic growth modifier: economic_freedom↑, meritocracy↑,
 *  low corruption. centralization↑ enables industrial policy. */
double civ_governance_economic_bonus(const civ_governance_state_t *gov);

/** Corruption resistance: meritocracy↑, representation↑ (oversight),
 *  secular↑ (less patronage via religious networks).
 *  centralization↑ concentrates power = more opportunity at the top. */
double civ_governance_corruption_resistance(const civ_governance_state_t *gov);

/** Trade willingness: economic_freedom↑, low militarization↑.
 *  centralization↑ enables trade deals but representation allows
 *  protectionist pressures. */
double civ_governance_trade_bonus(const civ_governance_state_t *gov);

/* ── Decisions ──────────────────────────────────────────────────── */

bool civ_governance_should_decide(const civ_governance_state_t *gov,
                                  double population, double culture_level);
civ_governance_decision_t
civ_governance_generate_decision(const civ_governance_state_t *gov,
                                 double population, double culture_level);
void civ_governance_apply_decision(civ_governance_state_t *gov,
                                   const civ_governance_decision_t *decision,
                                   int32_t choice);

/* ── Description & identity ─────────────────────────────────────── */

/** Rich description from all 7 axes — 20+ distinct governance labels */
const char *civ_governance_describe(const civ_governance_state_t *gov,
                                    char *buffer, size_t buffer_size);

/** Leader title based on governance structure */
char *civ_governance_generate_title(const civ_governance_state_t *gov,
                                    const char *language_id);

/* ── Succession ─────────────────────────────────────────────────── */

/** Age the leader. Returns true if succession event triggered. */
bool civ_governance_age_leader(civ_governance_state_t *gov);

/** Force a succession (death, coup, abdication, election). */
void civ_governance_trigger_succession(civ_governance_state_t *gov);

/** How likely succession goes smoothly 0.0–1.0 */
double civ_governance_succession_smoothness(const civ_governance_state_t *gov);

/* ── Emergency powers ────────────────────────────────────────────── */

/** Declare emergency (war, disaster, stability crisis).
 *  Concentrates power, reduces representation, boosts military bonus. */
void civ_governance_declare_emergency(civ_governance_state_t *gov);

/** End emergency, restoring normal governance. May fail if executive
 *  resists (power_grab > legitimacy). */
bool civ_governance_end_emergency(civ_governance_state_t *gov);

/** Tick emergency state — auto-expiry + drift toward permanence */
void civ_governance_update_emergency(civ_governance_state_t *gov, float dt);

/* ── Local preferences ──────────────────────────────────────────── */
void civ_governance_add_local_preference(civ_governance_state_t *gov,
                                         const char *pref_type, double strength);

#ifdef __cplusplus
}
#endif

#endif /* CIVILIZATION_GOVERNANCE_EVOLUTION_H */
