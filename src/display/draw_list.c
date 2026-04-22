#include "display/draw_list.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAP 4096

void civ_draw_list_init(civ_draw_list_t *dl, int initial_capacity) {
  int cap = initial_capacity > 0 ? initial_capacity : INITIAL_CAP;
  dl->cmds = (civ_draw_cmd_t *)calloc((size_t)cap, sizeof(civ_draw_cmd_t));
  dl->count = 0;
  dl->capacity = dl->cmds ? cap : 0;
  dl->draw_calls_this_frame = 0;
  dl->last_frame_ms = 0.0;
}

void civ_draw_list_clear(civ_draw_list_t *dl) {
  dl->count = 0;
  dl->draw_calls_this_frame = 0;
  dl->last_frame_ms = 0.0;
}

void civ_draw_list_destroy(civ_draw_list_t *dl) {
  free(dl->cmds);
  dl->cmds = NULL;
  dl->count = 0;
  dl->capacity = 0;
}

static void ensure_capacity(civ_draw_list_t *dl) {
  if (dl->count >= dl->capacity) {
    int new_cap = dl->capacity * 2;
    civ_draw_cmd_t *new_cmds =
        (civ_draw_cmd_t *)realloc(dl->cmds, (size_t)new_cap * sizeof(civ_draw_cmd_t));
    if (new_cmds) {
      dl->cmds = new_cmds;
      dl->capacity = new_cap;
    }
  }
}

static void push_cmd(civ_draw_list_t *dl, civ_draw_type_t type, float x, float y,
                     float w, float h, uint32_t color, uint8_t alpha,
                     int thickness, SDL_Texture *tex, const char *text,
                     int layer) {
  ensure_capacity(dl);
  civ_draw_cmd_t *cmd = &dl->cmds[dl->count++];
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = type;
  cmd->x = x;
  cmd->y = y;
  cmd->w = w;
  cmd->h = h;
  cmd->color = color;
  cmd->alpha = alpha;
  cmd->thickness = thickness;
  cmd->texture = tex;
  cmd->text = text;
  cmd->layer = layer;
  cmd->sort_key = ((uint32_t)layer << 24) | (uint32_t)(y * 10.0f);
}

void civ_draw_rect_filled(civ_draw_list_t *dl, float x, float y, float w, float h,
                          uint32_t color, uint8_t alpha, int layer) {
  push_cmd(dl, CIV_DRAW_RECT_FILLED, x, y, w, h, color, alpha, 0, NULL, NULL,
           layer);
}

void civ_draw_rect_outline(civ_draw_list_t *dl, float x, float y, float w, float h,
                           uint32_t color, int thickness, int layer) {
  push_cmd(dl, CIV_DRAW_RECT_OUTLINE, x, y, w, h, color, 255, thickness, NULL,
           NULL, layer);
}

void civ_draw_line(civ_draw_list_t *dl, float x1, float y1, float x2, float y2,
                   uint32_t color, int layer) {
  push_cmd(dl, CIV_DRAW_LINE, x1, y1, x2, y2, color, 255, 1, NULL, NULL, layer);
}

void civ_draw_text(civ_draw_list_t *dl, const char *text, float x, float y,
                   uint32_t color, int layer) {
  push_cmd(dl, CIV_DRAW_TEXT, x, y, 0, 0, color, 255, 0, NULL, text, layer);
}

static void flush_one(civ_draw_cmd_t *cmd, SDL_Renderer *r) {
  SDL_SetRenderDrawColor(r, (cmd->color >> 16) & 0xFF, (cmd->color >> 8) & 0xFF,
                         cmd->color & 0xFF, cmd->alpha);
  SDL_FRect rect = {cmd->x, cmd->y, cmd->w, cmd->h};

  switch (cmd->type) {
  case CIV_DRAW_RECT_FILLED:
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(r, &rect);
    break;
  case CIV_DRAW_RECT_OUTLINE:
    for (int i = 0; i < cmd->thickness; i++) {
      SDL_FRect or_rect = {cmd->x + (float)i, cmd->y + (float)i,
                           cmd->w - 2.0f * (float)i, cmd->h - 2.0f * (float)i};
      SDL_RenderRect(r, &or_rect);
    }
    break;
  case CIV_DRAW_LINE:
    SDL_RenderLine(r, cmd->x, cmd->y, cmd->w, cmd->h);
    break;
  case CIV_DRAW_TEXTURE:
    if (cmd->texture) SDL_RenderTexture(r, cmd->texture, NULL, &rect);
    break;
  default:
    break;
  }
}

static int cmd_compare(const void *a, const void *b) {
  const civ_draw_cmd_t *ca = (const civ_draw_cmd_t *)a;
  const civ_draw_cmd_t *cb = (const civ_draw_cmd_t *)b;
  if (ca->sort_key != cb->sort_key)
    return (ca->sort_key > cb->sort_key) ? 1 : -1;
  return 0;
}

void civ_draw_list_flush(civ_draw_list_t *dl, SDL_Renderer *r) {
  if (!dl || !dl->cmds || dl->count == 0) return;

  /* Sort by layer + y */
  qsort(dl->cmds, (size_t)dl->count, sizeof(civ_draw_cmd_t), cmd_compare);

  for (int i = 0; i < dl->count; i++) {
    flush_one(&dl->cmds[i], r);
  }

  dl->draw_calls_this_frame = dl->count;
}
