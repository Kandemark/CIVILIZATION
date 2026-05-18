/**
 * @file debug_overlay.h
 * @brief FPS counter and draw-call statistics overlay
 */
#ifndef CIV_DISPLAY_DEBUG_OVERLAY_H
#define CIV_DISPLAY_DEBUG_OVERLAY_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  bool enabled;
  double frame_time_ms;
  double smoothed_fps;
  int draw_calls;
  double avg_frame_ms;
  double min_frame_ms;
  double max_frame_ms;
  int frame_count;
  double accumulator;
} civ_debug_overlay_t;

void civ_debug_overlay_init(civ_debug_overlay_t *d);
void civ_debug_overlay_update(civ_debug_overlay_t *d, double dt_ms,
                              int draw_calls);
void civ_debug_overlay_render(civ_debug_overlay_t *d, SDL_Renderer *r, int win_w);

#ifdef __cplusplus
}
#endif
#endif
