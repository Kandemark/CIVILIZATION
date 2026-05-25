#include "core/character.h"
#include "core/profile.h"
#include "core/military/combat.h"
#include "ui/nuklear_ui.h"
#include "core/time_engine.h"
#include "core/world/nation.h"
#include "core/world/political_borders.h"
#include "core/world/map_view.h"
#include "core/world/wonders.h"
#include "display/camera.h"
#include "display/debug_overlay.h"
#include "display/layer.h"
#include "display/theme.h"
#include "ui/graph/graph.h"
#include "engine/font.h"
#include "engine/renderer.h"
#include "ui/panel/diplomacy_panel.h"
#include "ui/panel/governance_panel.h"
#include "ui/panel/research_panel.h"
#include "ui/panel/rulebook_panel.h"
#include "ui/panel/settlement_sidebar.h"
#include "ui/panel/unit_sidebar.h"
#include "ui/panel/wonders_panel.h"
#include "ui/scene.h"
#include "ui/screens/screens.h"
#include "ui/ui_common.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

/* ── Local state ──────────────────────────────────────────────────── */
static civ_camera_t               cam;
static civ_debug_overlay_t        debug;
static civ_render_map_context_t  *map_ctx = NULL;
static civ_font_t                *font_hud = NULL;
static int                        last_win_w, last_win_h;
static civ_unit_t             *selected_unit = NULL;
static civ_settlement_t       *selected_settlement = NULL;
static bool                    show_diplomacy, show_research;
static bool                    show_government, show_wonders, show_rulebook;
static bool                    has_contacted_rival;
static float                    time_accumulator = 0.0f;
static int                      time_speed = 1;
static float                    days_per_second = 1.0f;

/* Screen navigation */
typedef enum { SCR_MAP, SCR_DIPLOMACY, SCR_ECONOMY, SCR_MILITARY,
               SCR_TECHNOLOGY, SCR_GOVERNANCE, SCR_CULTURE,
               SCR_NEWS, SCR_DASHBOARD, SCR_WORK, SCR_FINANCE,
               SCR_HOUSING, SCR_EDUCATION, SCR_NETWORK, SCR_POLITICS,
               SCR_HEALTH, SCR_CONSTITUTION } civ_screen_t;
#include "core/constitution.h"
#include "core/npc_engine.h"
static civ_screen_t             current_screen = SCR_MAP;
static civ_map_view_type_t      current_map_view = CIV_MAP_VIEW_POLITICAL;
static bool                     flags_loaded = false;
static civ_layer_stack_t        layer_stack;
static bool                     layers_wired = false;

