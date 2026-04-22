#include "ui/widget/modal.h"
#include "display/theme.h"
#include "engine/renderer.h"
#include <stdlib.h>
#include <string.h>

civ_modal_t *civ_modal_create(const char *id, const char *title,
                              const char *message) {
  civ_modal_t *m = calloc(1, sizeof(*m));
  if (!m) return NULL;
  civ_widget_init(&m->base, CIV_WIDGET_MODAL, id, 0, 0, 400, 200);
  if (title) strncpy(m->title, title, sizeof(m->title) - 1);
  if (message) strncpy(m->message, message, sizeof(m->message) - 1);
  m->is_open = false;
  m->overlay_alpha = 0.0f;
  m->color_overlay = 0x000000;
  m->color_bg = g_theme.bg_dark;
  m->color_border = g_theme.primary;
  m->has_confirm = true;
  m->has_cancel = true;
  return m;
}

void civ_modal_destroy(civ_modal_t *m) { free(m); }

void civ_modal_show(civ_modal_t *m) {
  if (!m) return;
  m->is_open = true;
  m->confirmed = m->cancelled = false;
  civ_tween_start(&m->fade, CIV_TWEEN_FADE_IN, 0.25f, 0.0f, 1.0f);
}

void civ_modal_hide(civ_modal_t *m) {
  if (!m) return;
  civ_tween_start(&m->fade, CIV_TWEEN_FADE_OUT, 0.2f, 1.0f, 0.0f);
}

bool civ_modal_is_open(const civ_modal_t *m) {
  return m ? m->is_open : false;
}

bool civ_modal_was_confirmed(const civ_modal_t *m) {
  return m ? m->confirmed : false;
}

bool civ_modal_was_cancelled(const civ_modal_t *m) {
  return m ? m->cancelled : false;
}

static bool pt_in(float px, float py, float rx, float ry, float rw, float rh) {
  return px >= rx && py >= ry && px <= rx + rw && py <= ry + rh;
}

void civ_modal_update(civ_modal_t *m, civ_input_state_t *input, float dt,
                      int win_w, int win_h) {
  if (!m || !input) return;
  civ_tween_update(&m->fade, dt);
  m->overlay_alpha = civ_tween_value(&m->fade);

  if (m->fade.done && !m->fade.active && m->overlay_alpha < 0.01f) {
    m->is_open = false;
    return;
  }

  if (!m->is_open) return;

  /* Center the panel */
  m->base.x = (float)(win_w - (int)m->base.w) / 2.0f;
  m->base.y = (float)(win_h - (int)m->base.h) / 2.0f;

  if (!input->mouse_left_pressed) return;

  /* Confirm button */
  float by = m->base.y + m->base.h - 40.0f;
  if (m->has_confirm && pt_in((float)input->mouse_x, (float)input->mouse_y,
      m->base.x + 20.0f, by, 140.0f, 30.0f)) {
    m->confirmed = true;
    civ_modal_hide(m);
  }
  /* Cancel button */
  if (m->has_cancel && pt_in((float)input->mouse_x, (float)input->mouse_y,
      m->base.x + m->base.w - 160.0f, by, 140.0f, 30.0f)) {
    m->cancelled = true;
    civ_modal_hide(m);
  }
}

void civ_modal_render(civ_modal_t *m, SDL_Renderer *r, int win_w, int win_h) {
  if (!m || !r || (!m->is_open && m->overlay_alpha < 0.01f)) return;
  (void)win_w; (void)win_h;

  /* Dark overlay */
  uint8_t oa = (uint8_t)(m->overlay_alpha * 140.0f);
  if (oa > 0) {
    SDL_SetRenderDrawColor(r, 0, 0, 0, oa);
    SDL_FRect full = { 0, 0, (float)win_w, (float)win_h };
    SDL_RenderFillRect(r, &full);
  }

  if (!m->is_open) return;

  int x = (int)m->base.x, y = (int)m->base.y;
  int w = (int)m->base.w, h = (int)m->base.h;

  /* Panel background */
  civ_render_rect_filled_alpha(r, x, y, w, h, m->color_bg, 245);
  civ_render_rect_outline(r, x, y, w, h, m->color_border, 1);

  /* Title bar */
  civ_render_rect_filled_alpha(r, x, y, w, 32, m->color_bg, 230);

  /* Confirm/Cancel buttons rendered by caller via font system */
}
