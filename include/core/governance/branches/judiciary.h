/**
 * @file judiciary.h
 * @brief Judiciary — courts, cases, judicial review, rule of law
 *
 * Independent judiciary is neither "good" nor "bad" — it provides
 * stability and rights protection but can block executive action
 * and entrench elite interests. A dependent judiciary enables fast
 * executive action but provides no rights guarantees.
 *
 * Every tradeoff is explicit. No moral judgment.
 */
#ifndef CIV_GOVERNANCE_JUDICIARY_H
#define CIV_GOVERNANCE_JUDICIARY_H

#include "../../../common.h"
#include "../../../types.h"

/* Court levels */
typedef enum { CIV_COURT_LOCAL, CIV_COURT_APPEALS, CIV_COURT_SUPREME,
               CIV_COURT_CONSTITUTIONAL, CIV_COURT_LEVEL_COUNT } civ_court_level_t;

/* Case types */
typedef enum { CIV_CASE_CIVIL, CIV_CASE_CRIMINAL, CIV_CASE_CONSTITUTIONAL,
               CIV_CASE_ADMINISTRATIVE, CIV_CASE_TYPE_COUNT } civ_case_type_t;

/* Case status */
typedef enum { CIV_CASE_FILED, CIV_CASE_HEARD, CIV_CASE_DECIDED,
               CIV_CASE_APPEALED, CIV_CASE_DISMISSED } civ_case_status_t;

typedef struct {
  char   id[STRING_SHORT_LEN], name[STRING_MEDIUM_LEN];
  civ_court_level_t level;
  int    judge_count, max_judges;
  float  independence;     /* 0.0 executive-controlled → 1.0 fully independent */
  float  efficiency;       /* case processing speed */
  float  backlog;          /* pending cases */
  float  public_trust;     /* 0.0–1.0 */
  char   appointing_body[STRING_SHORT_LEN]; /* who appoints judges */
} civ_court_t;

typedef struct {
  char   id[STRING_SHORT_LEN], title[STRING_MEDIUM_LEN];
  civ_case_type_t type;
  civ_case_status_t status;
  float  complexity;       /* harder cases take longer */
  float  government_interest; /* how much the executive cares about outcome */
  int    age_days;         /* how long pending */
  char   ruling_summary[STRING_MEDIUM_LEN];
  bool   struck_down_law;  /* judicial review result */
  char   affected_rule_id[STRING_SHORT_LEN];
} civ_case_t;

typedef struct {
  civ_court_t  *courts;
  int           court_count, court_capacity;
  civ_case_t   *cases;
  int           case_count, case_capacity;
  float         rule_of_law;          /* 0.0–1.0 composite */
  float         judicial_independence; /* aggregate */
  float         due_process_index;
  int           laws_struck_down;     /* lifetime count */
  int           precedents_set;
} civ_judiciary_t;

civ_judiciary_t *civ_judiciary_create(void);
void civ_judiciary_destroy(civ_judiciary_t *j);
void civ_judiciary_update(civ_judiciary_t *j, float dt,
                          float gov_efficiency, float corruption,
                          float centralization, float representation);

civ_court_t *civ_judiciary_create_court(civ_judiciary_t *j, const char *name,
                                        civ_court_level_t level, int max_judges);
civ_case_t *civ_judiciary_file_case(civ_judiciary_t *j, civ_case_type_t type,
                                    const char *title, float complexity);
void civ_judiciary_process_docket(civ_judiciary_t *j, float dt);
bool civ_judiciary_can_strike_law(const civ_judiciary_t *j);
float civ_judiciary_rights_protection(const civ_judiciary_t *j);
float civ_judiciary_executive_constraint(const civ_judiciary_t *j);

#endif
