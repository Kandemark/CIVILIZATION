#ifndef CIV_UI_WIDGET_DROPDOWN_H
#define CIV_UI_WIDGET_DROPDOWN_H

#include "widget_base.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_DROPDOWN_MAX_ITEMS 32
#define CIV_DROPDOWN_ITEM_LEN  64

typedef struct {
  civ_widget_base_t base;
  char  items[CIV_DROPDOWN_MAX_ITEMS][CIV_DROPDOWN_ITEM_LEN];
  int   item_count;
  int   selected_index;
  bool  is_open;
  float list_h;
  uint32_t color_bg, color_border, color_item, color_item_hover;
} civ_dropdown_t;

civ_dropdown_t *civ_dropdown_create(const char *id, float x, float y,
                                    float w, float h);
void civ_dropdown_destroy(civ_dropdown_t *dd);
void civ_dropdown_add_item(civ_dropdown_t *dd, const char *text);
void civ_dropdown_set_selected(civ_dropdown_t *dd, int index);
int  civ_dropdown_get_selected(const civ_dropdown_t *dd);
const char *civ_dropdown_selected_text(const civ_dropdown_t *dd);
void civ_dropdown_update(civ_dropdown_t *dd, civ_input_state_t *input, float dt);
void civ_dropdown_render(civ_dropdown_t *dd, SDL_Renderer *r);

#ifdef __cplusplus
}
#endif
#endif
