/**
 * @file religious_body.h
 * @brief Religious governance — theocracies, priestly councils, divine authority
 *
 * Religious bodies can replace or supplement secular branches. A College
 * of Cardinals selecting a Pope, a Council of Guardians vetting laws,
 * a High Priesthood ruling by divine mandate — all are religious governance.
 *
 * No configuration is "bad": religious authority provides moral legitimacy
 * and cultural cohesion, but may suppress scientific inquiry and pluralism.
 * A secular state lacks this cohesion but enables freer thought.
 */
#ifndef CIV_GOVERNANCE_RELIGIOUS_BODY_H
#define CIV_GOVERNANCE_RELIGIOUS_BODY_H

#include "../../../common.h"
#include "../../../types.h"

typedef enum {
  CIV_RELIGIOUS_HIERARCHY,   /* Pope/Patriarch/Caliph with clerical hierarchy */
  CIV_RELIGIOUS_COUNCIL,     /* College/Assembly of religious scholars */
  CIV_RELIGIOUS_GUARDIAN,    /* Council of Guardians — veto power over secular law */
  CIV_RELIGIOUS_ORACULAR,    /* Prophet/oracle — divine revelation as law */
  CIV_RELIGIOUS_TYPE_COUNT
} civ_religious_body_type_t;

typedef struct {
  char   id[STRING_SHORT_LEN], title[STRING_MEDIUM_LEN];
  char   npc_id[STRING_SHORT_LEN];
  float  piety;           /* religious standing 0.0–1.0 */
  float  political_power; /* influence over secular affairs */
  int    followers;       /* size of their religious following */
} civ_religious_leader_t;

typedef struct {
  civ_religious_body_type_t type;
  civ_religious_leader_t   *leaders;
  int  leader_count, leader_capacity;
  float spiritual_authority;     /* 0.0–1.0, moral/religious weight */
  float political_influence;     /* 0.0–1.0, power over secular governance */
  float doctrinal_rigidity;      /* 0.0–1.0, resistance to reform */
  float cultural_cohesion_bonus; /* how much this unifies the population */
  float scientific_penalty;      /* drag on research from dogma */
  bool  can_veto_secular_laws;   /* Guardian/Veto power */
  bool  can_appoint_leaders;     /* e.g., Pope appoints bishops */
  char  dominant_faith[STRING_SHORT_LEN];
} civ_religious_body_t;

civ_religious_body_t *civ_religious_body_create(civ_religious_body_type_t type,
                                                 const char *dominant_faith);
void civ_religious_body_destroy(civ_religious_body_t *r);
void civ_religious_body_update(civ_religious_body_t *r, float dt,
                               float religious_authority_trait,
                               float tradition, float representation,
                               float corruption);

civ_religious_leader_t *civ_religious_body_add_leader(civ_religious_body_t *r,
                                                       const char *title,
                                                       const char *npc_id,
                                                       float piety);
float civ_religious_body_moral_legitimacy(const civ_religious_body_t *r);
float civ_religious_body_secular_constraint(const civ_religious_body_t *r);
bool civ_religious_body_veto_law(const civ_religious_body_t *r, const char *law_id);

#endif
