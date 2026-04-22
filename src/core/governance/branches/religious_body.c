#include "core/governance/branches/religious_body.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_religious_body_t *civ_religious_body_create(civ_religious_body_type_t type,
                                                 const char *dominant_faith) {
  civ_religious_body_t *r = CIV_MALLOC(sizeof(civ_religious_body_t));
  if (!r) return NULL;
  memset(r, 0, sizeof(*r));
  r->type = type;
  if (dominant_faith) strncpy(r->dominant_faith, dominant_faith, STRING_SHORT_LEN - 1);
  r->leader_capacity = 4;
  r->leaders = CIV_MALLOC(sizeof(civ_religious_leader_t) * r->leader_capacity);
  r->spiritual_authority = 0.60f;
  r->political_influence = 0.30f;
  r->doctrinal_rigidity = 0.55f;
  r->cultural_cohesion_bonus = 0.15f;
  r->scientific_penalty = 0.10f;
  r->can_veto_secular_laws = (type == CIV_RELIGIOUS_GUARDIAN);
  r->can_appoint_leaders = (type == CIV_RELIGIOUS_HIERARCHY);
  return r;
}

void civ_religious_body_destroy(civ_religious_body_t *r) {
  if (!r) return;
  free(r->leaders);
  free(r);
}

void civ_religious_body_update(civ_religious_body_t *r, float dt,
                               float religious_authority_trait,
                               float tradition, float representation,
                               float corruption) {
  if (!r) return;

  /* Spiritual authority from trait + tradition */
  r->spiritual_authority += (religious_authority_trait * 0.5f + tradition * 0.3f
                             - r->spiritual_authority) * 0.03f * dt;

  /* Political influence: spiritual authority translated into secular power */
  r->political_influence = r->spiritual_authority * 0.7f
                           - representation * 0.2f + tradition * 0.1f;
  if (r->political_influence < 0.0f) r->political_influence = 0.0f;
  if (r->political_influence > 0.90f) r->political_influence = 0.90f;

  /* Doctrinal rigidity: tradition + authority, reduced by representation */
  r->doctrinal_rigidity += (tradition * 0.5f + r->spiritual_authority * 0.3f
                            - representation * 0.2f - r->doctrinal_rigidity) * 0.04f * dt;
  if (r->doctrinal_rigidity < 0.10f) r->doctrinal_rigidity = 0.10f;
  if (r->doctrinal_rigidity > 0.95f) r->doctrinal_rigidity = 0.95f;

  /* Cohesion bonus: unifies population around shared faith */
  r->cultural_cohesion_bonus = r->spiritual_authority * 0.25f
                               + r->doctrinal_rigidity * 0.10f;

  /* Scientific penalty: dogma constrains inquiry */
  r->scientific_penalty = r->doctrinal_rigidity * 0.25f
                          + r->political_influence * 0.10f;

  /* Veto power for Guardian types */
  r->can_veto_secular_laws = (r->type == CIV_RELIGIOUS_GUARDIAN)
                             && r->political_influence > 0.30f;

  /* Corruption affects religious institutions too */
  r->spiritual_authority -= corruption * 0.02f * dt;
  if (r->spiritual_authority < 0.05f) r->spiritual_authority = 0.05f;

  /* Update leader piety */
  for (int i = 0; i < r->leader_count; i++) {
    r->leaders[i].piety += (r->spiritual_authority - r->leaders[i].piety) * 0.02f * dt;
    /* Corrupt leaders lose piety */
    r->leaders[i].piety -= corruption * (1.0f - r->leaders[i].piety) * 0.01f * dt;
  }
}

civ_religious_leader_t *civ_religious_body_add_leader(civ_religious_body_t *r,
                                                       const char *title,
                                                       const char *npc_id,
                                                       float piety) {
  if (!r || !title || r->leader_count >= r->leader_capacity) return NULL;
  civ_religious_leader_t *l = &r->leaders[r->leader_count];
  memset(l, 0, sizeof(*l));
  snprintf(l->id, STRING_SHORT_LEN, "RL_%d", r->leader_count);
  strncpy(l->title, title, STRING_MEDIUM_LEN - 1);
  if (npc_id) strncpy(l->npc_id, npc_id, STRING_SHORT_LEN - 1);
  l->piety = piety;
  l->political_power = piety * 0.5f;
  l->followers = 1000 + (int)(piety * 10000);
  r->leader_count++;
  return l;
}

float civ_religious_body_moral_legitimacy(const civ_religious_body_t *r) {
  return r ? r->spiritual_authority : 0.0f;
}

float civ_religious_body_secular_constraint(const civ_religious_body_t *r) {
  return r ? r->political_influence : 0.0f;
}

bool civ_religious_body_veto_law(const civ_religious_body_t *r, const char *law_id) {
  (void)law_id;
  return r ? r->can_veto_secular_laws : false;
}
