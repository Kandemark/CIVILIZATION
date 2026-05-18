/**
 * @file animation.h
 * @brief Tween-based animation system for UI transitions
 */
#ifndef CIV_DISPLAY_ANIMATION_H
#define CIV_DISPLAY_ANIMATION_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  CIV_TWEEN_NONE,
  CIV_TWEEN_FADE_IN,
  CIV_TWEEN_FADE_OUT,
  CIV_TWEEN_SLIDE_UP,
  CIV_TWEEN_SLIDE_DOWN,
  CIV_TWEEN_SLIDE_LEFT,
  CIV_TWEEN_SLIDE_RIGHT,
  CIV_TWEEN_SCALE_IN,
  CIV_TWEEN_SCALE_OUT,
} civ_tween_type_t;

typedef struct {
  civ_tween_type_t type;
  float duration;
  float elapsed;
  float from;
  float to;
  bool active;
  bool done;
} civ_tween_t;

void civ_tween_init(civ_tween_t *anim);
void civ_tween_start(civ_tween_t *anim, civ_tween_type_t type, float duration,
                    float from, float to);
void civ_tween_update(civ_tween_t *anim, float dt);
float civ_tween_value(const civ_tween_t *anim);
bool civ_tween_is_active(const civ_tween_t *anim);

/* Pre-built convenience: fade + slide combo for panels */
typedef struct {
  civ_tween_t fade;
  civ_tween_t slide_x;
  civ_tween_t slide_y;
  civ_tween_t scale;
  bool visible;
} civ_panel_anim_t;

void civ_panel_anim_init(civ_panel_anim_t *pa);
void civ_panel_anim_show(civ_panel_anim_t *pa, float duration);
void civ_panel_anim_hide(civ_panel_anim_t *pa, float duration);
void civ_panel_anim_update(civ_panel_anim_t *pa, float dt);
float civ_panel_anim_alpha(const civ_panel_anim_t *pa);
float civ_panel_anim_offset_x(const civ_panel_anim_t *pa);
float civ_panel_anim_offset_y(const civ_panel_anim_t *pa);
float civ_panel_anim_scale(const civ_panel_anim_t *pa);
bool civ_panel_anim_is_visible(const civ_panel_anim_t *pa);

#ifdef __cplusplus
}
#endif
#endif
