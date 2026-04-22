/**
 * @file scene_identity.c
 * @brief Identity setup — age, nationality, birthplace, ID card
 *
 * Widget-based with +/- buttons for age, real nation lookup from
 * nations_data for nationality, and proper currency matching by ISO.
 */
#include "core/character.h"
#include "core/economy/financial_markets.h"
#include "core/profile.h"
#include "core/world/nations_data.h"
#include "display/theme.h"
#include "engine/font.h"
#include "engine/renderer.h"
#include "ui/scene.h"
#include "ui/ui_common.h"
#include "ui/widget/button.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

static civ_font_t          *font_title = NULL;
static civ_font_t          *font_body  = NULL;
static civ_font_t          *font_small = NULL;
static civ_widget_button_t *btn_confirm = NULL;
static civ_widget_button_t *btn_age_up = NULL;
static civ_widget_button_t *btn_age_dn = NULL;

static char  nationality[64]   = "";
static int   age               = 25;
static char  birthplace[64]    = "";
static int   step              = 0;
static char  nation_search[32] = "";

/* Look up all nation names from nations_data for search */
static char  all_nations[256][64];
static int   all_nation_count = 0;

static void build_nation_list(civ_game_t *game) {
  if (all_nation_count > 0 || !game || !game->nations_data) return;
  for (uint32_t i = 0; i < game->nations_data->count && all_nation_count < 256; i++) {
    if (game->nations_data->nations[i].name[0])
      snprintf(all_nations[all_nation_count++], 64, "%s",
               game->nations_data->nations[i].name);
  }
}

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 24);
  font_body  = civ_font_load_system("Segoe UI", 16);
  font_small = civ_font_load_system("Segoe UI", 13);
  btn_confirm = civ_widget_button_create("id_confirm", 0,0,220,42, "CONFIRM IDENTITY");
  btn_age_up  = civ_widget_button_create("age_up",  0,0,50,38, "+");
  btn_age_dn  = civ_widget_button_create("age_dn",  0,0,50,38, "-");
  nationality[0] = '\0'; age = 25; birthplace[0] = '\0'; step = 0;
  nation_search[0] = '\0';
  all_nation_count = 0;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  float dt = 1.0f/60.0f;

  build_nation_list(game);

  if (input->esc_pressed) {
    if (step > 0) { step--; return; }
    civ_scene_manager_switch(SCENE_LIFE_ORIGIN); return;
  }

  /* Step 0: Age */
  if (step == 0) {
    int cx = input->win_w / 2;
    btn_age_up->base.x = (float)(cx - 30);
    btn_age_up->base.y = (float)(input->win_h/2 - 90);
    btn_age_dn->base.x = (float)(cx - 30);
    btn_age_dn->base.y = (float)(input->win_h/2 + 70);

    civ_widget_button_update(btn_age_up, input, dt);
    civ_widget_button_update(btn_age_dn, input, dt);

    if (civ_widget_button_was_clicked(btn_age_up)) { age++; if (age > 110) age = 110; }
    if (civ_widget_button_was_clicked(btn_age_dn)) { age--; if (age < 16) age = 16; }
    if (input->enter_pressed) step = 1;
  }

  /* Step 1: Nationality search */
  if (step == 1) {
    if (input->text_input[0]) {
      size_t len = strlen(nation_search);
      size_t nl = strlen(input->text_input);
      if (len + nl < 30) strcat(nation_search, input->text_input);
    }
    if (input->backspace_pressed && nation_search[0])
      nation_search[strlen(nation_search)-1] = '\0';

    if (input->enter_pressed && nationality[0]) step = 2;
  }

  /* Step 2: Birthplace */
  if (step == 2) {
    if (input->text_input[0]) {
      size_t len = strlen(birthplace);
      size_t nl = strlen(input->text_input);
      if (len + nl < 60) strcat(birthplace, input->text_input);
    }
    if (input->backspace_pressed && birthplace[0])
      birthplace[strlen(birthplace)-1] = '\0';
    if (input->enter_pressed && birthplace[0]) step = 3;
  }

  /* Step 3: Review — confirm */
  if (step == 3) {
    /* Position button over the ID card before update */
    int card_w = 480, card_h = 320;
    int cx = (input->win_w - card_w) / 2;
    btn_confirm->base.x = (float)(cx + card_w/2 - 110);
    btn_confirm->base.y = (float)(input->win_h/2 + card_h/2 - 50);
    btn_confirm->base.w = 220; btn_confirm->base.h = 38;
    civ_widget_button_update(btn_confirm, input, dt);

    if (civ_widget_button_was_clicked(btn_confirm)) {
      if (game->player_character) {
        civ_character_t *pc = (civ_character_t *)game->player_character;
        snprintf(pc->birthplace, sizeof(pc->birthplace), "%s", birthplace);
      }
      if (!game->player_character) {
        civ_character_t *pc = civ_character_create(
            game->current_profile ? game->current_profile->name : "Citizen");
        civ_character_apply_background(pc, CIV_BG_BUREAUCRATIC);
        game->player_character = pc;
      }

      /* Set nationality and fund wallet */
      if (nationality[0]) {
        civ_role_set(&game->player_role, &civ_role_private_citizen, nationality);
        if (game->market) {
          civ_market_engine_t *mkt = game->market;
          /* Find currency by matching nation ISO */
          const civ_nation_data_t *nd = game->nations_data
              ? civ_nations_data_get_by_name(game->nations_data, nationality) : NULL;
          const char *iso = nd ? nd->iso_a2 : NULL;

          bool found = false;
          if (iso) {
            for (int ci = 0; ci < mkt->currency_count; ci++) {
              /* Try ISO code match first */
              const char *cname = mkt->currencies[ci].name;
              if ((nd && strcasestr(cname, nd->iso_a3)) ||
                  strcasestr(cname, nationality)) {
                float amt = 500.0f * mkt->currencies[ci].current_rate;
                civ_wallet_add(&game->wallet, mkt->currencies[ci].iso, amt);
                printf("[IDENTITY] Starting: %.0f %s\n", amt, mkt->currencies[ci].iso);
                found = true; break;
              }
            }
          }
          if (!found) civ_wallet_add(&game->wallet, "USD", 500.0f);
        }
        printf("[IDENTITY] Citizen of %s\n", nationality);
      }
      civ_scene_manager_switch(SCENE_GAME);
    }
  }
}

