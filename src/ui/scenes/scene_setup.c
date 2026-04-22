#include "core/world/map_generator.h"
#include "core/world/nation.h"
#include "display/camera.h"
#include "display/theme.h"
#include "ui/ui_common.h"
#include "engine/font.h"
#include "engine/renderer.h"
#include "ui/nuklear_ui.h"
#include "ui/scene.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>

static civ_render_map_context_t *map_ctx = NULL;
static civ_camera_t              cam;
static int                       last_win_w, last_win_h;
static float                     view_timer = 0.0f;

static void init(void) {
  view_timer = 0.0f; map_ctx = NULL;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  view_timer += 0.016f;
  if (cam.map_width == 0 && game->world_map)
    civ_camera_init(&cam, game->world_map->width, game->world_map->height);
  civ_camera_update(&cam, 0.016f);
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
  if (input->esc_pressed)
    civ_scene_manager_switch(SCENE_MAIN_MENU);
}

static void render(SDL_Renderer *r, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  last_win_w = win_w; last_win_h = win_h;
  if (!game->world_map) {
    civ_render_rect_filled(r, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);
    return;
  }
  if (!map_ctx) {
    map_ctx = civ_render_map_context_create(r, win_w, win_h,
        game->world_map->width, game->world_map->height);
    if (map_ctx) {
      map_ctx->view_x = cam.x; map_ctx->view_y = cam.y; map_ctx->zoom = cam.zoom;
      civ_render_map_build_lods(map_ctx, game->world_map, r);
    }
  }
  if (map_ctx) {
    map_ctx->view_x = cam.x; map_ctx->view_y = cam.y;
    map_ctx->zoom = cam.zoom;
    civ_render_map(r, map_ctx, game->world_map, win_w, win_h,
                   CIV_MAP_VIEW_POLITICAL, NULL);
  }

  /* Nuklear overlay panel */
  struct nk_context *nk = g_nk_ctx;
  if (!nk) return;

  if (nk_begin(nk, "World Survey", nk_rect(0, (float)win_h - 130, (float)win_w, 130),
               NK_WINDOW_NO_SCROLLBAR)) {
    int nations = game->nation_manager
        ? ((civ_nation_manager_t *)game->nation_manager)->count : 0;
    char buf[128];
    float lat = 90.0f - (cam.y / (float)cam.map_height) * 180.0f;
    float lon = (cam.x / (float)cam.map_width) * 360.0f - 180.0f;
    while (lon < -180.0f) lon += 360.0f; while (lon > 180.0f) lon -= 360.0f;
    snprintf(buf, sizeof(buf), "%.0f%c %.0f%c  |  %d Nations  |  %dx%d",
        fabsf(lat), lat >= 0 ? 'N' : 'S', fabsf(lon), lon >= 0 ? 'E' : 'W',
        nations, game->world_map->width, game->world_map->height);
    nk_layout_row_dynamic(nk, 22, 1);
    nk_label(nk, buf, NK_TEXT_CENTERED);
    nk_layout_row_dynamic(nk, 18, 1);
    nk_label(nk, "Explore Earth with 200+ nations. Zoom and pan.", NK_TEXT_CENTERED);
    nk_layout_row_dynamic(nk, 36, 3);
    nk_spacing(nk, 1);
    if (nk_button_label(nk, "CONFIRM"))
      civ_scene_manager_switch(SCENE_LIFE_ORIGIN);
    nk_spacing(nk, 1);
    if (nk_button_label(nk, "BACK"))
      civ_scene_manager_switch(SCENE_MAIN_MENU);
    nk_spacing(nk, 1);
  }
  nk_end(nk);
  (void)input;
}

static void destroy(void) {
  if (map_ctx) civ_render_map_context_destroy(map_ctx), map_ctx = NULL;
}

civ_scene_t scene_setup = {.init = init, .update = update, .render = render,
                           .destroy = destroy, .next_scene_id = -1};
