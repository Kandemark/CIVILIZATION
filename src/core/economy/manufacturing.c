#include "core/economy/manufacturing.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CIV_MFG_INITIAL_FACTORY_CAP 16

civ_manufacturing_system_t *civ_manufacturing_create(void) {
  civ_manufacturing_system_t *m = CIV_MALLOC(sizeof(civ_manufacturing_system_t));
  if (!m) return NULL;
  memset(m, 0, sizeof(*m));
  m->factory_capacity = CIV_MFG_INITIAL_FACTORY_CAP;
  m->factories = CIV_MALLOC(sizeof(civ_factory_t) * m->factory_capacity);
  m->capacity_utilization = 0.70;
  m->automation_level = 0.15;
  m->supply_chain_efficiency = 0.60;
  return m;
}

void civ_manufacturing_destroy(civ_manufacturing_system_t *m) {
  if (!m) return;
  free(m->factories);
  free(m);
}

void civ_manufacturing_update(civ_manufacturing_system_t *m, civ_float_t time_delta,
                              civ_float_t tech_level, civ_float_t labor_available,
                              civ_float_t raw_materials_available, civ_float_t infrastructure_quality) {
  if (!m) return;
  (void)time_delta;

  m->total_industrial_output = 0.0;
  m->supply_chain_efficiency += (infrastructure_quality - m->supply_chain_efficiency) * 0.1;

  /* Automation rises with tech, reducing labor per unit */
  m->automation_level += (tech_level * 0.5 - m->automation_level) * 0.05;

  int total_workers_assigned = 0;
  int total_capacity = 0;

  for (int i = 0; i < m->factory_count; i++) {
    civ_factory_t *f = &m->factories[i];
    if (!f->active) continue;

    /* Workers limited by labor availability */
    int max_workers = f->capacity;
    if (labor_available > 0 && total_workers_assigned < (int)labor_available) {
      int unassigned = (int)labor_available - total_workers_assigned;
      f->workers = (unassigned < max_workers) ? unassigned : max_workers;
      total_workers_assigned += f->workers;
    } else {
      f->workers = 0;
    }

    /* Output: workers * productivity * supply_chain * automation bonus */
    civ_float_t auto_bonus = 1.0 + m->automation_level * 0.5;
    /* Raw material constraint */
    civ_float_t raw_per_factory = (m->factory_count > 0 && raw_materials_available > 0)
      ? raw_materials_available / m->factory_count : 1.0;
    civ_float_t raw_eff = (raw_per_factory > f->raw_material_ratio)
      ? 1.0 : raw_per_factory / (f->raw_material_ratio + 0.001);

    f->output_per_worker = (1.0 + tech_level * 0.8) * auto_bonus
                           * m->supply_chain_efficiency * raw_eff;
    civ_float_t factory_output = f->workers * f->output_per_worker;
    m->total_industrial_output += factory_output;
    total_capacity += f->capacity;
  }

  m->capacity_utilization = (total_capacity > 0)
    ? (civ_float_t)total_workers_assigned / (civ_float_t)total_capacity : 0.0;
}

civ_factory_t *civ_manufacturing_add_factory(civ_manufacturing_system_t *m,
                                             const char *product, int capacity) {
  if (!m || !product) return NULL;
  if (m->factory_count >= m->factory_capacity) {
    int nc = m->factory_capacity * 2;
    civ_factory_t *tmp = CIV_REALLOC(m->factories, sizeof(civ_factory_t) * nc);
    if (!tmp) return NULL;
    m->factories = tmp;
    m->factory_capacity = nc;
  }
  civ_factory_t *f = &m->factories[m->factory_count];
  memset(f, 0, sizeof(*f));
  snprintf(f->id, STRING_SHORT_LEN, "FACTORY_%d", m->factory_count);
  strncpy(f->product, product, STRING_SHORT_LEN - 1);
  f->capacity = capacity;
  f->active = true;
  f->capital_intensity = 0.3;
  f->raw_material_ratio = 1.0;
  m->factory_count++;
  return f;
}

void civ_manufacturing_set_workers(civ_factory_t *f, int workers) {
  if (!f || workers < 0 || workers > f->capacity) return;
  f->workers = workers;
}

civ_float_t civ_manufacturing_productivity(const civ_manufacturing_system_t *m) {
  if (!m || m->factory_count == 0) return 0.0;
  return m->total_industrial_output / (civ_float_t)m->factory_count;
}
