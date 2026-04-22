/**
 * @file npc_engine.c
 * @brief NPC decision engine — NPCs make choices, history unfolds
 */
#include "core/npc_engine.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Decision templates per category */
static const char *s_military[] = {
  "Military reforms initiated by %s in %s",
  "Border fortifications strengthened by order of %s",
  "Army conscription expanded under %s's directive",
  "Naval fleet modernization approved by %s",
  "Defense pact negotiations opened by %s",
  "Military budget increased by %s",
};
static const char *s_economic[] = {
  "Trade agreement signed under %s's administration in %s",
  "New currency regulations imposed by %s",
  "Market liberalization reforms pushed by %s",
  "Infrastructure investment approved by %s",
  "Tax reforms enacted by %s",
  "Agricultural subsidies introduced by %s",
};
static const char *s_political[] = {
  "Constitutional amendment proposed by %s in %s",
  "Cabinet reshuffle executed by %s",
  "Anti-corruption investigation launched against associates of %s",
  "Electoral reforms debated under %s's leadership",
  "New political coalition formed with %s's backing",
  "Local governance powers devolved by %s",
};
static const char *s_social[] = {
  "Education reforms championed by %s in %s",
  "Public health initiative launched by %s",
  "Cultural preservation edict issued by %s",
  "Religious tolerance decree signed by %s",
  "Urban development program initiated by %s",
  "Labor rights legislation introduced by %s",
};
static const char *s_diplomatic[] = {
  "Diplomatic mission dispatched by %s of %s",
  "Foreign aid package approved by %s",
  "Border dispute resolution proposed by %s",
  "International trade summit hosted by %s",
  "Non-aggression overture made by %s",
  "Cultural exchange program launched by %s",
};

civ_npc_engine_t *civ_npc_engine_create(void) {
  civ_npc_engine_t *eng = (civ_npc_engine_t *)malloc(sizeof(civ_npc_engine_t));
  if (!eng) return NULL;
  memset(eng, 0, sizeof(*eng));
  eng->npc_capacity = 64;
  eng->npcs = (civ_npc_t *)calloc((size_t)eng->npc_capacity, sizeof(civ_npc_t));
  return eng;
}

void civ_npc_engine_destroy(civ_npc_engine_t *eng) {
  if (!eng) return;
  free(eng->npcs);
  free(eng);
}

civ_npc_t *civ_npc_engine_add(civ_npc_engine_t *eng, const char *name,
                              const char *role, const char *nation_id) {
  if (!eng || eng->npc_count >= eng->npc_capacity) return NULL;
  civ_npc_t *npc = &eng->npcs[eng->npc_count++];
  memset(npc, 0, sizeof(*npc));
  strncpy(npc->name, name, CIV_NPC_NAME_MAX-1);
  strncpy(npc->role, role, sizeof(npc->role)-1);
  strncpy(npc->nation_id, nation_id, sizeof(npc->nation_id)-1);
  /* Randomize ideology */
  npc->ideology.traditional_vs_progressive = (float)(rand() % 100) / 100.0f;
  npc->ideology.authoritarian_vs_liberal    = (float)(rand() % 100) / 100.0f;
  npc->ideology.nationalist_vs_globalist    = (float)(rand() % 100) / 100.0f;
  npc->ideology.militarist_vs_pacifist      = (float)(rand() % 100) / 100.0f;
  npc->ideology.mercantile_vs_agrarian      = (float)(rand() % 100) / 100.0f;
  npc->ideology.religious_vs_secular         = (float)(rand() % 100) / 100.0f;
  npc->influence = 0.3f + (float)(rand() % 70) / 100.0f;
  npc->corruption = (float)(rand() % 40) / 100.0f;
  npc->competence = 0.3f + (float)(rand() % 70) / 100.0f;
  npc->age = 30 + rand() % 50;
  return npc;
}

