/**
 * @file flag_system.c
 * @brief Flag texture loader and renderer — PNG via stb_image.h
 */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "core/world/flag_system.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLAG_MAGIC 0x47414C46

civ_flag_system_t *civ_flag_system_create(SDL_Renderer *renderer) {
    civ_flag_system_t *fs = calloc(1, sizeof(*fs));
    if (!fs) return NULL;
    fs->capacity = CIV_FLAG_MAX_COUNT;
    fs->flags = calloc(fs->capacity, sizeof(civ_flag_entry_t));
    if (!fs->flags) { free(fs); return NULL; }
    fs->renderer = renderer;
    return fs;
}

void civ_flag_system_destroy(civ_flag_system_t *fs) {
    if (!fs) return;
    for (uint32_t i = 0; i < fs->count; i++) {
        if (fs->flags[i].texture) SDL_DestroyTexture(fs->flags[i].texture);
    }
    free(fs->flags);
    free(fs);
}

static uint32_t read_u32(FILE *f) { uint32_t v; fread(&v, 4, 1, f); return v; }
static uint16_t read_u16(FILE *f) { uint16_t v; fread(&v, 2, 1, f); return v; }

civ_result_t civ_flag_system_load(civ_flag_system_t *fs,
                                   const char *index_path,
                                   const char *flags_dir) {
    if (!fs || !index_path || !flags_dir)
        return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "NULL argument"};

    FILE *f = fopen(index_path, "rb");
    if (!f) return (civ_result_t){CIV_ERROR_IO, "Cannot open file"};

    uint32_t magic = read_u32(f);
    if (magic != FLAG_MAGIC) { fclose(f); return (civ_result_t){CIV_ERROR_INVALID_DATA, "Bad magic"}; }
    uint32_t version = read_u32(f); (void)version;
    uint32_t count = read_u32(f);

    if (count > fs->capacity) {
        civ_flag_entry_t *tmp = realloc(fs->flags, count * sizeof(civ_flag_entry_t));
        if (!tmp) { fclose(f); return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "realloc"}; }
        fs->flags = tmp;
        fs->capacity = count;
    }
    fs->count = count;

    /* Build flags directory path for later texture loading */
    snprintf(fs->flags_dir, sizeof(fs->flags_dir), "%s", flags_dir);

    for (uint32_t i = 0; i < count; i++) {
        civ_flag_entry_t *fe = &fs->flags[i];
        fe->country_id = read_u32(f);

        uint8_t name_len = 0;
        fread(&name_len, 1, 1, f);
        if (name_len >= sizeof(fe->filename)) name_len = sizeof(fe->filename) - 1;
        if (name_len > 0) fread(fe->filename, 1, name_len, f);
        fe->filename[name_len] = '\0';

        fe->width = read_u16(f);
        fe->height = read_u16(f);
        fe->texture = NULL;  /* loaded lazily */
    }

    fclose(f);
    return (civ_result_t){CIV_OK, "Loaded"};
}

/* Load all PNG textures — requires SDL_Renderer, call after window created */
int civ_flag_system_load_textures(civ_flag_system_t *fs) {
    if (!fs || !fs->renderer) return 0;
    int loaded = 0;

    for (uint32_t i = 0; i < fs->count; i++) {
        civ_flag_entry_t *fe = &fs->flags[i];
        if (fe->texture) continue;  /* already loaded */

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", fs->flags_dir, fe->filename);

        int w, h, channels;
        unsigned char *pixels = stbi_load(full_path, &w, &h, &channels, 4);
        if (!pixels) continue;

        SDL_Texture *tex = SDL_CreateTexture(fs->renderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, w, h);
        if (!tex) { stbi_image_free(pixels); continue; }

        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        SDL_UpdateTexture(tex, NULL, pixels, w * 4);
        stbi_image_free(pixels);

        fe->texture = tex;
        fe->width = w;
        fe->height = h;
        loaded++;
    }

    return loaded;
}

const civ_flag_entry_t *civ_flag_system_get(const civ_flag_system_t *fs,
                                             uint32_t country_id) {
    if (!fs) return NULL;
    for (uint32_t i = 0; i < fs->count; i++) {
        if (fs->flags[i].country_id == country_id)
            return &fs->flags[i];
    }
    return NULL;
}

/* Look up flag by ISO-A2 code */
const civ_flag_entry_t *civ_flag_system_get_by_iso(const civ_flag_system_t *fs,
                                                    const char *iso_a2) {
    if (!fs || !iso_a2) return NULL;
    /* Build expected filename: "xx.png" */
    char expected[16];
    snprintf(expected, sizeof(expected), "%s.png", iso_a2);
    for (uint32_t i = 0; i < fs->count; i++) {
        if (strcasecmp(fs->flags[i].filename, expected) == 0)
            return &fs->flags[i];
    }
    return NULL;
}

void civ_flag_render(SDL_Renderer *r, const civ_flag_entry_t *flag,
                     int x, int y, int w, int h) {
    if (!r || !flag) return;
    if (flag->texture) {
        SDL_FRect dst = { (float)x, (float)y, (float)w, (float)h };
        SDL_RenderTexture(r, flag->texture, NULL, &dst);
    } else {
        /* Fallback: colored rectangle with border */
        SDL_FRect dst = { (float)x, (float)y, (float)w, (float)h };
        SDL_SetRenderDrawColor(r, 0x33, 0x33, 0x33, 255);
        SDL_RenderFillRect(r, &dst);
        SDL_SetRenderDrawColor(r, 0x66, 0x66, 0x66, 255);
        SDL_RenderRect(r, &dst);
    }
}
