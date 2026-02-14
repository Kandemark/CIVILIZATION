/**
 * @file governance_evolution.c
 * @brief Implementation of decision-driven governance evolution
 */

#include "../../../include/core/governance_evolution.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void civ_governance_init(civ_governance_state_t *gov) {
  if (!gov)
    return;

  // Initial traits: High centralization, low representation (early
  // tribe/autocracy)
  gov->traits.centralization = 0.8;
  gov->traits.representation = 0.1;
  gov->traits.militarization = 0.4;
  gov->traits.religious_authority = 0.6;
  gov->traits.meritocracy = 0.2;

  gov->stability = 1.0;
  gov->legitimacy = 0.9;
  gov->corruption = 0.05;
  gov->reform_cooldown = 0;

  strncpy(gov->common_title_pattern, "Leader", STRING_SHORT_LEN - 1);
  gov->local_autonomy_preference = 0.2;
}

double civ_governance_efficiency(const civ_governance_state_t *gov,
                                 double population, double culture_level) {
  if (!gov)
    return 1.0;

  // Centralization helps early on but hurts as population grows (bureaucratic
  // drag)
  double size_penalty = (population / 1000000.0) * gov->traits.centralization;
  double efficiency =
      1.0 + (gov->traits.meritocracy * 0.3) - gov->corruption - size_penalty;

  return fmax(0.1, efficiency);
}

double civ_governance_happiness(const civ_governance_state_t *gov,
                                double population, double culture_level) {
  if (!gov)
    return 1.0;

  // People want more representation as culture/education increases
  double demand_for_rep = culture_level / 1000.0;
  double rep_gap = demand_for_rep - gov->traits.representation;

  double happiness =
      1.0 + (gov->traits.representation * 0.2) - (fmax(0, rep_gap) * 0.5);
  return fmax(0.1, happiness);
}

civ_result_t civ_governance_update(civ_governance_state_t *gov,
                                   double population, double culture_level) {
  if (!gov)
    return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "Invalid gov"};

  gov->reform_cooldown++;

  // Legitimacy decays if stability is low or corruption is high
  gov->legitimacy += (gov->stability - 1.0) * 0.01 - (gov->corruption * 0.02);
  gov->legitimacy = fmax(0.0, fmin(1.2, gov->legitimacy));

  // Stability is affected by legitimacy
  if (gov->legitimacy < 0.4) {
    gov->stability -= 0.02; // Risk of revolution
  } else {
    gov->stability += 0.005;
  }
  gov->stability = fmax(0.0, fmin(1.5, gov->stability));

  return (civ_result_t){CIV_OK, NULL};
}

bool civ_governance_should_decide(const civ_governance_state_t *gov,
                                  double population, double culture_level) {
  if (!gov || gov->reform_cooldown < 20)
    return false;

  // Trigger decisions when population crosses certain thresholds or legitimacy
  // is low
  if (gov->legitimacy < 0.5)
    return true;
  if (population > 10000 && gov->traits.centralization > 0.7)
    return true;

  return (rand() % 100 < 5); // 5% chance otherwise
}

civ_governance_decision_t
civ_governance_generate_decision(const civ_governance_state_t *gov,
                                 double population, double culture_level) {

  civ_governance_decision_t d = {0};

  if (gov->legitimacy < 0.5) {
    d.prompt =
        "Your citizens are questioning your right to rule. How do you respond?";
    d.option_a_text = "Crack down on dissent (Force)";
    d.a_centralization = 0.1;
    d.a_stability = 0.2;
    d.a_representation = -0.1;

    d.option_b_text = "Offer minor reforms (Compromise)";
    d.b_representation = 0.1;
    d.b_stability = -0.1;

    d.option_c_text = "Broaden the council (Democratize)";
    d.c_representation = 0.3;
    d.c_stability = -0.3;
  } else if (population > 10000) {
    d.prompt = "The administrative burden of a growing empire is heavy.";
    d.option_a_text = "Appoint regional governors (Federalism)";
    d.a_centralization = -0.2;
    d.a_stability = 0.1;

    d.option_b_text = "Expand the central palace (Bureaucracy)";
    d.b_centralization = 0.1;
    d.b_meritocracy = 0.1;

    d.option_c_text = "Automate records (Innovation)";
    d.c_meritocracy = 0.2;
    d.c_centralization = 0.05;
  } else {
    d.prompt = "A local noble suggests a new law of succession.";
    d.option_a_text = "Hereditary (Tradition)";
    d.a_meritocracy = -0.1;
    d.a_stability = 0.1;

    d.option_b_text = "Elected by peers (Oligarchy)";
    d.b_meritocracy = 0.05;
    d.b_representation = 0.05;

    d.option_c_text = "Test of wisdom (Merit)";
    d.c_meritocracy = 0.3;
    d.c_representation = 0.01;
  }

  return d;
}

