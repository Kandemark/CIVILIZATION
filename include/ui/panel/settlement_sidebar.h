#ifndef CIV_UI_PANEL_SETTLEMENT_SIDEBAR_H
#define CIV_UI_PANEL_SETTLEMENT_SIDEBAR_H

#include "../../core/game.h"
#include "../../engine/font.h"
#include "../../engine/input.h"
#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

void civ_settlement_sidebar_render(SDL_Renderer *r, civ_settlement_t *sett,
                                   civ_font_t *font, civ_input_state_t *input);

/* Returns recruitment type: 0=none, 1=infantry, 2=settler */
int civ_settlement_sidebar_click(civ_settlement_t *sett,
                                 civ_input_state_t *input);

#ifdef __cplusplus
}
#endif
#endif
