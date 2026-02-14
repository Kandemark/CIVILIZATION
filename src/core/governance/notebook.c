/**
 * @file notebook.c
 * @brief Implementation of player notebook system
 */

#include "../../../include/core/governance/notebook.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_notebook_t *civ_notebook_create(void) {
  civ_notebook_t *notebook =
      (civ_notebook_t *)CIV_MALLOC(sizeof(civ_notebook_t));
  if (!notebook)
    return NULL;

  memset(notebook, 0, sizeof(civ_notebook_t));
  notebook->note_capacity = 32;
  notebook->notes =
      (civ_note_t *)CIV_CALLOC(notebook->note_capacity, sizeof(civ_note_t));

  return notebook;
}

void civ_notebook_destroy(civ_notebook_t *notebook) {
  if (!notebook)
    return;
  CIV_FREE(notebook->notes);
  CIV_FREE(notebook);
}

civ_result_t civ_notebook_add_note(civ_notebook_t *notebook, const char *title,
                                   const char *content) {
  civ_result_t result = {CIV_OK, NULL};
  if (!notebook || !title || !content) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  if (notebook->note_count >= notebook->note_capacity) {
    notebook->note_capacity *= 2;
    notebook->notes = (civ_note_t *)CIV_REALLOC(
        notebook->notes, notebook->note_capacity * sizeof(civ_note_t));
  }

  if (notebook->notes) {
    civ_note_t *n = &notebook->notes[notebook->note_count++];
    snprintf(n->id, STRING_SHORT_LEN, "NOTE_%zu", notebook->note_count);
    strncpy(n->title, title, STRING_MEDIUM_LEN - 1);
    strncpy(n->content, content, STRING_MAX_LEN - 1);
    n->timestamp = time(NULL);
  } else {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

civ_result_t civ_notebook_remove_note(civ_notebook_t *notebook,
                                      const char *note_id) {
  if (!notebook || !note_id)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null pointer"};

  for (size_t i = 0; i < notebook->note_count; i++) {
    if (strcmp(notebook->notes[i].id, note_id) == 0) {
      /* Shift remaining notes */
      for (size_t j = i; j < notebook->note_count - 1; j++) {
        notebook->notes[j] = notebook->notes[j + 1];
      }
      notebook->note_count--;
      return (civ_result_t){CIV_OK, NULL};
    }
  }

  return (civ_result_t){CIV_ERROR_NOT_FOUND, "Note not found"};
}

civ_note_t *civ_notebook_find_note(const civ_notebook_t *notebook,
                                   const char *note_id) {
  if (!notebook || !note_id)
    return NULL;

  for (size_t i = 0; i < notebook->note_count; i++) {
    if (strcmp(notebook->notes[i].id, note_id) == 0) {
      return &notebook->notes[i];
    }
  }

  return NULL;
}
