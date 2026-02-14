/**
 * @file profile.c
 * @brief Player profile management implementation
 */

#include "../../include/core/profile.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PROFILES_DIR "saves/profiles"

static void ensure_profiles_dir(void) {
  printf("Ensuring profiles directory exists...\n");
  if (!SDL_CreateDirectory("saves")) {
    printf("SDL_CreateDirectory('saves') returned false (might exist)\n");
  }
  if (!SDL_CreateDirectory(PROFILES_DIR)) {
    printf("SDL_CreateDirectory('%s') returned false (might exist)\n",
           PROFILES_DIR);
  }
}

static void get_profile_path(const char *id, char *out_path, size_t size) {
  snprintf(out_path, size, "%s/%s.dat", PROFILES_DIR, id);
}

civ_player_profile_t *civ_profile_create(const char *name) {
  civ_player_profile_t *profile =
      (civ_player_profile_t *)malloc(sizeof(civ_player_profile_t));
  if (!profile)
    return NULL;

  memset(profile, 0, sizeof(civ_player_profile_t));

  // Copy safely
  size_t name_len = strlen(name);
  if (name_len >= CIV_PROFILE_NAME_MAX)
    name_len = CIV_PROFILE_NAME_MAX - 1;
  memcpy(profile->name, name, name_len);
  profile->name[name_len] = '\0';

  // Simple ID generation: sanitize name + timestamp
  snprintf(profile->id, CIV_PROFILE_ID_MAX, "%s_%lld", name,
           (long long)time(NULL));

  // Sanitize ID (remove spaces/special chars)
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

  char path[256];
  get_profile_path(profile->id, path, sizeof(path));

  SDL_IOStream *io = SDL_IOFromFile(path, "wb");
  if (!io)
    return false;

  size_t written = SDL_WriteIO(io, profile, sizeof(civ_player_profile_t));
  SDL_CloseIO(io);

  return (written == sizeof(civ_player_profile_t));
}

civ_player_profile_t *civ_profile_load(const char *id) {
  char path[256];
  get_profile_path(id, path, sizeof(path));

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

static SDL_EnumerationResult SDLCALL list_callback(void *userdata,
                                                   const char *dirname,
                                                   const char *fname) {
  ProfileListContext *ctx = (ProfileListContext *)userdata;

  const char *dot = strrchr(fname, '.');
  if (dot && strcmp(dot, ".dat") == 0) {
    if (ctx->count >= ctx->capacity) {
      int new_cap = ctx->capacity ? ctx->capacity * 2 : 4;
      char **new_list =
          (char **)realloc(ctx->profiles, new_cap * sizeof(char *));
      if (!new_list)
        return SDL_ENUM_FAILURE;
      ctx->profiles = new_list;
      ctx->capacity = new_cap;
    }

    // Extract ID (filename without extension)
    size_t len = dot - fname;
    ctx->profiles[ctx->count] = (char *)malloc(len + 1);
    memcpy(ctx->profiles[ctx->count], fname, len);
    ctx->profiles[ctx->count][len] = '\0';
    ctx->count++;
  }
  return SDL_ENUM_CONTINUE;
}

int civ_profile_list(char ***out_profiles) {
  ensure_profiles_dir();

  ProfileListContext ctx = {0};

  SDL_EnumerateDirectory(PROFILES_DIR, list_callback, &ctx);

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
