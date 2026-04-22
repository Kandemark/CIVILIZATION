/**
 * @file elections.h
 * @brief Elections — voting mechanics, campaign cycles, electoral systems
 *
 * NO system is inherently "good": first-past-the-post provides clear
 * winners but can distort representation. Proportional representation
 * reflects voter will but enables fragmentation. Indirect election
 * (electoral college, parliamentary selection) provides stability
 * but can diverge from popular will.
 *
 * Elections run autonomously per cycle. NPCs campaign, voters respond
 * to economic conditions + governance performance + cultural factors.
 * The player can participate but does not control outcomes.
 */
#ifndef CIV_GOVERNANCE_ELECTIONS_H
#define CIV_GOVERNANCE_ELECTIONS_H

#include "../../../common.h"
#include "../../../types.h"

/* Electoral systems */
typedef enum {
  CIV_ELECTION_FPTP,           /* first-past-the-post, single-member districts */
  CIV_ELECTION_PROPORTIONAL,   /* party-list proportional representation */
  CIV_ELECTION_MIXED,          /* mixed-member proportional */
  CIV_ELECTION_INDIRECT,       /* electoral college / parliamentary selection */
  CIV_ELECTION_TWO_ROUND,      /* runoff if no majority */
  CIV_ELECTION_RANKED_CHOICE,  /* instant-runoff / ranked voting */
  CIV_ELECTION_SORTITION,      /* random selection (Athenian-style) */
  CIV_ELECTION_TYPE_COUNT
} civ_election_type_t;

typedef struct {
  char   id[STRING_SHORT_LEN], name[STRING_MEDIUM_LEN];
  char   npc_id[STRING_SHORT_LEN];
  char   party_id[STRING_SHORT_LEN];
  float  campaign_strength;    /* 0.0–1.0, affected by charisma + funding */
  float  policy_platform;      /* 0.0–1.0, how coherent their platform is */
  float  public_support;       /* 0.0–1.0, current polling */
  float  funding;              /* campaign resources */
  int    debates_won;
  bool   is_incumbent;
} civ_candidate_t;

typedef struct {
  char   position_id[STRING_SHORT_LEN]; /* which government position is being elected */
  char   body_name[STRING_MEDIUM_LEN];  /* e.g. "Parliament", "Presidency" */
  civ_election_type_t type;
  civ_candidate_t *candidates;
  int    candidate_count, candidate_capacity;
  int    total_seats;           /* seats being contested */
  int    eligible_voters;       /* population eligible to vote */
  float  turnout;               /* 0.0–1.0, affected by engagement */
  float  voter_engagement;      /* 0.0–1.0 */
  int    turns_until_election;  /* countdown */
  int    cycle_length;          /* how often elections occur */
  bool   in_progress;
  bool   concluded;
} civ_election_t;

typedef struct {
  civ_election_t *elections;
  int  election_count, election_capacity;
  float democratic_health;      /* 0.0–1.0 composite */
  float voter_trust;            /* 0.0–1.0 */
  float electoral_fairness;     /* 0.0–1.0, affected by corruption */
  int   total_elections_held;
} civ_election_system_t;

civ_election_system_t *civ_election_create(void);
void civ_election_destroy(civ_election_system_t *es);
void civ_election_update(civ_election_system_t *es, float dt,
                         int total_population, float corruption,
                         float representation, float citizen_happiness,
                         float economic_confidence, float literacy_rate);

civ_election_t *civ_election_schedule(civ_election_system_t *es,
                                       const char *position_id,
                                       const char *body_name,
                                       civ_election_type_t type,
                                       int seats, int cycle_length);
civ_candidate_t *civ_election_add_candidate(civ_election_t *e,
                                             const char *name, const char *npc_id,
                                             const char *party_id, bool incumbent);
void civ_election_campaign(civ_election_t *e, float dt,
                           float literacy, float media_freedom);
void civ_election_tally_votes(civ_election_t *e);
civ_candidate_t *civ_election_winner(const civ_election_t *e);
float civ_election_turnout_rate(const civ_election_t *e);
float civ_election_representation_quality(const civ_election_system_t *es);

/* ── Election integrity ─────────────────────────────────────────── */
bool civ_election_cancel(civ_election_t *e, const char *reason);
bool civ_election_delay(civ_election_t *e, int turns, const char *reason);
bool civ_election_is_compromised(const civ_election_t *e);
float civ_election_integrity(const civ_election_system_t *es);
void civ_election_suppress_turnout(civ_election_t *e, float suppression_level);
bool civ_election_should_be_delayed(const civ_election_t *e, float stability,
                                     float corruption, bool emergency);

#endif
