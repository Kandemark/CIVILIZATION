/**
 * @file writing_system.c
 * @brief Implementation of writing system
 */

#include "../../../include/core/culture/writing_system.h"
#include "../../../include/common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


/* RNG for name generation */
static uint32_t name_rng_state = 0;
static void name_rng_seed(uint32_t seed) { name_rng_state = seed; }
static uint32_t name_rng_next(void) {
  name_rng_state = name_rng_state * 1103515245 + 12345;
  return (name_rng_state / 65536) % 32768;
}
static int32_t name_rng_range(int32_t min, int32_t max) {
  return min + (name_rng_next() % (max - min + 1));
}

civ_writing_system_manager_t *civ_writing_system_manager_create(void) {
  civ_writing_system_manager_t *manager =
      (civ_writing_system_manager_t *)CIV_MALLOC(
          sizeof(civ_writing_system_manager_t));
  if (!manager) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate writing system manager");
    return NULL;
  }

  civ_writing_system_manager_init(manager);
  return manager;
}

void civ_writing_system_manager_destroy(civ_writing_system_manager_t *manager) {
  if (!manager)
    return;

  for (size_t i = 0; i < manager->script_count; i++) {
    civ_writing_system_destroy(&manager->scripts[i]);
  }
  CIV_FREE(manager->scripts);
  CIV_FREE(manager);
}

void civ_writing_system_manager_init(civ_writing_system_manager_t *manager) {
  if (!manager)
    return;

  memset(manager, 0, sizeof(civ_writing_system_manager_t));
  manager->script_capacity = 32;
  manager->scripts = (civ_writing_system_t *)CIV_CALLOC(
      manager->script_capacity, sizeof(civ_writing_system_t));
}

civ_writing_system_t *civ_writing_system_create(const char *id,
                                                const char *name,
                                                civ_script_type_t type) {
  if (!id || !name)
    return NULL;

  civ_writing_system_t *script =
      (civ_writing_system_t *)CIV_MALLOC(sizeof(civ_writing_system_t));
  if (!script) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate writing system");
    return NULL;
  }

  memset(script, 0, sizeof(civ_writing_system_t));
  strncpy(script->id, id, sizeof(script->id) - 1);
  strncpy(script->name, name, sizeof(script->name) - 1);
  script->type = type;
  script->complexity = 0.5f;
  script->efficiency = 0.5f;
  script->creation_time = time(NULL);

  return script;
}

void civ_writing_system_destroy(civ_writing_system_t *script) {
  if (!script)
    return;
  CIV_FREE(script->consonants);
  CIV_FREE(script->vowels);
  CIV_FREE(script->symbols);
}

civ_result_t civ_writing_system_set_characters(civ_writing_system_t *script,
                                               const char *consonants,
                                               const char *vowels) {
  civ_result_t result = {CIV_OK, NULL};

  if (!script || !consonants || !vowels) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  size_t cons_len = strlen(consonants) + 1;
  size_t vow_len = strlen(vowels) + 1;

  script->consonants = (char *)CIV_MALLOC(cons_len);
  script->vowels = (char *)CIV_MALLOC(vow_len);

  if (!script->consonants || !script->vowels) {
    CIV_FREE(script->consonants);
    CIV_FREE(script->vowels);
    result.error = CIV_ERROR_OUT_OF_MEMORY;
    return result;
  }

  strcpy(script->consonants, consonants);
  strcpy(script->vowels, vowels);
  script->consonant_count = strlen(consonants);
  script->vowel_count = strlen(vowels);

  return result;
}

