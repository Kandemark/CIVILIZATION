#include "ui/widget/label.h"
#include "display/theme.h"
#include "engine/font.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

civ_label_t *civ_label_create(const char *id, float x, float y, float w, float h,
                              const char *text) {
  civ_label_t *lbl = (civ_label_t *)malloc(sizeof(civ_label_t));
  if (!lbl) return NULL;
  memset(lbl, 0, sizeof(*lbl));
  civ_widget_init(&lbl->base, CIV_WIDGET_LABEL, id, x, y, w, h);
  if (text) strncpy(lbl->text, text, sizeof(lbl->text) - 1);
  lbl->color = g_theme.text_primary;
  lbl->align = CIV_LABEL_LEFT;
  lbl->font_size = g_theme.font_md;
  lbl->font = civ_font_load_system("Inter", lbl->font_size);
  if (!lbl->font) lbl->font = civ_font_load_system("Segoe UI", lbl->font_size);
  lbl->base.enabled = false;
  return lbl;
}

void civ_label_destroy(civ_label_t *lbl) {
  if (!lbl) return;
  if (lbl->font) civ_font_destroy(lbl->font);
  free(lbl);
}

void civ_label_set_text(civ_label_t *lbl, const char *text) {
  if (!lbl) return;
  strncpy(lbl->text, text ? text : "", sizeof(lbl->text) - 1);
  lbl->text[sizeof(lbl->text) - 1] = '\0';
}

void civ_label_set_font(civ_label_t *lbl, struct civ_font *font) {
  if (!lbl) return;
  lbl->font = font;
}

void civ_label_render(civ_label_t *lbl, SDL_Renderer *r) {
  if (!lbl || !lbl->base.visible || !lbl->text[0]) return;
  if (!lbl->font) return;

  civ_text_align_t ta = (lbl->align == CIV_LABEL_CENTER) ? CIV_ALIGN_CENTER
                       : (lbl->align == CIV_LABEL_RIGHT) ? CIV_ALIGN_RIGHT
                       : CIV_ALIGN_LEFT;

  civ_font_render_aligned(r, lbl->font, lbl->text,
      (int)lbl->base.x, (int)lbl->base.y,
      (int)lbl->base.w, (int)lbl->base.h,
      lbl->color, ta, CIV_VALIGN_MIDDLE);
}
