/**
 * @file profile.c
 * @brief Player profile and per-profile save-slot management.
 */

#include "../../include/core/profile.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PROFILES_DIR "saves/profiles"
#define PROFILE_SAVE_SLOT_DIR "slots"

static void ensure_profiles_dir(void) {
  SDL_CreateDirectory("saves");
  SDL_CreateDirectory(PROFILES_DIR);
}

static void build_profile_dir(const char *id, char *out_path, size_t size) {
  snprintf(out_path, size, "%s/%s", PROFILES_DIR, id);
}

static void build_profile_meta_path(const char *id, char *out_path,
                                    size_t size) {
  snprintf(out_path, size, "%s/%s/profile.dat", PROFILES_DIR, id);
}

static void ensure_profile_dirs(const char *id) {
  char profile_dir[256];
  char slots_dir[320];

  build_profile_dir(id, profile_dir, sizeof(profile_dir));
  snprintf(slots_dir, sizeof(slots_dir), "%s/%s", profile_dir,
           PROFILE_SAVE_SLOT_DIR);

  SDL_CreateDirectory(profile_dir);
  SDL_CreateDirectory(slots_dir);
}

civ_player_profile_t *civ_profile_create(const char *name) {
  civ_player_profile_t *profile =
      (civ_player_profile_t *)malloc(sizeof(civ_player_profile_t));
  if (!profile)
    return NULL;

  memset(profile, 0, sizeof(civ_player_profile_t));

  size_t name_len = strlen(name);
  if (name_len >= CIV_PROFILE_NAME_MAX)
    name_len = CIV_PROFILE_NAME_MAX - 1;
  memcpy(profile->name, name, name_len);
  profile->name[name_len] = '\0';

  snprintf(profile->id, CIV_PROFILE_ID_MAX, "%s_%lld", name,
           (long long)time(NULL));

  for (int i = 0; profile->id[i]; i++) {
    char c = profile->id[i];
    if (c == ' ' || c == '/' || c == '\\' || c == ':' || c == '.') {
      profile->id[i] = '_';
    }
  }

  profile->created_at = (uint64_t)time(NULL);
  profile->last_played = (uint64_t)time(NULL);

  return profile;
}

void civ_profile_destroy(civ_player_profile_t *profile) {
  if (profile) {
    free(profile);
  }
}

bool civ_profile_save(civ_player_profile_t *profile) {
  if (!profile)
    return false;

  ensure_profiles_dir();
  ensure_profile_dirs(profile->id);

  char path[256];
  build_profile_meta_path(profile->id, path, sizeof(path));

  SDL_IOStream *io = SDL_IOFromFile(path, "wb");
  if (!io)
    return false;

  size_t written = SDL_WriteIO(io, profile, sizeof(civ_player_profile_t));
  SDL_CloseIO(io);

  return (written == sizeof(civ_player_profile_t));
}

civ_player_profile_t *civ_profile_load(const char *id) {
  char path[256];
  build_profile_meta_path(id, path, sizeof(path));

  SDL_IOStream *io = SDL_IOFromFile(path, "rb");
  if (!io)
    return NULL;

  civ_player_profile_t *profile =
      (civ_player_profile_t *)malloc(sizeof(civ_player_profile_t));
  if (!profile) {
    SDL_CloseIO(io);
    return NULL;
  }

  size_t read = SDL_ReadIO(io, profile, sizeof(civ_player_profile_t));
  SDL_CloseIO(io);

  if (read != sizeof(civ_player_profile_t)) {
    free(profile);
    return NULL;
  }

  return profile;
}

typedef struct {
  char **profiles;
  int count;
  int capacity;
} ProfileListContext;