/* ── Toast notification system ────────────────────────────────── */
#define CIV_TOAST_MAX   8
#define CIV_TOAST_LEN   128
static struct { char msg[CIV_TOAST_LEN]; uint32_t color; float ttl; } toasts[CIV_TOAST_MAX];
static int toast_count = 0;
static void toast_push(const char *msg, uint32_t color) {
  if (toast_count >= CIV_TOAST_MAX) {
    memmove(toasts, toasts + 1, (CIV_TOAST_MAX - 1) * sizeof(toasts[0]));
    toast_count--;
  }
  strncpy(toasts[toast_count].msg, msg, CIV_TOAST_LEN - 1);
  toasts[toast_count].color = color;
  toasts[toast_count].ttl = 4.0f;
  toast_count++;
}
static void toast_update(float dt) {
  for (int i = 0; i < toast_count; i++) {
    toasts[i].ttl -= dt;
    if (toasts[i].ttl < 0) { memmove(toasts + i, toasts + i + 1, (toast_count - i - 1) * sizeof(toasts[0])); toast_count--; i--; }
  }
}
static void toast_render(SDL_Renderer *r) {
  int tx = last_win_w / 2, ty = last_win_h - 80;
  for (int i = toast_count - 1; i >= 0; i--) {
    uint8_t alpha = (uint8_t)(toasts[i].ttl > 1.0f ? 230 : toasts[i].ttl * 230);
    int tw = 400, th = 28;
    civ_render_rect_filled_alpha(r, tx - tw/2, ty, tw, th, g_theme.panel_bg, alpha);
    civ_render_rect_outline(r, tx - tw/2, ty, tw, th, toasts[i].color, 1);
    civ_font_render_aligned(r, font_hud, toasts[i].msg, tx - tw/2 + 8, ty, tw - 16, th, toasts[i].color, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
    ty -= 34;
  }
}
static char                     selected_nation_id[64] = "";
static int16_t                  selected_nation_cid = -1;
static bool                     show_nation_detail = false;
static char                     hovered_country[128] = "";
static float                    hover_country_x, hover_country_y;

/* ── Visibility helper ────────────────────────────────────────────── */
static void update_visibility(civ_game_t *game) {
  if (!game || !game->world_map || !game->unit_manager) return;
  for (int32_t i = 0; i < game->world_map->width * game->world_map->height; i++)
    game->world_map->tiles[i].is_visible = false;
  for (size_t i = 0; i < game->unit_manager->unit_count; i++) {
    civ_unit_t *u = &game->unit_manager->units[i];
    int32_t r = u->visibility_range;
    for (int32_t dy = -r; dy <= r; dy++) {
      for (int32_t dx = -r; dx <= r; dx++) {
        if (dx * dx + dy * dy > r * r) continue;
        int32_t tx = (u->x + dx + game->world_map->width) % game->world_map->width;
        int32_t ty = u->y + dy;
        if (ty >= 0 && ty < game->world_map->height) {
          civ_map_tile_t *t = civ_map_get_tile(game->world_map, tx, ty);
          if (t) t->is_visible = true, t->is_explored = true;
        }
      }
    }
  }
}

/* ── Rendering helpers ─────────────────────────────────────────────── */
static void render_map_layer(SDL_Renderer *r, civ_game_t *game) {
  if (!game->world_map) return;
  civ_render_map(r, map_ctx, game->world_map, last_win_w, last_win_h,
                 current_map_view, game->resource_map);
}


static void render_units_layer(SDL_Renderer *r, civ_game_t *game) {
  if (!game->unit_manager || !font_hud) return;
  for (size_t i = 0; i < game->unit_manager->unit_count; i++) {
    civ_unit_t *u = &game->unit_manager->units[i];
    if (u->current_strength <= 0) continue;

    float sx, sy;
    civ_camera_world_to_screen(&cam, last_win_w, last_win_h, (float)u->x,
                               (float)u->y, &sx, &sy);
    float size = 48.0f * cam.zoom;
    if (!civ_camera_is_visible(&cam, last_win_w, last_win_h, (float)u->x,
                               (float)u->y, 24.0f))
      continue;

    if (selected_unit == u)
      civ_render_rect_filled_alpha(r, (int)(sx - size / 2 - 4),
                                   (int)(sy - size / 2 - 4), (int)size + 8,
                                   (int)size + 8, g_theme.info, 180);

    uint32_t color = u->has_moved ? 0x555555 : 0xFF2200;
    if (u->unit_type == CIV_UNIT_TYPE_SETTLER) color = 0x00FFCC;
    civ_render_rect_filled(r, (int)(sx - size / 2), (int)(sy - size / 2),
                           (int)size, (int)size, color);
    civ_render_rect_outline(r, (int)(sx - size / 2), (int)(sy - size / 2),
                            (int)size, (int)size, 0xFFFFFF, 1);
  }
}

static void render_settlements_layer(SDL_Renderer *r, civ_game_t *game) {
  if (!game->settlement_manager || !font_hud) return;
  for (size_t i = 0; i < game->settlement_manager->settlement_count; i++) {
    civ_settlement_t *s = &game->settlement_manager->settlements[i];
    float sx, sy;
    civ_camera_world_to_screen(&cam, last_win_w, last_win_h, s->x, s->y, &sx,
                               &sy);
    if (!civ_camera_is_visible(&cam, last_win_w, last_win_h, s->x, s->y, 26.0f))
      continue;

    float size = 52.0f * cam.zoom;
    if (selected_settlement == s)
      civ_render_rect_outline(r, (int)(sx - size / 2), (int)(sy - size / 2),
                              (int)size, (int)size, 0xFFFF00, 2);

    char label[64];
    sprintf(label, "%s (Pop: %lld)", s->name, s->population);
    civ_font_render_aligned(r, font_hud, label, (int)sx - 100, (int)sy + 30,
                            200, 20, 0xFFFFFF, CIV_ALIGN_CENTER,
                            CIV_VALIGN_TOP);
  }
}

static void render_hud_top(SDL_Renderer *r, civ_game_t *game) {
  /* Gradient top bar background */
  civ_render_gradient_vertical(r, 0, 0, last_win_w, 34,
      g_theme.hud_bg, g_theme.bg_deep);
  civ_render_line(r, 0, 34, last_win_w, 34, g_theme.hud_border);

  float lat = 90.0f - (cam.y / (float)cam.map_height) * 180.0f;
  float lon = (cam.x / (float)cam.map_width) * 360.0f - 180.0f;
  while (lon < -180.0f) lon += 360.0f;
  while (lon > 180.0f) lon -= 360.0f;

  char buf[128];
  /* Time — centered */
  char time_buf[64];
  if (game->time_engine) {
    civ_time_engine_format_hud((civ_time_engine_t *)game->time_engine,
                               time_buf, sizeof(time_buf));
  } else {
    snprintf(time_buf, sizeof(time_buf), "Turn %d", game->current_turn);
  }
  civ_font_render_aligned(r, font_hud, time_buf,
      last_win_w/2 - 130, 0, 260, 34,
      CIV_COLOR_PRIMARY, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

  /* Left: nation info if selected */
  if (selected_nation_id[0] && game->nation_manager) {
    civ_nation_t *nat = civ_nation_get_by_id(
        (civ_nation_manager_t *)game->nation_manager, selected_nation_id);
    if (nat && nat->iso_a2[0] && game->flag_system) {
      const civ_flag_entry_t *fl = civ_flag_system_get_by_iso(
          game->flag_system, nat->iso_a2);
      if (fl) civ_flag_render(r, fl, 6, 4, 32, 20);
    }
  }

  /* Right: wallet + coords */
  float bal = 0;
  if (game->wallet.count > 0 && game->market)
    bal = civ_wallet_total(&game->wallet, game->market);
  sprintf(buf, "$%.0f  |  %.0f%c %.0f%c  z%.1f",
          bal, fabsf(lat), lat >= 0 ? 'N' : 'S',
          fabsf(lon), lon >= 0 ? 'E' : 'W', cam.zoom * 10.0f);
  civ_font_render_aligned(r, font_hud, buf,
      last_win_w - 250, 0, 240, 34,
      CIV_COLOR_TEXT_DIM, CIV_ALIGN_RIGHT, CIV_VALIGN_MIDDLE);

  /* Hovered country — info card near cursor */
  if (hovered_country[0]) {
    int cx = (int)hover_country_x + 18, cy = (int)hover_country_y + 18;
    int cw = 210, ch = 72;
    if (cx + cw > last_win_w) cx = (int)hover_country_x - cw - 10;
    if (cy + ch > last_win_h) cy = (int)hover_country_y - ch - 10;

    civ_render_rect_filled_alpha(r, cx, cy, cw, ch, g_theme.panel_bg, 240);
    civ_render_rect_outline(r, cx, cy, cw, ch, g_theme.warning, 1);

    /* Flag icon */
    civ_nation_t *nat = NULL;
    if (game->nation_manager)
      nat = civ_nation_get_by_id((civ_nation_manager_t *)game->nation_manager,
                                  hovered_country);
    if (nat && nat->iso_a3[0] && game->flag_system) {
      const civ_flag_entry_t *flag = civ_flag_system_get_by_iso(
          game->flag_system, nat->iso_a2);
      if (flag) {
        civ_flag_render(r, flag, cx + 6, cy + 4, 32, 20);
      }
    }

    int text_x = cx + 44;
    civ_font_render_aligned(r, font_hud, hovered_country, text_x, cy + 4,
                            cw - 52, 20, g_theme.warning, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    char info[64] = "Independent State";
    if (nat) {
      snprintf(info, sizeof(info), "Pop: %lldM | Gov: %s",
               nat->population / 1000000,
               civ_government_proximity_label(nat->government));
    }
    civ_font_render_aligned(r, font_hud, info, text_x, cy + 26,
                            cw - 52, 18, g_theme.hud_text, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    civ_font_render_aligned(r, font_hud, "Click for details", text_x, cy + 48,
                            cw - 52, 16, g_theme.text_dim, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  }

}

static void render_city_labels(SDL_Renderer *r, civ_game_t *game) {
  /* Only show cities when zoomed in enough */
  if (cam.zoom < 2.0f || !game->cities_data || !font_hud) return;

  const float U = 4.0f;
  float inv_scale = 1.0f / (cam.zoom * U);
  float half_w = (last_win_w * 0.5f) * inv_scale;
  float half_h = (last_win_h * 0.5f) * inv_scale;

  int tx = (int)(cam.x - half_w);
  int ty = (int)(cam.y - half_h);
  int tw = (int)(half_w * 2.0f) + 1;
  int th = (int)(half_h * 2.0f) + 1;

  /* Query cities visible in viewport */
  uint32_t count = 0;
  const civ_city_data_t **cities = civ_cities_query_tiles(
      game->cities_data, tx, ty, tw, th, CIV_CITY_TIER_LARGE, &count);
  if (!cities) return;

  /* Show at most ~50 labels to avoid clutter */
  int shown = 0;
  for (uint32_t i = 0; i < count && shown < 50; i++) {
    const civ_city_data_t *city = cities[i];
    float sx, sy;
    civ_camera_world_to_screen(&cam, last_win_w, last_win_h,
                               (float)city->tile_x, (float)city->tile_y,
                               &sx, &sy);
    if (sx < -20 || sx > last_win_w + 20 || sy < -20 || sy > last_win_h + 20)
      continue;

    uint32_t label_color = city->capital_flag ? g_theme.warning : g_theme.text_secondary;
    int font_sz = city->tier <= CIV_CITY_TIER_MEGA ? 16 :
                  city->tier <= CIV_CITY_TIER_LARGE ? 13 : 11;

    /* Simple text render: just the city name */
    civ_font_render_aligned(r, font_hud, city->name,
                            (int)sx - 40, (int)sy + 6, 80, font_sz + 4,
                            label_color, CIV_ALIGN_CENTER, CIV_VALIGN_TOP);
    civ_render_rect_filled(r, (int)sx - 1, (int)sy - 1, 3, 3,
                           city->capital_flag ? g_theme.warning : 0x6688AA);
    shown++;
  }
  civ_cities_free_result(cities);
}

static void render_nation_detail_panel(SDL_Renderer *r, civ_game_t *game,
                                        civ_input_state_t *input) {
  if (!show_nation_detail || !game->nation_manager) return;

  civ_nation_t *nat = civ_nation_get_by_id(
      (civ_nation_manager_t *)game->nation_manager, selected_nation_id);

  /* Panel positioning: center-left */
  int px = 240, py = 50, pw = 360, ph = 340;

  /* Close button top-right */
  int close_x = px + pw - 26, close_y = py + 4;
  bool close_hov = civ_input_is_mouse_over(input, close_x, close_y, 20, 20);
  if (close_hov && input->mouse_left_pressed) {
    show_nation_detail = false;
    return;
  }

  /* Background */
  civ_render_rect_filled_alpha(r, px, py, pw, ph, g_theme.panel_bg, 245);
  civ_render_rect_outline(r, px, py, pw, ph, 0x1A2A4A, 1);
  /* Title bar */
  civ_render_rect_filled_alpha(r, px, py, pw, 30, g_theme.hud_border, 230);

  /* Close X */
  civ_font_render_aligned(r, font_hud, "X", close_x, close_y, 20, 20,
                          close_hov ? g_theme.danger : g_theme.text_dim,
                          CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

  if (!nat) {
    civ_font_render_aligned(r, font_hud, "Nation not found",
                            px + 12, py + 40, pw - 24, 20,
                            g_theme.danger, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    return;
  }

  /* Flag */
  int flag_x = px + 12, flag_y = py + 4;
  if (nat->iso_a2[0] && game->flag_system) {
    const civ_flag_entry_t *fl = civ_flag_system_get_by_iso(
        game->flag_system, nat->iso_a2);
    if (fl) civ_flag_render(r, fl, flag_x, flag_y, 48, 30);
  }

  /* Nation name */
  civ_font_render_aligned(r, font_hud, nat->name,
                          px + 68, py + 2, pw - 80, 24,
                          g_theme.warning, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

  /* Divider */
  civ_render_line(r, px + 10, py + 34, px + pw - 10, py + 34, g_theme.hud_border);

  /* Stats */
  char buf[256];
  int dy = py + 42;

  snprintf(buf, sizeof(buf), "Capital: %.1f%c %.1f%c  |  ISO: %s",
           fabsf(nat->capital_lat), nat->capital_lat >= 0 ? 'N' : 'S',
           fabsf(nat->capital_lon), nat->capital_lon >= 0 ? 'E' : 'W',
           nat->iso_a3[0] ? nat->iso_a3 : "—");
  civ_font_render_aligned(r, font_hud, buf, px + 12, dy, pw - 24, 15,
                          g_theme.hud_text, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  dy += 22;

  snprintf(buf, sizeof(buf), "Population: %lldM  |  GDP: $%.0fM  |  Growth: %+.1f%%",
           nat->population / 1000000, nat->economy.gdp,
           nat->economy.gdp_growth * 100.0f);
  civ_font_render_aligned(r, font_hud, buf, px + 12, dy, pw - 24, 15,
                          g_theme.text_secondary, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  dy += 20;

  snprintf(buf, sizeof(buf), "Unemp: %.1f%%  |  Infl: %.1f%%  |  Food: %.0fM",
           nat->economy.unemployment * 100.0f,
           nat->economy.inflation * 100.0f,
           nat->economy.food_production / 1000000.0f);
  civ_font_render_aligned(r, font_hud, buf, px + 12, dy, pw - 24, 14,
                          g_theme.hud_text, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  dy += 18;

  snprintf(buf, sizeof(buf), "Gov: %s  |  ISO: %s",
           civ_government_proximity_label(nat->government),
           nat->iso_a3[0] ? nat->iso_a3 : "—");
  civ_font_render_aligned(r, font_hud, buf, px + 12, dy, pw - 24, 15,
                          g_theme.text_secondary, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  dy += 20;

  /* Indices */
  civ_render_line(r, px + 10, dy, px + pw - 10, dy, g_theme.hud_border);
  dy += 6;
  civ_font_render_aligned(r, font_hud, "INDICES", px + 12, dy, 100, 14,
                          g_theme.warning, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  dy += 18;
  snprintf(buf, sizeof(buf), "Tech %+d  Econ %+d  Mil %+d  Cult %+d",
           nat->tech_index, nat->economic_index,
           nat->military_index, nat->cultural_index);
  civ_font_render_aligned(r, font_hud, buf, px + 12, dy, pw - 24, 14,
                          0x8899CC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  dy += 22;

  /* Resource summary */
  if (game->resource_map && game->world_map) {
    civ_nation_resource_profile_t rp;
    civ_nation_calculate_resources(nat, game->world_map,
                                    game->resource_map, &rp);
    civ_render_line(r, px + 10, dy, px + pw - 10, dy, g_theme.hud_border);
    dy += 6;
    snprintf(buf, sizeof(buf), "RESOURCES: %d types, %d total",
             rp.distinct_types, rp.total_resources);
    civ_font_render_aligned(r, font_hud, buf, px + 12, dy, pw - 24, 14,
                            g_theme.warning, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    dy += 16;
    /* Top 5 resources */
    int shown = 0;
    for (int t = 0; t < 20 && shown < 5; t++) {
      if (rp.quantities[t] > 0) {
        snprintf(buf, sizeof(buf), "  %-14s Qty:%5u Qual:%d",
                 civ_resource_type_name((civ_resource_type_t)t),
                 rp.quantities[t], rp.best_quality[t]);
        civ_font_render_aligned(r, font_hud, buf, px + 12, dy, pw - 24, 13,
                                0x778899, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
        dy += 14;
        shown++;
      }
    }
    if (rp.distinct_types == 0) {
      civ_font_render_aligned(r, font_hud, "  No resources in territory",
                              px + 12, dy, pw - 24, 13,
                              g_theme.text_dim, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
      dy += 14;
    }
  }

  /* Action buttons */
  dy += 6;
  civ_render_line(r, px + 10, dy, px + pw - 10, dy, g_theme.hud_border);
  dy += 8;

  const char *btn_labels[] = {"Governance", "Diplomacy", "Economy"};
  for (int b = 0; b < 3; b++) {
    int bx = px + 12 + b * 100;
    bool hov = civ_input_is_mouse_over(input, bx, dy, 90, 26);
    uint32_t bg = hov ? 0x1A3A5A : 0x0A1A2A;
    civ_render_rect_filled_alpha(r, bx, dy, 90, 26, bg, 220);
    civ_render_rect_outline(r, bx, dy, 90, 26, hov ? g_theme.info : g_theme.hud_border, 1);
    civ_font_render_aligned(r, font_hud, btn_labels[b],
                            bx, dy, 90, 26,
                            hov ? 0xFFFFFF : g_theme.hud_text,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
    if (hov && input->mouse_left_pressed) {
      if (b == 0) current_screen = SCR_GOVERNANCE;
      if (b == 1) current_screen = SCR_DIPLOMACY;
      if (b == 2) current_screen = SCR_ECONOMY;
      show_nation_detail = false;
    }
  }
}

static void render_view_selector(SDL_Renderer *r, civ_input_state_t *input) {
  /* View selector: small labeled buttons below the top bar on the right */
  const char *labels[] = {"P", "G", "E", "D", "C"};
  const char *names[]  = {"Political", "Geographical", "Economic",
                           "Demographical", "Cultural"};
  uint32_t colors[] = {0xCC4444, 0x44AA44, 0xCCAA00, 0xCC6600, 0x8844CC};
  int n = 5, btn_w = 26, btn_h = 20, gap = 3;
  int start_x = last_win_w - (n * btn_w + (n - 1) * gap) - 155;
  int start_y = 42;

  for (int i = 0; i < n; i++) {
    int bx = start_x + i * (btn_w + gap);
    bool active = ((int)current_map_view == i); /* POLITICAL=0, GEOGRAPHICAL=1, etc */
    bool hover = civ_input_is_mouse_over(input, bx, start_y, btn_w, btn_h);

    uint32_t bg = active ? colors[i] : (hover ? g_theme.hud_border : g_theme.panel_bg);
    uint8_t alpha = active ? 220 : (hover ? 180 : 140);
    civ_render_rect_filled_alpha(r, bx, start_y, btn_w, btn_h, bg, alpha);
    if (active) civ_render_rect_outline(r, bx, start_y, btn_w, btn_h, 0xFFFFFF, 1);

    civ_font_render_aligned(r, font_hud, labels[i], bx, start_y + 1,
                            btn_w, btn_h, active ? 0xFFFFFF : g_theme.hud_text,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

    /* Click to switch view */
    if (hover && input->mouse_left_pressed) {
      current_map_view = (civ_map_view_type_t)i;
    }
  }

  /* View label */
  civ_font_render_aligned(r, font_hud, names[(int)current_map_view],
                          start_x - 10, start_y - 1, 130, btn_h,
                          g_theme.text_secondary, CIV_ALIGN_RIGHT, CIV_VALIGN_MIDDLE);
}

static void render_hud_buttons(SDL_Renderer *r, civ_input_state_t *input) {
  /* Compact icon-style buttons in top bar, with dividers */
  const char *labels[] = {"R", "T", "G", "W", "D"};
  uint32_t colors[] = {g_theme.info, g_theme.info, g_theme.warning, 0xFFFF00, 0xFFFF00};
  bool *states[] = {&show_rulebook, &show_research, &show_government,
                    &show_wonders, &show_diplomacy};
  int n = 5, btn_s = 34, gap = 2;
  int total_w = n * btn_s + (n-1) * gap;
  int start_x = last_win_w - total_w - 14;

  for (int i = 0; i < n; i++) {
    int bx = start_x + i * (btn_s + gap);
    bool active = *states[i];
    bool hov = civ_input_is_mouse_over(input, bx, 3, btn_s, btn_s);
    uint32_t bg = active ? colors[i] : (hov ? 0x2A3A4A : 0x152030);
    civ_render_rect_filled_alpha(r, bx, 3, btn_s, btn_s, bg, active ? 200 : 160);
    civ_render_rect_outline(r, bx, 3, btn_s, btn_s,
                            active ? 0xFFFFFF : g_theme.hud_border, 1);
    civ_font_render_aligned(r, font_hud, labels[i], bx, 3, btn_s, btn_s,
                            active ? 0xFFFFFF : g_theme.hud_text,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  }
}

static void render_time_controls(SDL_Renderer *r, civ_game_t *game,
                                  civ_input_state_t *input) {
  /* Speed control buttons: PAUSE | 1x | 2x | 5x */
  const char *labels[] = {"| |",  "1x",  "2x",  "5x"};
  int speeds[] =         {0,      1,     2,     3};
  int n_btns = 4, btn_w = 44, btn_h = 34, gap = 4;
  int total_w = n_btns * btn_w + (n_btns - 1) * gap;
  int start_x = last_win_w - total_w - 20, btn_y = last_win_h - btn_h - 16;

  for (int i = 0; i < n_btns; i++) {
    int bx = start_x + i * (btn_w + gap);
    bool active = (time_speed == speeds[i]);
    bool hov = civ_input_is_mouse_over(input, bx, btn_y, btn_w, btn_h);
    uint32_t bg = active ? 0x004A7A : (hov ? 0x2A3A4A : g_theme.hud_border);
    civ_render_rect_filled_alpha(r, bx, btn_y, btn_w, btn_h, bg, 220);
    civ_render_rect_outline(r, bx, btn_y, btn_w, btn_h,
                            active ? g_theme.info : g_theme.hud_border, 1);
    civ_font_render_aligned(r, font_hud, labels[i], bx, btn_y, btn_w, btn_h,
                            active ? 0xFFFFFF : g_theme.hud_text,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
    /* Click to set speed */
    if (input->mouse_left_pressed && hov) time_speed = speeds[i];
  }

  /* Time display */
  char buf[64];
  if (game->time_engine) {
    civ_time_engine_format_hud((civ_time_engine_t *)game->time_engine,
                               buf, sizeof(buf));
  } else {
    snprintf(buf, sizeof(buf), "Turn %d", game->current_turn);
  }
  civ_font_render_aligned(r, font_hud, buf, start_x - 12, btn_y, start_x - 20,
                          btn_h, g_theme.text_secondary, CIV_ALIGN_RIGHT, CIV_VALIGN_MIDDLE);
  (void)game;
}

static void render_minimap_layer(SDL_Renderer *r, civ_game_t *game) {
  int mm_w = 200, mm_h = 140, mm_x = last_win_w - mm_w - 16, mm_y = last_win_h - mm_h - 16;
  /* Rounded background */
  civ_render_rounded_rect(r, mm_x - 2, mm_y - 2, mm_w + 4, mm_h + 4,
                          g_theme.panel_border_radius, g_theme.panel_bg);
  civ_render_rect_outline(r, mm_x - 2, mm_y - 2, mm_w + 4, mm_h + 4,
                          g_theme.hud_border, 1);
  civ_render_minimap(r, mm_x, mm_y, mm_w, mm_h, game->world_map, map_ctx);
  /* View rect on minimap */
  float view_w = last_win_w / (cam.zoom * 4.0f);
  float view_h = last_win_h / (cam.zoom * 4.0f);
  float rx = (cam.x / (float)game->world_map->width) * (float)mm_w + (float)mm_x -
             view_w / (float)game->world_map->width * (float)mm_w / 2.0f;
  float ry = (cam.y / (float)game->world_map->height) * (float)mm_h + (float)mm_y -
             view_h / (float)game->world_map->height * (float)mm_h / 2.0f;
  civ_render_rect_outline(r, (int)rx, (int)ry,
                          (int)(view_w / game->world_map->width * mm_w),
                          (int)(view_h / game->world_map->height * mm_h),
                          g_theme.map_border, 1);
}

/* ── Scene lifecycle ─────────────────────────────────────────────── */
static void init(void) {
  printf("[SCENE_GAME] Initializing...\n");
  civ_theme_init_default();
  font_hud = civ_font_load_system("Inter", 12);
  if (!font_hud) font_hud = civ_font_load_system("Segoe UI", 12);
  civ_debug_overlay_init(&debug);
  selected_unit = NULL;
  selected_settlement = NULL;
  map_ctx = NULL;

  /* Initialize the layer stack — all layers enabled by default */
  civ_layer_stack_init(&layer_stack);
  layers_wired = true;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  if (!game || !input) return;

  /* Camera — only on map screen */
  if (current_screen == SCR_MAP) {
    if (input->mouse_right_down)
      civ_camera_pan(&cam, -(float)input->delta_x, -(float)input->delta_y);
    if (fabsf(input->scroll_delta) > 0.1f) {
      float factor = input->scroll_delta > 0 ? 1.15f : 1.0f / 1.15f;
      civ_camera_zoom(&cam, factor,
                      cam.x + (input->mouse_x - last_win_w / 2.0f) /
                                  (cam.zoom * 4.0f),
                      cam.y + (input->mouse_y - last_win_h / 2.0f) /
                                  (cam.zoom * 4.0f));
    }
  }

  /* Initialize camera on first frame */
  if (cam.map_width == 0 && game->world_map) {
    civ_camera_init(&cam, game->world_map->width, game->world_map->height);
  }
  civ_camera_update(&cam, 1.0f / 60.0f);

  /* Auto-spawn initial units */
  if (game->unit_manager && game->unit_manager->unit_count == 0) {
    civ_unit_manager_spawn_unit(game->unit_manager, CIV_UNIT_TYPE_SETTLER,
                                "Settlers", 100, game->world_map->width / 2,
                                game->world_map->height / 2);
    civ_unit_manager_spawn_unit(game->unit_manager, CIV_UNIT_TYPE_INFANTRY,
                                "Barbarians", 80, game->world_map->width / 2 + 1,
                                game->world_map->height / 2);
    update_visibility(game);
  }

  /* ── Organic time flow ─────────────────────────────── */
  /* Space toggles pause, 1/2/3 keys set speed */
  if (input->esc_pressed && time_speed == 0) time_speed = 1;
  else if (input->esc_pressed) time_speed = 0;

  float speeds[] = {0.0f, 1.0f, 2.0f, 5.0f};
  float game_days = input->global_dt > 0.0f
      ? input->global_dt * days_per_second * speeds[time_speed]
      : 0.0f;

  /* Only advance if we have a valid delta (app_controller provides it) */
  if (game_days > 0.0f && game->time_engine && time_speed > 0) {
    time_accumulator += game_days;
    /* Advance time engine when enough days have passed (~30 days per old turn) */
    float days_per_advance = 365.0f / 12.0f; /* monthly advance */
    while (time_accumulator >= days_per_advance) {
      time_accumulator -= days_per_advance;
      civ_time_engine_advance_turn((civ_time_engine_t *)game->time_engine);
      game->current_turn++;
      /* Process systems that used to run on end_turn */
      civ_game_end_turn(game);
      /* Toast notifications for key events */
      if (game->current_turn % 10 == 0 && game->nation_manager) {
        civ_nation_manager_t *nm = (civ_nation_manager_t *)game->nation_manager;
        if (nm->count > 0) {
          int pi = nm->player_nation_index;
          if (pi >= 0 && pi < nm->count) {
            char tbuf[128];
            snprintf(tbuf, sizeof(tbuf), "%s: GDP $%.0fM | Growth %+.1f%%",
                     nm->nations[pi].name,
                     nm->nations[pi].economy.gdp,
                     nm->nations[pi].economy.gdp_growth * 100.0f);
            toast_push(tbuf, g_theme.info);
          }
        }
      }
      if (game->current_turn % 25 == 0)
        toast_push("Global markets updated", g_theme.warning);
    }
  }

  /* Nation hover detection — only on map screen */
  hovered_country[0] = '\0';
  if (current_screen == SCR_MAP && game->world_map && input->win_w > 0) {
    float wx, wy;
    civ_camera_screen_to_world(&cam, input->win_w, input->win_h,
                               input->mouse_x, input->mouse_y, &wx, &wy);
    int32_t tx = (int32_t)wx, ty = (int32_t)wy;
    if (tx >= 0 && ty >= 0 && tx < game->world_map->width && ty < game->world_map->height) {
      int16_t cid = civ_political_borders_tile_country(tx, ty, game->world_map->width);
      if (cid >= 0) {
        const char *nm = civ_political_borders_country_name(cid);
        if (nm) {
          snprintf(hovered_country, sizeof(hovered_country), "%s", nm);
        }
        hover_country_x = (float)input->mouse_x;
        hover_country_y = (float)input->mouse_y;
      }
    }
  }

  /* ── Click handling — only on map screen ───────────────── */
  if (!input->mouse_left_pressed) return;

  /* Nation selection on map click */
  if (current_screen == SCR_MAP && game->world_map) {
    float wx, wy;
    civ_camera_screen_to_world(&cam, input->win_w, input->win_h,
                               input->mouse_x, input->mouse_y, &wx, &wy);
    int32_t tx = (int32_t)wx, ty = (int32_t)wy;
    if (tx >= 0 && ty >= 0 && tx < game->world_map->width && ty < game->world_map->height) {
      int16_t cid = civ_political_borders_tile_country(tx, ty, game->world_map->width);
      const char *nm = civ_political_borders_country_name(cid);
      if (nm && cid >= 0) {
        selected_nation_cid = cid;
        snprintf(selected_nation_id, sizeof(selected_nation_id), "%s", nm);
        show_nation_detail = true;
        printf("[SELECT] Nation: %s (cid=%d)\n", nm, cid);
        return;
      } else {
        /* Clicked empty space — dismiss detail panel */
        show_nation_detail = false;
      }
    }
  }

  /* Panel toggle buttons */
  if (input->mouse_x > last_win_w - 680 && input->mouse_x < last_win_w - 530 &&
      input->mouse_y < 40) {
    show_diplomacy = !show_diplomacy;
    if (show_diplomacy) show_research = show_government = show_wonders = false;
  }
  if (input->mouse_x > last_win_w - 530 && input->mouse_x < last_win_w - 380 &&
      input->mouse_y < 40) {
    show_wonders = !show_wonders;
    if (show_wonders) show_diplomacy = show_research = show_government = false;
  }
  if (input->mouse_x > last_win_w - 380 && input->mouse_x < last_win_w - 230 &&
      input->mouse_y < 40) {
    show_government = !show_government;
    if (show_government) show_diplomacy = show_research = show_wonders = false;
  }
  if (input->mouse_x > last_win_w - 230 && input->mouse_y < 40) {
    show_research = !show_research;
    if (show_research)
      show_diplomacy = show_government = show_wonders = show_rulebook = false;
  }

  /* Panel click handlers */
  if (show_research && civ_research_panel_click(game, input, last_win_w, last_win_h))
    return;
  if (show_diplomacy &&
      civ_diplomacy_panel_click(game, input, last_win_w, last_win_h,
                                has_contacted_rival))
    return;

  /* World-space selection */
  float wx, wy;
  civ_camera_screen_to_world(&cam, last_win_w, last_win_h, input->mouse_x,
                             input->mouse_y, &wx, &wy);
  int32_t tx = (int32_t)floorf(wx), ty = (int32_t)floorf(wy);

  civ_unit_t *old_unit = selected_unit;
  civ_settlement_t *old_settlement = selected_settlement;
  selected_unit = NULL;
  selected_settlement = NULL;

  /* Check settlement hit */
  if (game->settlement_manager) {
    for (size_t i = 0; i < game->settlement_manager->settlement_count; i++) {
      if ((int32_t)game->settlement_manager->settlements[i].x == tx &&
          (int32_t)game->settlement_manager->settlements[i].y == ty) {
        selected_settlement = &game->settlement_manager->settlements[i];
        break;
      }
    }
  }

  /* Check unit hit */
  if (!selected_settlement && game->unit_manager) {
    for (size_t i = 0; i < game->unit_manager->unit_count; i++) {
      if (game->unit_manager->units[i].x == tx &&
          game->unit_manager->units[i].y == ty) {
        selected_unit = &game->unit_manager->units[i];
        break;
      }
    }
  }

  /* Settlement sidebar clicks */
  if (selected_settlement && !selected_settlement->is_producing) {
    int rc = civ_settlement_sidebar_click(selected_settlement, input);
    if (rc == 1)
      selected_settlement->is_producing = true,
      selected_settlement->production_type = 0,
      selected_settlement->production_target = 30.0f,
      selected_settlement->production_progress = 0.0f;
    else if (rc == 2)
      selected_settlement->is_producing = true,
      selected_settlement->production_type = 7,
      selected_settlement->production_target = 80.0f,
      selected_settlement->production_progress = 0.0f;
  }

  /* Unit sidebar: Found City */
  if (selected_unit && selected_unit->unit_type == CIV_UNIT_TYPE_SETTLER &&
      !selected_unit->has_moved && civ_unit_sidebar_click(selected_unit, input)) {
    civ_attempt_settlement_spawn(game->settlement_manager,
                                 (float)selected_unit->x,
                                 (float)selected_unit->y);
    selected_unit->current_strength = 0;
    selected_unit = NULL;
    update_visibility(game);
    return;
  }

  /* Deselect when clicking empty space */
  bool on_ui = (input->mouse_x < 220 && input->mouse_y > last_win_h - 170) ||
               (input->mouse_x < 300);
  if (!selected_unit && !selected_settlement && !on_ui) {
    selected_unit = old_unit;
    selected_settlement = old_settlement;
  }

  /* Unit movement (right click) */
  if (input->mouse_right_pressed && selected_unit && !selected_unit->has_moved) {
    int dx = abs(tx - selected_unit->x), dy = abs(ty - selected_unit->y);
    if (dx <= 1 && dy <= 1 && (dx + dy > 0))
      selected_unit->x = tx, selected_unit->y = ty,
      selected_unit->has_moved = true, update_visibility(game);
  }

  /* First contact check */
  if (game->unit_manager && game->settlement_manager && !has_contacted_rival) {
    for (size_t i = 0; i < game->unit_manager->unit_count; i++) {
      civ_unit_t *u = &game->unit_manager->units[i];
      if (strstr(u->name, "Rival")) continue;
      for (size_t j = 0; j < game->settlement_manager->settlement_count; j++) {
        civ_settlement_t *s = &game->settlement_manager->settlements[j];
        if (strcmp(s->id, "rival_capital") == 0) {
          float d2 = (float)((u->x - s->x) * (u->x - s->x) +
                             (u->y - s->y) * (u->y - s->y));
          if (d2 < 64.0f) has_contacted_rival = true;
        }
      }
    }
  }

  /* Update toast timers */
  toast_update(game ? 1.0f / 60.0f : 0.016f);
}

static void render(SDL_Renderer *renderer, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  if (!game || !game->world_map) return;
  last_win_w = win_w;
  last_win_h = win_h;

  /* Local currency for screens */
  const char *local_cur = "USD"; const char *local_sym = "$";
  if (game->wallet.count > 0) { local_cur = game->wallet.slots[0].currency_iso;
    civ_market_currency_t *mc = civ_market_get_currency(game->market, local_cur);
    if (mc) local_sym = mc->symbol; }

  /* Lazy-init flag textures — needs SDL_Renderer */
  if (!flags_loaded && game->flag_system) {
    game->flag_system->renderer = renderer;
    int n = civ_flag_system_load_textures(game->flag_system);
    if (n > 0) printf("[GAME] Loaded %d flag textures\n", n);
    flags_loaded = true;
  }

  /* Lazy-init map rendering context — only when on map screen */
  if (current_screen == SCR_MAP) {
    if (!map_ctx && game->world_map) {
      map_ctx = civ_render_map_context_create(renderer, win_w, win_h,
                                              game->world_map->width,
                                              game->world_map->height);
      if (map_ctx) {
        map_ctx->view_x = cam.x; map_ctx->view_y = cam.y;
        map_ctx->zoom = cam.zoom;
        /* Build LOD buffers for smooth global zoom */
        civ_render_map_build_lods(map_ctx, game->world_map, renderer);
      }
    }
    if (map_ctx) {
      map_ctx->view_x = cam.x; map_ctx->view_y = cam.y;
      map_ctx->zoom = cam.zoom;
    }
    float minZ = (float)last_win_h / ((float)game->world_map->height * 4.0f);
    if (cam.zoom < minZ) { cam.zoom = minZ; if (map_ctx) map_ctx->zoom = minZ; }
  }

  /* ── Main content: MAP screen only renders the map ─────── */
  if (current_screen == SCR_MAP) {
    render_map_layer(renderer, game);
    render_settlements_layer(renderer, game);
    render_units_layer(renderer, game);
    /* Border lines visible at zoom > 1.0x */
    if (cam.zoom > 1.0f && map_ctx)
      civ_render_map_borders(renderer, map_ctx, game->world_map,
                             win_w, win_h);
    render_minimap_layer(renderer, game);
    render_city_labels(renderer, game);

  } /* end MAP screen block */

  /* ── Nuklear shared UI (all screens) ──────────────────────── */
  {
    struct nk_context *nk = g_nk_ctx;
    if (nk) {
      float bal = (game->wallet.count > 0 && game->market)
          ? civ_wallet_total(&game->wallet, game->market) : 0.0f;
      float lat = 90.0f - (cam.y / (float)cam.map_height) * 180.0f;
      float lon = (cam.x / (float)cam.map_width) * 360.0f - 180.0f;
      while (lon < -180.0f) lon += 360.0f; while (lon > 180.0f) lon -= 360.0f;

      /* Top bar */
      if (nk_begin(nk, "HUD", nk_rect(0, 0, (float)win_w, 32),
                   NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(nk, 32, 4);
        char time_buf[64];
        if (game->time_engine)
          civ_time_engine_format_hud((civ_time_engine_t*)game->time_engine, time_buf, sizeof(time_buf));
        else snprintf(time_buf, sizeof(time_buf), "Turn %d", game->current_turn);
        nk_label(nk, time_buf, NK_TEXT_CENTERED);
        const char *vnames[] = {"Political","Geographical","Economic","Demographical","Cultural"};
        if (nk_button_label(nk, vnames[(int)current_map_view]))
          current_map_view = (civ_map_view_type_t)(((int)current_map_view + 1) % 5);
        char coord_buf[64];
        snprintf(coord_buf, sizeof(coord_buf), "$%.0f  %.0f%c %.0f%c",
            bal, fabsf(lat), lat>=0?'N':'S', fabsf(lon), lon>=0?'E':'W');
        nk_label(nk, coord_buf, NK_TEXT_RIGHT);
        nk_spacing(nk, 1);
      }
      nk_end(nk);

      /* Left sidebar */
      if (nk_begin(nk, "Nav", nk_rect(0, 32, 220, (float)win_h - 32),
                   NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(nk, 22, 1);
        nk_label(nk, "DOMINION", NK_TEXT_CENTERED);
        int nav_n = game->player_role.nav_count;
        for (int i = 0; i < nav_n && i < 14; i++) {
          civ_nav_screen_t ns = (civ_nav_screen_t)game->player_role.nav_screens[i];
          if (ns < 0) continue;
          nk_layout_row_dynamic(nk, 24, 1);
          bool active = (ns == CIV_NAV_MAP && current_screen == SCR_MAP) ||
              (ns == CIV_NAV_ECONOMY && current_screen == SCR_ECONOMY) ||
              (ns == CIV_NAV_GOVERNANCE && current_screen == SCR_GOVERNANCE) ||
              (ns == CIV_NAV_DIPLOMACY && current_screen == SCR_DIPLOMACY) ||
              (ns == CIV_NAV_DASHBOARD && current_screen == SCR_DASHBOARD);
          if (active) nk_button_set_behavior(nk, NK_BUTTON_DEFAULT);
          if (nk_button_label(nk, civ_nav_screen_label(ns))) {
            switch (ns) {
            case CIV_NAV_MAP: current_screen = SCR_MAP; break;
            case CIV_NAV_ECONOMY: current_screen = SCR_ECONOMY; break;
            case CIV_NAV_GOVERNANCE: current_screen = SCR_GOVERNANCE; break;
            case CIV_NAV_DIPLOMACY: current_screen = SCR_DIPLOMACY; break;
            case CIV_NAV_DASHBOARD: current_screen = SCR_DASHBOARD; break;
            case CIV_NAV_WORK: current_screen = SCR_WORK; break;
            case CIV_NAV_FINANCE: current_screen = SCR_FINANCE; break;
            case CIV_NAV_HOUSING: current_screen = SCR_HOUSING; break;
            case CIV_NAV_EDUCATION: current_screen = SCR_EDUCATION; break;
            default: current_screen = SCR_DASHBOARD; break;
            }
          }
          if (active) nk_button_set_behavior(nk, NK_BUTTON_DEFAULT);
        }
        nk_layout_row_dynamic(nk, 18, 1);
        { char wbuf[48]; snprintf(wbuf, sizeof(wbuf), "Balance: $%.0f", bal);
          nk_label(nk, wbuf, NK_TEXT_CENTERED); }
      }
      nk_end(nk);

      /* Nation detail popup */
      if (show_nation_detail && selected_nation_id[0] && game->nation_manager) {
        civ_nation_t *nat = civ_nation_get_by_id(
            (civ_nation_manager_t*)game->nation_manager, selected_nation_id);
        if (nat && nk_begin(nk, nat->name, nk_rect(240, 40, 360, 320),
                 NK_WINDOW_TITLE|NK_WINDOW_CLOSABLE|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
          nk_layout_row_dynamic(nk, 18, 1);
          { char buf[128]; snprintf(buf, sizeof(buf), "Pop: %lldM  GDP: $%.0fM  +%.1f%%",
              nat->population/1000000, nat->economy.gdp, nat->economy.gdp_growth*100);
            nk_label(nk, buf, NK_TEXT_LEFT); }
          { char buf[128]; snprintf(buf, sizeof(buf), "Unemp: %.1f%%  Infl: %.1f%%  Food: %.0fM",
              nat->economy.unemployment*100, nat->economy.inflation*100, nat->economy.food_production/1000000);
            nk_label(nk, buf, NK_TEXT_LEFT); }
          { char buf[128]; snprintf(buf, sizeof(buf), "Gov: %s  ISO: %s",
              civ_government_proximity_label(nat->government), nat->iso_a3);
            nk_label(nk, buf, NK_TEXT_LEFT); }
          { char buf[128]; snprintf(buf, sizeof(buf), "Indices: T%+d E%+d M%+d C%+d",
              nat->tech_index, nat->economic_index, nat->military_index, nat->cultural_index);
            nk_label(nk, buf, NK_TEXT_LEFT); }
          nk_layout_row_dynamic(nk, 28, 3);
          if (nk_button_label(nk, "Gov")) { current_screen = SCR_GOVERNANCE; show_nation_detail = false; }
          if (nk_button_label(nk, "Dip")) { current_screen = SCR_DIPLOMACY; show_nation_detail = false; }
          if (nk_button_label(nk, "Eco")) { current_screen = SCR_ECONOMY; show_nation_detail = false; }
          nk_end(nk);
        } else { show_nation_detail = false; }
      }

      /* Toasts */
      for (int i = toast_count - 1; i >= 0; i--) {
        if (nk_begin(nk, "T", nk_rect((float)(win_w/2-200), (float)(win_h-80-i*34), 400, 26),
                     NK_WINDOW_NO_SCROLLBAR)) {
          nk_layout_row_dynamic(nk, 20, 1);
          nk_label(nk, toasts[i].msg, NK_TEXT_CENTERED);
        }
        nk_end(nk);
      }
    }
  }

  /* ── Nuklear data screens (non-MAP) ───────────────────────── */
  if (current_screen != SCR_MAP) {
    struct nk_context *nk = g_nk_ctx;
    if (nk) {
      const char *titles[] = {"DIPLOMACY","ECONOMY","MILITARY","TECHNOLOGY","GOVERNANCE","CULTURE",
                              "NEWS","DASHBOARD","WORK","FINANCE","HOUSING","EDUCATION","NETWORK",
                              "POLITICS","HEALTH","CONSTITUTION"};
      const char *title = (current_screen >= 0 && current_screen < 16) ? titles[current_screen - 1] : "SCREEN";
      if (nk_begin(nk, title, nk_rect(230, 34, (float)win_w - 250, (float)win_h - 80),
                   NK_WINDOW_TITLE|NK_WINDOW_CLOSABLE)) {
        nk_layout_row_dynamic(nk, 20, 1);

        /* Technology screen */
        if (current_screen == SCR_TECHNOLOGY && game->technology_tree) {
          civ_innovation_system_t *is = game->technology_tree;
          for (int d = 0; d < CIV_TECH_DOMAIN_COUNT; d++) {
            char buf[128];
            snprintf(buf, sizeof(buf), "%-34s %+d (+%.1f/t)", is->domains[d].name, is->domains[d].index, is->domains[d].growth_rate);
            nk_label(nk, buf, NK_TEXT_LEFT);
          }
        }
        /* Governance screen */
        else if (current_screen == SCR_GOVERNANCE) {
          civ_government_t *gov = game->government;
          const char *gov_title = "YOUR GOVERNMENT";
          if (selected_nation_cid >= 0 && game->nation_manager) {
            civ_nation_t *nat = civ_nation_get_by_id((civ_nation_manager_t*)game->nation_manager, selected_nation_id);
            if (nat && nat->government) { gov = nat->government; gov_title = nat->name; }
          }
          char buf[256];
          snprintf(buf, sizeof(buf), "%s — %s", gov_title, civ_government_proximity_label(gov));
          nk_label(nk, buf, NK_TEXT_LEFT);
          snprintf(buf, sizeof(buf), "Stability: %.0f%%  Legitimacy: %.0f%%  Efficiency: %.0f%%  Happiness: %.0f%%",
              gov->stability*100, gov->legitimacy*100, gov->efficiency*100, gov->profile.citizen_happiness*100);
          nk_label(nk, buf, NK_TEXT_LEFT);
          snprintf(buf, sizeof(buf), "Auth: %.0f%%  Rep: %.0f%%  Rigidity: %.0f%%  Ranking: %.0f",
              gov->profile.authority_concentration*100, gov->profile.representation_index*100,
              gov->profile.institutional_rigidity*100, gov->profile.governance_ranking);
          nk_label(nk, buf, NK_TEXT_LEFT);
          nk_label(nk, "POLITICAL POSITIONS:", NK_TEXT_LEFT);
          for (size_t i = 0; i < gov->position_count && i < 20; i++) {
            snprintf(buf, sizeof(buf), "%s — %s · %s · %d seats", gov->positions[i].title,
                gov->positions[i].selection_method, gov->positions[i].term, gov->positions[i].position_count);
            nk_label(nk, buf, NK_TEXT_LEFT);
          }
        }
        /* Economy screen */
        else if (current_screen == SCR_ECONOMY) {
          if (game->market) {
            civ_market_engine_t *mkt = game->market;
            char buf[192];
            float avg_inf = 0; for (int i=0; i<5 && i<mkt->currency_count; i++) avg_inf+=mkt->currencies[i].inflation; avg_inf/=5;
            snprintf(buf, sizeof(buf), "Inflation: %.1f%%  |  Commodities: %d  |  Currencies: %d",
                avg_inf*100, mkt->commodity_count, mkt->currency_count);
            nk_label(nk, buf, NK_TEXT_LEFT);
            nk_label(nk, "FOREX:", NK_TEXT_LEFT);
            for (int ci=0; ci<10 && ci<mkt->currency_count; ci++) {
              snprintf(buf, sizeof(buf), "%-4s %10.4f %+6.1f%% %s",
                  mkt->currencies[ci].iso, mkt->currencies[ci].current_rate,
                  (mkt->currencies[ci].current_rate/mkt->currencies[ci].base_rate-1)*100,
                  mkt->currencies[ci].name);
              nk_label(nk, buf, NK_TEXT_LEFT);
            }
          }
        }
        /* News screen */
        else if (current_screen == SCR_NEWS) {
          if (game->npc_engine) {
            civ_npc_engine_t *ne = (civ_npc_engine_t*)game->npc_engine;
            civ_decision_t decisions[12];
            int nd = civ_npc_engine_get_recent(ne, 12, decisions);
            if (nd == 0) nk_label(nk, "No news yet.", NK_TEXT_LEFT);
            for (int i = 0; i < nd; i++) {
              char buf[256]; const char *cats[] = {"[MIL]","[ECO]","[POL]","[SOC]","[DIP]"};
              snprintf(buf, sizeof(buf), "%s %s — %s", cats[decisions[i].category%5], decisions[i].description, decisions[i].nation_id);
              nk_label(nk, buf, NK_TEXT_LEFT);
            }
          }
        }
        /* Delegated screens */
        else if (current_screen == SCR_DASHBOARD)
          civ_screen_dashboard_render(renderer, game, font_hud, 0,0,0,0,0, input, local_cur, local_sym);
        else if (current_screen == SCR_WORK)
          civ_screen_work_render(renderer, game, font_hud, 0,0,0,0,0, input, local_cur, local_sym);
        else if (current_screen == SCR_FINANCE)
          civ_screen_finance_render(renderer, game, font_hud, 0,0,0,0,0, input, local_cur, local_sym);
        else if (current_screen == SCR_HOUSING)
          civ_screen_housing_render(renderer, game, font_hud, 0,0,0,0,0, input, local_cur, local_sym);
        else if (current_screen == SCR_EDUCATION)
          civ_screen_education_render(renderer, game, font_hud, 0,0,0,0,0, input, local_cur, local_sym);
        else if (current_screen == SCR_NETWORK)
          civ_screen_network_render(renderer, game, font_hud, 0,0,0,0,0, input, local_cur, local_sym);
        else if (current_screen == SCR_POLITICS)
          civ_screen_politics_render(renderer, game, font_hud, 0,0,0,0,0, input, local_cur, local_sym);
        else if (current_screen == SCR_HEALTH)
          civ_screen_health_render(renderer, game, font_hud, 0,0,0,0,0, input, local_cur, local_sym);
        else if (current_screen == SCR_CONSTITUTION)
          civ_screen_constitution_render(renderer, game, font_hud, 0,0,0,0,0, input, local_cur, local_sym);
        else
          nk_label(nk, "Screen under development", NK_TEXT_CENTERED);
      }
      nk_end(nk);
    }
  }

  /* ── Debug overlay ─────────────────────────────────────────── */
  if (debug.enabled)
    civ_debug_overlay_render(&debug, renderer, win_w);

  /* ESC dismisses nation detail */
  if (input->esc_pressed && show_nation_detail)
    show_nation_detail = false;

  if (input->esc_pressed && (show_research || show_government || show_wonders ||
                              show_rulebook || show_diplomacy))
    show_research = show_government = show_wonders = show_rulebook =
        show_diplomacy = false;
}

static void destroy(void) {
  if (map_ctx) civ_render_map_context_destroy(map_ctx), map_ctx = NULL;
  if (font_hud) civ_font_destroy(font_hud), font_hud = NULL;
}

civ_scene_t scene_game = {.init = init,
                          .update = update,
                          .render = render,
                          .destroy = destroy,
                          .next_scene_id = -1};
