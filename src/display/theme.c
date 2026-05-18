#include "../../include/display/theme.h"

civ_theme_t g_theme;

void civ_theme_init_default(void) {
  g_theme = (civ_theme_t){
      .bg_deep = 0x03050C,
      .bg_dark = 0x070B1A,
      .bg_medium = 0x0D1525,
      .bg_light = 0x162033,
      .primary = 0x00D4FF,
      .primary_dark = 0x004A7A,
      .accent = 0x00FFD2,
      .accent_warm = 0xFFD400,
      .accent_warn = 0xFF4A6A,
      .text_primary = 0xF0F8FF,
      .text_secondary = 0xA0B4C8,
      .text_dim = 0x4A5A6A,
      .space_xs = 4,
      .space_sm = 8,
      .space_md = 16,
      .space_lg = 24,
      .space_xl = 32,
      .font_xs = 10,
      .font_sm = 12,
      .font_md = 16,
      .font_lg = 20,
      .font_xl = 28,
      .font_title = 36,
      .panel_min_w = 280,
      .panel_min_h = 200,
      .panel_border_radius = 6,
      .panel_border_width = 1,
      .anim_fade_ms = 0.2f,
      .anim_slide_ms = 0.25f,
  };
}

uint32_t civ_theme_mix(uint32_t a, uint32_t b, float t) {
  uint8_t ar = (a >> 16) & 0xFF, ag = (a >> 8) & 0xFF, ab = a & 0xFF;
  uint8_t br = (b >> 16) & 0xFF, bg = (b >> 8) & 0xFF, bb = b & 0xFF;
  uint8_t mr = (uint8_t)(ar + (br - ar) * t);
  uint8_t mg = (uint8_t)(ag + (bg - ag) * t);
  uint8_t mb = (uint8_t)(ab + (bb - ab) * t);
  return 0xFF000000 | ((uint32_t)mr << 16) | ((uint32_t)mg << 8) | mb;
}
