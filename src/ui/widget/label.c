#include "../../../include/ui/widget/label.h"
#include "../../../include/display/theme.h"
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
  lbl->base.enabled = false; /* labels don't receive input by default */
  return lbl;
}

void civ_label_destroy(civ_label_t *lbl) { free(lbl); }

void civ_label_set_text(civ_label_t *lbl, const char *text) {
  if (!lbl) return;
  strncpy(lbl->text, text ? text : "", sizeof(lbl->text) - 1);
  lbl->text[sizeof(lbl->text) - 1] = '\0';
}

void civ_label_render(civ_label_t *lbl, SDL_Renderer *r) {
  (void)lbl;
  (void)r;
  /* Text rendering uses the font system — handled by owner */
}
