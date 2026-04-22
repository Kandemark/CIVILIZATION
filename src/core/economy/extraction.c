#include "core/economy/extraction.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CIV_EXTRACT_INITIAL_SITE_CAP 16

civ_extraction_system_t *civ_extraction_create(void) {
  civ_extraction_system_t *e = CIV_MALLOC(sizeof(civ_extraction_system_t));
  if (!e) return NULL;
  memset(e, 0, sizeof(*e));
  e->site_capacity = CIV_EXTRACT_INITIAL_SITE_CAP;
  e->sites = CIV_MALLOC(sizeof(civ_extraction_site_t) * e->site_capacity);
  e->sustainability_index = 0.80;
  return e;
}

void civ_extraction_destroy(civ_extraction_system_t *e) {
  if (!e) return;
  free(e->sites);
  free(e);
}

void civ_extraction_update(civ_extraction_system_t *e, civ_float_t time_delta,
                           civ_float_t tech_level, civ_float_t labor_available,
                           civ_float_t geography_mineral_richness) {
  if (!e) return;
  e->total_output = 0.0;

  civ_float_t depletion_sum = 0.0;
  for (int i = 0; i < e->site_count; i++) {
    civ_extraction_site_t *s = &e->sites[i];
    if (s->depleted) continue;

    /* Efficiency from tech and labor */
    s->efficiency = 0.3 + tech_level * 0.5 + (labor_available > 1000 ? 0.2 : 0.0);
    if (s->efficiency > 1.0) s->efficiency = 1.0;

    /* Extraction rate: deposit * efficiency * mineral richness */
    s->extraction_rate = s->deposit_size * s->efficiency * 0.02
                         * geography_mineral_richness * time_delta * 0.1;

    /* Deplete the deposit */
    s->deposit_size -= s->extraction_rate;
    if (s->deposit_size <= 0) {
      s->deposit_size = 0;
      s->depleted = true;
      s->extraction_rate = 0;
    }

    s->depletion_factor = (s->deposit_size > 0)
      ? s->extraction_rate / s->deposit_size : 0.0;
    depletion_sum += s->depletion_factor;
    e->total_output += s->extraction_rate;
  }

  /* Sustainability: worsens with depletion, improves with renewable practices */
  civ_float_t avg_depletion = (e->site_count > 0) ? depletion_sum / e->site_count : 0.0;
  e->sustainability_index += ((1.0 - avg_depletion * 10.0) - e->sustainability_index) * 0.05;
  if (e->sustainability_index < 0.0) e->sustainability_index = 0.0;
  if (e->sustainability_index > 1.0) e->sustainability_index = 1.0;
}

civ_extraction_site_t *civ_extraction_add_site(civ_extraction_system_t *e,
                                               civ_extraction_type_t type,
                                               const char *resource,
                                               civ_float_t deposit_size) {
  if (!e || !resource) return NULL;
  if (e->site_count >= e->site_capacity) {
    int nc = e->site_capacity * 2;
    civ_extraction_site_t *tmp = CIV_REALLOC(e->sites, sizeof(civ_extraction_site_t) * nc);
    if (!tmp) return NULL;
    e->sites = tmp;
    e->site_capacity = nc;
  }
  civ_extraction_site_t *s = &e->sites[e->site_count];
  memset(s, 0, sizeof(*s));
  s->type = type;
  strncpy(s->resource, resource, STRING_SHORT_LEN - 1);
  s->deposit_size = deposit_size;
  e->site_count++;
  return s;
}

civ_float_t civ_extraction_output_for(const civ_extraction_system_t *e, civ_extraction_type_t type) {
  if (!e) return 0.0;
  civ_float_t out = 0.0;
  for (int i = 0; i < e->site_count; i++)
    if (e->sites[i].type == type) out += e->sites[i].extraction_rate;
  return out;
}

civ_float_t civ_extraction_depletion_rate(const civ_extraction_system_t *e) {
  if (!e || e->site_count == 0) return 0.0;
  civ_float_t sum = 0.0;
  for (int i = 0; i < e->site_count; i++) sum += e->sites[i].depletion_factor;
  return sum / e->site_count;
}
