#ifndef CIV_UI_PANEL_UNIT_SIDEBAR_H
#define CIV_UI_PANEL_UNIT_SIDEBAR_H

#include "../../core/game.h"
#include "../../core/military/combat.h"
#include "../../engine/font.h"
#include "../../engine/input.h"
#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

void civ_unit_sidebar_render(SDL_Renderer *r, civ_unit_t *unit, civ_font_t *font,
                             civ_input_state_t *input);

/* Returns button click type: 0=none, 1=found city */
int civ_unit_sidebar_click(civ_unit_t *unit, civ_input_state_t *input);

#ifdef __cplusplus
}
#endif
#endif
