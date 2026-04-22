/**
 * @file nations_data.c
 * @brief Loader for data/nations.bin master nation index
 */

#include "core/world/nations_data.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NATIONS_MAGIC 0x4E414E54

civ_nations_data_t *civ_nations_data_create(void) {
    civ_nations_data_t *nd = calloc(1, sizeof(*nd));
    if (!nd) return NULL;
    nd->capacity = CIV_NATION_DATA_MAX;
    nd->nations = calloc(nd->capacity, sizeof(civ_nation_data_t));
    if (!nd->nations) { free(nd); return NULL; }
    return nd;
}

void civ_nations_data_destroy(civ_nations_data_t *nd) {
    if (!nd) return;
    free(nd->nations);
    free(nd);
}

static uint32_t read_u32(FILE *f) {
    uint32_t v;
    fread(&v, 4, 1, f);
    return v;
}

static uint16_t read_u16(FILE *f) {
    uint16_t v;
    fread(&v, 2, 1, f);
    return v;
}

static uint8_t read_u8(FILE *f) {
    uint8_t v;
    fread(&v, 1, 1, f);
    return v;
}

static void read_str(FILE *f, uint8_t len, char *out, size_t out_size) {
    if (len >= out_size) len = (uint8_t)(out_size - 1);
    if (len > 0) fread(out, 1, len, f);
    out[len] = '\0';
}

civ_result_t civ_nations_data_load(civ_nations_data_t *nd, const char *filepath) {
    if (!nd || !filepath) return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "NULL argument"};

    FILE *f = fopen(filepath, "rb");
    if (!f) return (civ_result_t){CIV_ERROR_IO, "Cannot open file"};

    uint32_t magic = read_u32(f);
    if (magic != NATIONS_MAGIC) { fclose(f); return (civ_result_t){CIV_ERROR_INVALID_DATA, "Bad magic"}; }
    uint32_t version = read_u32(f);
    (void)version;
    uint32_t count = read_u32(f);

    if (count > nd->capacity) {
        civ_nation_data_t *tmp = realloc(nd->nations, count * sizeof(civ_nation_data_t));
        if (!tmp) { fclose(f); return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "realloc"}; }
        nd->nations = tmp;
        nd->capacity = count;
    }
    nd->count = count;

    for (uint32_t i = 0; i < count; i++) {
        civ_nation_data_t *n = &nd->nations[i];

        n->id = read_u32(f);

        uint8_t iso_a3_len = read_u8(f);
        read_str(f, iso_a3_len, n->iso_a3, sizeof(n->iso_a3));
        uint8_t iso_a2_len = read_u8(f);
        read_str(f, iso_a2_len, n->iso_a2, sizeof(n->iso_a2));

        uint16_t name_len = read_u16(f);
        read_str(f, (uint8_t)(name_len > 255 ? 255 : name_len), n->name, CIV_NATION_NAME_MAX);

        n->color_rgb = read_u32(f);

        uint8_t cont_len = read_u8(f);
        read_str(f, cont_len, n->continent, sizeof(n->continent));
        uint8_t reg_len = read_u8(f);
        read_str(f, reg_len, n->region, sizeof(n->region));
        uint8_t sub_len = read_u8(f);
        read_str(f, sub_len, n->subregion, sizeof(n->subregion));

        n->population_est = read_u32(f);
        n->gdp_est_millions = read_u32(f);
        fread(&n->capital_lon, 4, 1, f);
        fread(&n->capital_lat, 4, 1, f);
        fread(&n->centroid_lon, 4, 1, f);
        fread(&n->centroid_lat, 4, 1, f);
        fread(&n->area_sqkm, 4, 1, f);
    }

    fclose(f);
    return (civ_result_t){CIV_OK, "Loaded"};
}

const civ_nation_data_t *civ_nations_data_get_by_id(
    const civ_nations_data_t *nd, uint32_t id) {
    if (!nd) return NULL;
    if (id < nd->count) return &nd->nations[id];
    return NULL;
}

const civ_nation_data_t *civ_nations_data_get_by_iso(
    const civ_nations_data_t *nd, const char *iso_a3) {
    if (!nd || !iso_a3) return NULL;
    for (uint32_t i = 0; i < nd->count; i++) {
        if (strcmp(nd->nations[i].iso_a3, iso_a3) == 0)
            return &nd->nations[i];
    }
    return NULL;
}

const civ_nation_data_t *civ_nations_data_get_by_name(
    const civ_nations_data_t *nd, const char *name) {
    if (!nd || !name) return NULL;
    for (uint32_t i = 0; i < nd->count; i++) {
        if (strcmp(nd->nations[i].name, name) == 0)
            return &nd->nations[i];
    }
    return NULL;
}
