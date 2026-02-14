/**
 * @file writing_system.h
 * @brief Writing system/script system for languages
 */

#ifndef CIVILIZATION_WRITING_SYSTEM_H
#define CIVILIZATION_WRITING_SYSTEM_H

#include "../../common.h"
#include "../../types.h"

/* Writing system type */
typedef enum {
  CIV_SCRIPT_ALPHABETIC = 0, /* Roman, Greek, Cyrillic */
  CIV_SCRIPT_ABJAD,          /* Arabic, Hebrew */
  CIV_SCRIPT_ABUGIDA,        /* Devanagari, Thai */
  CIV_SCRIPT_LOGOSYLLABIC,   /* Chinese, Japanese Kanji */
  CIV_SCRIPT_SYLLABIC,       /* Japanese Hiragana/Katakana */
  CIV_SCRIPT_PICTOGRAPHIC,   /* Early pictographic systems */
  CIV_SCRIPT_OTHER
} civ_script_type_t;

/* Writing system */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  civ_script_type_t type;

  char parent_script_id[STRING_SHORT_LEN]; /* Evolved from */
  civ_float_t complexity;                  /* 0.0 to 1.0 */
  civ_float_t efficiency; /* 0.0 to 1.0 - how easy to learn/write */

  /* Character sets for name generation */
  char *consonants;
  char *vowels;
  char *symbols;
  size_t consonant_count;
  size_t vowel_count;
  size_t symbol_count;

  time_t creation_time;
} civ_writing_system_t;

/* Writing system manager */
typedef struct {
  civ_writing_system_t *scripts;
  size_t script_count;
  size_t script_capacity;
} civ_writing_system_manager_t;

/* Function declarations */
civ_writing_system_manager_t *civ_writing_system_manager_create(void);
void civ_writing_system_manager_destroy(civ_writing_system_manager_t *manager);
void civ_writing_system_manager_init(civ_writing_system_manager_t *manager);

civ_writing_system_t *civ_writing_system_create(const char *id,
                                                const char *name,
                                                civ_script_type_t type);
void civ_writing_system_destroy(civ_writing_system_t *script);
civ_result_t civ_writing_system_set_characters(civ_writing_system_t *script,
                                               const char *consonants,
                                               const char *vowels);
civ_result_t
civ_writing_system_evolve_from(civ_writing_system_manager_t *manager,
                               const civ_writing_system_t *parent,
                               const char *new_id, const char *new_name);
char *civ_writing_system_generate_name(const civ_writing_system_t *script,
                                       size_t min_length, size_t max_length);
civ_result_t civ_writing_system_evolve_symbols(civ_writing_system_t *script,
                                               civ_float_t intensity);
civ_result_t
civ_writing_system_manager_add(civ_writing_system_manager_t *manager,
                               civ_writing_system_t *script);
civ_writing_system_t *
civ_writing_system_manager_find(const civ_writing_system_manager_t *manager,
                                const char *id);

#endif /* CIVILIZATION_WRITING_SYSTEM_H */
