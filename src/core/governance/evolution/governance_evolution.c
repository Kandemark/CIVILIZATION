/**
 * @file governance_evolution.c
 * @brief 7-axis governance evolution — every axis feeds gameplay, no "bad" configs
 */
#include "core/governance/evolution/governance_evolution.h"
#include "common.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ====================================================================
 * INIT
 * ==================================================================== */
void civ_governance_init(civ_governance_state_t *gov) {
  if (!gov) return;
  memset(gov, 0, sizeof(*gov));

  /* Start from a moderate tribal/emergent state — configurable per faction */
  gov->traits.centralization    = 0.55;  /* tribal chief → early consolidation */
  gov->traits.representation    = 0.15;  /* limited — elders' council */
  gov->traits.militarization    = 0.30;  /* warrior class exists */
  gov->traits.religious_authority = 0.40; /* shamans have influence */
  gov->traits.meritocracy       = 0.25;  /* mostly hereditary, some merit */
  gov->traits.economic_freedom  = 0.50;  /* barter/trade, no formal regulation */
  gov->traits.tradition_index   = 0.70;  /* custom-bound early society */

  gov->stability         = 0.60;
  gov->legitimacy        = 0.60;
  gov->corruption        = 0.05;
  gov->reform_cooldown   = 10;
  gov->leader_age        = 35;
  gov->succession_stability = 0.70;
  gov->local_autonomy_preference = 0.40;
}

/* ====================================================================
 * GAMEPLAY EFFECT QUERIES
 * Every axis feeds into at least one game mechanic.
 * NO configuration is zero-sum bad — each trades one benefit for another.
 * ==================================================================== */

double civ_governance_efficiency(const civ_governance_state_t *gov,
                                 double population, double culture_level) {
  if (!gov) return 0.50;
  const civ_governance_traits_t *t = &gov->traits;

  /* Meritocracy directly boosts administrative competence */
  double eff = 0.40 + t->meritocracy * 0.35;

  /* Centralization: faster decisions = higher short-term efficiency,
   * but diminishing returns as population grows (can't micro-manage) */
  eff += t->centralization * 0.15 * (1.0 - population / 2000000.0);

  /* Economic freedom: market signals improve resource allocation */
  eff += t->economic_freedom * 0.10;

  /* Corruption erodes efficiency proportionally */
  eff *= (1.0 - gov->corruption * 0.6);

  /* Emergency powers temporarily boost efficiency */
  if (gov->emergency_active)
    eff *= (1.0 + gov->emergency_power_grab * 0.15);

  if (eff < 0.10) eff = 0.10;
  if (eff > 0.95) eff = 0.95;
  return eff;
}

double civ_governance_happiness(const civ_governance_state_t *gov,
                                double population, double culture_level) {
  if (!gov) return 0.50;
  (void)population;
  const civ_governance_traits_t *t = &gov->traits;

  /* Representation: inverted-U — too little feels oppressed, too much = gridlock */
  double rep_happy;
  if (t->representation < 0.15)      rep_happy = t->representation * 0.5;
  else if (t->representation > 0.75) rep_happy = 0.75 - (t->representation - 0.75) * 0.3;
  else                               rep_happy = 0.55 + t->representation * 0.25;

  /* Economic freedom: consumer choice and entrepreneurship */
  double econ_happy = 0.40 + t->economic_freedom * 0.35;

  /* Tradition: cultural comfort — inverted-U for a different reason */
  /* Too little = rootless; too much = stifling */
  double trad_happy;
  if (t->tradition_index < 0.20)      trad_happy = 0.30 + t->tradition_index;
  else if (t->tradition_index > 0.80) trad_happy = 0.85 - (t->tradition_index - 0.80) * 0.5;
  else                                trad_happy = 0.50 + t->tradition_index * 0.15;

  /* Militarization: draft and martial law reduce happiness */
  double mil_penalty = t->militarization * 0.15;

  /* Religious authority: comfort for believers, alienation for non-believers.
   * The effect is modulated by culture_level (higher culture = more pluralism) */
  double rel_happy = t->religious_authority * (0.20 - culture_level * 0.15);

  /* Stability feeds back into happiness */
  double stability_bonus = gov->stability * 0.15;

  /* Succession crisis hurts */
  double succession_penalty = gov->succession_crisis * 0.001;

  double happy = rep_happy * 0.25 + econ_happy * 0.25 + trad_happy * 0.15
                 + stability_bonus + rel_happy - mil_penalty - succession_penalty;

  /* Corruption erodes trust */
  happy *= (1.0 - gov->corruption * 0.3);

  if (happy < 0.05) happy = 0.05;
  if (happy > 0.95) happy = 0.95;
  return happy;
}

