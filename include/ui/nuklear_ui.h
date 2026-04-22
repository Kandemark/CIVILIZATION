/**
 * @file nuklear_ui.h
 * @brief Nuklear immediate-mode GUI integration for Dominion
 *
 * Thin wrapper around Nuklear with SDL3 render + input backend.
 * Call nk_ui_init() once, nk_ui_begin()/nk_ui_end() each frame.
 * After nk_ui_begin(), use standard Nuklear API (nk_* functions).
 */
#ifndef CIV_UI_NUKLEAR_UI_H
#define CIV_UI_NUKLEAR_UI_H

#include <SDL3/SDL.h>

/* Include Nuklear */
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#include "nuklear.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Init the Nuklear context with SDL3 window + renderer. Call once. */
struct nk_context *nk_ui_init(SDL_Window *win, SDL_Renderer *renderer);

/* Call at start of each frame. Returns the Nuklear context for scene use. */
struct nk_context *nk_ui_begin(void);

/* Call at end of each frame. Renders all Nuklear draw commands. */
void nk_ui_end(void);

/* Shutdown and free resources. */
void nk_ui_shutdown(void);

/* Handle an SDL event (call from event loop). */
int nk_ui_handle_event(const SDL_Event *event);

/* Convenience: set default dark Dominion theme */
void nk_ui_theme_dominion(struct nk_context *ctx);

#ifdef __cplusplus
}
#endif
#endif
