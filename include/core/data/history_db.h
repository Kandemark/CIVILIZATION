/**
 * @file history_db.h
 * @brief Masterpiece Journaling Database (Event-Sourced Chronicles)
 */

#ifndef CIVILIZATION_HISTORY_DB_H
#define CIVILIZATION_HISTORY_DB_H

#include "../../common.h"
#include "../../types.h"

struct civ_game;

typedef enum {
  CIV_JOURNAL_WORLD_GEN_START = 0,
  CIV_JOURNAL_TECTONIC_SHIFT,
  CIV_JOURNAL_EROSION_PASS,
  CIV_JOURNAL_BIOME_FINALIZED,
  CIV_JOURNAL_PLAYER_SPAWN,
  CIV_JOURNAL_SETTLEMENT_FOUNDED,
  CIV_JOURNAL_SETTLEMENT_GROWTH,
  CIV_JOURNAL_POLICY_CHANGED,
  CIV_JOURNAL_NATURAL_DISASTER,
  CIV_JOURNAL_GENERIC_LOG
} civ_journal_event_type_t;

typedef struct {
  uint64_t sequence_id;
  uint32_t timestamp;
  civ_journal_event_type_t type;
  char context[128];
  uint8_t data[256]; /* Generic binary blob for event-specific data */
  size_t data_size;
} civ_event_t;

typedef struct {
  civ_event_t *journal;
  size_t event_count;
  size_t capacity;
  char db_path[256];
  uint32_t format_version;
} civ_journal_t;

/* Robust Journal Interface */
civ_journal_t *civ_journal_create(const char *path);
void civ_journal_destroy(civ_journal_t *j);

civ_result_t civ_journal_log(civ_journal_t *j, civ_journal_event_type_t type,
                             const char *context, const void *data,
                             size_t size);
civ_result_t civ_journal_flush(civ_journal_t *j);
civ_result_t civ_journal_load(civ_journal_t *j, const char *path);

/* Query helpers */
size_t civ_journal_count_by_type(const civ_journal_t *j,
                                 civ_journal_event_type_t type);

#endif /* CIVILIZATION_HISTORY_DB_H */