double civ_governance_military_bonus(const civ_governance_state_t *gov) {
  if (!gov) return 1.0;
  const civ_governance_traits_t *t = &gov->traits;

  /* Militarization: direct combat effectiveness */
  double bonus = 1.0 + t->militarization * 0.25;

  /* Centralization: unified command structure */
  bonus += t->centralization * 0.10;

  /* Meritocracy: competent officers */
  bonus += t->meritocracy * 0.08;

  /* Emergency: wartime mobilization */
  if (gov->emergency_active)
    bonus += gov->emergency_power_grab * 0.12;

  /* Coup risk: high militarization + low legitimacy */
  /* Not a bonus, but tracked — returned separately via stability */

  if (bonus < 0.80) bonus = 0.80;
  if (bonus > 1.50) bonus = 1.50;
  return bonus;
}

double civ_governance_research_bonus(const civ_governance_state_t *gov) {
  if (!gov) return 1.0;
  const civ_governance_traits_t *t = &gov->traits;

  /* Secular governance enables free inquiry */
  double bonus = 1.0 + (1.0 - t->religious_authority) * 0.20;

  /* Meritocracy: best minds rise */
  bonus += t->meritocracy * 0.15;

  /* Economic freedom: private R&D investment */
  bonus += t->economic_freedom * 0.10;

  /* Tradition: provides institutional stability for long-term research,
   * but extreme traditionalism suppresses new ideas */
  if (t->tradition_index > 0.85)
    bonus -= (t->tradition_index - 0.85) * 0.40;
  else
    bonus += t->tradition_index * 0.05; /* moderate tradition helps */

  if (bonus < 0.50) bonus = 0.50;
  if (bonus > 1.60) bonus = 1.60;
  return bonus;
}

double civ_governance_cohesion_bonus(const civ_governance_state_t *gov) {
  if (!gov) return 1.0;
  const civ_governance_traits_t *t = &gov->traits;

  /* Tradition and religious authority bind communities */
  double bonus = 0.60 + t->tradition_index * 0.25 + t->religious_authority * 0.15;

  /* Representation helps diverse groups feel heard */
  bonus += t->representation * 0.10;

  /* Centralization can unify but also alienate peripheries */
  bonus -= t->centralization * (1.0 - t->representation) * 0.15;

  if (bonus < 0.30) bonus = 0.30;
  if (bonus > 1.0)  bonus = 1.0;
  return bonus;
}

double civ_governance_economic_bonus(const civ_governance_state_t *gov) {
  if (!gov) return 1.0;
  const civ_governance_traits_t *t = &gov->traits;

  double bonus = 1.0 + t->economic_freedom * 0.20 + t->meritocracy * 0.10;

  /* Centralization enables industrial policy and infrastructure investment */
  bonus += t->centralization * 0.08;

  /* Corruption is the main drag */
  bonus *= (1.0 - gov->corruption * 0.5);

  if (bonus < 0.60) bonus = 0.60;
  if (bonus > 1.40) bonus = 1.40;
  return bonus;
}

double civ_governance_corruption_resistance(const civ_governance_state_t *gov) {
  if (!gov) return 0.50;
  const civ_governance_traits_t *t = &gov->traits;

  /* Meritocracy: less patronage */
  double resist = 0.30 + t->meritocracy * 0.35;

  /* Representation: oversight, transparency */
  resist += t->representation * 0.20;

  /* Secular: religious patronage networks are a vector */
  resist += (1.0 - t->religious_authority) * 0.10;

  /* Centralization concentrates power — more opportunity at the top */
  resist -= t->centralization * (1.0 - t->representation) * 0.15;

  if (resist < 0.10) resist = 0.10;
  if (resist > 0.90) resist = 0.90;
  return resist;
}

