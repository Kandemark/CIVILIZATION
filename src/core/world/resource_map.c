/**
 * @file resource_map.c
 * @brief Loader and query interface for data/resources.bin
 */

#include "core/world/resource_map.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESOURCE_MAGIC 0x52534F52

static const char *RESOURCE_NAMES[CIV_RESOURCE_COUNT] = {
    "Oil", "Natural Gas", "Coal", "Iron", "Copper",
    "Bauxite", "Gold", "Silver", "Uranium", "Diamonds",
    "Phosphates", "Rare Earth", "Lithium", "Cobalt", "Tin",
    "Zinc", "Lead", "Nickel", "Chromite", "Manganese"
};

civ_resource_map_t *civ_resource_map_create(uint32_t width, uint32_t height) {
    civ_resource_map_t *rm = calloc(1, sizeof(*rm));
    if (!rm) return NULL;
    rm->width = width;
    rm->height = height;
    return rm;
}

void civ_resource_map_destroy(civ_resource_map_t *rm) {
    if (!rm) return;
    for (int i = 0; i < CIV_RESOURCE_COUNT; i++) {
        free(rm->deposits[i]);
    }
    free(rm);
}

static uint32_t read_u32(FILE *f) { uint32_t v; fread(&v, 4, 1, f); return v; }
static uint16_t read_u16(FILE *f) { uint16_t v; fread(&v, 2, 1, f); return v; }
static uint8_t  read_u8(FILE *f)  { uint8_t v;  fread(&v, 1, 1, f); return v; }

civ_result_t civ_resource_map_load(civ_resource_map_t *rm, const char *filepath) {
    if (!rm || !filepath) return (civ_result_t){CIV_ERROR_INVALID_ARGUMENT, "NULL argument"};

    FILE *f = fopen(filepath, "rb");
    if (!f) return (civ_result_t){CIV_ERROR_IO, "Cannot open file"};

    uint32_t magic = read_u32(f);
    if (magic != RESOURCE_MAGIC) { fclose(f); return (civ_result_t){CIV_ERROR_INVALID_DATA, "Bad magic"}; }
    uint32_t version = read_u32(f); (void)version;
    uint32_t width = read_u32(f);
    uint32_t height = read_u32(f);
    uint32_t type_count = read_u32(f);

    if (type_count > CIV_RESOURCE_COUNT) {
        fclose(f);
        return (civ_result_t){CIV_ERROR_INVALID_DATA, "Too many resource types"};
    }

    for (uint32_t t = 0; t < type_count; t++) {
        uint16_t count = read_u16(f);
        rm->deposit_count[t] = count;
        if (count == 0) continue;

        rm->deposits[t] = calloc(count, sizeof(civ_resource_deposit_t));
        if (!rm->deposits[t]) {
            fclose(f);
            return (civ_result_t){CIV_ERROR_OUT_OF_MEMORY, "calloc deposits"};
        }

        for (uint16_t i = 0; i < count; i++) {
            rm->deposits[t][i].x = read_u16(f);
            rm->deposits[t][i].y = read_u16(f);
            rm->deposits[t][i].quantity = read_u16(f);
            rm->deposits[t][i].quality = read_u8(f);
        }
    }

    fclose(f);
    return (civ_result_t){CIV_OK, "Loaded"};
}

/* Linear scan over sparse deposits for the given tile and type */
static const civ_resource_deposit_t *find_tile(
    const civ_resource_map_t *rm, int32_t x, int32_t y,
    civ_resource_type_t type) {

    if (!rm || x < 0 || y < 0 || (uint32_t)x >= rm->width || (uint32_t)y >= rm->height)
        return NULL;

    uint16_t count = rm->deposit_count[type];
    const civ_resource_deposit_t *deps = rm->deposits[type];
    if (!deps) return NULL;

    for (uint16_t i = 0; i < count; i++) {
        if (deps[i].x == (uint16_t)x && deps[i].y == (uint16_t)y)
            return &deps[i];
    }
    return NULL;
}

bool civ_resource_map_has_type(const civ_resource_map_t *rm,
                                int32_t x, int32_t y,
                                civ_resource_type_t type) {
    return find_tile(rm, x, y, type) != NULL;
}

uint16_t civ_resource_map_get_quantity(const civ_resource_map_t *rm,
                                        int32_t x, int32_t y,
                                        civ_resource_type_t type) {
    const civ_resource_deposit_t *d = find_tile(rm, x, y, type);
    return d ? d->quantity : 0;
}

uint8_t civ_resource_map_get_quality(const civ_resource_map_t *rm,
                                      int32_t x, int32_t y,
                                      civ_resource_type_t type) {
    const civ_resource_deposit_t *d = find_tile(rm, x, y, type);
    return d ? d->quality : 0;
}

uint16_t civ_resource_map_total_at_tile(const civ_resource_map_t *rm,
                                         int32_t x, int32_t y) {
    if (!rm) return 0;
    uint16_t total = 0;
    for (int t = 0; t < CIV_RESOURCE_COUNT; t++) {
        const civ_resource_deposit_t *d = find_tile(rm, x, y, t);
        if (d) total += d->quantity;
    }
    return total;
}

int civ_resource_map_type_count_at_tile(const civ_resource_map_t *rm,
                                         int32_t x, int32_t y) {
    if (!rm) return 0;
    int count = 0;
    for (int t = 0; t < CIV_RESOURCE_COUNT; t++) {
        if (find_tile(rm, x, y, t)) count++;
    }
    return count;
}

const char *civ_resource_type_name(civ_resource_type_t type) {
    if (type < 0 || type >= CIV_RESOURCE_COUNT) return "Unknown";
    return RESOURCE_NAMES[type];
}
