#include "ui/widget/scroll_area.h"
#include "display/theme.h"
#include "engine/renderer.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

civ_scroll_area_t *civ_scroll_area_create(const char *id, float x, float y,
                                          float w, float h, int content_w,
                                          int content_h) {
  civ_scroll_area_t *sa =
      (civ_scroll_area_t *)malloc(sizeof(civ_scroll_area_t));
  if (!sa) return NULL;
  memset(sa, 0, sizeof(*sa));
  civ_widget_init(&sa->base, CIV_WIDGET_SCROLL_AREA, id, x, y, w, h);
  sa->content_w = content_w;
  sa->content_h = content_h;
  sa->scroll_x = sa->scroll_y = 0;
  sa->target_scroll_x = sa->target_scroll_y = 0;
  sa->scrollbar_w = 8;
  sa->color_bg = g_theme.bg_dark;
  sa->color_scrollbar = 0x1A2A3A;
  sa->color_scrollbar_hover = g_theme.primary;
  sa->content_tex = NULL;
  return sa;
}

void civ_scroll_area_destroy(civ_scroll_area_t *sa) {
  if (!sa) return;
  if (sa->content_tex) SDL_DestroyTexture(sa->content_tex);
  free(sa);
}

void civ_scroll_area_set_content_size(civ_scroll_area_t *sa, int cw, int ch) {
  if (!sa) return;
  sa->content_w = cw;
  sa->content_h = ch;
  if (sa->content_tex) SDL_DestroyTexture(sa->content_tex), sa->content_tex = NULL;
}

SDL_Renderer *civ_scroll_area_begin(civ_scroll_area_t *sa, SDL_Renderer *main_r) {
  if (!sa || !main_r) return NULL;

  /* Create content texture on first use */
  if (!sa->content_tex) {
    sa->content_tex = SDL_CreateTexture(main_r, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET, sa->content_w, sa->content_h);
    if (!sa->content_tex) return NULL;
  }

  /* Clear content texture */
  SDL_SetRenderTarget(main_r, sa->content_tex);
  SDL_SetRenderDrawColor(main_r, 0, 0, 0, 0);
  SDL_RenderClear(main_r);
  return main_r;
}

void civ_scroll_area_end(civ_scroll_area_t *sa, SDL_Renderer *main_r) {
  if (!sa || !main_r || !sa->content_tex) return;

  /* Reset render target to default */
  SDL_SetRenderTarget(main_r, NULL);

  /* Blit visible portion of content texture */
  int src_y = (int)sa->scroll_y;
  int src_h = (int)sa->base.h;
  if (src_y + src_h > sa->content_h) src_h = sa->content_h - src_y;
  if (src_h <= 0) return;

  SDL_FRect src = { 0, (float)src_y, (float)sa->content_w, (float)src_h };
  SDL_FRect dst = { sa->base.x, sa->base.y, sa->base.w, (float)src_h };
  SDL_RenderTexture(main_r, sa->content_tex, &src, &dst);
}

void civ_scroll_area_update(civ_scroll_area_t *sa, civ_input_state_t *input,
                            float dt) {
  if (!sa || !input || !sa->base.visible) return;

  /* Mouse wheel scroll */
  if (fabsf(input->scroll_delta) > 0.1f) {
    sa->target_scroll_y -= input->scroll_delta * 30.0f;
  }

  /* Clamp */
  float max_sy = (float)(sa->content_h - (int)sa->base.h);
  if (max_sy < 0) max_sy = 0;
  if (sa->target_scroll_y < 0) sa->target_scroll_y = 0;
  if (sa->target_scroll_y > max_sy) sa->target_scroll_y = max_sy;

  /* Smooth scroll */
  float lerp = 1.0f - expf(-12.0f * dt);
  sa->scroll_y += (sa->target_scroll_y - sa->scroll_y) * lerp;

  /* Scrollbar drag */
  float sb_x = sa->base.x + sa->base.w - (float)sa->scrollbar_w;
  float sb_h = (sa->content_h > 0)
                   ? sa->base.h * sa->base.h / (float)sa->content_h
                   : sa->base.h;
  if (sb_h < 20.0f) sb_h = 20.0f;
  float sb_y = sa->base.y + (sa->scroll_y / (float)sa->content_h) * sa->base.h;

  sa->hover_scrollbar =
      civ_input_is_mouse_over(input, (int)sb_x, (int)sb_y,
                              sa->scrollbar_w, (int)sb_h);

  if (sa->hover_scrollbar && input->mouse_left_pressed) {
    sa->dragging_scrollbar = true;
    sa->drag_start_y = (float)input->mouse_y;
    sa->drag_start_scroll = sa->target_scroll_y;
  }
  if (sa->dragging_scrollbar) {
    if (input->mouse_left_down) {
      float dy = (float)input->mouse_y - sa->drag_start_y;
      sa->target_scroll_y =
          sa->drag_start_scroll + dy * (float)sa->content_h / sa->base.h;
    } else {
      sa->dragging_scrollbar = false;
    }
  }
}

void civ_scroll_area_render(civ_scroll_area_t *sa, SDL_Renderer *r) {
  if (!sa || !r || !sa->base.visible) return;

  int x = (int)sa->base.x, y = (int)sa->base.y;
  int w = (int)sa->base.w, h = (int)sa->base.h;

  civ_render_rect_filled(r, x, y, w, h, sa->color_bg);

  /* Scrollbar */
  if (sa->content_h > h) {
    float sb_h = (float)(h * h) / (float)sa->content_h;
    if (sb_h < 20.0f) sb_h = 20.0f;
    float sb_y_offset = (sa->scroll_y / (float)sa->content_h) * (float)h;
    int sb_x = x + w - sa->scrollbar_w;
    uint32_t sb_c = sa->hover_scrollbar || sa->dragging_scrollbar
                        ? sa->color_scrollbar_hover
                        : sa->color_scrollbar;
    civ_render_rect_filled_alpha(r, sb_x, y, sa->scrollbar_w, h,
                                 sa->color_scrollbar, 80);
    civ_render_rect_filled(r, sb_x + 1, (int)((float)y + sb_y_offset),
                           sa->scrollbar_w - 2, (int)sb_h, sb_c);
  }

  /* Border */
  civ_render_rect_outline(r, x, y, w, h, 0x1A2A3A, 1);
}
