/**
 * @file flag_system.h
 * @brief National flag texture management loaded from disk
 *
 * Flags are downloaded by tools/generate_flags.py from flagcdn.com
 * as PNG files and stored in data/flags/. Indexed by country_id.
 * PNG textures are loaded via stb_image.h. Falls back to colored
 * rectangles for nations without flag data.
 */
#ifndef CIV_FLAG_SYSTEM_H
#define CIV_FLAG_SYSTEM_H

#include "../../common.h"
#include <SDL3/SDL.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_FLAG_MAX_COUNT  512

/* ── Single flag entry ─────────────────────────────────────────── */
typedef struct {
    uint32_t    country_id;
    SDL_Texture *texture;       /* loaded PNG, or NULL if failed */
    int         width, height;
    char        filename[64];
} civ_flag_entry_t;

/* ── Flag system ────────────────────────────────────────────────── */
typedef struct {
    civ_flag_entry_t *flags;
    uint32_t          count;
    uint32_t          capacity;
    SDL_Renderer     *renderer;
    char              flags_dir[256];
} civ_flag_system_t;

/* ── Lifecycle ─────────────────────────────────────────────────── */
civ_flag_system_t *civ_flag_system_create(SDL_Renderer *renderer);
void               civ_flag_system_destroy(civ_flag_system_t *fs);

/* Load index from data/flags/index.bin (metadata only, no textures) */
civ_result_t       civ_flag_system_load(civ_flag_system_t *fs,
                                        const char *index_path,
                                        const char *flags_dir);

/* Load all PNG textures via stb_image — requires SDL_Renderer */
int                civ_flag_system_load_textures(civ_flag_system_t *fs);

/* ── Lookup ────────────────────────────────────────────────────── */
const civ_flag_entry_t *civ_flag_system_get(const civ_flag_system_t *fs,
                                             uint32_t country_id);
const civ_flag_entry_t *civ_flag_system_get_by_iso(const civ_flag_system_t *fs,
                                                    const char *iso_a2);

/* ── Render ────────────────────────────────────────────────────── */
/* Draw flag scaled to the given rect */
void civ_flag_render(SDL_Renderer *r, const civ_flag_entry_t *flag,
                     int x, int y, int w, int h);

#ifdef __cplusplus
}
#endif
#endif
