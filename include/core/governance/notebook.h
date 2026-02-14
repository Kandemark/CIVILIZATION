/**
 * @file notebook.h
 * @brief Player Notebook system
 */

#ifndef CIVILIZATION_NOTEBOOK_H
#define CIVILIZATION_NOTEBOOK_H

#include "../../common.h"
#include "../../types.h"

/* Note */
typedef struct {
  char id[STRING_SHORT_LEN];
  char title[STRING_MEDIUM_LEN];
  char content[STRING_MAX_LEN];
  time_t timestamp;
} civ_note_t;

/* Notebook */
typedef struct {
  civ_note_t *notes;
  size_t note_count;
  size_t note_capacity;
} civ_notebook_t;

/* Functions */
civ_notebook_t *civ_notebook_create(void);
void civ_notebook_destroy(civ_notebook_t *notebook);

civ_result_t civ_notebook_add_note(civ_notebook_t *notebook, const char *title,
                                   const char *content);
civ_result_t civ_notebook_remove_note(civ_notebook_t *notebook,
                                      const char *note_id);
civ_note_t *civ_notebook_find_note(const civ_notebook_t *notebook,
                                   const char *note_id);

#endif /* CIVILIZATION_NOTEBOOK_H */
