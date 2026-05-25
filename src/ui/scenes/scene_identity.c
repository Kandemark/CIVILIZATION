/**
 * @file scene_identity.c
 * @brief Identity setup — Nuklear UI
 */
#include "core/character.h"
#include "core/economy/financial_markets.h"
#include "core/profile.h"
#include "core/world/nations_data.h"
#include "ui/nuklear_ui.h"
#include "ui/scene.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>

static char nationality[64] = "";
static int  age = 25;
static char birthplace[64] = "";
static int  step = 0;
static char nation_search[32] = "";
static char all_nations[256][64];
static int  all_count = 0;

static void build_list(civ_game_t *game) {
  if (all_count > 0 || !game || !game->nations_data) return;
  for (uint32_t i = 0; i < game->nations_data->count && all_count < 256; i++)
    if (game->nations_data->nations[i].name[0])
      snprintf(all_nations[all_count++], 64, "%s", game->nations_data->nations[i].name);
}

static void init(void) {
  nationality[0] = '\0'; age = 25; birthplace[0] = '\0'; step = 0;
  nation_search[0] = '\0'; all_count = 0;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  build_list(game);
  if (input->esc_pressed) {
    if (step > 0) { step--; return; }
    civ_scene_manager_switch(SCENE_LIFE_ORIGIN); return;
  }
  if (step == 1) {
    if (input->text_input[0] && strlen(nation_search) < 30)
      strcat(nation_search, input->text_input);
    if (input->backspace_pressed && nation_search[0])
      nation_search[strlen(nation_search)-1] = '\0';
  }
  if (step == 2) {
    if (input->text_input[0] && strlen(birthplace) < 60)
      strcat(birthplace, input->text_input);
    if (input->backspace_pressed && birthplace[0])
      birthplace[strlen(birthplace)-1] = '\0';
  }
  if (step == 3 && input->enter_pressed) {
    if (!game->player_character) {
      civ_character_t *pc = civ_character_create(game->current_profile?game->current_profile->name:"Citizen");
      civ_character_apply_background(pc, CIV_BG_BUREAUCRATIC);
      game->player_character = pc;
    }
    if (nationality[0]) {
      civ_role_set(&game->player_role, &civ_role_private_citizen, nationality);
      if (game->market) {
        civ_market_engine_t *mkt = game->market;
        const civ_nation_data_t *nd = game->nations_data
            ? civ_nations_data_get_by_name(game->nations_data, nationality) : NULL;
        int found = 0;
        for (int ci = 0; ci < mkt->currency_count && !found; ci++)
          if ((nd && strcasestr(mkt->currencies[ci].name, nd->iso_a3)) ||
              strcasestr(mkt->currencies[ci].name, nationality)) {
            civ_wallet_add(&game->wallet, mkt->currencies[ci].iso,
                500.0f * mkt->currencies[ci].current_rate); found = 1;
          }
        if (!found) civ_wallet_add(&game->wallet, "USD", 500.0f);
      }
    }
    civ_scene_manager_switch(SCENE_GAME);
  }
}

