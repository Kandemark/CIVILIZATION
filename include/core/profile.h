/**
 * @file profile.h
 * @brief Player profile management
 */

#ifndef CIVILIZATION_PROFILE_H
#define CIVILIZATION_PROFILE_H

#include "../common.h"
#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#define CIV_PROFILE_NAME_MAX 32
#define CIV_PROFILE_ID_MAX 32
#define CIV_PROFILE_PATH_MAX 64

typedef struct {
  char name[CIV_PROFILE_NAME_MAX];
  char id[CIV_PROFILE_ID_MAX]; // UUID or sanitized name
  uint64_t created_at;
  uint64_t last_played;
  char avatar_path[CIV_PROFILE_PATH_MAX];
} civ_player_profile_t;

/**
 * Create a new player profile
 * @param name Player name
 * @return New profile or NULL on failure
 */
civ_player_profile_t *civ_profile_create(const char *name);

/**
 * Destroy profile object (free memory)
 * @param profile Profile to destroy
 */
void civ_profile_destroy(civ_player_profile_t *profile);

/**
 * Save profile to disk
 * @param profile Profile to save
 * @return True on success
 */
bool civ_profile_save(civ_player_profile_t *profile);

/**
 * Load profile from disk by ID
 * @param id Profile ID
 * @return Loaded profile or NULL
 */
civ_player_profile_t *civ_profile_load(const char *id);

/**
 * List available profiles
 * @param out_profiles Pointer to array of strings (allocated)
 * @return Number of profiles found
 */
int civ_profile_list(char ***out_profiles);

/**
 * Free profile list
 * @param profiles Array of strings
 * @param count Number of profiles
 */
void civ_profile_free_list(char **profiles, int count);

#ifdef __cplusplus
}
#endif

#endif /* CIVILIZATION_PROFILE_H */
