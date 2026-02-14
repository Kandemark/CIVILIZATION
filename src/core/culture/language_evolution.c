/**
 * @file language_evolution.c
 * @brief Implementation of language evolution system
 */

#include "../../../include/core/culture/language_evolution.h"
#include "../../../include/common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Simple RNG for language evolution */
static uint32_t lang_rng_state = 0;
static civ_float_t lang_rng_float(void) {
  lang_rng_state = lang_rng_state * 1103515245 + 12345;
  return (civ_float_t)((lang_rng_state / 65536) % 32768) / 32768.0f;
}

civ_language_evolution_t *civ_language_evolution_create(void) {
  civ_language_evolution_t *evolution =
      (civ_language_evolution_t *)CIV_MALLOC(sizeof(civ_language_evolution_t));
  if (!evolution) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate language evolution");
    return NULL;
  }

  civ_language_evolution_init(evolution);
  return evolution;
}

void civ_language_evolution_destroy(civ_language_evolution_t *evolution) {
  if (!evolution)
    return;

  for (size_t i = 0; i < evolution->language_count; i++) {
    civ_language_destroy(&evolution->languages[i]);
  }
  CIV_FREE(evolution->languages);
  CIV_FREE(evolution);
}

void civ_language_evolution_init(civ_language_evolution_t *evolution) {
  if (!evolution)
    return;

  memset(evolution, 0, sizeof(civ_language_evolution_t));
  evolution->evolution_rate = 0.001f;
  evolution->divergence_threshold = 0.3f;
  evolution->language_capacity = 32;
  evolution->languages = (civ_language_t *)CIV_CALLOC(
      evolution->language_capacity, sizeof(civ_language_t));
}

civ_language_t *civ_language_create(const char *id, const char *name,
                                    const char *parent_id) {
  if (!id || !name)
    return NULL;

  civ_language_t *language =
      (civ_language_t *)CIV_MALLOC(sizeof(civ_language_t));
  if (!language) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate language");
    return NULL;
  }

  memset(language, 0, sizeof(civ_language_t));
  strncpy(language->id, id, sizeof(language->id) - 1);
  strncpy(language->name, name, sizeof(language->name) - 1);
  if (parent_id) {
    strncpy(language->parent_id, parent_id, sizeof(language->parent_id) - 1);
  }

  /* Default phonology */
  strcpy(language->phonology.consonants, "ptkmns");
  strcpy(language->phonology.vowels, "aiu");
  language->phonology.nasal_ratio = 0.2f;
  language->phonology.fricative_ratio = 0.2f;

  /* Default grammar */
  language->grammar.morphology = CIV_MORPH_ISOLATING;
  language->grammar.word_order = CIV_SYNTAX_SVO;

  language->vocabulary_capacity = 64;
  language->vocabulary = (civ_vocab_entry_t *)CIV_CALLOC(
      language->vocabulary_capacity, sizeof(civ_vocab_entry_t));

  language->loanword_capacity = 16;
  language->loanwords = (civ_vocab_entry_t *)CIV_CALLOC(
      language->loanword_capacity, sizeof(civ_vocab_entry_t));

  language->complexity = 0.5f;
  language->prestige = 0.5f;
  language->speakers = 1.0f;
  language->creation_time = time(NULL);
  language->last_evolution = language->creation_time;

  return language;
}

void civ_language_destroy(civ_language_t *language) {
  if (!language)
    return;
  CIV_FREE(language->vocabulary);
  CIV_FREE(language->loanwords);
}

civ_result_t civ_language_evolution_update(civ_language_evolution_t *evolution,
                                           civ_float_t time_delta) {
  civ_result_t result = {CIV_OK, NULL};

  if (!evolution) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  /* Evolve all languages */
  for (size_t i = 0; i < evolution->language_count; i++) {
    civ_language_evolve(&evolution->languages[i], time_delta);
  }

  return result;
}

