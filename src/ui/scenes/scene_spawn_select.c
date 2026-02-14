/**
 * @file scene_spawn_select.c
 * @brief Scene for selecting initial spawn community
 */

#include "../../../include/core/game.h"
#include "../../../include/engine/font.h"
#include "../../../include/engine/renderer.h"
#include "../../../include/ui/scene.h"
#include "../../../include/ui/ui_common.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>

static civ_font_t *font_temp = NULL;
static civ_render_map_context_t *map_ctx = NULL;
static int last_win_h = 0;

static void init(void) {
  font_temp = civ_font_load_system("Segoe UI", 32);
  printf("[SCENE_SPAWN_SELECT] Initializing...\n");
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  if (!map_ctx)
    return;

  /* Handle Drag Panning (Right Mouse) */
  if (input->mouse_right_down) {
    float move_speed = 1.0f / (map_ctx->zoom * 4.0f);
    map_ctx->view_x -= (float)input->delta_x * move_speed;
    map_ctx->view_y -= (float)input->delta_y * move_speed;
  }

  /* Handle Zoom (Scroll) */
  if (fabsf(input->scroll_delta) > 0.1f) {
    float zoom_speed = 1.15f;
    if (input->scroll_delta > 0)
      map_ctx->zoom *= zoom_speed;
    else
      map_ctx->zoom /= zoom_speed;

    if (map_ctx->zoom < 0.005f)
      map_ctx->zoom = 0.005f;
    if (map_ctx->zoom > 2.0f)
      map_ctx->zoom = 2.0f;
  }

  /* Transition to GAME on click, but not if dragging or clicking UI */
  if (input->mouse_left_pressed && !input->mouse_right_down) {
    /* Block clicks on the bottom command panel (160h) */
    if (input->mouse_y < (last_win_h - 160)) {
      printf("Spawn selection complete at [%d, %d]. Transitioning to GAME.\n",
             input->mouse_x, input->mouse_y);
      /* MASTERPIECE 2.0: Autosave before entering the world */
      if (game->current_profile) {
        char save_path[256];
        snprintf(save_path, sizeof(save_path), "%s.civ",
                 game->current_profile->name);
        civ_game_save(game, save_path);
        printf("[SCENE_SPAWN_SELECT] Genesis autosave created: %s\n",
               save_path);
      }
      civ_scene_manager_switch(SCENE_GAME);
    }
  }
}

static void render(SDL_Renderer *renderer, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  if (!game || !game->world_map) {
    civ_render_rect_filled(renderer, 0, 0, win_w, win_h, 0x010204);
    return;
  }

  /* 0. Map Context Init */
  if (!map_ctx) {
    map_ctx = civ_render_map_context_create(renderer, win_w, win_h,
                                            game->world_map->width,
                                            game->world_map->height);
    if (map_ctx) {
      map_ctx->view_x = (float)game->world_map->width / 2.0f;
      map_ctx->view_y = (float)game->world_map->height / 2.0f;
      map_ctx->zoom = 0.5f;
    }
  }

  /* 1. Map Render (Background for selection) */
  if (map_ctx) {
    civ_render_map(renderer, map_ctx, game->world_map, win_w, win_h);
  } else {
    civ_render_rect_filled(renderer, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);
  }

  /* 2. Atmospheric Pulse Overlay */
  float pulse = (sinf(SDL_GetTicks() * 0.001f) * 0.5f + 0.5f);
  civ_render_rect_filled_alpha(renderer, 0, win_h - 300, win_w, 300,
                               CIV_COLOR_GLOW, (uint8_t)(pulse * 40));

  /* 3. Bottom Command Panel */
  int panel_h = 160;
  civ_render_rect_filled_alpha(renderer, 0, win_h - panel_h, win_w, panel_h,
                               CIV_COLOR_BG_MEDIUM, 220);
  civ_render_line(renderer, 0, win_h - panel_h, win_w, win_h - panel_h,
                  0x1A2A3A);

  /* 4. Status Guidance */
  last_win_h = win_h;

  /* Top Navigational HUD (Masterpiece 2.0) */
  civ_render_rect_filled_alpha(renderer, 0, 0, win_w, 40, 0x050A14, 220);
  civ_render_line(renderer, 0, 40, win_w, 40, 0x1A2A3A);

  if (map_ctx && font_temp) {
    float lat = 90.0f - (map_ctx->view_y / (float)map_ctx->map_height) * 180.0f;
    float lon = (map_ctx->view_x / (float)map_ctx->map_width) * 360.0f - 180.0f;
    while (lon < -180.0f)
      lon += 360.0f;
    while (lon > 180.0f)
      lon -= 360.0f;

    char buf[128];
    sprintf(buf, "ORBITAL RECONNAISSANCE: %.2f%c | %.2f%c", fabsf(lat),
            lat >= 0 ? 'N' : 'S', fabsf(lon), lon >= 0 ? 'E' : 'W');

    civ_font_render_aligned(renderer, font_temp, buf, 20, 0, win_w - 40, 40,
                            CIV_COLOR_PRIMARY, CIV_ALIGN_LEFT,
                            CIV_VALIGN_MIDDLE);
  }

  if (font_temp) {
    civ_font_render_aligned(
        renderer, font_temp,
        "PLANETARY ANALYSIS COMPLETE. SELECT INITIAL COLONY DROP SITE.", 40,
        win_h - panel_h + 40, win_w - 80, 40, CIV_COLOR_PRIMARY,
        CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

    uint8_t alpha =
        (uint8_t)((sinf(SDL_GetTicks() * 0.004f) * 0.5f + 0.5f) * 150 + 100);

    civ_font_render_aligned(renderer, font_temp,
                            "(CLICK ANYWHERE TO START EVOLUTION)", 0,
                            win_h - panel_h + 85, win_w, 30, CIV_COLOR_TEXT_DIM,
                            CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  }
}

static void destroy(void) {
  if (font_temp)
    civ_font_destroy(font_temp);
  font_temp = NULL;

  if (map_ctx) {
    civ_render_map_context_destroy(map_ctx);
    map_ctx = NULL;
  }
}

civ_scene_t scene_spawn_select = {.init = init,
                                  .update = update,
                                  .render = render,
                                  .destroy = destroy,
                                  .next_scene_id = -1};
