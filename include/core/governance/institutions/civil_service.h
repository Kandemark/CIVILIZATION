/**
 * @file civil_service.h
 * @brief Civil service — career bureaucracy, merit-based vs patronage
 *
 * A merit-based civil service provides stable, competent administration
 * that persists through political transitions — but becomes rigid and
 * resistant to reform. A patronage system is politically responsive
 * and flexible — but corrupt and inefficient.
 *
 * Neither is "good." The tradeoff is competence vs control.
 * This is a core gameplay axis tied to the meritocracy trait.
 */
#ifndef CIV_GOVERNANCE_CIVIL_SERVICE_H
#define CIV_GOVERNANCE_CIVIL_SERVICE_H

#include "../../../common.h"
#include "../../../types.h"

typedef enum {
  CIV_CS_RECRUITMENT_EXAM,      /* competitive examination */
  CIV_CS_RECRUITMENT_APPOINTMENT, /* political appointment */
  CIV_CS_RECRUITMENT_PATRONAGE,   /* connection/family-based */
  CIV_CS_RECRUITMENT_HYBRID,      /* mixed exam + appointment */
  CIV_CS_RECRUITMENT_COUNT
} civ_cs_recruitment_t;

typedef struct {
  char   id[STRING_SHORT_LEN], department[STRING_SHORT_LEN];
  int    headcount;
  float  avg_competence;        /* 0.0–1.0 */
  float  avg_loyalty;           /* to the state vs to politicians */
  float  morale;                /* 0.0–1.0 */
  float  budget;
  float  output;                /* administrative output */
} civ_cs_department_t;

typedef struct {
  civ_cs_department_t *departments;
  int  dept_count, dept_capacity;
  civ_cs_recruitment_t recruitment_model;
  float total_employees;
  float avg_competence;           /* aggregate across all depts */
  float institutional_memory;     /* 0.0–1.0, knowledge preserved across transitions */
  float bureaucratic_rigidity;    /* 0.0–1.0, resistance to policy change */
  float corruption_vulnerability; /* 0.0–1.0 */
  float political_neutrality;     /* 0.0–1.0, serves the state vs serves the ruler */
  float budget_efficiency;        /* output per budget unit */
} civ_civil_service_t;

civ_civil_service_t *civ_civil_service_create(void);
void civ_civil_service_destroy(civ_civil_service_t *cs);
void civ_civil_service_update(civ_civil_service_t *cs, float dt,
                              float meritocracy_trait, float corruption,
                              float education_level, float total_budget,
                              float government_efficiency);

civ_cs_department_t *civ_civil_service_add_department(civ_civil_service_t *cs,
                                                       const char *name, int headcount);
void civ_civil_service_set_recruitment(civ_civil_service_t *cs,
                                       civ_cs_recruitment_t model);
float civ_civil_service_governance_buffer(const civ_civil_service_t *cs);
float civ_civil_service_reform_resistance(const civ_civil_service_t *cs);
float civ_civil_service_competence_bonus(const civ_civil_service_t *cs);

#endif
