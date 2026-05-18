/**
 * @file draw_list.h
 * @brief Deferred draw list — collect, sort by layer, flush
 */
#ifndef CIV_DISPLAY_DRAW_LIST_H
#define CIV_DISPLAY_DRAW_LIST_H

#include <SDL3/SDL.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  CIV_DRAW_RECT_FILLED,
  CIV_DRAW_RECT_OUTLINE,
  CIV_DRAW_LINE,
  CIV_DRAW_TEXT,
  CIV_DRAW_CIRCLE,
  CIV_DRAW_TEXTURE,
  CIV_DRAW_COUNT
} civ_draw_type_t;

typedef struct {
  civ_draw_type_t type;
  float x, y, w, h;
  uint32_t color;
  uint8_t alpha;
  int thickness;
  SDL_Texture *texture;
  const char *text;
  int layer;
  uint32_t sort_key;
} civ_draw_cmd_t;

typedef struct {
  civ_draw_cmd_t *cmds;
  int count;
  int capacity;
  int draw_calls_this_frame;
  double last_frame_ms;
} civ_draw_list_t;

void civ_draw_list_init(civ_draw_list_t *dl, int initial_capacity);
void civ_draw_list_clear(civ_draw_list_t *dl);
void civ_draw_list_destroy(civ_draw_list_t *dl);

/* Immediate-mode API: append commands */
void civ_draw_rect_filled(civ_draw_list_t *dl, float x, float y, float w, float h,
                          uint32_t color, uint8_t alpha, int layer);
void civ_draw_rect_outline(civ_draw_list_t *dl, float x, float y, float w, float h,
                           uint32_t color, int thickness, int layer);
void civ_draw_line(civ_draw_list_t *dl, float x1, float y1, float x2, float y2,
                   uint32_t color, int layer);
void civ_draw_text(civ_draw_list_t *dl, const char *text, float x, float y,
                   uint32_t color, int layer);

/* Flush all commands to the SDL renderer */
void civ_draw_list_flush(civ_draw_list_t *dl, SDL_Renderer *r);

#ifdef __cplusplus
}
#endif
#endif
