/**
 * @file cities_data.c
 * @brief Loader and spatial-query interface for data/cities.bin
 */

#include "core/world/cities_data.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CITIES_MAGIC 0x54494349
#define GRID_CELLS    32

civ_cities_data_t *civ_cities_data_create(uint32_t map_w, uint32_t map_h) {
    civ_cities_data_t *cd = calloc(1, sizeof(*cd));
    if (!cd) return NULL;
    cd->capacity = CIV_CITIES_MAX;
    cd->cities = calloc(cd->capacity, sizeof(civ_city_data_t));
    if (!cd->cities) { free(cd); return NULL; }
    cd->map_width = map_w;
    cd->map_height = map_h;
    return cd;
}

void civ_cities_data_destroy(civ_cities_data_t *cd) {
    if (!cd) return;
    for (int gy = 0; gy < GRID_CELLS; gy++)
        for (int gx = 0; gx < GRID_CELLS; gx++)
            free(cd->grid[gy][gx].indices);
    free(cd->cities);
    free(cd);
}

static uint32_t read_u32(FILE *f) { uint32_t v; fread(&v, 4, 1, f); return v; }
static uint16_t read_u16(FILE *f) { uint16_t v; fread(&v, 2, 1, f); return v; }
static uint8_t  read_u8(FILE *f)  { uint8_t v;  fread(&v, 1, 1, f); return v; }

static void read_str(FILE *f, uint16_t len, char *out, size_t out_size) {
    if (len >= out_size) len = (uint16_t)(out_size - 1);
    if (len > 0) fread(out, 1, len, f);
    out[len] = '\0';
}

civ_result_t civ_cities_data_load(civ_cities_data_t *cd, const char *filepath) {
    if (!cd || !filepath) return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "NULL argument"};

    FILE *f = fopen(filepath, "rb");
    if (!f) return (civ_result_t){CIV_ERROR_IO, "Cannot open file"};

    uint32_t magic = read_u32(f);
    if (magic != CITIES_MAGIC) { fclose(f); return (civ_result_t){CIV_ERROR_INVALID_DATA, "Bad magic"}; }
    uint32_t version = read_u32(f); (void)version;
    uint32_t width = read_u32(f);
    uint32_t height = read_u32(f);
    uint32_t count = read_u32(f);

    if (count > cd->capacity) {
        civ_city_data_t *tmp = realloc(cd->cities, count * sizeof(civ_city_data_t));
        if (!tmp) { fclose(f); return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "realloc"}; }
        cd->cities = tmp;
        cd->capacity = count;
    }
    cd->count = count;

    /* Cell dimensions for spatial grid */
    uint32_t cell_w = (width + GRID_CELLS - 1) / GRID_CELLS;
    uint32_t cell_h = (height + GRID_CELLS - 1) / GRID_CELLS;

    for (uint32_t i = 0; i < count; i++) {
        civ_city_data_t *c = &cd->cities[i];

        uint16_t name_len = read_u16(f);
        read_str(f, name_len, c->name, CIV_CITY_NAME_MAX);
        uint8_t iso_len = read_u8(f);
        read_str(f, iso_len, c->iso_a2, sizeof(c->iso_a2));

        c->tile_x = read_u16(f);
        c->tile_y = read_u16(f);
        c->population = read_u32(f);
        c->capital_flag = read_u8(f);
        c->tier = read_u8(f);

        /* Insert into spatial grid */
        int gx = c->tile_x / cell_w;
        int gy = c->tile_y / cell_h;
        if (gx >= 0 && gx < GRID_CELLS && gy >= 0 && gy < GRID_CELLS) {
            uint32_t gc = cd->grid[gy][gx].count;
            uint32_t cap = cd->grid[gy][gx].capacity;
            if (gc >= cap) {
                cap = cap ? cap * 2 : 64;
                uint32_t *tmp = realloc(cd->grid[gy][gx].indices, cap * sizeof(uint32_t));
                if (!tmp) continue;
                cd->grid[gy][gx].indices = tmp;
                cd->grid[gy][gx].capacity = cap;
            }
            cd->grid[gy][gx].indices[gc] = i;
            cd->grid[gy][gx].count = gc + 1;
        }
    }

    fclose(f);
    return (civ_result_t){CIV_OK, "Loaded"};
}