double civ_governance_trade_bonus(const civ_governance_state_t *gov) {
  if (!gov) return 1.0;
  const civ_governance_traits_t *t = &gov->traits;

  double bonus = 1.0 + t->economic_freedom * 0.20;

  /* Low militarization = open borders */
  bonus += (1.0 - t->militarization) * 0.10;

  /* Centralization enables trade agreements */
  bonus += t->centralization * 0.05;

  if (bonus < 0.60) bonus = 0.60;
  if (bonus > 1.40) bonus = 1.40;
  return bonus;
}

/* ====================================================================
 * SUCCESSION
 * ==================================================================== */
bool civ_governance_age_leader(civ_governance_state_t *gov) {
  if (!gov) return false;
  gov->leader_age++;

  /* Mortality risk curve: negligible before 50, rises after */
  double mortality = 0.0;
  if (gov->leader_age > 60)
    mortality = (gov->leader_age - 60) * 0.003;
  if (gov->leader_age > 80)
    mortality = 0.06 + (gov->leader_age - 80) * 0.02;

  /* Lower if at war (assassination, battlefield risk) */
  if (gov->traits.militarization > 0.6)
    mortality += gov->traits.militarization * 0.01;

  /* Higher in corrupt states (palace intrigue) */
  mortality += gov->corruption * 0.02;

  /* Succession crisis increases instability mortality */
  if (gov->succession_crisis > 0)
    mortality += gov->succession_crisis * 0.005;

  double roll = (double)rand() / (double)RAND_MAX;
  if (roll < mortality) {
    civ_governance_trigger_succession(gov);
    return true;
  }
  return false;
}

void civ_governance_trigger_succession(civ_governance_state_t *gov) {
  if (!gov) return;

  /* The smoothness of succession depends on the governance structure */
  double smoothness = civ_governance_succession_smoothness(gov);

  /* Hereditary/meritocratic states have clearer rules */
  if (gov->traits.meritocracy < 0.30) {
    /* Hereditary: clear line if tradition is strong */
    smoothness = gov->traits.tradition_index * 0.8 + gov->succession_stability * 0.2;
  } else if (gov->traits.meritocracy > 0.70) {
    /* Merit-based: institutional process, less personalistic */
    smoothness = gov->traits.meritocracy * 0.6 + gov->stability * 0.4;
  } else {
    /* Mixed: depends on stability and institutional strength */
    smoothness = gov->stability * 0.5 + gov->legitimacy * 0.3 + gov->succession_stability * 0.2;
  }

  /* Crisis severity: 0 = smooth transition, 100 = civil war */
  gov->succession_crisis = (int32_t)((1.0 - smoothness) * 100.0);
  if (gov->succession_crisis < 0) gov->succession_crisis = 0;

  /* Reset leader */
  gov->leader_age = 25 + rand() % 30;
  gov->stability -= (1.0 - smoothness) * 0.3;
  gov->legitimacy -= (1.0 - smoothness) * 0.2;

  /* Crisis creates reform opportunity */
  gov->reform_cooldown = 0;

  /* Tradition states: succession reinforces the system */
  if (gov->traits.tradition_index > 0.70 && smoothness > 0.6) {
    gov->legitimacy += 0.05;
    gov->traits.tradition_index += 0.02;
    if (gov->traits.tradition_index > 1.0) gov->traits.tradition_index = 1.0;
  }

  /* Merit states: succession is a chance to refresh */
  if (gov->traits.meritocracy > 0.60 && smoothness > 0.5) {
    gov->corruption *= 0.85; /* fresh blood */
    gov->traits.meritocracy += 0.01;
    if (gov->traits.meritocracy > 1.0) gov->traits.meritocracy = 1.0;
  }
}

double civ_governance_succession_smoothness(const civ_governance_state_t *gov) {
  if (!gov) return 0.5;
  /* Multiple factors, none dominant */
  return gov->stability * 0.25 + gov->legitimacy * 0.20
         + gov->traits.tradition_index * 0.20
         + gov->traits.meritocracy * 0.15
         + (1.0 - gov->corruption) * 0.10
         + gov->succession_stability * 0.10;
}

/* ====================================================================
 * EMERGENCY POWERS
 * ==================================================================== */
