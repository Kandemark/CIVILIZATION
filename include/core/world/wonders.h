/**
 * @file wonders.h
 * @brief Wonders and Great Works system
 */

#ifndef CIVILIZATION_WONDERS_H
#define CIVILIZATION_WONDERS_H

#include "../../common.h"
#include "../../types.h"

/* Wonder Types */
typedef enum {
  CIV_WONDER_PYRAMIDS = 0,
  CIV_WONDER_GREAT_LIBRARY,
  CIV_WONDER_COLOSSUS,
  CIV_WONDER_HANGING_GARDENS,
  CIV_WONDER_ORACLE,
  CIV_WONDER_LIGHTHOUSE,
  CIV_WONDER_COUNT
} civ_wonder_type_t;

/* Wonder Effects */
typedef struct {
  civ_float_t science_mult;
  civ_float_t culture_mult;
  civ_float_t production_mult;
  civ_float_t gold_mult;
  civ_float_t military_str_bonus;
} civ_wonder_effects_t;

/* Wonder Structure */
typedef struct {
  civ_wonder_type_t type;
  char name[STRING_MEDIUM_LEN];
  char description[STRING_MAX_LEN];

  civ_float_t production_cost;
  char required_tech_id[STRING_SHORT_LEN];
  bool requires_coast;

  civ_wonder_effects_t effects;

  bool is_built;
  char builder_id[STRING_SHORT_LEN]; /* ID of settlement or nation */
} civ_wonder_t;

/* Wonder Manager */
typedef struct {
  civ_wonder_t wonders[CIV_WONDER_COUNT];
} civ_wonder_manager_t;

/* Functions */
civ_wonder_manager_t *civ_wonder_manager_create(void);
void civ_wonder_manager_destroy(civ_wonder_manager_t *manager);
void civ_wonder_manager_init(civ_wonder_manager_t *manager);

bool civ_wonder_can_build(const civ_wonder_manager_t *manager,
                          civ_wonder_type_t type, const char *tech_id,
                          bool is_coastal);
void civ_wonder_mark_built(civ_wonder_manager_t *manager,
                           civ_wonder_type_t type, const char *builder_id);

civ_wonder_effects_t
civ_wonder_calculate_global_bonuses(const civ_wonder_manager_t *manager,
                                    const char *owner_id);

#endif /* CIVILIZATION_WONDERS_H */