static void add_decision(civ_npc_engine_t *eng, const char *desc,
                         const char *actor, const char *nation,
                         int32_t year, int32_t day, uint32_t cat,
                         float stab, float econ, float diplo) {
  civ_decision_t *d = &eng->recent_decisions[eng->decision_head];
  memset(d, 0, sizeof(*d));
  strncpy(d->description, desc, sizeof(d->description)-1);
  strncpy(d->actor_name, actor, sizeof(d->actor_name)-1);
  strncpy(d->nation_id, nation, sizeof(d->nation_id)-1);
  d->global_year = year; d->global_day = day;
  d->category = cat;
  d->stability_effect = stab; d->economic_effect = econ;
  d->diplomatic_effect = diplo;
  eng->decision_head = (eng->decision_head + 1) % CIV_NPC_DECISION_MAX;
  if (eng->decision_count < CIV_NPC_DECISION_MAX) eng->decision_count++;
}

void civ_npc_engine_process_turn(civ_npc_engine_t *eng, void *nation_mgr,
                                 int32_t global_year, int32_t global_day) {
  if (!eng || eng->npc_count == 0) return;

  /* Each turn, a few NPCs make decisions */
  int decisions_this_turn = 2 + rand() % 4;
  for (int d = 0; d < decisions_this_turn; d++) {
    int idx = rand() % eng->npc_count;
    civ_npc_t *npc = &eng->npcs[idx];
    if (npc->influence < 0.2f) continue; /* low-influence NPCs act less */

    /* Choose category based on ideology */
    uint32_t cat;
    if (npc->ideology.militarist_vs_pacifist < 0.35f) cat = 0;      /* military */
    else if (npc->ideology.mercantile_vs_agrarian < 0.4f) cat = 1;  /* economic */
    else if (npc->ideology.authoritarian_vs_liberal < 0.3f) cat = 2; /* political */
    else if (npc->ideology.religious_vs_secular < 0.3f) cat = 3;    /* social */
    else cat = 4; /* diplomatic */

    const char **templates;
    int tcount;
    switch (cat) {
    case 0: templates = s_military;    tcount = 6; break;
    case 1: templates = s_economic;    tcount = 6; break;
    case 2: templates = s_political;   tcount = 6; break;
    case 3: templates = s_social;      tcount = 6; break;
    default: templates = s_diplomatic; tcount = 6; break;
    }

    char desc[192];
    int tidx = rand() % tcount;
    /* Corruption chance: 20% chance of a corrupt action */
    if ((float)(rand()%100)/100.0f < npc->corruption) {
      const char *corrupt[] = {
        "Embezzlement scandal linked to %s of %s",
        "%s of %s accused of nepotism in appointments",
        "Bribery allegations surface against %s",
      };
      int ci = rand() % 3;
      snprintf(desc, sizeof(desc), corrupt[ci], npc->name, npc->nation_id);
      add_decision(eng, desc, npc->name, npc->nation_id,
                   global_year, global_day, cat, -0.05f, -0.03f, -0.02f);
    } else {
      snprintf(desc, sizeof(desc), templates[tidx], npc->name, npc->nation_id);
      float se = (float)(rand()%20-5)/100.0f;  /* -0.05 to +0.15 */
      float ee = (float)(rand()%20-5)/100.0f;
      float de = (float)(rand()%15-3)/100.0f;
      add_decision(eng, desc, npc->name, npc->nation_id,
                   global_year, global_day, cat, se, ee, de);
    }
  }
  (void)nation_mgr;
}

int civ_npc_engine_get_recent(civ_npc_engine_t *eng, int max,
                              civ_decision_t *out) {
  if (!eng || !out) return 0;
  int count = 0, pos = eng->decision_head;
  for (int i = 0; i < eng->decision_count && count < max; i++) {
    pos = (pos - 1 + CIV_NPC_DECISION_MAX) % CIV_NPC_DECISION_MAX;
    out[count++] = eng->recent_decisions[pos];
  }
  return count;
}
