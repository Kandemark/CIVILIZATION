/**
 * @file history_db.c
 * @brief Masterpiece Journaling Database Implementation
 */

#include "../../../include/core/data/history_db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_journal_t *civ_journal_create(const char *path) {
  civ_journal_t *j = CIV_MALLOC(sizeof(civ_journal_t));
  if (!j)
    return NULL;

  memset(j, 0, sizeof(civ_journal_t));
  j->capacity = 1000;
  j->journal = CIV_CALLOC(j->capacity, sizeof(civ_event_t));
  if (path)
    strncpy(j->db_path, path, 255);

  return j;
}

void civ_journal_destroy(civ_journal_t *j) {
  if (j) {
    civ_journal_flush(j);
    CIV_FREE(j->journal);
    CIV_FREE(j);
  }
}

civ_result_t civ_journal_log(civ_journal_t *j, civ_journal_event_type_t type,
                             const char *context, const void *data,
                             size_t size) {
  if (!j)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, NULL};

  if (j->event_count >= j->capacity) {
    j->capacity *= 2;
    j->journal = CIV_REALLOC(j->journal, j->capacity * sizeof(civ_event_t));
  }

  civ_event_t *e = &j->journal[j->event_count++];
  e->sequence_id = j->event_count;
  e->timestamp = (uint32_t)time(NULL);
  e->type = type;
  e->data_size = (size > 256) ? 256 : size;

  if (context)
    strncpy(e->context, context, 127);
  if (data && size > 0)
    memcpy(e->data, data, e->data_size);

  /* Auto-flush every 100 events for robustness */
  if (j->event_count % 100 == 0)
    civ_journal_flush(j);

  return (civ_result_t){CIV_OK, NULL};
}

civ_result_t civ_journal_flush(civ_journal_t *j) {
  if (!j || !j->db_path[0])
    return (civ_result_t){CIV_OK, NULL};

  FILE *f = fopen(j->db_path, "wb");
  if (!f)
    return (civ_result_t){CIV_ERROR_IO, "Journal IO Error"};

  uint64_t count = (uint64_t)j->event_count;
  fwrite(&count, sizeof(uint64_t), 1, f);
  fwrite(j->journal, sizeof(civ_event_t), j->event_count, f);
  fclose(f);

  return (civ_result_t){CIV_OK, "Journal Flushed"};
}

civ_result_t civ_journal_load(civ_journal_t *j, const char *path) {
  if (!j || !path)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, NULL};

  FILE *f = fopen(path, "rb");
  if (!f)
    return (civ_result_t){CIV_ERROR_IO, "Journal Not Found"};

  uint64_t count;
  if (fread(&count, sizeof(uint64_t), 1, f) != 1) {
    fclose(f);
    return (civ_result_t){CIV_ERROR_IO, "Empty Journal"};
  }

  if (count > j->capacity) {
    j->capacity = (size_t)count + 100;
    j->journal = CIV_REALLOC(j->journal, j->capacity * sizeof(civ_event_t));
  }

  j->event_count = fread(j->journal, sizeof(civ_event_t), (size_t)count, f);
  strncpy(j->db_path, path, 255);
  fclose(f);

  return (civ_result_t){CIV_OK, "Journal Loaded"};
}