void civ_governance_declare_emergency(civ_governance_state_t *gov) {
  if (!gov) return;
  gov->emergency_active = true;
  gov->emergency_remaining = 40; /* ~40 turns default */
  gov->emergency_power_grab = 0.30 + (1.0 - gov->traits.representation) * 0.30;
  if (gov->emergency_power_grab > 0.80) gov->emergency_power_grab = 0.80;

  /* Immediate effects */
  gov->traits.centralization += 0.15;
  gov->traits.representation -= 0.10;
  gov->traits.militarization += 0.05;

  /* Clamp */
  if (gov->traits.centralization > 1.0) gov->traits.centralization = 1.0;
  if (gov->traits.representation < 0.0) gov->traits.representation = 0.0;
  if (gov->traits.militarization > 1.0) gov->traits.militarization = 1.0;
}

bool civ_governance_end_emergency(civ_governance_state_t *gov) {
  if (!gov || !gov->emergency_active) return true;

  /* Executive may resist returning power */
  double resistance = gov->emergency_power_grab * (1.0 - gov->traits.representation);
  double restoration = gov->traits.representation * 0.5 + gov->legitimacy * 0.3
                       + (1.0 - gov->corruption) * 0.2;

  if (restoration > resistance) {
    gov->emergency_active = false;
    gov->emergency_remaining = 0;
    gov->emergency_power_grab = 0.0;
    gov->traits.centralization -= 0.10;
    gov->traits.representation += 0.05;
    gov->traits.militarization -= 0.03;
    if (gov->traits.centralization < 0.0) gov->traits.centralization = 0.0;
    if (gov->traits.representation > 1.0) gov->traits.representation = 1.0;
    return true;
  }
  /* Executive keeps emergency powers — normalization fails */
  gov->emergency_remaining = 20;
  gov->stability -= 0.08;
  gov->legitimacy -= 0.10;
  return false;
}

void civ_governance_update_emergency(civ_governance_state_t *gov, float dt) {
  if (!gov || !gov->emergency_active) return;
  gov->emergency_remaining--;
  if (gov->emergency_remaining <= 0)
    civ_governance_end_emergency(gov);
  /* Drift: the longer emergency lasts, the more normal it becomes */
  gov->traits.centralization += 0.0005 * dt;
  gov->traits.representation -= 0.0003 * dt;
  if (gov->traits.representation < 0.0) gov->traits.representation = 0.0;
  (void)dt;
}

/* ====================================================================
 * UPDATE
 * ==================================================================== */
civ_result_t civ_governance_update(civ_governance_state_t *gov,
                                   double population, double culture_level) {
  if (!gov) {
    civ_result_t r = {CIV_ERROR_INVALID_ARGUMENT, "null gov"};
    return r;
  }

  /* Age the leader */
  civ_governance_age_leader(gov);

  /* Emergency tick */
  civ_governance_update_emergency(gov, 1.0f);

  /* Succession crisis decays over time */
  if (gov->succession_crisis > 0) {
    gov->succession_crisis--;
    gov->stability -= gov->succession_crisis * 0.0001;
  }

  /* Reform cooldown: time since last decision */
  if (gov->reform_cooldown < 100) gov->reform_cooldown++;

  /* Corruption: drifts toward equilibrium based on resistance */
  double corruption_equilibrium = 1.0 - civ_governance_corruption_resistance(gov);
  gov->corruption += (corruption_equilibrium - gov->corruption) * 0.01;

  /* Legitimacy: drifts based on representation + happiness */
  double happiness = civ_governance_happiness(gov, population, culture_level);
  double legit_target = happiness * 0.6 + gov->traits.representation * 0.3
                        + gov->stability * 0.1;
  gov->legitimacy += (legit_target - gov->legitimacy) * 0.02;

  /* Stability: erosion from corruption, boost from cohesion */
  double cohesion = civ_governance_cohesion_bonus(gov);
  double stab_target = cohesion * 0.4 + gov->legitimacy * 0.3
                       - gov->corruption * 0.3 + 0.30;
  gov->stability += (stab_target - gov->stability) * 0.015;

  /* Succession stability: reinforced by tradition + meritocracy */
  gov->succession_stability += (gov->traits.tradition_index * 0.4
                                + gov->traits.meritocracy * 0.3
                                - gov->corruption * 0.3
                                - gov->succession_stability) * 0.02;

  /* Clamp */
  if (gov->stability < 0.0)  gov->stability = 0.0;
  if (gov->stability > 1.0)  gov->stability = 1.0;
  if (gov->legitimacy < 0.0) gov->legitimacy = 0.0;
  if (gov->legitimacy > 1.0) gov->legitimacy = 1.0;
  if (gov->corruption < 0.0) gov->corruption = 0.0;

  civ_result_t r = {CIV_OK, "updated"};
  return r;
}

