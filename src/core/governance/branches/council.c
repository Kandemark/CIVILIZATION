#include "core/governance/branches/council.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_council_t *civ_council_create(civ_council_type_t type, int max_members) {
  civ_council_t *c = CIV_MALLOC(sizeof(civ_council_t));
  if (!c) return NULL;
  memset(c, 0, sizeof(*c));
  c->type = type;
  c->max_members = max_members > 0 ? max_members : 7;
  c->member_capacity = c->max_members;
  c->members = CIV_MALLOC(sizeof(civ_council_member_t) * c->member_capacity);
  c->collective_strength = 0.50f;
  c->internal_cohesion = 0.65f;
  c->decision_speed = 0.40f;
  c->corruption_resistance = 0.55f;
  c->has_rotating_chair = (type == CIV_COUNCIL_FEDERAL);
  return c;
}

void civ_council_destroy(civ_council_t *c) {
  if (!c) return;
  free(c->members);
  free(c);
}

void civ_council_update(civ_council_t *c, float dt,
                        float centralization, float legitimacy,
                        float corruption, float tradition) {
  if (!c) return;

  /* Decision speed: larger councils are slower, cohesion helps */
  c->decision_speed = (1.0f / (c->member_count + 1)) * 3.0f
                      + c->internal_cohesion * 0.3f + centralization * 0.2f;
  if (c->decision_speed > 1.0f) c->decision_speed = 1.0f;
  if (c->decision_speed < 0.05f) c->decision_speed = 0.05f;

  /* Internal cohesion: tradition binds, corruption divides */
  float cohesion_target = tradition * 0.3f + legitimacy * 0.3f
                          + (1.0f - corruption) * 0.2f + 0.2f;
  c->internal_cohesion += (cohesion_target - c->internal_cohesion) * 0.05f * dt;

  /* Collective strength: member count * avg power * cohesion */
  c->collective_strength = 0.0f;
  for (int i = 0; i < c->member_count; i++)
    c->collective_strength += c->members[i].power_share * c->members[i].competence;
  c->collective_strength *= c->internal_cohesion;
  if (c->collective_strength > 1.0f) c->collective_strength = 1.0f;

  /* Corruption resistance: higher with more members (oversight), lower with low cohesion */
  c->corruption_resistance = (c->member_count / (float)c->max_members) * 0.4f
                             + c->internal_cohesion * 0.3f + (1.0f - corruption) * 0.3f;

  /* Chair rotation */
  if (c->has_rotating_chair && c->member_count > 0) {
    c->chair_rotation_turns++;
    if (c->chair_rotation_turns >= 20) {
      civ_council_rotate_chair(c);
      c->chair_rotation_turns = 0;
    }
  }
}

civ_council_member_t *civ_council_add_member(civ_council_t *c, const char *name,
                                              const char *npc_id, float power_share) {
  if (!c || !name || c->member_count >= c->member_capacity) return NULL;
  civ_council_member_t *m = &c->members[c->member_count];
  memset(m, 0, sizeof(*m));
  snprintf(m->id, STRING_SHORT_LEN, "CM_%d", c->member_count);
  strncpy(m->name, name, STRING_MEDIUM_LEN - 1);
  if (npc_id) strncpy(m->member_id, npc_id, STRING_SHORT_LEN - 1);
  m->power_share = power_share;
  m->competence = 0.40f + (float)(rand() % 40) / 100.0f;
  m->loyalty = 0.50f + (float)(rand() % 30) / 100.0f;
  c->member_count++;
  return m;
}

void civ_council_remove_member(civ_council_t *c, int index) {
  if (!c || index < 0 || index >= c->member_count) return;
  memmove(&c->members[index], &c->members[index + 1],
          (c->member_count - index - 1) * sizeof(civ_council_member_t));
  c->member_count--;
}

void civ_council_rotate_chair(civ_council_t *c) {
  if (!c || c->member_count == 0) return;
  c->current_chair_index = (c->current_chair_index + 1) % c->member_count;
}

float civ_council_decision_power(const civ_council_t *c) {
  return c ? (c->collective_strength * c->decision_speed) : 0.0f;
}

bool civ_council_can_act(const civ_council_t *c) {
  return c ? (c->internal_cohesion > 0.25f && c->member_count > 0) : false;
}
