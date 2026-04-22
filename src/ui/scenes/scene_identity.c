/**
 * @file scene_identity.c
 * @brief Full identity setup — nationality, age, birthplace, national ID
 *
 * Replaces spawn_select. After life origin assessment, the player
 * defines their full identity: nationality, age, birthplace state.
 * A national ID card is previewed before entering the world.
 */
#include "core/character.h"
#include "core/profile.h"
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

/* Identity fields */
static char  nationality[64]   = "";
static int   age               = 25;
static char  birthplace[64]    = "";
static int   step              = 0; /* 0=age, 1=nationality, 2=birthplace, 3=review */

/* Nationality search */
static char  nation_search[32] = "";
static int   nation_list_offset = 0;

static void init(void) {
  font_title = civ_font_load_system("Segoe UI", 24);
  font_body  = civ_font_load_system("Segoe UI", 16);
  font_small = civ_font_load_system("Segoe UI", 13);
  btn_confirm = civ_widget_button_create("id_confirm", 0,0,200,42, "CONFIRM IDENTITY");
  nationality[0] = '\0'; age = 25; birthplace[0] = '\0'; step = 0;
  nation_search[0] = '\0'; nation_list_offset = 0;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  float dt = 1.0f/60.0f;
  civ_widget_button_update(btn_confirm, input, dt);

  if (input->esc_pressed && step > 0) { step--; return; }
  if (input->esc_pressed && step == 0) {
    civ_scene_manager_switch(SCENE_LIFE_ORIGIN); return;
  }

  /* Age step: arrow keys or click */
  if (step == 0) {
    if (input->mouse_left_pressed) {
      int cx = input->win_w/2, cy = input->win_h/2;
      /* Up/down arrow areas */
      if (input->mouse_x > cx-100 && input->mouse_x < cx+100) {
        if (input->mouse_y > cy-80 && input->mouse_y < cy-20) age++;
        if (input->mouse_y > cy+20 && input->mouse_y < cy+80) age--;
        if (age < 16) age = 16; if (age > 110) age = 110;
      }
    }
    if (input->enter_pressed) step = 1;
  }

  /* Nationality step: text search */
  if (step == 1) {
    if (input->text_input[0]) {
      size_t len = strlen(nation_search);
      size_t nl = strlen(input->text_input);
      if (len + nl < 30) strcat(nation_search, input->text_input);
    }
    if (input->backspace_pressed) {
      size_t len = strlen(nation_search);
      if (len > 0) nation_search[len-1] = '\0';
    }
    if (input->enter_pressed && nationality[0]) step = 2;
  }

  /* Birthplace step */
  if (step == 2) {
    if (input->text_input[0]) {
      size_t len = strlen(birthplace);
      size_t nl = strlen(input->text_input);
      if (len + nl < 60) strcat(birthplace, input->text_input);
    }
    if (input->backspace_pressed) {
      size_t len = strlen(birthplace);
      if (len > 0) birthplace[len-1] = '\0';
    }
    if (input->enter_pressed && birthplace[0]) step = 3;
  }

  /* Review step: confirm */
  if (step == 3 && civ_widget_button_was_clicked(btn_confirm)) {
    if (game->player_character) {
      civ_character_t *pc = (civ_character_t *)game->player_character;
      snprintf(pc->birthplace, sizeof(pc->birthplace), "%s", birthplace);
      if (nationality[0]) {
        /* Set birthplace coords from country centroid — approximate */
        /* For now just store the country name */
      }
      printf("[IDENTITY] %s, age %d, from %s, %s\n",
             pc->name, age, birthplace, nationality);
    }
    /* Set nationality + local currency wallet */
    if (nationality[0]) {
      civ_role_set(&game->player_role, &civ_role_private_citizen, nationality);
  if (!game->player_character) { civ_character_t *pc = civ_character_create(game->current_profile?game->current_profile->name:"Citizen"); civ_character_apply_background(pc, CIV_BG_BUREAUCRATIC); game->player_character = pc; }
      /* Give starting money in local currency */
      if (game->market) {
        civ_market_engine_t *mkt = game->market;
        /* Find currency for this nation */
        for (int ci = 0; ci < mkt->currency_count; ci++) {
          if (strcasestr(nationality, mkt->currencies[ci].iso) ||
              strcasestr(mkt->currencies[ci].name, nationality)) {
            float local_amt = 500.0f * mkt->currencies[ci].current_rate;
            civ_wallet_add(&game->wallet, mkt->currencies[ci].iso, local_amt);
            printf("[IDENTITY] Starting balance: %.0f %s\n",
                   local_amt, mkt->currencies[ci].iso);
            break;
          }
        }
        /* Fallback: add USD if no match */
        if (game->wallet.count == 0)
          civ_wallet_add(&game->wallet, "USD", 500.0f);
      }
      printf("[IDENTITY] Citizen of %s\n", nationality);
    }
    civ_scene_manager_switch(SCENE_GAME);
  }
  (void)game;
}

