/**
 * @file history_db.c
 * @brief Structured journaling database implementation.
 */

#include "../../../include/core/data/history_db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CIV_JOURNAL_FORMAT_MAGIC 0x434A4442u /* CJDB */
#define CIV_JOURNAL_FORMAT_VERSION 1u

typedef struct {
  uint32_t magic;
  uint32_t version;
  uint64_t event_count;
} civ_journal_file_header_t;

static civ_result_t civ_journal_reserve(civ_journal_t *j, size_t min_capacity) {
  if (!j)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null journal"};

  if (min_capacity <= j->capacity)
    return (civ_result_t){CIV_OK, NULL};

  size_t new_capacity = j->capacity ? j->capacity : 256;
  while (new_capacity < min_capacity) {
    new_capacity *= 2;
  }

  civ_event_t *new_journal =
      (civ_event_t *)CIV_REALLOC(j->journal, new_capacity * sizeof(civ_event_t));
  if (!new_journal)
    return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "Journal realloc failed"};

  j->journal = new_journal;
  j->capacity = new_capacity;
  return (civ_result_t){CIV_OK, NULL};
}

civ_journal_t *civ_journal_create(const char *path) {
  civ_journal_t *j = (civ_journal_t *)CIV_MALLOC(sizeof(civ_journal_t));
  if (!j)
    return NULL;

  memset(j, 0, sizeof(civ_journal_t));
  j->format_version = CIV_JOURNAL_FORMAT_VERSION;

  civ_result_t r = civ_journal_reserve(j, 1024);
  if (CIV_FAILED(r)) {
    CIV_FREE(j);
    return NULL;
  }

  if (path)
    strncpy(j->db_path, path, sizeof(j->db_path) - 1);

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
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null journal"};

  civ_result_t reserve_result = civ_journal_reserve(j, j->event_count + 1);
  if (CIV_FAILED(reserve_result))
    return reserve_result;

  civ_event_t *e = &j->journal[j->event_count++];
  memset(e, 0, sizeof(*e));
  e->sequence_id = (uint64_t)j->event_count;
  e->timestamp = (uint32_t)time(NULL);
  e->type = type;
  e->data_size = (size > sizeof(e->data)) ? sizeof(e->data) : size;

  if (context)
    strncpy(e->context, context, sizeof(e->context) - 1);
  if (data && e->data_size > 0)
    memcpy(e->data, data, e->data_size);

  if ((j->event_count % 256) == 0)
    return civ_journal_flush(j);

  return (civ_result_t){CIV_OK, NULL};
}

civ_result_t civ_journal_flush(civ_journal_t *j) {
  if (!j || !j->db_path[0])
    return (civ_result_t){CIV_OK, NULL};

  FILE *f = fopen(j->db_path, "wb");
  if (!f)
    return (civ_result_t){CIV_ERROR_IO, "Journal IO Error"};

  civ_journal_file_header_t header = {CIV_JOURNAL_FORMAT_MAGIC,
                                      j->format_version,
                                      (uint64_t)j->event_count};

  if (fwrite(&header, sizeof(header), 1, f) != 1) {
    fclose(f);
    return (civ_result_t){CIV_ERROR_IO, "Failed writing journal header"};
  }

  if (j->event_count > 0 &&
      fwrite(j->journal, sizeof(civ_event_t), j->event_count, f) !=
          j->event_count) {
    fclose(f);
    return (civ_result_t){CIV_ERROR_IO, "Failed writing journal events"};
  }

  fclose(f);
  return (civ_result_t){CIV_OK, "Journal Flushed"};
}

civ_result_t civ_journal_load(civ_journal_t *j, const char *path) {
  if (!j || !path)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null args"};

  FILE *f = fopen(path, "rb");
  if (!f)
    return (civ_result_t){CIV_ERROR_IO, "Journal Not Found"};

  civ_journal_file_header_t header = {0};
  if (fread(&header, sizeof(header), 1, f) != 1) {
    fclose(f);
    return (civ_result_t){CIV_ERROR_IO, "Invalid Journal Header"};
  }

  if (header.magic != CIV_JOURNAL_FORMAT_MAGIC) {
    fclose(f);
    return (civ_result_t){CIV_ERROR_INVALID_STATE, "Unsupported journal file"};
  }

  civ_result_t reserve_result = civ_journal_reserve(j, (size_t)header.event_count);
  if (CIV_FAILED(reserve_result)) {
    fclose(f);
    return reserve_result;
  }

  size_t read_count = 0;
  if (header.event_count > 0) {
    read_count =
        fread(j->journal, sizeof(civ_event_t), (size_t)header.event_count, f);
  }
  fclose(f);

  if (read_count != (size_t)header.event_count)
    return (civ_result_t){CIV_ERROR_IO, "Corrupted journal data"};

  j->event_count = read_count;
  j->format_version = header.version;
  strncpy(j->db_path, path, sizeof(j->db_path) - 1);

  return (civ_result_t){CIV_OK, "Journal Loaded"};
}

size_t civ_journal_count_by_type(const civ_journal_t *j,
                                 civ_journal_event_type_t type) {
  if (!j)
    return 0;

  size_t count = 0;
  for (size_t i = 0; i < j->event_count; i++) {
    if (j->journal[i].type == type)
      count++;
  }
  return count;
}
