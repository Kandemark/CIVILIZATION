#ifndef CIV_UI_WIDGET_TAB_BAR_H
#define CIV_UI_WIDGET_TAB_BAR_H

#include "widget_base.h"
#include <SDL3/SDL.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_TAB_BAR_MAX_TABS 16
#define CIV_TAB_LABEL_LEN    32

typedef struct {
  civ_widget_base_t base;
  char  labels[CIV_TAB_BAR_MAX_TABS][CIV_TAB_LABEL_LEN];
  int   tab_count;
  int   active_tab;      /* -1 = none */
  float tab_width;
  uint32_t color_bg, color_active, color_hover, color_text;
} civ_tab_bar_t;

civ_tab_bar_t *civ_tab_bar_create(const char *id, float x, float y,
                                  float w, float h);
void civ_tab_bar_destroy(civ_tab_bar_t *tb);
void civ_tab_bar_add_tab(civ_tab_bar_t *tb, const char *label);
void civ_tab_bar_set_active(civ_tab_bar_t *tb, int index);
int  civ_tab_bar_get_active(const civ_tab_bar_t *tb);
void civ_tab_bar_update(civ_tab_bar_t *tb, civ_input_state_t *input, float dt);
void civ_tab_bar_render(civ_tab_bar_t *tb, SDL_Renderer *r);

#ifdef __cplusplus
}
#endif
#endif
