/**
 * @file vexillology.c
 * @brief Implementation of multi-layer flag design
 */

#include "../../../include/core/visuals/vexillology.h"
#include "../../../include/common.h"
#include <stdio.h>
#include <string.h>


void civ_vexillology_init(civ_flag_t *flag) {
  if (!flag)
    return;
  memset(flag, 0, sizeof(civ_flag_t));
  flag->layer_count = 0;
  strncpy(flag->description, "A blank slate.", STRING_MEDIUM_LEN - 1);

  /* Add base layer */
  civ_vexillology_add_layer(flag, CIV_FLAG_PLAIN, CIV_SYMBOL_NONE, "#FFFFFF",
                            "#000000");
}

void civ_vexillology_add_layer(civ_flag_t *flag, civ_flag_pattern_t pattern,
                               civ_flag_symbol_t symbol, const char *p_color,
                               const char *s_color) {
  if (!flag || flag->layer_count >= 4)
    return;

  civ_flag_layer_t *layer = &flag->layers[flag->layer_count];
  layer->pattern = pattern;
  layer->symbol = symbol;
  if (p_color)
    strncpy(layer->primary_color, p_color, 15);
  if (s_color)
    strncpy(layer->secondary_color, s_color, 15);

  flag->layer_count++;

  /* Update description */
  snprintf(flag->description, STRING_MEDIUM_LEN - 1,
           "Flag with %d layers. Core pattern: %d, Symbol: %d.",
           (int)flag->layer_count, (int)flag->layers[0].pattern,
           (int)flag->layers[0].symbol);

  flag->uniqueness_hash = civ_vexillology_calculate_hash(flag);
}

uint64_t civ_vexillology_calculate_hash(const civ_flag_t *flag) {
  if (!flag)
    return 0;
  uint64_t hash = 5381;
  for (int i = 0; i < flag->layer_count; i++) {
    hash = ((hash << 5) + hash) + flag->layers[i].pattern;
    hash = ((hash << 5) + hash) + flag->layers[i].symbol;
  }
  return hash;
}

const char *civ_vexillology_describe(const civ_flag_t *flag) {
  if (!flag)
    return "Invalid Flag";
  return flag->description;
}
