#include "display/debug_overlay.h"
#include <stdio.h>

void civ_debug_overlay_init(civ_debug_overlay_t *d) {
  d->enabled = true;
  d->frame_time_ms = 0.0;
  d->smoothed_fps = 0.0;
  d->draw_calls = 0;
  d->avg_frame_ms = 0.0;
  d->min_frame_ms = 9999.0;
  d->max_frame_ms = 0.0;
  d->frame_count = 0;
  d->accumulator = 0.0;
}

void civ_debug_overlay_update(civ_debug_overlay_t *d, double dt_ms,
                              int draw_calls) {
  d->frame_time_ms = dt_ms;
  d->draw_calls = draw_calls;
  d->frame_count++;
  d->accumulator += dt_ms;

  if (d->frame_time_ms < d->min_frame_ms) d->min_frame_ms = d->frame_time_ms;
  if (d->frame_time_ms > d->max_frame_ms) d->max_frame_ms = d->frame_time_ms;

  d->avg_frame_ms = d->accumulator / (double)d->frame_count;

  double alpha = 0.05;
  d->smoothed_fps = d->smoothed_fps * (1.0 - alpha) +
                    (dt_ms > 0.0 ? (1000.0 / dt_ms) * alpha : 0.0);

  if (d->frame_count > 600) {
    d->frame_count = 0;
    d->accumulator = 0.0;
    d->min_frame_ms = 9999.0;
    d->max_frame_ms = 0.0;
  }
}

void civ_debug_overlay_render(civ_debug_overlay_t *d, SDL_Renderer *r,
                              int win_w) {
  if (!d->enabled || !r) return;

  char buf[128];
  int y = 4;
  int h = 16;
  int x = win_w - 200;

  SDL_SetRenderDrawColor(r, 0, 0, 0, 160);
  SDL_FRect bg = {(float)x - 4, (float)y - 2, 200.0f, 56.0f};
  SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
  SDL_RenderFillRect(r, &bg);

  SDL_SetRenderDrawColor(r, 0, 200, 200, 255);
  SDL_FRect line;
  snprintf(buf, sizeof(buf), "FPS: %.0f  %.1fms", d->smoothed_fps,
           d->frame_time_ms);
  /* Text rendering needs the font system — placeholder drawn via primitives */
  /* Actual text rendering handled by the panel layer using civ_font */

  snprintf(buf, sizeof(buf), "draws: %d  avg: %.1fms", d->draw_calls,
           d->avg_frame_ms);
  (void)buf; /* Used when font renderer is attached */
  (void)line;
}
