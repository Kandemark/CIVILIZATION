/**
 * @file screens.h
 * @brief Individual screen render functions — one per game screen
 *
 * Each screen has a single render function. scene_game.c just calls
 * the right one based on current_screen. This keeps scene_game.c lean
 * and makes each screen independently maintainable.
 */
#ifndef CIV_UI_SCREENS_H
#define CIV_UI_SCREENS_H

#include "../../core/game.h"
#include "../../engine/font.h"
#include "../../engine/input.h"
#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Screen render signature: renderer, game, font, content area, total screen height, input, local currency */
typedef void (*civ_screen_render_fn)(SDL_Renderer*, civ_game_t*, civ_font_t*,
                                     int x, int y, int w, int h, int scr_h,
                                     civ_input_state_t*, const char *cur, const char *sym);

void civ_screen_dashboard_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f, int x, int y, int w, int h, int sh, civ_input_state_t *in, const char *cur, const char *sym);
void civ_screen_work_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f, int x, int y, int w, int h, int sh, civ_input_state_t *in, const char *cur, const char *sym);
void civ_screen_finance_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f, int x, int y, int w, int h, int sh, civ_input_state_t *in, const char *cur, const char *sym);
void civ_screen_housing_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f, int x, int y, int w, int h, int sh, civ_input_state_t *in, const char *cur, const char *sym);
void civ_screen_education_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f, int x, int y, int w, int h, int sh, civ_input_state_t *in, const char *cur, const char *sym);
void civ_screen_network_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f, int x, int y, int w, int h, int sh, civ_input_state_t *in, const char *cur, const char *sym);
void civ_screen_politics_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f, int x, int y, int w, int h, int sh, civ_input_state_t *in, const char *cur, const char *sym);
void civ_screen_health_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f, int x, int y, int w, int h, int sh, civ_input_state_t *in, const char *cur, const char *sym);
void civ_screen_constitution_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f, int x, int y, int w, int h, int sh, civ_input_state_t *in, const char *cur, const char *sym);
void civ_screen_economy_render(SDL_Renderer *r, civ_game_t *g, civ_font_t *f, int x, int y, int w, int h, int sh, civ_input_state_t *in, const char *cur, const char *sym);

#ifdef __cplusplus
}
#endif
#endif
