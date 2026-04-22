#include "core/world/map_generator.h"
#include "core/world/nation.h"
#include "display/camera.h"
#include "display/theme.h"
#include "engine/font.h"
#include "engine/renderer.h"
#include "ui/scene.h"
#include "ui/ui_common.h"
#include "ui/widget/button.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>

static civ_font_t                *font_title = NULL;
static civ_font_t                *font_body = NULL;
static civ_render_map_context_t  *map_ctx = NULL;
static civ_camera_t               cam;
static int                        last_win_w, last_win_h;
static float                      view_timer = 0.0f;
static civ_widget_button_t       *btn_confirm = NULL;
static civ_widget_button_t       *btn_back = NULL;

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 22);
  font_body  = civ_font_load_system("Segoe UI", 16);
  view_timer = 0.0f;
  map_ctx = NULL;
  printf("[SETUP] World preview ready.\n");
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  view_timer += 0.016f;

  if (cam.map_width == 0 && game->world_map) {
    civ_camera_init(&cam, game->world_map->width, game->world_map->height);
  }
  civ_camera_update(&cam, 0.016f);

  /* Lazy-create buttons */
  if (!btn_confirm) {
    btn_confirm = civ_widget_button_create("confirm", 0, 0, 200, 38, "CONFIRM");
    btn_back    = civ_widget_button_create("back", 0, 0, 160, 38, "BACK");
  }

  int bw = last_win_w > 0 ? last_win_w : 1280;
  int bh = last_win_h > 0 ? last_win_h : 720;
  btn_confirm->base.x = (float)(bw / 2 - 100);
  btn_confirm->base.y = (float)(bh - 90);
  btn_back->base.x    = (float)(bw / 2 - 280);
  btn_back->base.y    = (float)(bh - 90);

  civ_widget_button_update(btn_confirm, input, 0.016f);
  civ_widget_button_update(btn_back, input, 0.016f);

  if (civ_widget_button_was_clicked(btn_confirm) || input->enter_pressed) {
    printf("[SETUP] Confirmed.\n");
    civ_scene_manager_switch(SCENE_LIFE_ORIGIN);
  }
  if (civ_widget_button_was_clicked(btn_back) || input->esc_pressed) {
    civ_scene_manager_switch(SCENE_MAIN_MENU);
  }

  cam.target_x = cam.x + sinf(view_timer * 0.3f) * 0.02f;
  if (input->mouse_right_down) {
    float ms = 1.0f / (cam.zoom * 4.0f);
    cam.target_x -= (float)input->delta_x * ms;
    cam.target_y -= (float)input->delta_y * ms;
  }
  if (fabsf(input->scroll_delta) > 0.1f) {
    float factor = input->scroll_delta > 0 ? 1.15f : 1.0f / 1.15f;
    civ_camera_zoom(&cam, factor, cam.x, cam.y);
  }
}

static void render(SDL_Renderer *r, int win_w, int win_h, civ_game_t *game,
                   civ_input_state_t *input) {
  (void)input;
  last_win_w = win_w; last_win_h = win_h;

  if (!game->world_map) {
    civ_render_rect_filled(r, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);
    if (font_title)
      civ_font_render_aligned(r, font_title, "LOADING WORLD DATA...", 0, 0,
                              win_w, win_h, CIV_COLOR_PRIMARY, CIV_ALIGN_CENTER,
                              CIV_VALIGN_MIDDLE);
    return;
  }

  if (!map_ctx) {
    map_ctx = civ_render_map_context_create(r, win_w, win_h,
        game->world_map->width, game->world_map->height);
    if (map_ctx) {
      map_ctx->view_x = cam.x; map_ctx->view_y = cam.y;
      map_ctx->zoom = cam.zoom;
      civ_render_map_build_lods(map_ctx, game->world_map, r);
    }
  }
  if (map_ctx) {
    map_ctx->view_x = cam.x; map_ctx->view_y = cam.y;
    map_ctx->zoom = cam.zoom;
    civ_render_map(r, map_ctx, game->world_map, win_w, win_h,
                   CIV_MAP_VIEW_POLITICAL, NULL);
  }

  /* Bottom panel */
  int panel_h = 130;
  civ_render_rect_filled_alpha(r, 0, win_h - panel_h, win_w, panel_h,
                               g_theme.panel_bg, 230);
  civ_render_line(r, 0, win_h - panel_h, win_w, win_h - panel_h,
                  g_theme.hud_border);

  /* Top bar */
  civ_render_rect_filled_alpha(r, 0, 0, win_w, 36, g_theme.hud_bg, 220);
  civ_render_line(r, 0, 36, win_w, 36, g_theme.hud_border);

  if (font_title) {
    float lat = 90.0f - (cam.y / (float)cam.map_height) * 180.0f;
    float lon = (cam.x / (float)cam.map_width) * 360.0f - 180.0f;
    while (lon < -180.0f) lon += 360.0f; while (lon > 180.0f) lon -= 360.0f;

    char buf[256];
    int nations = 0;
    if (game->nation_manager)
      nations = ((civ_nation_manager_t *)game->nation_manager)->count;
    snprintf(buf, sizeof(buf),
        "WORLD SURVEY  |  %.0f%c %.0f%c  |  %d Nations  |  %dx%d",
        fabsf(lat), lat >= 0 ? 'N' : 'S',
        fabsf(lon), lon >= 0 ? 'E' : 'W',
        nations, game->world_map->width, game->world_map->height);
    civ_font_render_aligned(r, font_title, buf, 20, 0, win_w - 40, 36,
                            CIV_COLOR_PRIMARY, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

    if (font_body) {
      civ_font_render_aligned(r, font_body,
          "Explore Earth with 200+ nations. Zoom, pan, and select your path.",
          20, win_h - panel_h + 14, win_w - 40, 28,
          g_theme.text_secondary, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
    }
  }

  /* Buttons */
  if (btn_confirm) civ_widget_button_render(btn_confirm, r, font_body);
  if (btn_back)    civ_widget_button_render(btn_back, r, font_body);
}

static void destroy(void) {
  if (font_title)  civ_font_destroy(font_title), font_title = NULL;
  if (font_body)   civ_font_destroy(font_body),  font_body = NULL;
  if (btn_confirm) civ_widget_button_destroy(btn_confirm), btn_confirm = NULL;
  if (btn_back)    civ_widget_button_destroy(btn_back),    btn_back = NULL;
  if (map_ctx)     civ_render_map_context_destroy(map_ctx), map_ctx = NULL;
}

civ_scene_t scene_setup = {.init = init, .update = update, .render = render,
                           .destroy = destroy, .next_scene_id = -1};