/* ====================================================================
 * DECISIONS
 * ==================================================================== */
bool civ_governance_should_decide(const civ_governance_state_t *gov,
                                  double population, double culture_level) {
  if (!gov) return false;
  if (gov->reform_cooldown < 15) return false;

  /* Triggers: legitimacy crisis, population pressure, succession, random */
  if (gov->legitimacy < 0.30 && population > 500) return true;
  if (gov->succession_crisis > 30) return true;
  if (population > 50000 && gov->traits.representation < 0.20) return true;
  if (gov->corruption > 0.40) return true;
  if (gov->emergency_active && gov->emergency_remaining < 10) return true;
  if ((double)rand() / RAND_MAX < 0.03) return true;

  (void)culture_level;
  return false;
}

civ_governance_decision_t
civ_governance_generate_decision(const civ_governance_state_t *gov,
                                 double population, double culture_level) {
  civ_governance_decision_t d;
  memset(&d, 0, sizeof(d));

  /* Choose scenario based on current state */
  if (gov->succession_crisis > 30) {
    /* SUCCESSION CRISIS */
    d.prompt = "The succession is contested. How shall we resolve it?";
    d.option_a_text = "The strongest claimant takes power by force";
    d.option_b_text = "A council of nobles/elders selects the successor";
    d.option_c_text = "Let the people choose their next leader";
    d.a_militarization = 0.12; d.a_centralization = 0.08;
    d.a_stability = -0.05; d.a_meritocracy = -0.05;
    d.b_tradition = 0.08; d.b_centralization = 0.03;
    d.b_stability = 0.05; d.b_meritocracy = 0.02;
    d.c_representation = 0.12; d.c_centralization = -0.08;
    d.c_stability = -0.02; d.c_meritocracy = 0.05;
  } else if (gov->legitimacy < 0.30) {
    /* LEGITIMACY CRISIS */
    d.prompt = "The people question our right to rule. What do we do?";
    d.option_a_text = "Crack down — tighten control and suppress dissent";
    d.option_b_text = "Offer reforms while maintaining core authority";
    d.option_c_text = "Embrace broad reforms — let the people shape governance";
    d.a_centralization = 0.10; d.a_militarization = 0.08;
    d.a_representation = -0.05; d.a_stability = 0.05;
    d.b_representation = 0.06; d.b_stability = 0.03;
    d.b_economic = 0.04;
    d.c_representation = 0.15; d.c_centralization = -0.10;
    d.c_stability = -0.08; d.c_economic = -0.04;
    d.c_meritocracy = 0.05;
  } else if (gov->corruption > 0.35) {
    /* CORRUPTION SCANDAL */
    d.prompt = "Corruption scandals undermine public trust. How do we respond?";
    d.option_a_text = "Purge the corrupt — harsh punishments, made an example";
    d.option_b_text = "Establish an independent anti-corruption commission";
    d.option_c_text = "Redirect attention — launch a foreign adventure";
    d.a_stability = -0.05; d.a_meritocracy = 0.08; d.a_centralization = 0.05;
    d.b_meritocracy = 0.06; d.b_representation = 0.04; d.b_stability = 0.03;
    d.c_militarization = 0.10; d.c_centralization = 0.05;
    d.c_stability = 0.05; d.c_representation = -0.04;
    d.c_economic = -0.05;
  } else {
    /* DEFAULT: governance evolution — adaptive reform */
    d.prompt = "The realm evolves. What direction shall we take?";
    d.option_a_text = "Consolidate power — stronger central authority";
    d.option_b_text = "Maintain the current balance — steady as she goes";
    d.option_c_text = "Distribute power — strengthen local and popular rule";
    d.a_centralization = 0.08; d.a_representation = -0.04;
    d.a_stability = 0.03; d.a_economic = 0.03;
    d.b_stability = 0.05; d.b_tradition = 0.03;
    d.c_centralization = -0.08; d.c_representation = 0.10;
    d.c_stability = -0.03; d.c_meritocracy = 0.04;
    d.c_economic = -0.02;
  }

  d.min_population = population;
  d.min_culture = culture_level;
  return d;
}

