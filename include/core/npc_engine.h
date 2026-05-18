/**
 * @file npc_engine.h
 * @brief NPC decision engine — independent actors making choices
 *
 * Each nation has NPC actors (leaders, ministers, influential figures)
 * who make decisions based on ideology, national state, relationships,
 * and historical context. Decisions produce events that shape history.
 */
#ifndef CIV_CORE_NPC_ENGINE_H
#define CIV_CORE_NPC_ENGINE_H

#include "../common.h"
#include "../types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_NPC_NAME_MAX   48
#define CIV_NPC_MAX         8  /* per nation */
#define CIV_NPC_DECISION_MAX 64

/* ── NPC ideology axes ──────────────────────────────────────────── */
typedef struct {
  float traditional_vs_progressive;  /* 0=traditional, 1=progressive */
  float authoritarian_vs_liberal;   /* 0=authoritarian, 1=liberal */
  float nationalist_vs_globalist;   /* 0=nationalist, 1=globalist */
  float militarist_vs_pacifist;     /* 0=militarist, 1=pacifist */
  float mercantile_vs_agrarian;     /* 0=mercantile, 1=agrarian */
  float religious_vs_secular;       /* 0=religious, 1=secular */
} civ_npc_ideology_t;

/* ── NPC actor ──────────────────────────────────────────────────── */
typedef struct {
  char           name[CIV_NPC_NAME_MAX];
  char           role[48];        /* "President", "Minister of Trade", etc. */
  civ_npc_ideology_t ideology;
  float          influence;       /* 0-1, how much they affect decisions */
  float          corruption;      /* 0-1, likelihood of self-serving actions */
  float          competence;      /* 0-1, effectiveness at their role */
  int32_t        age;
  char           nation_id[32];   /* which nation they belong to */
} civ_npc_t;

/* ── Decision outcome ───────────────────────────────────────────── */
typedef struct {
  char     description[192];   /* what happened */
  char     actor_name[48];     /* who made the decision */
  char     nation_id[32];      /* which nation */
  int32_t  global_year;        /* when it happened */
  int32_t  global_day;
  float    stability_effect;   /* impact on nation stability */
  float    economic_effect;    /* impact on economy */
  float    diplomatic_effect;  /* impact on foreign relations */
  uint32_t category;           /* 0=military,1=economic,2=political,3=social,4=diplomatic */
} civ_decision_t;

/* ── NPC Engine ──────────────────────────────────────────────────── */
typedef struct {
  civ_npc_t      *npcs;
  int32_t         npc_count;
  int32_t         npc_capacity;
  civ_decision_t  recent_decisions[CIV_NPC_DECISION_MAX];
  int32_t         decision_count;
  int32_t         decision_head; /* ring buffer head */
} civ_npc_engine_t;

civ_npc_engine_t *civ_npc_engine_create(void);
void              civ_npc_engine_destroy(civ_npc_engine_t *eng);

/* Add an NPC to a nation */
civ_npc_t *civ_npc_engine_add(civ_npc_engine_t *eng, const char *name,
                              const char *role, const char *nation_id);

/* Process one turn of NPC decisions for all nations */
void civ_npc_engine_process_turn(civ_npc_engine_t *eng, void *nation_mgr,
                                 int32_t global_year, int32_t global_day);

/* Get recent decisions as a readable string buffer */
int  civ_npc_engine_get_recent(civ_npc_engine_t *eng, int max,
                               civ_decision_t *out);

#ifdef __cplusplus
}
#endif
#endif
