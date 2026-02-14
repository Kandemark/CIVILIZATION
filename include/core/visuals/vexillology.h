/**
 * @file vexillology.h
 * @brief Procedural Flag Design and Validation
 */

#ifndef CIVILIZATION_VEXILLOLOGY_H
#define CIVILIZATION_VEXILLOLOGY_H

#include "../../common.h"
#include "../../types.h"

/* Flag Pattern */
typedef enum {
  CIV_FLAG_HORIZONTAL_STRIPES = 0,
  CIV_FLAG_VERTICAL_STRIPES,
  CIV_FLAG_SALTIRE, /* Diagonal cross */
  CIV_FLAG_CROSS,
  CIV_FLAG_TRI_COLOR,
  CIV_FLAG_CANTON, /* Corner square */
  CIV_FLAG_PALE,   /* Center vertical bar */
  CIV_FLAG_FESS,   /* Center horizontal bar */
  CIV_FLAG_PLAIN
} civ_flag_pattern_t;

/* Flag Symbol */
typedef enum {
  CIV_SYMBOL_NONE = 0,
  CIV_SYMBOL_LION,    /* Power/Royal */
  CIV_SYMBOL_EAGLE,   /* Vision/Empire */
  CIV_SYMBOL_STAR,    /* Unity/Progress */
  CIV_SYMBOL_WHEAT,   /* Prosperity/Food */
  CIV_SYMBOL_HAMMER,  /* Labor/Industry */
  CIV_SYMBOL_ANCHOR,  /* Maritime/Trade */
  CIV_SYMBOL_SUN,     /* Divine/Knowledge */
  CIV_SYMBOL_MOON,    /* Night/Mystery */
  CIV_SYMBOL_MOUNTAIN /* Resilience */
} civ_flag_symbol_t;

/* Flag Layer */
typedef struct {
  civ_flag_pattern_t pattern;
  civ_flag_symbol_t symbol;
  char primary_color[16];
  char secondary_color[16];
} civ_flag_layer_t;

/* National Flag Definition */
typedef struct {
  civ_flag_layer_t layers[4]; /* Support up to 4 layers */
  uint8_t layer_count;
  char description[STRING_MEDIUM_LEN];
  uint64_t uniqueness_hash;
} civ_flag_t;

/* Functions */
void civ_vexillology_init(civ_flag_t *flag);
void civ_vexillology_add_layer(civ_flag_t *flag, civ_flag_pattern_t pattern,
                               civ_flag_symbol_t symbol, const char *p_color,
                               const char *s_color);
const char *civ_vexillology_describe(const civ_flag_t *flag);
uint64_t civ_vexillology_calculate_hash(const civ_flag_t *flag);

#endif /* CIVILIZATION_VEXILLOLOGY_H */
