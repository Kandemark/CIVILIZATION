/**
 * @file religion_system.h
 * @brief Emergent Religious System
 */

#ifndef CIVILIZATION_RELIGION_SYSTEM_H
#define CIVILIZATION_RELIGION_SYSTEM_H

#include "../../common.h"
#include "../../types.h"

/* Religious Tenet */
typedef enum {
  CIV_TENET_RITUALISM = 0,
  CIV_TENET_PROSELYTISM,
  CIV_TENET_ASCETICISM,
  CIV_TENET_MILITANCY,
  CIV_TENET_PACIFISM,
  CIV_TENET_SYMBOLISY
} civ_religion_tenet_t;

/* Religion Unit */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  char origin_culture_id[STRING_SHORT_LEN];

  civ_religion_tenet_t tenets[4];
  size_t tenet_count;

  civ_float_t fervor;       /* 0.0 to 1.0 - how intense followers are */
  civ_float_t global_reach; /* Percentage of world population */
  civ_float_t decline_rate; /* Natural loss of followers */
  bool is_extinct;

  time_t creation_time;
} civ_religion_t;

/* Religion Manager */
typedef struct {
  civ_religion_t *religions;
  size_t religion_count;
  size_t religion_capacity;
} civ_religion_system_t;

/* Functions */
civ_religion_system_t *civ_religion_system_create(void);
void civ_religion_system_destroy(civ_religion_system_t *system);

civ_religion_t *civ_religion_emerge(civ_religion_system_t *system,
                                    const char *name, const char *culture_id);
civ_result_t civ_religion_spread(civ_religion_t *religion,
                                 const char *target_region_id,
                                 civ_float_t rate);

#endif /* CIVILIZATION_RELIGION_SYSTEM_H */
