/**
 * @file state_persistence.c
 * @brief Implementation of state persistence
 */

#include "../../../include/core/simulation_engine/state_persistence.h"
#include "../../../include/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

civ_state_persistence_t *
civ_state_persistence_create(const char *save_directory) {
  civ_state_persistence_t *sp =
      (civ_state_persistence_t *)CIV_MALLOC(sizeof(civ_state_persistence_t));
  if (!sp) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate state persistence");
    return NULL;
  }

  civ_state_persistence_init(sp, save_directory);
  return sp;
}

void civ_state_persistence_destroy(civ_state_persistence_t *sp) {
  if (!sp)
    return;
  CIV_FREE(sp->save_directory);
  CIV_FREE(sp);
}

void civ_state_persistence_init(civ_state_persistence_t *sp,
                                const char *save_directory) {
  if (!sp)
    return;

  memset(sp, 0, sizeof(civ_state_persistence_t));
  if (save_directory) {
    size_t len = strlen(save_directory) + 1;
    sp->save_directory = (char *)CIV_MALLOC(len);
    if (sp->save_directory) {
      strcpy(sp->save_directory, save_directory);
    }
  }
  sp->compression_enabled = false;
  sp->encryption_enabled = false;
}

civ_result_t civ_state_persistence_save(civ_state_persistence_t *sp,
                                        const char *filename, const void *data,
                                        size_t data_size) {
  civ_result_t result = {CIV_OK, NULL};

  if (!sp || !filename || !data) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  /* Build full path */
  char full_path[512];
  if (sp->save_directory) {
    snprintf(full_path, sizeof(full_path), "%s/%s", sp->save_directory,
             filename);
  } else {
    strncpy(full_path, filename, sizeof(full_path) - 1);
  }

  FILE *file = fopen(full_path, "wb");
  if (!file) {
    result.error = CIV_ERROR_IO;
    result.message = "Failed to open file for writing";
    return result;
  }

  size_t written = fwrite(data, 1, data_size, file);
  fclose(file);

  if (written != data_size) {
    result.error = CIV_ERROR_IO;
    result.message = "Failed to write all data";
  }

  return result;
}

civ_result_t civ_state_persistence_load(civ_state_persistence_t *sp,
                                        const char *filename, void *data,
                                        size_t *data_size) {
  civ_result_t result = {CIV_OK, NULL};

  if (!sp || !filename || !data || !data_size) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  /* Build full path */
  char full_path[512];
  if (sp->save_directory) {
    snprintf(full_path, sizeof(full_path), "%s/%s", sp->save_directory,
             filename);
  } else {
    strncpy(full_path, filename, sizeof(full_path) - 1);
  }

  FILE *file = fopen(full_path, "rb");
  if (!file) {
    return (civ_result_t){.error = CIV_ERROR_NOT_FOUND,
                          .message = "File not found"};
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (*data_size < (size_t)file_size) {
    fclose(file);
    result.error = CIV_ERROR_INVALID_ARGUMENT;
    result.message = "Buffer too small";
    return result;
  }

  size_t read = fread(data, 1, file_size, file);
  fclose(file);

  if (read != (size_t)file_size) {
    result.error = CIV_ERROR_IO;
    result.message = "Failed to read all data";
  } else {
    *data_size = read;
  }

  return result;
}

civ_result_t civ_state_persistence_list_saves(civ_state_persistence_t *sp,
                                              char **filenames, size_t *count) {
  civ_result_t result = {CIV_OK, NULL};

  if (!sp || !filenames || !count) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  /* Simplified - would need platform-specific directory listing */
  *count = 0;
  return (civ_result_t){.error = CIV_ERROR_INVALID_STATE,
                        .message = "Not implemented"};
}