civ_result_t
civ_writing_system_evolve_from(civ_writing_system_manager_t *manager,
                               const civ_writing_system_t *parent,
                               const char *new_id, const char *new_name) {
  civ_result_t result = {CIV_OK, NULL};

  if (!manager || !parent || !new_id || !new_name) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  civ_writing_system_t *new_script =
      civ_writing_system_create(new_id, new_name, parent->type);
  if (!new_script) {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
    return result;
  }

  strncpy(new_script->parent_script_id, parent->id,
          sizeof(new_script->parent_script_id) - 1);

  /* Inherit characters with variation */
  if (parent->consonants && parent->vowels) {
    /* Copy parent characters */
    size_t cons_len = strlen(parent->consonants) + 1;
    size_t vow_len = strlen(parent->vowels) + 1;
    new_script->consonants = (char *)CIV_MALLOC(cons_len);
    new_script->vowels = (char *)CIV_MALLOC(vow_len);

    if (new_script->consonants && new_script->vowels) {
      strcpy(new_script->consonants, parent->consonants);
      strcpy(new_script->vowels, parent->vowels);
      new_script->consonant_count = parent->consonant_count;
      new_script->vowel_count = parent->vowel_count;
    }
  }

  /* Slight variation in complexity */
  new_script->complexity = CLAMP(
      parent->complexity + (name_rng_next() % 100 - 50) * 0.01f, 0.0f, 1.0f);
  new_script->efficiency = CLAMP(
      parent->efficiency + (name_rng_next() % 100 - 50) * 0.01f, 0.0f, 1.0f);

  civ_writing_system_manager_add(manager, new_script);

  return result;
}

char *civ_writing_system_generate_name(const civ_writing_system_t *script,
                                       size_t min_length, size_t max_length) {
  if (!script || !script->consonants || !script->vowels)
    return NULL;

  size_t length =
      min_length + (name_rng_next() % (max_length - min_length + 1));
  char *name = (char *)CIV_MALLOC(length + 1);
  if (!name)
    return NULL;

  name_rng_seed((uint32_t)time(NULL));

  for (size_t i = 0; i < length; i++) {
    if (i % 2 == 0 && script->consonant_count > 0) {
      /* Consonant */
      int32_t idx = name_rng_range(0, (int32_t)script->consonant_count - 1);
      name[i] = script->consonants[idx];
    } else if (script->vowel_count > 0) {
      /* Vowel */
      int32_t idx = name_rng_range(0, (int32_t)script->vowel_count - 1);
      name[i] = script->vowels[idx];
    } else {
      name[i] = 'a' + (name_rng_next() % 26);
    }
  }
  name[length] = '\0';

  /* Capitalize first letter */
  if (name[0] >= 'a' && name[0] <= 'z') {
    name[0] = name[0] - 'a' + 'A';
  }

  return name;
}

civ_result_t civ_writing_system_evolve_symbols(civ_writing_system_t *script,
                                               civ_float_t intensity) {
  if (!script)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null script"};

  /* Procedurally shift character inventory */
  if (intensity > 0.1f && script->consonants) {
    size_t idx = name_rng_next() % script->consonant_count;
    /* Simple shift: replace a consonant with a neighbor in the alphabet for now
     */
    if (script->consonants[idx] >= 'a' && script->consonants[idx] < 'z') {
      script->consonants[idx]++;
    }
  }

  return (civ_result_t){CIV_OK, NULL};
}

civ_result_t
civ_writing_system_manager_add(civ_writing_system_manager_t *manager,
                               civ_writing_system_t *script) {
  civ_result_t result = {CIV_OK, NULL};

  if (!manager || !script) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  if (manager->script_count >= manager->script_capacity) {
    manager->script_capacity *= 2;
    manager->scripts = (civ_writing_system_t *)CIV_REALLOC(
        manager->scripts,
        manager->script_capacity * sizeof(civ_writing_system_t));
  }

  if (manager->scripts) {
    manager->scripts[manager->script_count++] = *script;
  } else {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

civ_writing_system_t *
civ_writing_system_manager_find(const civ_writing_system_manager_t *manager,
                                const char *id) {
  if (!manager || !id)
    return NULL;

  for (size_t i = 0; i < manager->script_count; i++) {
    if (strcmp(manager->scripts[i].id, id) == 0) {
      return (civ_writing_system_t *)&manager->scripts[i];
    }
  }

  return NULL;
}