civ_result_t civ_language_evolve(civ_language_t *language,
                                 civ_float_t time_delta) {
  civ_result_t result = {CIV_OK, NULL};

  if (!language) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  /* Language complexity evolves based on speakers and prestige */
  const civ_float_t evolution_rate = 0.001f;
  civ_float_t complexity_change =
      (language->speakers * language->prestige - language->complexity) *
      evolution_rate * time_delta;
  language->complexity =
      CLAMP(language->complexity + complexity_change, 0.0f, 1.0f);

  /* Prestige changes based on speakers and complexity */
  civ_float_t prestige_change =
      (language->speakers * language->complexity - language->prestige) *
      evolution_rate * time_delta * 0.5f;
  language->prestige = CLAMP(language->prestige + prestige_change, 0.0f, 1.0f);

  /* Feature evolution */
  civ_language_evolve_phonology(language, evolution_rate * time_delta);
  civ_language_evolve_grammar(language, evolution_rate * time_delta);
  civ_language_evolve_vocabulary(language, evolution_rate * time_delta);

  language->last_evolution = time(NULL);
  return result;
}

civ_result_t civ_language_evolve_phonology(civ_language_t *language,
                                           civ_float_t intensity) {
  /* Randomly add/remove/swap phonemes */
  if (intensity > 0.05f) {
    /* Placeholder for actual procedural phoneme shift */
    language->phonology.nasal_ratio += (lang_rng_float() - 0.5f) * 0.1f;
  }
  return (civ_result_t){CIV_OK, NULL};
}

civ_result_t civ_language_evolve_grammar(civ_language_t *language,
                                         civ_float_t intensity) {
  /* Randomly shift grammar rules */
  if (intensity > 0.1f) {
    if (lang_rng_float() < 0.01f) {
      language->grammar.word_order =
          (civ_syntax_type_t)((int)lang_rng_float() * 6);
    }
  }
  return (civ_result_t){CIV_OK, NULL};
}

civ_result_t civ_language_evolve_vocabulary(civ_language_t *language,
                                            civ_float_t intensity) {
  /* Shift words slightly (sound change) */
  for (size_t i = 0; i < language->vocabulary_size; i++) {
    if (lang_rng_float() < intensity) {
      /* Apply a simple sound shift: vowels change to next vowel */
      char *v =
          strpbrk(language->vocabulary[i].word, language->phonology.vowels);
      if (v) {
        size_t v_idx = strcspn(language->phonology.vowels, (char[]){*v, '\0'});
        if (v_idx < strlen(language->phonology.vowels) - 1) {
          *v = language->phonology.vowels[v_idx + 1];
        }
      }
    }
  }
  return (civ_result_t){CIV_OK, NULL};
}

civ_float_t civ_language_calculate_similarity(const civ_language_t *a,
                                              const civ_language_t *b) {
  if (!a || !b)
    return 0.0f;

  /* Check if one is parent of the other */
  civ_float_t parent_similarity = 0.0f;
  if (strlen(a->parent_id) > 0 && strcmp(a->parent_id, b->id) == 0) {
    parent_similarity = 0.6f; /* Child inherits from parent */
  } else if (strlen(b->parent_id) > 0 && strcmp(b->parent_id, a->id) == 0) {
    parent_similarity = 0.6f;
  } else if (strlen(a->parent_id) > 0 && strlen(b->parent_id) > 0 &&
             strcmp(a->parent_id, b->parent_id) == 0) {
    parent_similarity = 0.4f; /* Siblings share parent */
  }

  /* Complexity similarity */
  civ_float_t complexity_similarity =
      1.0f - (civ_float_t)fabs((double)(a->complexity - b->complexity));

  /* Combined similarity */
  return CLAMP(parent_similarity + complexity_similarity * 0.4f, 0.0f, 1.0f);
}

/* Vocabulary helper */
const char *civ_language_get_word(const civ_language_t *language,
                                  const char *concept_id) {
  if (!language || !concept_id)
    return NULL;

  for (size_t i = 0; i < language->vocabulary_size; i++) {
    if (strcmp(language->vocabulary[i].concept_id, concept_id) == 0) {
      return language->vocabulary[i].word;
    }
  }

  return NULL;
}