/* ── Render helpers ────────────────────────────────────────────── */

static void render_age(SDL_Renderer *r, int win_w, int win_h) {
  int cx = win_w/2, cy = win_h/2;
  civ_font_render_aligned(r, font_title, "HOW OLD ARE YOU?", 0, 80,
      win_w, 40, CIV_COLOR_PRIMARY, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

  /* Age number, large */
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", age);
  civ_font_render_aligned(r, font_title, buf, cx-120, cy-50, 240, 60,
      g_theme.text_primary, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

  /* +/- buttons */
  civ_widget_button_render(btn_age_up, r, font_body);
  civ_widget_button_render(btn_age_dn, r, font_body);

  civ_font_render_aligned(r, font_small, "Press Enter to continue",
      0, win_h-50, win_w, 18, g_theme.text_dim, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
}

static void render_nationality(SDL_Renderer *r, int win_w, int win_h,
                                civ_input_state_t *input) {
  civ_font_render_aligned(r, font_title, "WHAT IS YOUR NATIONALITY?", 0, 60,
      win_w, 40, CIV_COLOR_PRIMARY, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

  /* Search box */
  int bx_w = 420, bx_h = 38, bx_x = (win_w-bx_w)/2, bx_y = 140;
  civ_render_rect_filled(r, bx_x, bx_y, bx_w, bx_h, g_theme.panel_bg);
  civ_render_rect_outline(r, bx_x, bx_y, bx_w, bx_h,
      nationality[0] ? g_theme.primary : g_theme.hud_border, 1);
  const char *sd = nation_search[0] ? nation_search : "Type country name...";
  civ_font_render_aligned(r, font_body, sd, bx_x+12, bx_y,
      bx_w-24, bx_h, nationality[0] ? g_theme.text_primary : g_theme.text_dim,
      CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

  /* Selected */
  if (nationality[0]) {
    civ_render_rect_filled_alpha(r, bx_x, bx_y+46, bx_w, 30,
        g_theme.primary_dark, 200);
    civ_font_render_aligned(r, font_body, nationality, bx_x+12, bx_y+46,
        bx_w-24, 30, g_theme.warning, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  }

  /* Search results from real nation list */
  if (nation_search[0]) {
    int matches = 0, my = 200;
    for (int i = 0; i < all_nation_count && matches < 8; i++) {
      if (strcasestr(all_nations[i], nation_search)) {
        bool hov = civ_input_is_mouse_over(input, bx_x, my, bx_w, 26);
        civ_render_rect_filled_alpha(r, bx_x, my, bx_w, 26,
            hov ? g_theme.bg_light : g_theme.panel_bg, 200);
        civ_font_render_aligned(r, font_small, all_nations[i], bx_x+12, my,
            bx_w-24, 26, hov ? g_theme.text_primary : g_theme.hud_text,
            CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
        if (hov && input->mouse_left_pressed) {
          snprintf(nationality, sizeof(nationality), "%s", all_nations[i]);
          nation_search[0] = '\0';
        }
        my += 28; matches++;
      }
    }
  }

  civ_font_render_aligned(r, font_small, "Enter to confirm selection",
      0, win_h-40, win_w, 18, g_theme.text_dim, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
}

static void render_birthplace(SDL_Renderer *r, int win_w, int win_h) {
  civ_font_render_aligned(r, font_title, "WHERE WERE YOU BORN?", 0, 60,
      win_w, 40, CIV_COLOR_PRIMARY, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

  char buf[128];
  snprintf(buf, sizeof(buf), "Citizen of %s", nationality);
  civ_font_render_aligned(r, font_body, buf, 0, 105, win_w, 24,
      g_theme.warning, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

  int bx_w = 420, bx_h = 38, bx_x = (win_w-bx_w)/2, bx_y = 170;
  civ_render_rect_filled(r, bx_x, bx_y, bx_w, bx_h, g_theme.panel_bg);
  civ_render_rect_outline(r, bx_x, bx_y, bx_w, bx_h,
      birthplace[0] ? g_theme.primary : g_theme.hud_border, 1);
  const char *bd = birthplace[0] ? birthplace : "City or state/province...";
  civ_font_render_aligned(r, font_body, bd, bx_x+12, bx_y,
      bx_w-24, bx_h, birthplace[0] ? g_theme.text_primary : g_theme.text_dim,
      CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

  civ_font_render_aligned(r, font_small, "Type your city or state of birth",
      0, win_h-40, win_w, 18, g_theme.text_dim, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
}

static void render_review(SDL_Renderer *r, int win_w, int win_h,
                           civ_game_t *game) {
  int card_w = 480, card_h = 320;
  int cx = (win_w-card_w)/2, cy = (win_h-card_h)/2;

  civ_render_rect_filled_alpha(r, cx, cy, card_w, card_h, g_theme.panel_bg, 245);
  civ_render_rect_outline(r, cx, cy, card_w, card_h, g_theme.warning, 2);

  civ_font_render_aligned(r, font_title, "NATIONAL IDENTITY CARD",
      cx+20, cy+16, card_w-40, 28, g_theme.warning,
      CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  civ_render_line(r, cx+20, cy+50, cx+card_w-20, cy+50, g_theme.hud_border);

  int ly = cy + 62;
  char buf[128];
  const char *name = game->current_profile ? game->current_profile->name : "Unknown";

  snprintf(buf, sizeof(buf), "NAME:           %s", name);
  civ_font_render_aligned(r, font_body, buf, cx+30, ly, card_w-60, 22,
      g_theme.text_primary, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); ly += 26;
  snprintf(buf, sizeof(buf), "AGE:              %d", age);
  civ_font_render_aligned(r, font_body, buf, cx+30, ly, card_w-60, 22,
      g_theme.text_secondary, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); ly += 26;
  snprintf(buf, sizeof(buf), "NATIONALITY:     %s", nationality);
  civ_font_render_aligned(r, font_body, buf, cx+30, ly, card_w-60, 22,
      g_theme.warning, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); ly += 26;
  snprintf(buf, sizeof(buf), "BIRTHPLACE:      %s", birthplace);
  civ_font_render_aligned(r, font_body, buf, cx+30, ly, card_w-60, 22,
      g_theme.text_secondary, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); ly += 26;

  if (game->player_character) {
    civ_character_t *pc = (civ_character_t *)game->player_character;
    snprintf(buf, sizeof(buf), "BACKGROUND:      %s", civ_background_name(pc->background));
    civ_font_render_aligned(r, font_body, buf, cx+30, ly, card_w-60, 22,
        g_theme.hud_text, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  }

  /* Confirm button (positioned in update) */
  civ_widget_button_render(btn_confirm, r, font_body);
}

static bool text_started = false;
static SDL_Window *sdl_win = NULL;

static void render(SDL_Renderer *r, int win_w, int win_h, civ_game_t *game,
                   civ_input_state_t *input) {
  if (!text_started) {
    sdl_win = SDL_GetRenderWindow(r);
    if (sdl_win) { SDL_StartTextInput(sdl_win); text_started = true; }
  }

  civ_render_rect_filled(r, 0, 0, win_w, win_h, g_theme.bg_deep);

  /* Progress bar */
  int pw = 400, ph = 3, px = (win_w-pw)/2, py = 30;
  civ_render_rect_filled(r, px, py, pw, ph, g_theme.hud_border);
  civ_render_rect_filled(r, px, py, (pw*(step+1))/4, ph, g_theme.primary);
  char sl[32];
  snprintf(sl, sizeof(sl), "Step %d of 4", step+1);
  civ_font_render_aligned(r, font_small, sl, 0, 10, win_w, 18,
      g_theme.text_dim, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

  switch (step) {
  case 0: render_age(r, win_w, win_h); break;
  case 1: render_nationality(r, win_w, win_h, input); break;
  case 2: render_birthplace(r, win_w, win_h); break;
  case 3: render_review(r, win_w, win_h, game); break;
  }
}

static void destroy(void) {
  if (sdl_win) { SDL_StopTextInput(sdl_win); sdl_win = NULL; }
  text_started = false;
  if (font_title)  civ_font_destroy(font_title), font_title = NULL;
  if (font_body)   civ_font_destroy(font_body),  font_body = NULL;
  if (font_small)  civ_font_destroy(font_small), font_small = NULL;
  if (btn_confirm) civ_widget_button_destroy(btn_confirm), btn_confirm = NULL;
  if (btn_age_up)  civ_widget_button_destroy(btn_age_up),  btn_age_up = NULL;
  if (btn_age_dn)  civ_widget_button_destroy(btn_age_dn),  btn_age_dn = NULL;
}

civ_scene_t scene_identity = {.init = init, .update = update,
    .render = render, .destroy = destroy, .next_scene_id = -1};
