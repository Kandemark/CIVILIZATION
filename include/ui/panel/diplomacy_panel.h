/**
 * @file diplomacy_panel.h
 * @brief Diplomacy panel — relations, treaties, stances, war declarations
 */
#ifndef CIV_UI_PANEL_DIPLOMACY_H
#define CIV_UI_PANEL_DIPLOMACY_H

#include "../../core/game.h"
#include "../../engine/font.h"
#include "../../engine/input.h"
#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

void civ_diplomacy_panel_render(SDL_Renderer *r, civ_game_t *game,
                                civ_font_t *font, civ_input_state_t *input,
                                int win_w, int win_h, bool has_contacted_rival);

/* Handle click input — returns true if click was consumed */
bool civ_diplomacy_panel_click(civ_game_t *game, civ_input_state_t *input,
                               int win_w, int win_h, bool has_contacted_rival);

#ifdef __cplusplus
}
#endif
#endif