static void render_step_age(SDL_Renderer *r, int win_w, int win_h,
                            civ_input_state_t *input) {
  int cx = win_w/2;
  civ_font_render_aligned(r, font_title, "HOW OLD ARE YOU?", 0, 80,
                          win_w, 40, CIV_COLOR_PRIMARY, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  /* Age display */
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", age);
  civ_font_render_aligned(r, font_title, buf, cx-120, win_h/2-60, 240, 80,
                          0xFFFFFF, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  /* Up arrow */
  civ_font_render_aligned(r, font_body, "^ Increase", cx-100, win_h/2-100,
                          200, 30, 0x8899AA, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  /* Down arrow */
  civ_font_render_aligned(r, font_body, "v Decrease", cx-100, win_h/2+80,
                          200, 30, 0x8899AA, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  civ_font_render_aligned(r, font_small, "Click arrows or press Enter to continue",
                          0, win_h-60, win_w, 20, 0x556677, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  (void)input;
}

static void render_step_nationality(SDL_Renderer *r, int win_w, int win_h,
                                    civ_input_state_t *input) {
  civ_font_render_aligned(r, font_title, "WHAT IS YOUR NATIONALITY?", 0, 60,
                          win_w, 40, CIV_COLOR_PRIMARY, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  /* Search box */
  int bx_w = 400, bx_h = 40, bx_x = (win_w-bx_w)/2, bx_y = 140;
  civ_render_rect_filled(r, bx_x, bx_y, bx_w, bx_h, 0x0A1220);
  civ_render_rect_outline(r, bx_x, bx_y, bx_w, bx_h,
                          nationality[0] ? CIV_COLOR_PRIMARY : 0x1A2A3A, 1);
  const char *search_display = nation_search[0] ? nation_search : "Type country name...";
  civ_font_render_aligned(r, font_body, search_display, bx_x+12, bx_y,
                          bx_w-24, bx_h, nationality[0]?0xFFFFFF:0x556677,
                          CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

  /* Selected nationality */
  if (nationality[0]) {
    civ_render_rect_filled_alpha(r, bx_x, bx_y+50, bx_w, 30, 0x003A5A, 200);
    civ_font_render_aligned(r, font_body, nationality, bx_x+12, bx_y+50,
                            bx_w-24, 30, 0xFFCC00, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  }

  /* Quick pick: match search against known countries */
  if (nation_search[0]) {
    /* We'll match against a hardcoded list of common countries for now */
    const char *common[] = {"Russia","China","India","Brazil","Nigeria",
        "Indonesia","Pakistan","Bangladesh","Mexico","Japan","Ethiopia",
        "Egypt","Germany","Turkey","France","United Kingdom","Italy",
        "South Africa","Kenya","Canada","Australia","Argentina"};
    int matches = 0, my = 200;
    for (int i = 0; i < 22 && matches < 6; i++) {
      if (strcasestr(common[i], nation_search) || strcasestr(nation_search, common[i])) {
        bool hov = civ_input_is_mouse_over(input, bx_x, my, bx_w, 26);
        civ_render_rect_filled_alpha(r, bx_x, my, bx_w, 26,
                                     hov?0x162033:0x0A1220, 200);
        civ_font_render_aligned(r, font_small, common[i], bx_x+12, my,
                                bx_w-24, 26, hov?0xFFFFFF:0x8899AA,
                                CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
        if (hov && input->mouse_left_pressed) {
          snprintf(nationality, sizeof(nationality), "%s", common[i]);
          nation_search[0] = '\0';
        }
        my += 28; matches++;
      }
    }
  }

  civ_font_render_aligned(r, font_small, "Enter to confirm nationality",
                          0, win_h-40, win_w, 18, 0x556677, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
}

static void render_step_birthplace(SDL_Renderer *r, int win_w, int win_h,
                                   civ_input_state_t *input) {
  civ_font_render_aligned(r, font_title, "WHERE WERE YOU BORN?", 0, 60,
                          win_w, 40, CIV_COLOR_PRIMARY, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  char buf[128];
  snprintf(buf, sizeof(buf), "Citizen of %s", nationality);
  civ_font_render_aligned(r, font_body, buf, 0, 105, win_w, 24, 0xFFCC00,
                          CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

  /* Birthplace input */
  int bx_w = 400, bx_h = 40, bx_x = (win_w-bx_w)/2, bx_y = 170;
  civ_render_rect_filled(r, bx_x, bx_y, bx_w, bx_h, 0x0A1220);
  civ_render_rect_outline(r, bx_x, bx_y, bx_w, bx_h,
                          birthplace[0] ? CIV_COLOR_PRIMARY : 0x1A2A3A, 1);
  const char *bp_display = birthplace[0] ? birthplace : "City or state/province...";
  civ_font_render_aligned(r, font_body, bp_display, bx_x+12, bx_y,
                          bx_w-24, bx_h, birthplace[0]?0xFFFFFF:0x556677,
                          CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

  civ_font_render_aligned(r, font_small, "Type your city or state of birth",
                          0, win_h-40, win_w, 18, 0x556677, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  (void)input;
}

static void render_step_review(SDL_Renderer *r, int win_w, int win_h,
                               civ_game_t *game, civ_input_state_t *input) {
  /* National ID card */
  int card_w = 480, card_h = 340;
  int cx = (win_w-card_w)/2, cy = (win_h-card_h)/2;

  civ_render_rect_filled_alpha(r, cx, cy, card_w, card_h, 0x0A1220, 245);
  civ_render_rect_outline(r, cx, cy, card_w, card_h, 0xFFCC00, 2);

  civ_font_render_aligned(r, font_title, "NATIONAL IDENTITY CARD",
                          cx+20, cy+16, card_w-40, 30, 0xFFCC00,
                          CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
  civ_render_line(r, cx+20, cy+52, cx+card_w-20, cy+52, 0x2A3A3A);

  int ly = cy + 65;
  char buf[128];
  const char *name = game->current_profile ? game->current_profile->name : "Unknown";

  snprintf(buf, sizeof(buf), "NAME:           %s", name);
  civ_font_render_aligned(r, font_body, buf, cx+30, ly, card_w-60, 22,
                          0xFFFFFF, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); ly += 28;
  snprintf(buf, sizeof(buf), "AGE:              %d", age);
  civ_font_render_aligned(r, font_body, buf, cx+30, ly, card_w-60, 22,
                          0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); ly += 28;
  snprintf(buf, sizeof(buf), "NATIONALITY:     %s", nationality);
  civ_font_render_aligned(r, font_body, buf, cx+30, ly, card_w-60, 22,
                          0xFFCC00, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); ly += 28;
  snprintf(buf, sizeof(buf), "BIRTHPLACE:      %s", birthplace);
  civ_font_render_aligned(r, font_body, buf, cx+30, ly, card_w-60, 22,
                          0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); ly += 28;

  if (game->player_character) {
    civ_character_t *pc = (civ_character_t *)game->player_character;
    snprintf(buf, sizeof(buf), "BACKGROUND:      %s", civ_background_name(pc->background));
    civ_font_render_aligned(r, font_body, buf, cx+30, ly, card_w-60, 22,
                            0x8899AA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP); ly += 28;
    snprintf(buf, sizeof(buf), "ROLE:            %s", pc->current_role);
    civ_font_render_aligned(r, font_body, buf, cx+30, ly, card_w-60, 22,
                            0x8899AA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  }

  ly = cy + card_h - 56;
  btn_confirm->base.x = (float)(cx + card_w/2 - 100);
  btn_confirm->base.y = (float)ly;
  btn_confirm->base.w = 200; btn_confirm->base.h = 38;
  civ_widget_button_render(btn_confirm, r, font_body);
  (void)input;
}

static bool text_started = false;
static SDL_Window *sdl_win = NULL;

static void render(SDL_Renderer *r, int win_w, int win_h, civ_game_t *game,
                   civ_input_state_t *input) {
  if (!text_started) {
    sdl_win = SDL_GetRenderWindow(r);
    if (sdl_win) { SDL_StartTextInput(sdl_win); text_started = true; }
  }
  civ_render_rect_filled(r, 0, 0, win_w, win_h, CIV_COLOR_BG_DARK);

  /* Progress bar */
  int prog_w = 400, prog_h = 3, prog_x = (win_w-prog_w)/2, prog_y = 32;
  civ_render_rect_filled(r, prog_x, prog_y, prog_w, prog_h, 0x1A2A3A);
  civ_render_rect_filled(r, prog_x, prog_y, (prog_w*(step+1))/4, prog_h,
                         CIV_COLOR_PRIMARY);
  char step_label[32];
  snprintf(step_label, sizeof(step_label), "Step %d of 4", step+1);
  civ_font_render_aligned(r, font_small, step_label, 0, 10, win_w, 18,
                          0x667788, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

  switch (step) {
  case 0: render_step_age(r, win_w, win_h, input); break;
  case 1: render_step_nationality(r, win_w, win_h, input); break;
  case 2: render_step_birthplace(r, win_w, win_h, input); break;
  case 3: render_step_review(r, win_w, win_h, game, input); break;
  }
}

static void destroy(void) {
  if (sdl_win) { SDL_StopTextInput(sdl_win); sdl_win = NULL; }
  text_started = false;
  if (font_title) civ_font_destroy(font_title), font_title = NULL;
  if (font_body)  civ_font_destroy(font_body),  font_body = NULL;
  if (font_small) civ_font_destroy(font_small), font_small = NULL;
  if (btn_confirm) civ_widget_button_destroy(btn_confirm), btn_confirm = NULL;
}

civ_scene_t scene_identity = {.init = init, .update = update,
    .render = render, .destroy = destroy, .next_scene_id = -1};