const civ_city_data_t **civ_cities_query_tiles(
    const civ_cities_data_t *cd,
    int32_t tile_x, int32_t tile_y,
    int32_t tile_w, int32_t tile_h,
    uint32_t min_tier,
    uint32_t *out_count) {

    if (out_count) *out_count = 0;
    if (!cd || tile_w <= 0 || tile_h <= 0) return NULL;

    uint32_t cell_w = (cd->map_width + GRID_CELLS - 1) / GRID_CELLS;
    uint32_t cell_h = (cd->map_height + GRID_CELLS - 1) / GRID_CELLS;

    int gx_start = tile_x / (int32_t)cell_w;
    int gy_start = tile_y / (int32_t)cell_h;
    int gx_end   = (tile_x + tile_w) / (int32_t)cell_w;
    int gy_end   = (tile_y + tile_h) / (int32_t)cell_h;
    if (gx_start < 0) gx_start = 0;
    if (gy_start < 0) gy_start = 0;
    if (gx_end >= GRID_CELLS) gx_end = GRID_CELLS - 1;
    if (gy_end >= GRID_CELLS) gy_end = GRID_CELLS - 1;

    /* Collect matching cities */
    uint32_t capacity = 256;
    uint32_t count = 0;
    const civ_city_data_t **result = calloc(capacity, sizeof(civ_city_data_t *));
    if (!result) return NULL;

    int tx_end = tile_x + tile_w;
    int ty_end = tile_y + tile_h;

    for (int gy = gy_start; gy <= gy_end; gy++) {
        for (int gx = gx_start; gx <= gx_end; gx++) {
            uint32_t gc = cd->grid[gy][gx].count;
            const uint32_t *indices = cd->grid[gy][gx].indices;
            for (uint32_t k = 0; k < gc; k++) {
                const civ_city_data_t *c = &cd->cities[indices[k]];
                if (c->tier > min_tier) continue;
                /* Handle equatorial wrap for x */
                int cx = c->tile_x;
                if (cx < tile_x) cx += (int32_t)cd->map_width;
                if (cx >= tile_x && cx < tx_end &&
                    c->tile_y >= tile_y && c->tile_y < ty_end) {
                    if (count >= capacity) {
                        capacity *= 2;
                        const civ_city_data_t **tmp = realloc(
                            result, capacity * sizeof(civ_city_data_t *));
                        if (!tmp) { civ_cities_free_result(result); return NULL; }
                        result = tmp;
                    }
                    result[count++] = c;
                }
            }
        }
    }

    if (out_count) *out_count = count;
    return result;
}

const civ_city_data_t **civ_cities_for_country(
    const civ_cities_data_t *cd,
    const char *iso_a2,
    uint32_t *out_count) {

    if (out_count) *out_count = 0;
    if (!cd || !iso_a2) return NULL;

    uint32_t capacity = 64;
    uint32_t count = 0;
    const civ_city_data_t **result = calloc(capacity, sizeof(civ_city_data_t *));
    if (!result) return NULL;

    for (uint32_t i = 0; i < cd->count; i++) {
        if (cd->cities[i].iso_a2[0] && strcmp(cd->cities[i].iso_a2, iso_a2) == 0) {
            if (count >= capacity) {
                capacity *= 2;
                const civ_city_data_t **tmp = realloc(
                    result, capacity * sizeof(civ_city_data_t *));
                if (!tmp) { civ_cities_free_result(result); return NULL; }
                result = tmp;
            }
            result[count++] = &cd->cities[i];
        }
    }

    if (out_count) *out_count = count;
    return result;
}

void civ_cities_free_result(const civ_city_data_t **result) {
    free((void *)result);
}