static void render(SDL_Renderer *r, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  (void)r; (void)input;
  struct nk_context *nk = g_nk_ctx;
  if (!nk) return;

  float pw = 440, ph = (step == 1) ? 420 : 340;
  float px = ((float)win_w - pw) / 2, py = ((float)win_h - ph) / 2;

  char title[32];
  snprintf(title, sizeof(title), "Identity — Step %d/4", step + 1);

  if (nk_begin(nk, title, nk_rect(px, py, pw, ph), NK_WINDOW_TITLE)) {
    nk_layout_row_dynamic(nk, 6, 1);
    /* Progress */
    nk_layout_row_dynamic(nk, 4, 1);
    nk_progress(nk, (nk_size*)&step, 4, NK_MODIFIABLE);

    switch (step) {
    case 0: /* Age */
      nk_layout_row_dynamic(nk, 28, 1);
      nk_label(nk, "HOW OLD ARE YOU?", NK_TEXT_CENTERED);
      nk_layout_row_dynamic(nk, 48, 3);
      nk_spacing(nk, 1);
      if (nk_button_label(nk, "-")) { age--; if (age < 16) age = 16; }
      nk_spacing(nk, 1);
      { char ab[8]; snprintf(ab, sizeof(ab), "%d", age);
        nk_layout_row_dynamic(nk, 48, 1);
        nk_label(nk, ab, NK_TEXT_CENTERED); }
      nk_spacing(nk, 1);
      if (nk_button_label(nk, "+")) { age++; if (age > 110) age = 110; }
      nk_layout_row_dynamic(nk, 12, 1);
      nk_layout_row_dynamic(nk, 36, 1);
      if (nk_button_label(nk, "NEXT")) step = 1;
      break;
    case 1: /* Nationality */
      nk_layout_row_dynamic(nk, 24, 1);
      nk_label(nk, "WHAT IS YOUR NATIONALITY?", NK_TEXT_CENTERED);
      nk_layout_row_dynamic(nk, 34, 1);
      nk_edit_string_zero_terminated(nk, NK_EDIT_SIMPLE, nation_search, sizeof(nation_search), nk_filter_default);
      if (nationality[0]) {
        nk_layout_row_dynamic(nk, 22, 1);
        nk_label(nk, nationality, NK_TEXT_CENTERED);
      }
      if (nation_search[0]) {
        for (int i = 0; i < all_count; i++) {
          if (strcasestr(all_nations[i], nation_search)) {
            nk_layout_row_dynamic(nk, 24, 1);
            if (nk_button_label(nk, all_nations[i])) {
              snprintf(nationality, sizeof(nationality), "%s", all_nations[i]);
              nation_search[0] = '\0';
            }
          }
        }
      }
      nk_layout_row_dynamic(nk, 10, 1);
      nk_layout_row_dynamic(nk, 36, 1);
      if (nk_button_label(nk, nationality[0] ? "NEXT" : "(select a nationality)"))
        if (nationality[0]) step = 2;
      break;
    case 2: /* Birthplace */
      nk_layout_row_dynamic(nk, 24, 1);
      nk_label(nk, "WHERE WERE YOU BORN?", NK_TEXT_CENTERED);
      nk_layout_row_dynamic(nk, 20, 1);
      { char buf[128]; snprintf(buf, sizeof(buf), "Citizen of %s", nationality);
        nk_label(nk, buf, NK_TEXT_CENTERED); }
      nk_layout_row_dynamic(nk, 34, 1);
      nk_edit_string_zero_terminated(nk, NK_EDIT_SIMPLE, birthplace, sizeof(birthplace), nk_filter_default);
      nk_layout_row_dynamic(nk, 36, 1);
      if (nk_button_label(nk, birthplace[0] ? "NEXT" : "(type your city or state)"))
        if (birthplace[0]) step = 3;
      break;
    case 3: /* Review */
      nk_layout_row_dynamic(nk, 24, 1);
      nk_label(nk, "NATIONAL IDENTITY CARD", NK_TEXT_CENTERED);
      nk_layout_row_dynamic(nk, 4, 1);
      { char buf[256];
        snprintf(buf, sizeof(buf), "Name: %s", game->current_profile?game->current_profile->name:"Unknown");
        nk_layout_row_dynamic(nk, 20, 1); nk_label(nk, buf, NK_TEXT_LEFT);
        snprintf(buf, sizeof(buf), "Age: %d", age);
        nk_layout_row_dynamic(nk, 20, 1); nk_label(nk, buf, NK_TEXT_LEFT);
        snprintf(buf, sizeof(buf), "Nationality: %s", nationality);
        nk_layout_row_dynamic(nk, 20, 1); nk_label(nk, buf, NK_TEXT_LEFT);
        snprintf(buf, sizeof(buf), "Birthplace: %s", birthplace);
        nk_layout_row_dynamic(nk, 20, 1); nk_label(nk, buf, NK_TEXT_LEFT);
      }
      nk_layout_row_dynamic(nk, 38, 1);
      if (nk_button_label(nk, "CONFIRM IDENTITY")) step = 3;
      break;
    }
  }
  nk_end(nk);

  /* Trigger confirm from button press */
  if (step == 3 && input->enter_pressed) { /* handled in update */ }
}

static void destroy(void) {}

civ_scene_t scene_identity = {.init = init, .update = update,
    .render = render, .destroy = destroy, .next_scene_id = -1};