void civ_governance_apply_decision(civ_governance_state_t *gov,
                                   const civ_governance_decision_t *decision,
                                   int32_t choice) {
  if (!gov || !decision) return;

  /* Select deltas based on choice */
  const double *deltas = NULL;
  switch (choice) {
    case 0:
      deltas = &decision->a_centralization;
      gov->stability += decision->a_stability;
      break;
    case 1:
      deltas = &decision->b_centralization;
      gov->stability += decision->b_stability;
      break;
    case 2:
      deltas = &decision->c_centralization;
      gov->stability += decision->c_stability;
      break;
    default:
      return;
  }

  /* Apply trait deltas — the struct layout matches the order in civ_governance_traits_t:
   * centralization, representation, militarization, religious_authority,
   * meritocracy, economic_freedom, tradition_index */
  gov->traits.centralization    += deltas[0];
  gov->traits.representation    += deltas[1];
  gov->traits.militarization    += deltas[2];
  gov->traits.religious_authority += deltas[3];
  gov->traits.meritocracy       += deltas[4];
  gov->traits.economic_freedom  += deltas[5];
  gov->traits.tradition_index   += deltas[6];

  /* Clamp all traits */
  #define CLAMP_TRAIT(t) do { if (t < 0.0) t = 0.0; if (t > 1.0) t = 1.0; } while(0)
  CLAMP_TRAIT(gov->traits.centralization);
  CLAMP_TRAIT(gov->traits.representation);
  CLAMP_TRAIT(gov->traits.militarization);
  CLAMP_TRAIT(gov->traits.religious_authority);
  CLAMP_TRAIT(gov->traits.meritocracy);
  CLAMP_TRAIT(gov->traits.economic_freedom);
  CLAMP_TRAIT(gov->traits.tradition_index);
  #undef CLAMP_TRAIT

  gov->reform_cooldown = 0;
}

/* ====================================================================
 * DESCRIPTION — 20+ governance labels from 7-axis state
 * ==================================================================== */
const char *civ_governance_describe(const civ_governance_state_t *gov,
                                    char *buffer, size_t buffer_size) {
  if (!gov || !buffer) return "Unknown";
  const civ_governance_traits_t *t = &gov->traits;

  /* Build from most to least defining axis */
  if (t->religious_authority > 0.65 && t->centralization > 0.60)
    snprintf(buffer, buffer_size, "%s Theocracy",
             t->representation > 0.30 ? "Representative" : "Centralized");
  else if (t->militarization > 0.70 && t->representation < 0.25)
    snprintf(buffer, buffer_size, "%s Military Junta",
             t->centralization > 0.70 ? "Unitary" : "Federated");
  else if (t->militarization > 0.55)
    snprintf(buffer, buffer_size, "%s Stratocracy",
             t->meritocracy > 0.50 ? "Meritocratic" : "Command");
  else if (t->economic_freedom > 0.80 && t->representation > 0.50)
    snprintf(buffer, buffer_size, "%s Market Democracy",
             t->centralization > 0.60 ? "Unitary" : "Federal");
  else if (t->economic_freedom > 0.80)
    snprintf(buffer, buffer_size, "%s Plutocracy",
             t->tradition_index > 0.60 ? "Traditional" : "Commercial");
  else if (t->meritocracy > 0.70 && t->representation > 0.45)
    snprintf(buffer, buffer_size, "%s Technocratic Republic",
             t->centralization > 0.55 ? "Unitary" : "Federal");
  else if (t->meritocracy > 0.70)
    snprintf(buffer, buffer_size, "%s Technocracy",
             t->centralization > 0.70 ? "Centralized" : "Distributed");
  else if (t->representation > 0.65 && t->centralization < 0.40)
    snprintf(buffer, buffer_size, "%s Confederation",
             t->tradition_index > 0.60 ? "Traditional" : "Progressive");
  else if (t->representation > 0.65)
    snprintf(buffer, buffer_size, "%s Democracy",
             t->centralization > 0.60 ? "Unitary" : "Federal");
  else if (t->representation > 0.40)
    snprintf(buffer, buffer_size, "%s Republic",
             t->centralization > 0.60 ? "Centralized" : "Federal");
  else if (t->centralization > 0.75 && t->tradition_index > 0.60)
    snprintf(buffer, buffer_size, "%s Monarchy",
             t->religious_authority > 0.50 ? "Divine" :
             t->meritocracy > 0.40 ? "Enlightened" : "Absolute");
  else if (t->centralization > 0.75)
    snprintf(buffer, buffer_size, "%s Autocracy",
             t->militarization > 0.40 ? "Military" :
             t->economic_freedom > 0.60 ? "Developmental" : "Centralized");
  else if (t->centralization < 0.30 && t->representation < 0.30)
    snprintf(buffer, buffer_size, "%s Oligarchy",
             t->tradition_index > 0.60 ? "Noble" :
             t->economic_freedom > 0.60 ? "Merchant" : "Council");
  else if (t->tradition_index > 0.75 && t->representation > 0.35)
    snprintf(buffer, buffer_size, "%s Constitutional Monarchy",
             t->centralization > 0.55 ? "Centralized" : "Federal");
  else if (t->tradition_index > 0.75)
    snprintf(buffer, buffer_size, "%s Feudal Lordship",
             t->religious_authority > 0.50 ? "Sacred" : "Secular");
  else if (t->economic_freedom < 0.25 && t->centralization > 0.60)
    snprintf(buffer, buffer_size, "%s Command Economy",
             t->representation > 0.30 ? "Council" : "Directorate");
  else if (t->representation < 0.15 && t->centralization < 0.40)
    snprintf(buffer, buffer_size, "Warlord Confederacy");
  else if (gov->emergency_active)
    snprintf(buffer, buffer_size, "%s under Emergency Rule",
             t->militarization > 0.50 ? "Military Administration" : "Provisional Government");
  else
    snprintf(buffer, buffer_size, "%s %s",
             t->centralization > 0.55 ? "Centralized" : "Decentralized",
             t->representation > 0.35 ? "Polity" : "Chiefdom");

  return buffer;
}

