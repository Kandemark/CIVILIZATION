/**
 * @file political_violence.h
 * @brief Political violence — coups, assassinations, civil war, purges
 *
 * Models the dark side of governance without moralizing. Violence
 * emerges from conditions, not because any system is "evil."
 * High militarization + low legitimacy = coup risk.
 * High corruption + low stability = assassination risk.
 * Extreme centralization + ethnic divisions = civil war risk.
 * A ruler with consolidated power can purge opposition — with
 * consequences for competence, legitimacy, and international standing.
 *
 * These events happen autonomously to ALL nations, not just the player's.
 */
#ifndef CIV_GOVERNANCE_POLITICAL_VIOLENCE_H
#define CIV_GOVERNANCE_POLITICAL_VIOLENCE_H

#include "../../../common.h"
#include "../../../types.h"

typedef enum {
  CIV_VIOLENCE_COUP_ATTEMPT,
  CIV_VIOLENCE_ASSASSINATION,
  CIV_VIOLENCE_CIVIL_WAR,
  CIV_VIOLENCE_PURGE,
  CIV_VIOLENCE_INSURRECTION,
  CIV_VIOLENCE_TYPE_COUNT
} civ_violence_type_t;

typedef struct {
  civ_violence_type_t type;
  bool  occurred;
  float severity;         /* 0.0–1.0 */
  float stability_damage;
  float legitimacy_damage;
  int   casualties;
  int   turns_since_event;
  char  description[STRING_MAX_LEN];
} civ_violence_event_t;

typedef struct {
  civ_violence_event_t *history;
  int  history_count, history_capacity;
  float coup_risk;           /* 0.0–1.0 */
  float assassination_risk;  /* 0.0–1.0 */
  float civil_war_risk;      /* 0.0–1.0 */
  float insurrection_risk;   /* 0.0–1.0 */
  float political_repression; /* 0.0–1.0, state violence against citizens */
  int   total_casualties;
  int   coups_attempted;
  int   coups_succeeded;
} civ_political_violence_t;

civ_political_violence_t *civ_political_violence_create(void);
void civ_political_violence_destroy(civ_political_violence_t *pv);
void civ_political_violence_update(civ_political_violence_t *pv, float dt,
                                   float militarization, float centralization,
                                   float legitimacy, float stability,
                                   float corruption, float representation,
                                   float power_consolidation, bool constitution_suspended,
                                   float faction_count, float cohesion);

/* Query */
bool  civ_political_violence_coup_possible(const civ_political_violence_t *pv);
bool  civ_political_violence_civil_war_possible(const civ_political_violence_t *pv);
float civ_political_violence_instability_index(const civ_political_violence_t *pv);

/* Actions (these are autonomous — no player triggers them) */
void  civ_political_violence_attempt_coup(civ_political_violence_t *pv);
void  civ_political_violence_trigger_assassination(civ_political_violence_t *pv);
void  civ_political_violence_trigger_civil_war(civ_political_violence_t *pv);
void  civ_political_violence_trigger_purge(civ_political_violence_t *pv, int target_population);

#endif
