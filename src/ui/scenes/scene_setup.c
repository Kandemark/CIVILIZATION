/**
 * @file scene_setup.c
 * @brief World generation confirmation — map is already loaded, just preview
 *
 * The Earth map (or procedural atlas) was loaded during civ_game_initialize().
 * This scene shows a brief preview and lets the player confirm before proceeding.
 */
#include "../../../include/core/world/map_generator.h"
#include "../../../include/display/camera.h"
#include "../../../include/engine/font.h"
#include "../../../include/engine/renderer.h"
#include "../../../include/ui/scene.h"
#include "../../../include/ui/ui_common.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>

static civ_font_t                *font_title = NULL;
static civ_font_t                *font_body = NULL;
static civ_render_map_context_t  *map_ctx = NULL;
static civ_camera_t               cam;
static int                        last_win_w, last_win_h;
static float                      view_timer = 0.0f;

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 36);
  font_body  = civ_font_load_system("Segoe UI", 22);
  view_timer = 0.0f;
  map_ctx = NULL;
  printf("[SETUP] World preview ready.\n");
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  view_timer += 0.016f;

  /* Camera init */
  if (cam.map_width == 0 && game->world_map) {
    civ_camera_init(&cam, game->world_map->width, game->world_map->height);
  }
  civ_camera_update(&cam, 0.016f);

  /* Slow auto-pan */
  cam.target_x = cam.x + sinf(view_timer * 0.3f) * 0.02f;

  /* Manual pan */
  if (input->mouse_right_down) {
    float ms = 1.0f / (cam.zoom * 4.0f);
    cam.target_x -= (float)input->delta_x * ms;
    cam.target_y -= (float)input->delta_y * ms;
  }

  /* Zoom */
  if (fabsf(input->scroll_delta) > 0.1f) {
    float factor = input->scroll_delta > 0 ? 1.15f : 1.0f / 1.15f;
    civ_camera_zoom(&cam, factor, cam.x, cam.y);
  }

  /* Confirm: click or enter */
  if (input->mouse_left_pressed || input->enter_pressed) {
    printf("[SETUP] Confirmed. Proceeding to life origin.\n");
    civ_scene_manager_switch(SCENE_LIFE_ORIGIN);
  }

  if (input->esc_pressed) {
    civ_scene_manager_switch(SCENE_MAIN_MENU);
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

  /* Map context */
  if (!map_ctx) {
    map_ctx = civ_render_map_context_create(r, win_w, win_h,
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
    civ_render_map(r, map_ctx, game->world_map, win_w, win_h);
  }

  /* Bottom panel */
  int panel_h = 130;
  civ_render_rect_filled_alpha(r, 0, win_h - panel_h, win_w, panel_h,
                               CIV_COLOR_BG_MEDIUM, 220);
  civ_render_line(r, 0, win_h - panel_h, win_w, win_h - panel_h, 0x1A2A3A);

  /* Top bar */
  civ_render_rect_filled_alpha(r, 0, 0, win_w, 40, 0x050A14, 220);
  civ_render_line(r, 0, 40, win_w, 40, 0x1A2A3A);

  if (font_title) {
    float lat = 90.0f - (cam.y / (float)cam.map_height) * 180.0f;
    float lon = (cam.x / (float)cam.map_width) * 360.0f - 180.0f;
    while (lon < -180.0f) lon += 360.0f; while (lon > 180.0f) lon -= 360.0f;

    char buf[128];
    sprintf(buf, "PLANETARY SURVEY: %.2f%c | %.2f%c",
            fabsf(lat), lat >= 0 ? 'N' : 'S', fabsf(lon), lon >= 0 ? 'E' : 'W');
    civ_font_render_aligned(r, font_title, buf, 20, 0, win_w - 40, 40,
                            CIV_COLOR_PRIMARY, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

    /* Instructions */
    float pulse = (sinf((float)SDL_GetTicks() * 0.004f) * 0.5f + 0.5f);
    uint32_t prompt_c = pulse > 0.6f ? CIV_COLOR_PRIMARY : CIV_COLOR_TEXT;
    civ_font_render_aligned(r, font_title, "CONFIRM TERRESTRIAL INSERTION",
                            0, win_h - panel_h + 30, win_w, 40,
                            prompt_c, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
    if (font_body)
      civ_font_render_aligned(r, font_body,
                              "(CLICK OR PRESS ENTER TO CONFIRM · ESC TO RETURN)",
                              0, win_h - panel_h + 75, win_w, 30,
                              CIV_COLOR_TEXT_DIM, CIV_ALIGN_CENTER,
                              CIV_VALIGN_MIDDLE);
  }
}

static void destroy(void) {
  if (font_title) civ_font_destroy(font_title), font_title = NULL;
  if (font_body)  civ_font_destroy(font_body),  font_body = NULL;
  if (map_ctx)    civ_render_map_context_destroy(map_ctx), map_ctx = NULL;
}

civ_scene_t scene_setup = {.init = init, .update = update, .render = render,
                           .destroy = destroy, .next_scene_id = -1};