/* ====================================================================
 * TITLES
 * ==================================================================== */
char *civ_governance_generate_title(const civ_governance_state_t *gov,
                                    const char *language_id) {
  (void)language_id;
  static char title[STRING_MEDIUM_LEN];
  const civ_governance_traits_t *t = gov ? &gov->traits : NULL;

  if (!t) { strcpy(title, "Chieftain"); return title; }

  if (t->religious_authority > 0.70 && t->centralization > 0.60)
    strcpy(title, "High Priest");
  else if (t->militarization > 0.70)
    strcpy(title, t->representation > 0.25 ? "Commander-in-Chief" : "Generalissimo");
  else if (t->meritocracy > 0.70 && t->representation > 0.50)
    strcpy(title, "First Administrator");
  else if (t->meritocracy > 0.70)
    strcpy(title, "Chief Executive");
  else if (t->representation > 0.65)
    strcpy(title, t->centralization < 0.40 ? "First Among Equals" : "President");
  else if (t->centralization > 0.75 && t->tradition_index > 0.60) {
    if (t->religious_authority > 0.50) strcpy(title, "Divine Sovereign");
    else if (t->meritocracy > 0.40) strcpy(title, "Enlightened Ruler");
    else strcpy(title, "Sovereign");
  }
  else if (t->tradition_index > 0.75)
    strcpy(title, t->centralization > 0.55 ? "High Lord" : "Lord Paramount");
  else if (t->centralization > 0.70)
    strcpy(title, "Supreme Leader");
  else if (t->economic_freedom > 0.80)
    strcpy(title, "Chief Merchant");
  else
    strcpy(title, "Leader");

  return title;
}

/* ====================================================================
 * LOCAL PREFERENCES
 * ==================================================================== */
void civ_governance_add_local_preference(civ_governance_state_t *gov,
                                         const char *pref_type, double strength) {
  if (!gov || !pref_type) return;
  if (strstr(pref_type, "centralize"))
    gov->local_autonomy_preference -= strength * 0.1;
  else if (strstr(pref_type, "autonomous"))
    gov->local_autonomy_preference += strength * 0.1;
  else if (strstr(pref_type, "lord"))
    gov->traits.tradition_index += strength * 0.02;
  else if (strstr(pref_type, "republic"))
    gov->traits.representation += strength * 0.02;
}
