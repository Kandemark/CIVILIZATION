/**
 * @file council.h
 * @brief Collective governance — councils, juntas, presidiums, directorates
 *
 * Replaces or supplements the executive in governance forms where power
 * is held by a group rather than an individual. A Swiss Federal Council,
 * a Soviet Presidium, a military junta, a tribal council of elders —
 * all are council-based governance.
 *
 * No council type is "bad": collective leadership prevents tyranny but
 * can be slow. A junta provides decisive military command but excludes
 * civilian input. A tribal council preserves tradition but resists reform.
 */
#ifndef CIV_GOVERNANCE_COUNCIL_H
#define CIV_GOVERNANCE_COUNCIL_H

#include "../../../common.h"
#include "../../../types.h"

typedef enum {
  CIV_COUNCIL_FEDERAL,      /* Swiss-style: equals among equals, rotating chair */
  CIV_COUNCIL_PRESIDIUM,    /* Soviet-style: party/state committee, strong chair */
  CIV_COUNCIL_JUNTA,        /* Military committee, ranked by rank */
  CIV_COUNCIL_ELDERS,       /* Tribal/clan elders, consensus-based */
  CIV_COUNCIL_OLIGARCHIC,   /* Wealth/power-based council, merchant princes */
  CIV_COUNCIL_TECHNOCRATIC, /* Expert/appointed council, credentials matter */
  CIV_COUNCIL_TYPE_COUNT
} civ_council_type_t;

typedef struct {
  char   id[STRING_SHORT_LEN], name[STRING_MEDIUM_LEN];
  char   member_id[STRING_SHORT_LEN];  /* NPC ID */
  float  power_share;       /* fraction of council's total power */
  float  competence;
  float  loyalty;
  int    tenure;
} civ_council_member_t;

typedef struct {
  civ_council_type_t    type;
  civ_council_member_t *members;
  int  member_count, member_capacity, max_members;
  float collective_strength;   /* 0.0–1.0, replaces executive_strength */
  float internal_cohesion;     /* 0.0–1.0, factionalism when low */
  float decision_speed;        /* 0.0–1.0, slower than individual executive */
  float corruption_resistance; /* collective oversight can help or hurt */
  bool  has_rotating_chair;
  int   chair_rotation_turns;
  int   current_chair_index;
} civ_council_t;

civ_council_t *civ_council_create(civ_council_type_t type, int max_members);
void civ_council_destroy(civ_council_t *c);
void civ_council_update(civ_council_t *c, float dt,
                        float centralization, float legitimacy,
                        float corruption, float tradition);

civ_council_member_t *civ_council_add_member(civ_council_t *c, const char *name,
                                              const char *npc_id, float power_share);
void civ_council_remove_member(civ_council_t *c, int index);
void civ_council_rotate_chair(civ_council_t *c);
float civ_council_decision_power(const civ_council_t *c);
bool civ_council_can_act(const civ_council_t *c);  /* false when deadlocked */

#endif