void civ_governance_apply_decision(civ_governance_state_t *gov,
                                   const civ_governance_decision_t *decision,
                                   int32_t choice) {
  if (!gov || !decision)
    return;

  if (choice == 0) {
    gov->traits.centralization += decision->a_centralization;
    gov->traits.representation += decision->a_representation;
    gov->stability += decision->a_stability;
  } else if (choice == 1) {
    gov->traits.centralization += decision->b_centralization;
    gov->traits.representation += decision->b_representation;
    gov->stability += decision->b_stability;
  } else {
    gov->traits.centralization += decision->c_centralization;
    gov->traits.representation += decision->c_representation;
    gov->stability += decision->c_stability;
  }

  // Clamp traits
  gov->traits.centralization = fmax(0.0, fmin(1.0, gov->traits.centralization));
  gov->traits.representation = fmax(0.0, fmin(1.0, gov->traits.representation));
  gov->stability = fmax(0.0, fmin(1.5, gov->stability));

  gov->reform_cooldown = 0;
}

const char *civ_governance_describe(const civ_governance_state_t *gov,
                                    char *buffer, size_t buffer_size) {
  if (!gov || !buffer)
    return "Unknown";

  if (gov->traits.representation > 0.7) {
    if (gov->traits.centralization > 0.6)
      return "Unitary Democracy";
    return "Federal Republic";
  }

  if (gov->traits.representation > 0.3) {
    if (gov->traits.meritocracy > 0.5)
      return "Technocratic Council";
    return "Constitutional Monarchy";
  }

  if (gov->traits.centralization > 0.8) {
    if (gov->traits.militarization > 0.6)
      return "Military Dictatorship";
    return "Absolute Autocracy";
  }

  return "Feudal Lordship";
}

char *civ_governance_generate_title(const civ_governance_state_t *gov,
                                    const char *language_id) {
  if (!gov)
    return NULL;

  char *title = (char *)CIV_MALLOC(STRING_SHORT_LEN);
  if (!title)
    return NULL;

  /* Simplified: Title depends on centralization and militarization */
  if (gov->traits.centralization > 0.8) {
    if (gov->traits.militarization > 0.7)
      strncpy(title, "Generalissimo", STRING_SHORT_LEN - 1);
    else
      strncpy(title, "Autocrat", STRING_SHORT_LEN - 1);
  } else if (gov->traits.representation > 0.7) {
    strncpy(title, "President", STRING_SHORT_LEN - 1);
  } else {
    strncpy(title, "Lord", STRING_SHORT_LEN - 1);
  }

  return title;
}

void civ_governance_add_local_preference(civ_governance_state_t *gov,
                                         const char *pref_type,
                                         double strength) {
  if (!gov || !pref_type)
    return;

  /* Shifts preference towards local autonomy or specific structures */
  if (strcmp(pref_type, "Lords") == 0) {
    gov->local_autonomy_preference =
        fmin(1.0, gov->local_autonomy_preference + strength);
    /* High preference for local lords reduces centralization */
    gov->traits.centralization *= (1.0 - strength * 0.1);
  }
}
