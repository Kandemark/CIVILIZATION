#include "../../../include/core/character.h"
#include "../../../include/core/profile.h"
#include "../../../include/core/military/combat.h"
#include "../../../include/core/time_engine.h"
#include "../../../include/core/world/nation.h"
#include "../../../include/core/world/political_borders.h"
#include "../../../include/core/world/map_view.h"
#include "../../../include/core/world/wonders.h"
#include "../../../include/display/camera.h"
#include "../../../include/display/debug_overlay.h"
#include "../../../include/display/layer.h"
#include "../../../include/display/theme.h"
#include "../../../include/ui/graph/graph.h"
#include "../../../include/engine/font.h"
#include "../../../include/engine/renderer.h"
#include "../../../include/ui/panel/diplomacy_panel.h"
#include "../../../include/ui/panel/governance_panel.h"
#include "../../../include/ui/panel/research_panel.h"
#include "../../../include/ui/panel/rulebook_panel.h"
#include "../../../include/ui/panel/settlement_sidebar.h"
#include "../../../include/ui/panel/unit_sidebar.h"
#include "../../../include/ui/panel/wonders_panel.h"
#include "../../../include/ui/scene.h"
#include "../../../include/ui/screens/screens.h"
#include "../../../include/ui/ui_common.h"
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
#include "../../../include/core/constitution.h"
#include "../../../include/core/npc_engine.h"
static civ_screen_t             current_screen = SCR_MAP;
static char                     selected_nation_id[64] = "";
static int16_t                  selected_nation_cid = -1;
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
  civ_render_map(r, map_ctx, game->world_map, last_win_w, last_win_h);
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
                                   (int)size + 8, 0x00A0FF, 180);

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
  civ_render_rect_filled_alpha(r, 0, 0, last_win_w, 38, 0x050A14, 235);
  civ_render_line(r, 0, 38, last_win_w, 38, 0x1A2A3A);

  float lat = 90.0f - (cam.y / (float)cam.map_height) * 180.0f;
  float lon = (cam.x / (float)cam.map_width) * 360.0f - 180.0f;
  while (lon < -180.0f) lon += 360.0f;
  while (lon > 180.0f) lon -= 360.0f;

  char buf[128];
  /* Time — left section */
  char time_buf[64];
  if (game->time_engine) {
    civ_time_engine_format_hud((civ_time_engine_t *)game->time_engine,
                               time_buf, sizeof(time_buf));
  } else {
    snprintf(time_buf, sizeof(time_buf), "Turn %d", game->current_turn);
  }
  civ_font_render_aligned(r, font_hud, time_buf, 228, 0, 260, 38,
                          CIV_COLOR_PRIMARY, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

  /* Science */
  if (game->technology_tree) {
    civ_innovation_system_t *it = game->technology_tree;
    sprintf(buf, "Sci +%.0f  Idx %+d", it->total_budget, it->aggregate_index);
    civ_font_render_aligned(r, font_hud, buf, 500, 0, 240, 38, 0x00A0FF,
                            CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
  }

  /* Coords */
  sprintf(buf, "%.0f%c %.0f%c  z%.1f",
          fabsf(lat), lat >= 0 ? 'N' : 'S',
          fabsf(lon), lon >= 0 ? 'E' : 'W',
          cam.zoom * 10.0f);
  civ_font_render_aligned(r, font_hud, buf, 750, 0, 200, 38,
                          CIV_COLOR_TEXT_DIM, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

  /* Hovered country — info card */
  if (hovered_country[0]) {
    civ_font_render_aligned(r, font_hud, hovered_country, 255, 0, 180, 38,
                            0xFFCC00, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
    /* Info card near cursor */
    int cx = (int)hover_country_x + 18, cy = (int)hover_country_y + 18;
    int cw = 180, ch = 72;
    /* Keep on screen */
    if (cx + cw > last_win_w) cx = (int)hover_country_x - cw - 10;
    if (cy + ch > last_win_h) cy = (int)hover_country_y - ch - 10;

    civ_render_rect_filled_alpha(r, cx, cy, cw, ch, 0x0A1220, 240);
    civ_render_rect_outline(r, cx, cy, cw, ch, 0xFFCC00, 1);
    civ_font_render_aligned(r, font_hud, hovered_country, cx + 8, cy + 4,
                            cw - 16, 20, 0xFFCC00, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    /* Look up nation data */
    civ_nation_t *nat = NULL;
    if (game->nation_manager)
      nat = civ_nation_get_by_id((civ_nation_manager_t *)game->nation_manager,
                                  hovered_country);
    char info[64] = "Independent State";
    if (nat) {
      snprintf(info, sizeof(info), "Pop: %lldM | Gov: %s",
               nat->population / 1000000,
               civ_government_proximity_label(nat->government));
    }
    civ_font_render_aligned(r, font_hud, info, cx + 8, cy + 26,
                            cw - 16, 18, 0x8899AA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
    civ_font_render_aligned(r, font_hud, "Click for details", cx + 8, cy + 48,
                            cw - 16, 16, 0x556677, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  }

}

static void render_hud_buttons(SDL_Renderer *r, civ_input_state_t *input) {
  /* Compact icon-style buttons in top bar, with dividers */
  const char *labels[] = {"R", "T", "G", "W", "D"};
  uint32_t colors[] = {0x00A0FF, 0x00A0FF, 0xFFCC00, 0xFFFF00, 0xFFFF00};
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
                            active ? 0xFFFFFF : 0x1A2A3A, 1);
    civ_font_render_aligned(r, font_hud, labels[i], bx, 3, btn_s, btn_s,
                            active ? 0xFFFFFF : 0x8899AA,
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
    uint32_t bg = active ? 0x004A7A : (hov ? 0x2A3A4A : 0x1A2A3A);
    civ_render_rect_filled_alpha(r, bx, btn_y, btn_w, btn_h, bg, 220);
    civ_render_rect_outline(r, bx, btn_y, btn_w, btn_h,
                            active ? 0x00A0FF : 0x1A2A3A, 1);
    civ_font_render_aligned(r, font_hud, labels[i], bx, btn_y, btn_w, btn_h,
                            active ? 0xFFFFFF : 0x8899AA,
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
                          btn_h, 0xAABBCC, CIV_ALIGN_RIGHT, CIV_VALIGN_MIDDLE);
  (void)game;
}

static void render_minimap_layer(SDL_Renderer *r, civ_game_t *game) {
  int mm_w = 200, mm_h = 150, mm_x = 20, mm_y = last_win_h - mm_h - 20;
  civ_render_minimap(r, mm_x, mm_y, mm_w, mm_h, game->world_map, NULL);
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
                          0xFFFFFF, 1);
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
  map_ctx = NULL; /* Created on first render with valid window size */
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
        printf("[SELECT] Nation: %s (cid=%d)\n", nm, cid);
        current_screen = SCR_GOVERNANCE; /* jump to governance */
        return;
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
}

static void render(SDL_Renderer *renderer, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  if (!game || !game->world_map) return;
  last_win_w = win_w;
  last_win_h = win_h;

  /* Local currency for screens */
  const char *local_cur = "USD"; const char *local_sym = "$";
  if (game->wallet.count > 0) { local_cur = game->wallet.slots[0].currency_iso;
    civ_market_currency_t *mc = civ_market_get_currency((civ_market_engine_t*)game->market, local_cur);
    if (mc) local_sym = mc->symbol; }

  /* Lazy-init map rendering context — only when on map screen */
  if (current_screen == SCR_MAP) {
    if (!map_ctx && game->world_map) {
      map_ctx = civ_render_map_context_create(renderer, win_w, win_h,
                                              game->world_map->width,
                                              game->world_map->height);
      if (map_ctx) {
        map_ctx->view_x = cam.x; map_ctx->view_y = cam.y;
        map_ctx->zoom = cam.zoom;
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
    render_minimap_layer(renderer, game);
  } else {
    /* Full-area screen background — covers everything below top bar */
    int sx = 0, sy = 38, sw = win_w, sh = win_h - sy - 50;
    civ_render_rect_filled_alpha(renderer, sx, sy, sw, sh, 0x060A14, 255);

    /* Screen header */
    const char *titles[] = {"DIPLOMACY", "ECONOMY", "MILITARY",
                            "TECHNOLOGY", "GOVERNANCE", "CULTURE"};
    int si = current_screen - 1;
    if (si >= 0 && si < 6) {
      civ_font_render_aligned(renderer, font_hud, titles[si],
                              sx + 230, sy + 12, sw - 250, 28,
                              CIV_COLOR_PRIMARY, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
    }

    int cx = 230, cy = sy + 50, cw = sw - cx - 20, ch = sh - 80;

    /* ── Screen content — delegated to screen modules ── */
    if (current_screen == SCR_TECHNOLOGY && game->technology_tree) {
      civ_innovation_system_t *is = game->technology_tree;
      int dy = cy;
      for (int d = 0; d < CIV_TECH_DOMAIN_COUNT; d++) {
        char buf[128];
        snprintf(buf, sizeof(buf), "%-34s %+d (+%.1f/t)", is->domains[d].name, is->domains[d].index, is->domains[d].growth_rate);
        uint32_t tc = is->domains[d].index > 200 ? 0x44FF44 : is->domains[d].index > 0 ? 0xAACCAA : 0xFF6644;
        civ_font_render_aligned(renderer, font_hud, buf, cx, dy, cw, 20, tc, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); dy += 24;
      }
    } else if (current_screen == SCR_GOVERNANCE) {
      civ_government_t *gov = NULL; const char *gov_title = "YOUR GOVERNMENT";
      if (selected_nation_cid >= 0 && game->nation_manager) { civ_nation_t *nat = civ_nation_get_by_id((civ_nation_manager_t *)game->nation_manager, selected_nation_id); if (nat && nat->government) { gov = nat->government; gov_title = nat->name; } }
      if (!gov) gov = game->government;
      int dy = cy; char buf[256];
      snprintf(buf, sizeof(buf), "%s — %s", gov_title, civ_government_proximity_label(gov));
      civ_font_render_aligned(renderer, font_hud, buf, cx, dy, cw, 24, 0x00A0FF, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); dy += 32;
      snprintf(buf, sizeof(buf), "Stability: %.0f%%  Legitimacy: %.0f%%  Efficiency: %.0f%%  Happiness: %.0f%%", gov->stability*100, gov->legitimacy*100, gov->efficiency*100, gov->profile.citizen_happiness*100);
      civ_font_render_aligned(renderer, font_hud, buf, cx, dy, cw, 18, 0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); dy += 28;
      snprintf(buf, sizeof(buf), "Auth: %.0f%%  Rep: %.0f%%  Rigidity: %.0f%%  Balance: %.0f%%  Ranking: %.0f", gov->profile.authority_concentration*100, gov->profile.representation_index*100, gov->profile.institutional_rigidity*100, gov->profile.power_balance*100, gov->profile.governance_ranking);
      civ_font_render_aligned(renderer, font_hud, buf, cx, dy, cw, 16, 0x8899AA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); dy += 30;
      civ_render_line(renderer, cx, dy, cx + cw, dy, 0x1A2A3A); dy += 8;
      civ_font_render_aligned(renderer, font_hud, "POLITICAL POSITIONS", cx, dy, 200, 20, 0xFFCC00, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); dy += 26;
      for (size_t i = 0; i < gov->position_count && dy < sy + sh - 20; i++) { int indent = gov->positions[i].hierarchy_level * 18;
        snprintf(buf, sizeof(buf), "%s", gov->positions[i].title); civ_font_render_aligned(renderer, font_hud, buf, cx + indent, dy, cw - indent, 18, 0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); dy += 18;
        snprintf(buf, sizeof(buf), "%s · %s · %d seat%s", gov->positions[i].selection_method, gov->positions[i].term, gov->positions[i].position_count, gov->positions[i].position_count > 1 ? "s" : ""); civ_font_render_aligned(renderer, font_hud, buf, cx + indent + 12, dy, cw - indent - 12, 14, 0x667788, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); dy += 22; }
    } else if (current_screen == SCR_ECONOMY) {
      civ_screen_economy_render(renderer, game, font_hud, cx, cy, cw, ch, sh - sy, input, local_cur, local_sym);
    } else if (current_screen == SCR_NEWS) {
      int dy = cy;
      civ_font_render_aligned(renderer, font_hud, "GLOBAL NEWS FEED", cx, dy, cw, 24, 0xFFCC00, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); dy += 32;
      if (game->npc_engine) { civ_npc_engine_t *ne = (civ_npc_engine_t *)game->npc_engine; civ_decision_t decisions[12]; int nd = civ_npc_engine_get_recent(ne, 12, decisions);
        for (int i = 0; i < nd && dy < sy + sh - 20; i++) { civ_decision_t *d = &decisions[i]; char buf[256]; const char *cats[] = {"[MIL]","[ECO]","[POL]","[SOC]","[DIP]"};
          snprintf(buf, sizeof(buf), "%s %s", cats[d->category % 5], d->description); civ_font_render_aligned(renderer, font_hud, buf, cx, dy, cw, 18, 0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); dy += 18;
          snprintf(buf, sizeof(buf), "  %s — Year %d, Day %d | Stab %+.2f Eco %+.2f Dip %+.2f", d->nation_id, d->global_year, d->global_day, d->stability_effect, d->economic_effect, d->diplomatic_effect); civ_font_render_aligned(renderer, font_hud, buf, cx+12, dy, cw-12, 14, 0x556677, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); dy += 20; }
        if (nd == 0) civ_font_render_aligned(renderer, font_hud, "No news yet", cx, dy, cw, 24, 0x556677, CIV_ALIGN_CENTER, CIV_VALIGN_TOP); }
    } else if (current_screen == SCR_DASHBOARD) {
      civ_screen_dashboard_render(renderer, game, font_hud, cx, cy, cw, ch, sh - sy, input, local_cur, local_sym);
    } else if (current_screen == SCR_WORK) {
      civ_screen_work_render(renderer, game, font_hud, cx, cy, cw, ch, sh - sy, input, local_cur, local_sym);
    } else if (current_screen == SCR_FINANCE) {
      civ_screen_finance_render(renderer, game, font_hud, cx, cy, cw, ch, sh - sy, input, local_cur, local_sym);
    } else if (current_screen == SCR_HOUSING) {
      civ_screen_housing_render(renderer, game, font_hud, cx, cy, cw, ch, sh - sy, input, local_cur, local_sym);
    } else if (current_screen == SCR_EDUCATION) {
      civ_screen_education_render(renderer, game, font_hud, cx, cy, cw, ch, sh - sy, input, local_cur, local_sym);
    } else if (current_screen == SCR_NETWORK) {
      civ_screen_network_render(renderer, game, font_hud, cx, cy, cw, ch, sh - sy, input, local_cur, local_sym);
    } else if (current_screen == SCR_POLITICS) {
      civ_screen_politics_render(renderer, game, font_hud, cx, cy, cw, ch, sh - sy, input, local_cur, local_sym);
    } else if (current_screen == SCR_HEALTH) {
      civ_screen_health_render(renderer, game, font_hud, cx, cy, cw, ch, sh - sy, input, local_cur, local_sym);
    } else if (current_screen == SCR_CONSTITUTION) {
      civ_screen_constitution_render(renderer, game, font_hud, cx, cy, cw, ch, sh - sy, input, local_cur, local_sym);
    } else {
      civ_font_render_aligned(renderer, font_hud, "Screen under development", cx, cy + 60, cw, 30, 0x556677, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
    }
  }

  /* ── Layer 4-5: Settlements + Units ─────── */
  /* ── Layer 7: HUD top bar ─────────────── */
  render_hud_top(renderer, game);

  /* ── Layer 8: HUD buttons ─────────────── */
  render_hud_buttons(renderer, input);

  /* ── Layer 9: Sidebar navigation — full height left panel ─────── */
  {
    int sb_w = 220, sb_x = 0, sb_y = 0;
    /* Full-height background — top bar renders over it */
    civ_render_rect_filled_alpha(renderer, sb_x, sb_y, sb_w, win_h,
                                 0x080C18, 250);
    civ_render_line(renderer, sb_w, 0, sb_w, win_h, 0x1A2A3A);

    int cy = 44; /* below top bar */
    /* Nav title */
    civ_font_render_aligned(renderer, font_hud, "DOMINION", sb_x + 8, cy,
                            sb_w - 16, 22, CIV_COLOR_PRIMARY,
                            CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
    cy += 28;

    /* Role-based dynamic nav */
    int nav_n = game->player_role.nav_count, nav_h = 26;
    for (int i = 0; i < nav_n && i < 14; i++) {
      civ_nav_screen_t ns = (civ_nav_screen_t)game->player_role.nav_screens[i];
      if (ns < 0) continue;
      civ_screen_t m = SCR_MAP;
      switch (ns) { case CIV_NAV_DASHBOARD:m=SCR_DASHBOARD;break; case CIV_NAV_WORK:m=SCR_WORK;break; case CIV_NAV_FINANCE:m=SCR_FINANCE;break; case CIV_NAV_HOUSING:m=SCR_HOUSING;break; case CIV_NAV_EDUCATION:m=SCR_EDUCATION;break; case CIV_NAV_NETWORK:m=SCR_NETWORK;break; case CIV_NAV_POLITICS:m=SCR_POLITICS;break; case CIV_NAV_HEALTH:m=SCR_HEALTH;break; case CIV_NAV_CONSTITUTION:m=SCR_CONSTITUTION;break; case CIV_NAV_NEWS:m=SCR_NEWS;break; case CIV_NAV_ECONOMY:m=SCR_ECONOMY;break; case CIV_NAV_GOVERNANCE:m=SCR_GOVERNANCE;break; case CIV_NAV_DIPLOMACY:m=SCR_DIPLOMACY;break; case CIV_NAV_MAP:m=SCR_MAP;break; default:m=SCR_DASHBOARD;break; }
      bool active = (current_screen == m);
      bool hov = civ_input_is_mouse_over(input, sb_x+4, cy, sb_w-8, nav_h);
      uint32_t bg = active?0x003A5A:(hov?0x162033:0x080C18);
      civ_render_rect_filled_alpha(renderer, sb_x+4, cy, sb_w-8, nav_h, bg, active?240:(hov?200:0));
      if (active) civ_render_rect_filled(renderer, sb_x+4, cy, 3, nav_h, CIV_COLOR_PRIMARY);
      const char *label = civ_nav_screen_label(ns);
      civ_font_render_aligned(renderer, font_hud, label, sb_x+18, cy, sb_w-26, nav_h, active?0xFFFFFF:0xAABBCC, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
      if (input->mouse_left_pressed && hov) current_screen = m;
      cy += nav_h+3;
    }

    /* Player info at bottom of sidebar */
    cy = win_h - 110;
    civ_render_line(renderer, sb_x + 8, cy, sb_w - 8, cy, 0x1A2A3A);
    cy += 8;
    if (game->current_profile) {
      civ_font_render_aligned(renderer, font_hud, game->current_profile->name,
                              sb_x + 8, cy, sb_w - 16, 18, 0x8899AA,
                              CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
    }
    cy += 22;
    if (game->player_character) {
      civ_character_t *pc = (civ_character_t *)game->player_character;
      char bg_buf[48];
      snprintf(bg_buf, sizeof(bg_buf), "%s", civ_background_name(pc->background));
      civ_font_render_aligned(renderer, font_hud, bg_buf,
                              sb_x + 8, cy, sb_w - 16, 16, 0x667788,
                              CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
    }
  }

  /* ── Layer 12: Time controls ─────────── */
  render_time_controls(renderer, game, input);

  /* ── Layer 13: Notifications ─────────── */
  /* TODO: toast notification system */

  /* ── Layer 14: Debug overlay ─────────── */
  if (debug.enabled)
    civ_debug_overlay_render(&debug, renderer, win_w);

  /* ESC key closes all panels */
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