civ_result_t civ_language_add_word(civ_language_t *language,
                                   const char *concept_id, const char *word) {
  civ_result_t result = {CIV_OK, NULL};

  if (!language || !concept_id || !word) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  /* Expand if needed */
  if (language->vocabulary_size >= language->vocabulary_capacity) {
    language->vocabulary_capacity *= 2;
    language->vocabulary = (civ_vocab_entry_t *)CIV_REALLOC(
        language->vocabulary,
        language->vocabulary_capacity * sizeof(civ_vocab_entry_t));
  }

  if (language->vocabulary) {
    strncpy(language->vocabulary[language->vocabulary_size].concept_id,
            concept_id, STRING_SHORT_LEN - 1);
    strncpy(language->vocabulary[language->vocabulary_size].word, word,
            STRING_SHORT_LEN - 1);
    language->vocabulary_size++;
  } else {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

civ_language_t *civ_language_evolve_from(civ_language_evolution_t *evolution,
                                         const civ_language_t *parent,
                                         const char *new_id,
                                         const char *new_name) {
  if (!evolution || !parent || !new_id || !new_name)
    return NULL;

  /* Create new language evolved from parent */
  civ_language_t *new_lang = civ_language_create(new_id, new_name, parent->id);
  if (!new_lang)
    return NULL;

  /* Inherit characteristics with variation */
  new_lang->complexity =
      CLAMP(parent->complexity + (lang_rng_float() - 0.5f) * 0.2f, 0.0f, 1.0f);
  new_lang->prestige =
      parent->prestige * 0.8f; /* New language starts with less prestige */
  new_lang->speakers = parent->speakers * 0.1f; /* Starts with fewer speakers */

  /* Add to evolution system */
  civ_language_evolution_add(evolution, new_lang);

  return new_lang;
}

civ_result_t civ_language_evolution_add(civ_language_evolution_t *evolution,
                                        civ_language_t *language) {
  civ_result_t result = {CIV_OK, NULL};

  if (!evolution || !language) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  /* Expand if needed */
  if (evolution->language_count >= evolution->language_capacity) {
    evolution->language_capacity *= 2;
    evolution->languages = (civ_language_t *)CIV_REALLOC(
        evolution->languages,
        evolution->language_capacity * sizeof(civ_language_t));
  }

  if (evolution->languages) {
    evolution->languages[evolution->language_count++] = *language;
  } else {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

civ_language_t *
civ_language_evolution_find(const civ_language_evolution_t *evolution,
                            const char *id) {
  if (!evolution || !id)
    return NULL;

  for (size_t i = 0; i < evolution->language_count; i++) {
    if (strcmp(evolution->languages[i].id, id) == 0) {
      return (civ_language_t *)&evolution->languages[i];
    }
  }

  return NULL;
}

civ_language_t *civ_language_split_dialect(civ_language_evolution_t *evolution,
                                           const civ_language_t *parent,
                                           const char *dialect_name) {
  if (!evolution || !parent || !dialect_name)
    return NULL;

  char dialect_id[STRING_SHORT_LEN];
  snprintf(dialect_id, STRING_SHORT_LEN, "%s_dia", parent->id);

  civ_language_t *dialect =
      civ_language_evolve_from(evolution, parent, dialect_id, dialect_name);
  if (dialect) {
    /* Dialects are extremely similar to parent initially */
    dialect->complexity = parent->complexity;
    dialect->prestige = parent->prestige * 0.9f;
  }
  return dialect;
}

civ_result_t civ_language_borrow_word(civ_language_t *target,
                                      const civ_language_t *source,
                                      const char *concept_id) {
  if (!target || !source || !concept_id)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null pointer"};

  const char *word = civ_language_get_word(source, concept_id);
  if (!word)
    return (civ_result_t){CIV_ERROR_NOT_FOUND, "Source word not found"};

  if (target->loanword_count >= target->loanword_capacity) {
    target->loanword_capacity *= 2;
    target->loanwords = (civ_vocab_entry_t *)CIV_REALLOC(
        target->loanwords,
        target->loanword_capacity * sizeof(civ_vocab_entry_t));
  }

  if (target->loanwords) {
    strncpy(target->loanwords[target->loanword_count].concept_id, concept_id,
            STRING_SHORT_LEN - 1);
    strncpy(target->loanwords[target->loanword_count].word, word,
            STRING_SHORT_LEN - 1);
    target->loanword_count++;
    return (civ_result_t){CIV_OK, NULL};
  }

  return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "OOM"};
}
