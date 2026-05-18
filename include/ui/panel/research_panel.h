#ifndef CIV_UI_PANEL_RESEARCH_H
#define CIV_UI_PANEL_RESEARCH_H

#include "../../core/game.h"
#include "../../engine/font.h"
#include "../../engine/input.h"
#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

void civ_research_panel_render(SDL_Renderer *r, civ_game_t *game,
                               civ_font_t *font, civ_input_state_t *input,
                               int win_w, int win_h);
bool civ_research_panel_click(civ_game_t *game, civ_input_state_t *input,
                              int win_w, int win_h);

#ifdef __cplusplus
}
#endif
#endif
