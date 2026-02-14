/**
 * @file relations.h
 * @brief Diplomacy and relations system
 */

#ifndef CIVILIZATION_RELATIONS_H
#define CIVILIZATION_RELATIONS_H

#include "../../common.h"
#include "../../types.h"

/* Relation level enumeration */
typedef enum {
  CIV_RELATION_LEVEL_WAR = -2,
  CIV_RELATION_LEVEL_HOSTILE = -1,
  CIV_RELATION_LEVEL_NEUTRAL = 0,
  CIV_RELATION_LEVEL_FRIENDLY = 1,
  CIV_RELATION_LEVEL_ALLIED = 2
} civ_relation_level_t;

/* AI diplomatic stances */
typedef enum {
  CIV_STANCE_FRIENDLY = 0,
  CIV_STANCE_NEUTRAL,
  CIV_STANCE_WARY,
  CIV_STANCE_HOSTILE
} civ_ai_stance_t;

/* AI personalities */
typedef enum {
  CIV_PERSONALITY_BALANCED = 0,
  CIV_PERSONALITY_AGGRESSIVE,
  CIV_PERSONALITY_EXPANSIONIST,
  CIV_PERSONALITY_MERCANTILE,
  CIV_PERSONALITY_CULTURAL
} civ_personality_type_t;

/* Treaty type enumeration */
typedef enum {
  CIV_TREATY_TYPE_TRADE_AGREEMENT = 0,
  CIV_TREATY_TYPE_NON_AGGRESSION,
  CIV_TREATY_TYPE_DEFENSIVE_PACT,
  CIV_TREATY_TYPE_MILITARY_ALLIANCE,
  CIV_TREATY_TYPE_RESEARCH_PARTNERSHIP
} civ_treaty_type_t;

/* Diplomatic relation structure */
typedef struct {
  char nation_a[STRING_SHORT_LEN];
  char nation_b[STRING_SHORT_LEN];
  civ_relation_level_t relation_level;
  civ_ai_stance_t current_stance;
  civ_float_t trust;
  civ_float_t opinion_score; /* -100.0 to 100.0 */
  civ_personality_type_t personality;

  /* Casus Belli & History */
  civ_float_t grievances; /* Accumulated historical wrongs */
  char primary_casus_belli[STRING_MEDIUM_LEN];

  time_t last_updated;
} civ_diplomatic_relation_t;

/* Treaty structure */
typedef struct {
  char treaty_id[STRING_SHORT_LEN];
  civ_treaty_type_t treaty_type;
  char **signatories;
  size_t signatory_count;
  time_t start_date;
  int32_t duration_days;
  bool active;
} civ_treaty_t;

/* Diplomacy system structure */
typedef struct {
  civ_diplomatic_relation_t *relations;
  size_t relation_count;
  size_t relation_capacity;
  civ_treaty_t *treaties;
  size_t treaty_count;
  size_t treaty_capacity;
} civ_diplomacy_system_t;

/* Function declarations */
civ_diplomacy_system_t *civ_diplomacy_system_create(void);
void civ_diplomacy_system_destroy(civ_diplomacy_system_t *ds);
void civ_diplomacy_system_init(civ_diplomacy_system_t *ds);

void civ_diplomacy_system_initialize_relations(civ_diplomacy_system_t *ds,
                                               const char **nation_ids,
                                               size_t nation_count);
civ_diplomatic_relation_t *
civ_diplomacy_system_get_relation(civ_diplomacy_system_t *ds,
                                  const char *nation_a, const char *nation_b);
void civ_diplomacy_system_update_relations(civ_diplomacy_system_t *ds,
                                           time_t current_date);
civ_result_t civ_diplomacy_system_propose_treaty(civ_diplomacy_system_t *ds,
                                                 const char *proposer,
                                                 const char *recipient,
                                                 civ_treaty_type_t type,
                                                 int32_t duration_days);

/* Conflict & Justification */
void civ_diplomacy_add_grievance(civ_diplomatic_relation_t *rel,
                                 civ_float_t amount, const char *reason);
bool civ_diplomacy_has_legitimate_war_goal(
    const civ_diplomatic_relation_t *rel);

#endif /* CIVILIZATION_RELATIONS_H */
