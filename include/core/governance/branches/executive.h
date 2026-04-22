/**
 * @file executive.h
 * @brief Executive branch — veto, orders, decrees, cabinet, appointments
 *
 * A strong executive enables decisive action in crises but concentrates
 * power. A weak executive ensures deliberation but risks gridlock.
 * Neither is "good" — the optimal balance depends on circumstances.
 */
#ifndef CIV_GOVERNANCE_EXECUTIVE_H
#define CIV_GOVERNANCE_EXECUTIVE_H

#include "../../../common.h"
#include "../../../types.h"

/* Executive action types */
typedef enum { CIV_EXEC_ORDER, CIV_EXEC_DECREE, CIV_EXEC_DIRECTIVE,
               CIV_EXEC_APPOINTMENT, CIV_EXEC_PARDON, CIV_EXEC_TYPE_COUNT } civ_exec_action_t;

/* Action status */
typedef enum { CIV_EXEC_DRAFT, CIV_EXEC_ISSUED, CIV_EXEC_CHALLENGED,
               CIV_EXEC_UPHELD, CIV_EXEC_STRUCK_DOWN, CIV_EXEC_EXPIRED } civ_exec_status_t;

typedef struct {
  char   id[STRING_SHORT_LEN], title[STRING_MEDIUM_LEN];
  civ_exec_action_t type;
  civ_exec_status_t status;
  float  power_cost;        /* how much political capital this costs */
  float  effect_magnitude;  /* 0.0–1.0 */
  int    turns_remaining;   /* for temporary orders */
  bool   is_permanent;
  char   target[STRING_SHORT_LEN]; /* what this affects */
} civ_executive_action_t;

/* Veto tracking */
typedef struct {
  int   vetoes_issued;
  int   vetoes_overridden;
  int   vetoes_sustained;
  float veto_power;         /* 0.0–1.0, how hard veto is to override */
  int   bills_awaiting_signature;
} civ_veto_state_t;

/* Executive branch */
typedef struct {
  civ_executive_action_t *actions;
  int action_count, action_capacity;
  civ_veto_state_t veto;
  float executive_strength;     /* composite 0.0–1.0 */
  float political_capital;      /* 0.0–1.0, spent on actions */
  float decree_power;           /* ability to bypass legislature */
  float power_consolidation;    /* 0.0–1.0, deliberate power accumulation */
  bool  constitution_suspended; /* ruler has suspended constitutional protections */
  int   turns_suspended;        /* how long constitution has been suspended */
  int   appointments_made;
  int   pardons_issued;
  bool  can_issue_decrees;      /* enabled by constitution or emergency */
} civ_executive_t;

civ_executive_t *civ_executive_create(void);
void civ_executive_destroy(civ_executive_t *e);
void civ_executive_update(civ_executive_t *e, float dt,
                          float centralization, float legitimacy,
                          float emergency_power, float representation);

civ_executive_action_t *civ_executive_issue_order(civ_executive_t *e,
                                                   civ_exec_action_t type,
                                                   const char *title, float power_cost);
civ_executive_action_t *civ_executive_issue_decree(civ_executive_t *e,
                                                    const char *title, float effect);
bool civ_executive_veto_bill(civ_executive_t *e, const char *bill_id);
bool civ_executive_override_possible(const civ_executive_t *e, float legislative_support);
void civ_executive_sign_bill(civ_executive_t *e);
void civ_executive_process_actions(civ_executive_t *e, float dt);

float civ_executive_power_index(const civ_executive_t *e);

/* ── Constitution suspension / power consolidation ──────────────── */
bool  civ_executive_suspend_constitution(civ_executive_t *e);
bool  civ_executive_restore_constitution(civ_executive_t *e);
bool  civ_executive_is_constitution_suspended(const civ_executive_t *e);
float civ_executive_power_consolidation(const civ_executive_t *e);
void  civ_executive_consolidate_power(civ_executive_t *e, float amount);

/* Track: has the ruler accumulated enough power to ignore checks? */
bool  civ_executive_can_rule_by_decree(const civ_executive_t *e);
bool  civ_executive_is_autocratic_takeover(const civ_executive_t *e);

#endif
