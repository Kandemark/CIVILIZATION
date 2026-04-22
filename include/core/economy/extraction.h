/**
 * @file extraction.h
 * @brief Resource extraction — mining, logging, fishing, hunting, raw materials
 */
#ifndef CIV_ECONOMY_EXTRACTION_H
#define CIV_ECONOMY_EXTRACTION_H

#include "../../common.h"
#include "../../types.h"

typedef enum { CIV_EXTRACT_MINING, CIV_EXTRACT_LOGGING, CIV_EXTRACT_FISHING,
               CIV_EXTRACT_HUNTING, CIV_EXTRACT_QUARRYING, CIV_EXTRACT_TYPE_COUNT } civ_extraction_type_t;

typedef struct {
  civ_extraction_type_t type;
  char                  resource[STRING_SHORT_LEN];
  civ_float_t           deposit_size;       /* remaining resource */
  civ_float_t           extraction_rate;    /* per cycle output */
  civ_float_t           depletion_factor;   /* how fast it runs out */
  civ_float_t           efficiency;         /* 0.0-1.0 */
  bool                  depleted;
} civ_extraction_site_t;

typedef struct {
  civ_extraction_site_t *sites;
  int                    site_count;
  int                    site_capacity;
  civ_float_t            total_output;      /* aggregate raw material units */
  civ_float_t            sustainability_index; /* 0.0-1.0 */
} civ_extraction_system_t;

civ_extraction_system_t *civ_extraction_create(void);
void civ_extraction_destroy(civ_extraction_system_t *e);
void civ_extraction_update(civ_extraction_system_t *e, civ_float_t time_delta,
                           civ_float_t tech_level, civ_float_t labor_available,
                           civ_float_t geography_mineral_richness);

civ_extraction_site_t *civ_extraction_add_site(civ_extraction_system_t *e,
                                               civ_extraction_type_t type,
                                               const char *resource,
                                               civ_float_t deposit_size);
civ_float_t civ_extraction_output_for(const civ_extraction_system_t *e, civ_extraction_type_t type);
civ_float_t civ_extraction_depletion_rate(const civ_extraction_system_t *e);

#endif
