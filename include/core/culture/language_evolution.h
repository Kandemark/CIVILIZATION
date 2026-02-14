/**
 * @file language_evolution.h
 * @brief Language evolution system
 */

#ifndef CIVILIZATION_LANGUAGE_EVOLUTION_H
#define CIVILIZATION_LANGUAGE_EVOLUTION_H

#include "../../common.h"
#include "../../types.h"
#include "writing_system.h"

/* Phonology - sound system */
typedef struct {
  char consonants[STRING_SHORT_LEN];
  char vowels[STRING_SHORT_LEN];
  civ_float_t nasal_ratio;
  civ_float_t fricative_ratio;
} civ_phonology_t;

/* Grammar - word structures and syntax */
typedef enum {
  CIV_MORPH_ISOLATING = 0,
  CIV_MORPH_AGGLUTINATIVE,
  CIV_MORPH_FUSIONAL,
  CIV_MORPH_POLYSYNTHETIC
} civ_morphology_type_t;

typedef enum {
  CIV_SYNTAX_SVO = 0,
  CIV_SYNTAX_SOV,
  CIV_SYNTAX_VSO,
  CIV_SYNTAX_VOS,
  CIV_SYNTAX_OVS,
  CIV_SYNTAX_OSV
} civ_syntax_type_t;

typedef struct {
  civ_morphology_type_t morphology;
  civ_syntax_type_t word_order;
  bool has_gender;
  bool has_cases;
  int case_count;
} civ_grammar_t;

/* Vocabulary Entry */
typedef struct {
  char concept_id[STRING_SHORT_LEN]; /* Universal ID like "WATER", "MOTH" */
  char word[STRING_SHORT_LEN];       /* The representation in this language */
} civ_vocab_entry_t;

/* Language - evolution-based, no predefined families */
typedef struct {
  char id[STRING_SHORT_LEN];
  char name[STRING_MEDIUM_LEN];
  char parent_id[STRING_SHORT_LEN]; /* Parent language (for evolution) */

  char writing_system_id[STRING_SHORT_LEN]; /* Associated writing system */

  civ_phonology_t phonology;
  civ_grammar_t grammar;

  civ_vocab_entry_t *vocabulary;
  size_t vocabulary_size;
  size_t vocabulary_capacity;

  civ_float_t complexity; /* 0.0 to 1.0 */
  civ_float_t prestige;   /* 0.0 to 1.0 */
  civ_float_t speakers;   /* Number of speakers (normalized) */

  civ_float_t vitality_score; /* 0.0 to 1.0 - below 0.1 = dying */
  bool is_extinct;

  /* Advanced Evolution Features */
  char dialects[4]
               [STRING_SHORT_LEN]; /* Names/IDs of closely related dialects */
  size_t dialect_count;

  civ_vocab_entry_t *loanwords; /* Borrowed from other languages */
  size_t loanword_count;
  size_t loanword_capacity;

  time_t creation_time;
  time_t last_evolution;
} civ_language_t;

/* Language evolution system */
typedef struct {
  civ_language_t *languages;
  size_t language_count;
  size_t language_capacity;

  civ_float_t evolution_rate;
  civ_float_t divergence_threshold;
} civ_language_evolution_t;

/* Function declarations */
civ_language_evolution_t *civ_language_evolution_create(void);
void civ_language_evolution_destroy(civ_language_evolution_t *evolution);
void civ_language_evolution_init(civ_language_evolution_t *evolution);

civ_language_t *civ_language_create(const char *id, const char *name,
                                    const char *parent_id);
civ_language_t *civ_language_evolve_from(civ_language_evolution_t *evolution,
                                         const civ_language_t *parent,
                                         const char *new_id,
                                         const char *new_name);
void civ_language_destroy(civ_language_t *language);
civ_result_t civ_language_evolution_update(civ_language_evolution_t *evolution,
                                           civ_float_t time_delta);
civ_result_t civ_language_evolve(civ_language_t *language,
                                 civ_float_t time_delta);
civ_result_t civ_language_evolve_phonology(civ_language_t *language,
                                           civ_float_t intensity);
civ_result_t civ_language_evolve_grammar(civ_language_t *language,
                                         civ_float_t intensity);
civ_result_t civ_language_evolve_vocabulary(civ_language_t *language,
                                            civ_float_t intensity);

civ_float_t civ_language_calculate_similarity(const civ_language_t *a,
                                              const civ_language_t *b);
civ_result_t civ_language_evolution_add(civ_language_evolution_t *evolution,
                                        civ_language_t *language);
civ_language_t *
civ_language_evolution_find(const civ_language_evolution_t *evolution,
                            const char *id);

/* Vocabulary helper */
const char *civ_language_get_word(const civ_language_t *language,
                                  const char *concept_id);
civ_result_t civ_language_add_word(civ_language_t *language,
                                   const char *concept_id, const char *word);

/* Dialects & Borrowing */
civ_language_t *civ_language_split_dialect(civ_language_evolution_t *evolution,
                                           const civ_language_t *parent,
                                           const char *dialect_name);
civ_result_t civ_language_borrow_word(civ_language_t *target,
                                      const civ_language_t *source,
                                      const char *concept_id);

#endif /* CIVILIZATION_LANGUAGE_EVOLUTION_H */
