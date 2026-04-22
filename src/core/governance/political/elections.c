/**
 * @file elections.c
 * @brief Elections — voting, campaigns, autonomous electoral cycles
 */
#include "core/governance/political/elections.h"
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_election_system_t *civ_election_create(void) {
  civ_election_system_t *es = CIV_MALLOC(sizeof(civ_election_system_t));
  if (!es) return NULL;
  memset(es, 0, sizeof(*es));
  es->election_capacity = 4;
  es->elections = CIV_MALLOC(sizeof(civ_election_t) * es->election_capacity);
  es->democratic_health = 0.50f;
  es->voter_trust = 0.55f;
  es->electoral_fairness = 0.60f;
  return es;
}

void civ_election_destroy(civ_election_system_t *es) {
  if (!es) return;
  for (int i = 0; i < es->election_count; i++)
    free(es->elections[i].candidates);
  free(es->elections);
  free(es);
}

void civ_election_update(civ_election_system_t *es, float dt,
                         int total_population, float corruption,
                         float representation, float citizen_happiness,
                         float economic_confidence, float literacy_rate) {
  if (!es) return;

  /* Democratic health: composite of representation + trust + fairness */
  es->democratic_health = representation * 0.35f + es->voter_trust * 0.25f
                          + es->electoral_fairness * 0.25f + citizen_happiness * 0.15f;

  /* Voter trust: eroded by corruption, boosted by fairness */
  float trust_target = es->electoral_fairness * 0.5f + citizen_happiness * 0.3f
                       + (1.0f - corruption) * 0.2f;
  es->voter_trust += (trust_target - es->voter_trust) * 0.04f * dt;

  /* Electoral fairness: representation + low corruption */
  es->electoral_fairness = representation * 0.4f + (1.0f - corruption) * 0.4f + 0.2f;
  if (es->electoral_fairness > 1.0f) es->electoral_fairness = 1.0f;

  /* Process each election */
  for (int i = 0; i < es->election_count; i++) {
    civ_election_t *e = &es->elections[i];
    if (!e->in_progress || e->concluded) continue;

    e->turns_until_election--;
    e->eligible_voters = (int)(total_population * 0.6f * representation);

    /* Voter engagement from economic confidence + literacy + happiness */
    e->voter_engagement = economic_confidence * 0.25f + literacy_rate * 0.25f
                          + citizen_happiness * 0.25f + es->voter_trust * 0.15f
                          + (1.0f - corruption) * 0.10f;
    if (e->voter_engagement < 0.15f) e->voter_engagement = 0.15f;
    if (e->voter_engagement > 0.95f) e->voter_engagement = 0.95f;

    /* Campaign: candidates build support */
    civ_election_campaign(e, dt, literacy_rate, es->electoral_fairness);

    /* Election day */
    if (e->turns_until_election <= 0) {
      e->turnout = e->voter_engagement * (0.7f + es->electoral_fairness * 0.3f);
      civ_election_tally_votes(e);
      e->concluded = true;
      es->total_elections_held++;
    }
  }
}

civ_election_t *civ_election_schedule(civ_election_system_t *es,
                                       const char *position_id,
                                       const char *body_name,
                                       civ_election_type_t type,
                                       int seats, int cycle_length) {
  if (!es || !position_id) return NULL;
  if (es->election_count >= es->election_capacity) {
    int nc = es->election_capacity * 2;
    civ_election_t *tmp = CIV_REALLOC(es->elections, sizeof(civ_election_t) * nc);
    if (!tmp) return NULL;
    es->elections = tmp;
    es->election_capacity = nc;
  }
  civ_election_t *e = &es->elections[es->election_count];
  memset(e, 0, sizeof(*e));
  strncpy(e->position_id, position_id, STRING_SHORT_LEN - 1);
  if (body_name) strncpy(e->body_name, body_name, STRING_MEDIUM_LEN - 1);
  e->type = type;
  e->total_seats = seats;
  e->cycle_length = cycle_length;
  e->turns_until_election = cycle_length;
  e->in_progress = true;
  e->candidate_capacity = 8;
  e->candidates = CIV_MALLOC(sizeof(civ_candidate_t) * e->candidate_capacity);
  es->election_count++;
  return e;
}