static SDL_EnumerationResult SDLCALL list_profiles_callback(void *userdata,
                                                            const char *dirname,
                                                            const char *fname) {
  (void)dirname;
  ProfileListContext *ctx = (ProfileListContext *)userdata;

  if (fname[0] == '.')
    return SDL_ENUM_CONTINUE;

  if (ctx->count >= ctx->capacity) {
    int new_cap = ctx->capacity ? ctx->capacity * 2 : 4;
    char **new_list = (char **)realloc(ctx->profiles, new_cap * sizeof(char *));
    if (!new_list)
      return SDL_ENUM_FAILURE;
    ctx->profiles = new_list;
    ctx->capacity = new_cap;
  }

  size_t len = strlen(fname);
  ctx->profiles[ctx->count] = (char *)malloc(len + 1);
  if (!ctx->profiles[ctx->count])
    return SDL_ENUM_FAILURE;
  memcpy(ctx->profiles[ctx->count], fname, len + 1);
  ctx->count++;

  return SDL_ENUM_CONTINUE;
}

int civ_profile_list(char ***out_profiles) {
  ensure_profiles_dir();

  ProfileListContext ctx = {0};
  SDL_EnumerateDirectory(PROFILES_DIR, list_profiles_callback, &ctx);

  if (out_profiles) {
    *out_profiles = ctx.profiles;
  } else {
    civ_profile_free_list(ctx.profiles, ctx.count);
  }

  return ctx.count;
}

void civ_profile_free_list(char **profiles, int count) {
  if (!profiles)
    return;
  for (int i = 0; i < count; i++) {
    free(profiles[i]);
  }
  free(profiles);
}

bool civ_profile_get_save_path(const char *profile_id, const char *slot_name,
                               char *out_path, size_t out_path_size) {
  if (!profile_id || !slot_name || !out_path || out_path_size == 0)
    return false;

  ensure_profiles_dir();
  ensure_profile_dirs(profile_id);

  int written = snprintf(out_path, out_path_size, "%s/%s/%s/%s.civ",
                         PROFILES_DIR, profile_id, PROFILE_SAVE_SLOT_DIR,
                         slot_name);
  return written > 0 && (size_t)written < out_path_size;
}

typedef struct {
  char **entries;
  int count;
  int capacity;
} SaveListContext;

static SDL_EnumerationResult SDLCALL list_saves_callback(void *userdata,
                                                         const char *dirname,
                                                         const char *fname) {
  (void)dirname;
  SaveListContext *ctx = (SaveListContext *)userdata;

  const char *dot = strrchr(fname, '.');
  if (!(dot && strcmp(dot, ".civ") == 0))
    return SDL_ENUM_CONTINUE;

  if (ctx->count >= ctx->capacity) {
    int new_cap = ctx->capacity ? ctx->capacity * 2 : 4;
    char **new_entries =
        (char **)realloc(ctx->entries, new_cap * sizeof(char *));
    if (!new_entries)
      return SDL_ENUM_FAILURE;
    ctx->entries = new_entries;
    ctx->capacity = new_cap;
  }

  size_t len = (size_t)(dot - fname);
  ctx->entries[ctx->count] = (char *)malloc(len + 1);
  if (!ctx->entries[ctx->count])
    return SDL_ENUM_FAILURE;

  memcpy(ctx->entries[ctx->count], fname, len);
  ctx->entries[ctx->count][len] = '\0';
  ctx->count++;
  return SDL_ENUM_CONTINUE;
}

int civ_profile_list_saves(const char *profile_id, char ***out_saves) {
  if (!profile_id)
    return 0;

  ensure_profiles_dir();
  ensure_profile_dirs(profile_id);

  char slots_dir[320];
  snprintf(slots_dir, sizeof(slots_dir), "%s/%s/%s", PROFILES_DIR, profile_id,
           PROFILE_SAVE_SLOT_DIR);

  SaveListContext ctx = {0};
  SDL_EnumerateDirectory(slots_dir, list_saves_callback, &ctx);

  if (out_saves) {
    *out_saves = ctx.entries;
  } else {
    civ_profile_free_list(ctx.entries, ctx.count);
  }

  return ctx.count;
}
