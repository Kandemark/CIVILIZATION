/**
 * @file theme.h
 * @brief Visual design tokens — colors, spacing, font sizing
 */
#ifndef CIV_DISPLAY_THEME_H
#define CIV_DISPLAY_THEME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  /* Backgrounds */
  uint32_t bg_deep;    /* Deep space / void */
  uint32_t bg_dark;    /* Panel backgrounds */
  uint32_t bg_medium;  /* Cards, raised surfaces */
  uint32_t bg_light;   /* Elevated / hover state */

  /* Brand */
  uint32_t primary;       /* Cyan blue — interactive elements */
  uint32_t primary_dark;  /* Darker variant for outlines */
  uint32_t accent;        /* Teal/green — success, active */
  uint32_t accent_warm;   /* Gold/amber — warnings, gov */
  uint32_t accent_warn;   /* Red — danger, war */

  /* Text */
  uint32_t text_primary;   /* Main body text */
  uint32_t text_secondary; /* Dimmed / less important */
  uint32_t text_dim;       /* Disabled / hint text */

  /* HUD */
  uint32_t hud_bg;         /* Top bar / sidebar background */
  uint32_t hud_border;     /* HUD divider lines */
  uint32_t hud_text;       /* HUD text default */
  uint32_t hud_accent;     /* Highlight text in HUD */

  /* Panels */
  uint32_t panel_bg;       /* Floating panel background */
  uint32_t panel_header;   /* Panel title bar background */

  /* Status */
  uint32_t success;        /* Green — positive indicators */
  uint32_t danger;         /* Red — negative / war */
  uint32_t warning;        /* Amber — caution */
  uint32_t info;           /* Blue — informational */

  /* Map */
  uint32_t map_water;      /* Ocean water */
  uint32_t map_water_dim;  /* Unexplored water */
  uint32_t map_land;       /* Unclaimed land */
  uint32_t map_land_dim;   /* Unexplored land */
  uint32_t map_border;     /* Border lines */

  /* Spacing (UI grid units, px) */
  int space_xs;   /* 4 */
  int space_sm;   /* 8 */
  int space_md;   /* 16 */
  int space_lg;   /* 24 */
  int space_xl;   /* 32 */

  /* Font sizes */
  int font_xs;    /* 10 */
  int font_sm;    /* 12 */
  int font_md;    /* 16 */
  int font_lg;    /* 20 */
  int font_xl;    /* 28 */
  int font_title; /* 36 */

  /* Panel defaults */
  int panel_min_w;
  int panel_min_h;
  int panel_border_radius;
  int panel_border_width;

  /* Animation */
  float anim_fade_ms;  /* default fade duration in seconds */
  float anim_slide_ms; /* default slide duration in seconds */
} civ_theme_t;

extern civ_theme_t g_theme; /* Global theme singleton */

void civ_theme_init_default(void);
uint32_t civ_theme_mix(uint32_t a, uint32_t b, float t);

#ifdef __cplusplus
}
#endif
#endif