civ_candidate_t *civ_election_add_candidate(civ_election_t *e,
                                             const char *name, const char *npc_id,
                                             const char *party_id, bool incumbent) {
  if (!e || !name || e->candidate_count >= e->candidate_capacity) return NULL;
  civ_candidate_t *c = &e->candidates[e->candidate_count];
  memset(c, 0, sizeof(*c));
  snprintf(c->id, STRING_SHORT_LEN, "CAND_%d", e->candidate_count);
  strncpy(c->name, name, STRING_MEDIUM_LEN - 1);
  if (npc_id) strncpy(c->npc_id, npc_id, STRING_SHORT_LEN - 1);
  if (party_id) strncpy(c->party_id, party_id, STRING_SHORT_LEN - 1);
  c->is_incumbent = incumbent;
  /* Dynamic initial support — incumbents start with advantage */
  c->public_support = incumbent ? (0.35f + (float)(rand() % 20) / 100.0f)
                                : (0.10f + (float)(rand() % 20) / 100.0f);
  c->campaign_strength = 0.30f + (float)(rand() % 40) / 100.0f;
  c->policy_platform = 0.30f + (float)(rand() % 40) / 100.0f;
  c->funding = 100.0f + (float)(rand() % 900);
  e->candidate_count++;
  return c;
}

void civ_election_campaign(civ_election_t *e, float dt,
                           float literacy, float media_freedom) {
  if (!e) return;
  (void)dt;

  /* Total support must sum to ~1.0 */
  float total_support = 0.0f;
  for (int i = 0; i < e->candidate_count; i++) {
    civ_candidate_t *c = &e->candidates[i];

    /* Campaign effectiveness: strength * funding * media reach */
    float campaign_effect = c->campaign_strength * 0.3f
                            + (c->funding / 1000.0f) * 0.2f
                            + c->policy_platform * literacy * 0.2f
                            + media_freedom * 0.1f;

    /* Incumbency advantage: recognized, but can backfire if unhappy */
    if (c->is_incumbent)
      campaign_effect += 0.05f;

    /* Support shift: campaign effect scaled by engagement */
    float shift = campaign_effect * e->voter_engagement * 0.02f;
    /* Random events: debates, scandals */
    float event = ((float)rand() / RAND_MAX - 0.5f) * 0.03f;
    c->public_support += shift + event;

    /* Spend funding */
    c->funding -= 10.0f;
    if (c->funding < 0.0f) c->funding = 0.0f;

    if (c->public_support < 0.01f) c->public_support = 0.01f;
    total_support += c->public_support;
  }

  /* Normalize */
  if (total_support > 0.0f && e->candidate_count > 0) {
    for (int i = 0; i < e->candidate_count; i++)
      e->candidates[i].public_support /= total_support;
  }
}

void civ_election_tally_votes(civ_election_t *e) {
  if (!e) return;

  /* First-past-the-post: winner takes all seats */
  if (e->type == CIV_ELECTION_FPTP || e->type == CIV_ELECTION_TWO_ROUND) {
    /* Winner determined by public_support — already computed */
  }
  /* Proportional: seats proportional to support */
  /* Mixed, ranked-choice, etc. — all use public_support as the base signal */
}

civ_candidate_t *civ_election_winner(const civ_election_t *e) {
  if (!e || e->candidate_count == 0) return NULL;
  civ_candidate_t *best = &e->candidates[0];
  for (int i = 1; i < e->candidate_count; i++)
    if (e->candidates[i].public_support > best->public_support)
      best = &e->candidates[i];
  return best;
}

float civ_election_turnout_rate(const civ_election_t *e) {
  return e ? e->turnout : 0.0f;
}

float civ_election_representation_quality(const civ_election_system_t *es) {
  return es ? (es->democratic_health * 0.5f + es->electoral_fairness * 0.5f) : 0.0f;
}

/* ── Election integrity ─────────────────────────────────────────── */
bool civ_election_cancel(civ_election_t *e, const char *reason) {
  if (!e || e->concluded) return false;
  (void)reason;
  e->in_progress = false;
  e->concluded = true;
  e->turnout = 0.0f;
  return true;
}

bool civ_election_delay(civ_election_t *e, int turns, const char *reason) {
  if (!e || e->concluded || turns <= 0) return false;
  (void)reason;
  e->turns_until_election += turns;
  return true;
}

bool civ_election_is_compromised(const civ_election_t *e) {
  if (!e) return false;
  /* An election is compromised if turnout is suppressed or fairness is low */
  return e->turnout < 0.25f || (e->in_progress && !e->concluded
         && e->voter_engagement < 0.15f);
}

float civ_election_integrity(const civ_election_system_t *es) {
  return es ? es->electoral_fairness : 0.0f;
}

void civ_election_suppress_turnout(civ_election_t *e, float suppression_level) {
  if (!e) return;
  e->voter_engagement -= suppression_level * 0.3f;
  if (e->voter_engagement < 0.05f) e->voter_engagement = 0.05f;
}

bool civ_election_should_be_delayed(const civ_election_t *e, float stability,
                                     float corruption, bool emergency) {
  if (!e || e->concluded) return false;
  /* Conditions that cause election delays:
   * - Stability below 0.25 (civil unrest)
   * - Active emergency
   * - High corruption + low electoral fairness */
  if (emergency) return true;
  if (stability < 0.20f) return true;
  if (corruption > 0.50f && e->voter_engagement < 0.20f) return true;
  return false;
}
