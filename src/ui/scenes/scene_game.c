#include "../../../include/core/military/combat.h"
#include "../../../include/core/world/map_view.h"
#include "../../../include/engine/font.h"
#include "../../../include/engine/renderer.h"
#include "../../../include/ui/scene.h"
#include "../../../include/ui/ui_common.h"
#include <SDL3/SDL.h>
#include <stdio.h>

/* Local state */
static civ_render_map_context_t *map_ctx = NULL;
static civ_font_t *font_hud = NULL;
static int last_win_h = 0;
static int last_win_w = 0;
static int32_t hover_tile_x = -1;
static int32_t hover_tile_y = -1;
static civ_unit_t *selected_unit = NULL;
static civ_settlement_t *selected_settlement = NULL;
static bool show_diplomacy = false;
static bool show_research = false;
static bool show_government = false;
static bool show_wonders = false;
static bool show_rulebook = false;
static bool has_contacted_rival = false;

static void update_visibility(civ_game_t *game) {
  if (!game || !game->world_map || !game->unit_manager)
    return;

  /* Reset visible flag */
  for (int32_t i = 0; i < game->world_map->width * game->world_map->height;
       i++) {
    game->world_map->tiles[i].is_visible = false;
  }

  /* Set visible based on units */
  for (size_t i = 0; i < game->unit_manager->unit_count; i++) {
    civ_unit_t *u = &game->unit_manager->units[i];
    int32_t r = u->visibility_range;

    for (int32_t dy = -r; dy <= r; dy++) {
      for (int32_t dx = -r; dx <= r; dx++) {
        if (dx * dx + dy * dy <= r * r) {
          int32_t tx = u->x + dx;
          int32_t ty = u->y + dy;

          /* Handle map wrapping for visibility */
          tx = (tx + game->world_map->width) % game->world_map->width;

          if (ty >= 0 && ty < game->world_map->height) {
            civ_map_tile_t *tile = civ_map_get_tile(game->world_map, tx, ty);
            if (tile) {
              tile->is_visible = true;
              tile->is_explored = true;
            }
          }
        }
      }
    }
  }
}

static void render_wonders_panel(SDL_Renderer *renderer,
                                 civ_wonder_manager_t *manager) {
  if (!manager)
    return;

  int win_w = last_win_w;
  int win_h = last_win_h;
  int rpb_w = 650;
  int rpb_h = 550;
  int rpb_x = (win_w - rpb_w) / 2;
  int rpb_y = (win_h - rpb_h) / 2;

  /* Draw Background */
  civ_render_rect_filled_alpha(renderer, rpb_x, rpb_y, rpb_w, rpb_h, 0x111111,
                               230);
  civ_render_rect_outline(renderer, rpb_x, rpb_y, rpb_w, rpb_h, 0xFFFF00, 2);

  /* Title */
  civ_font_render_aligned(renderer, font_hud, "WONDERS OF THE WORLD", win_w / 2,
                          rpb_y + 30, 400, 40, 0x00FFFF, CIV_ALIGN_CENTER,
                          CIV_VALIGN_MIDDLE);

  int curr_y = rpb_y + 80;
  int box_h = 70;

  for (int i = 0; i < CIV_WONDER_COUNT; i++) {
    civ_wonder_t *w = &manager->wonders[i];
    uint32_t box_color = w->is_built ? 0x444444 : 0x222222;
    civ_render_rect_filled(renderer, rpb_x + 20, curr_y, rpb_w - 40, box_h,
                           box_color);
    civ_render_rect_outline(renderer, rpb_x + 20, curr_y, rpb_w - 40, box_h,
                            0x666666, 1);

    /* Wonder Name */
    uint32_t text_color = w->is_built ? 0x888888 : 0xFFFFFF;
    civ_font_render_aligned(renderer, font_hud, w->name, rpb_x + 30,
                            curr_y + 10, 300, 30, text_color, CIV_ALIGN_LEFT,
                            CIV_VALIGN_TOP);

    /* Description */
    civ_font_render_aligned(renderer, font_hud, w->description, rpb_x + 30,
                            curr_y + 40, 400, 20, 0xAAAAAA, CIV_ALIGN_LEFT,
                            CIV_VALIGN_TOP);

    /* Status or Bonus */
    if (w->is_built) {
      char built_msg[64];
      snprintf(built_msg, sizeof(built_msg), "Built by: %s", w->builder_id);
      civ_font_render_aligned(renderer, font_hud, built_msg,
                              rpb_x + rpb_w - 180, curr_y + 25, 150, 25,
                              0xAA8800, CIV_ALIGN_RIGHT, CIV_VALIGN_MIDDLE);
    } else {
      char bonus_text[64] = "";
      if (w->effects.science_mult > 0)
        snprintf(bonus_text, 64, "+%d%% Science",
                 (int)(w->effects.science_mult * 100));
      else if (w->effects.production_mult > 0)
        snprintf(bonus_text, 64, "+%d%% Production",
                 (int)(w->effects.production_mult * 100));
      else if (w->effects.gold_mult > 0)
        snprintf(bonus_text, 64, "+%d%% Gold",
                 (int)(w->effects.gold_mult * 100));

      civ_font_render_aligned(renderer, font_hud, bonus_text,
                              rpb_x + rpb_w - 180, curr_y + 25, 150, 25,
                              0x00FF00, CIV_ALIGN_RIGHT, CIV_VALIGN_MIDDLE);
    }
    curr_y += box_h + 10;
  }
}

static void render_governance_panel(SDL_Renderer *renderer,
                                    civ_government_t *gov) {
  int win_w = last_win_w;
  int win_h = last_win_h;
  int gpb_w = 500;
  int gpb_h = 400;
  int gpb_x = (win_w - gpb_w) / 2;
  int gpb_y = (win_h - gpb_h) / 2;

  civ_render_rect_filled_alpha(renderer, gpb_x, gpb_y, gpb_w, gpb_h, 0x1A1405,
                               250);
  civ_render_rect_outline(renderer, gpb_x, gpb_y, gpb_w, gpb_h, 0xFFCC00, 2);
  civ_render_line(renderer, gpb_x, gpb_y + 50, gpb_x + gpb_w, gpb_y + 50,
                  0x3A2A1A);

  civ_font_render_aligned(renderer, font_hud, "NATIONAL GOVERNANCE", gpb_x + 20,
                          gpb_y, gpb_w - 40, 50, 0xFFFFFF, CIV_ALIGN_CENTER,
                          CIV_VALIGN_MIDDLE);

  int curr_y = gpb_y + 70;
  const char *type_str = "Chiefdom";
  if (gov->government_type == CIV_GOV_DESPOTISM)
    type_str = "Despotism";
  else if (gov->government_type == CIV_GOV_MONARCHY)
    type_str = "Monarchy";
  else if (gov->government_type == CIV_GOV_REPUBLIC)
    type_str = "Republic";
  else if (gov->government_type == CIV_GOV_DEMOCRACY)
    type_str = "Democracy";

  char buf[128];
  sprintf(buf, "TYPE: %s", type_str);
  civ_font_render_aligned(renderer, font_hud, buf, gpb_x + 30, curr_y,
                          gpb_w - 60, 30, 0xFFCC00, CIV_ALIGN_LEFT,
                          CIV_VALIGN_TOP);
  curr_y += 40;

  civ_font_render_aligned(renderer, font_hud, "STABILITY", gpb_x + 30, curr_y,
                          120, 20, 0xAAAAAA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  civ_render_rect_filled(renderer, gpb_x + 160, curr_y + 5, 200, 10, 0x333333);
  civ_render_rect_filled(renderer, gpb_x + 160, curr_y + 5,
                         (int)(200 * gov->stability), 10, 0x00A0FF);
  curr_y += 30;

  civ_font_render_aligned(renderer, font_hud, "LEGITIMACY", gpb_x + 30, curr_y,
                          120, 20, 0xAAAAAA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  civ_render_rect_filled(renderer, gpb_x + 160, curr_y + 5, 200, 10, 0x333333);
  civ_render_rect_filled(renderer, gpb_x + 160, curr_y + 5,
                         (int)(200 * gov->legitimacy), 10, 0x00FF88);
  curr_y += 30;

  civ_font_render_aligned(renderer, font_hud, "EFFICIENCY", gpb_x + 30, curr_y,
                          120, 20, 0xAAAAAA, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  civ_render_rect_filled(renderer, gpb_x + 160, curr_y + 5, 200, 10, 0x333333);
  civ_render_rect_filled(renderer, gpb_x + 160, curr_y + 5,
                         (int)(200 * gov->efficiency), 10, 0xFFCC00);
  curr_y += 50;

  civ_float_t taxes = civ_government_collect_taxes(gov);
  sprintf(buf, "ESTIMATED ANNUAL TAX REVENUE: %.1f GOLD", taxes);
  civ_font_render_aligned(renderer, font_hud, buf, gpb_x + 30, curr_y,
                          gpb_w - 60, 30, 0xFFFFFF, CIV_ALIGN_LEFT,
                          CIV_VALIGN_TOP);
  curr_y += 40;

  /* Phase 11: Stature & Identity */
  civ_render_line(renderer, gpb_x + 20, curr_y, gpb_x + gpb_w - 20, curr_y,
                  0x3A2A1A);
  curr_y += 10;

  const char *tier_names[] = {
      "FAILED STATE",   "FRONTIER NATION", "DEVELOPING STATE", "STABLE STATE",
      "REGIONAL POWER", "GREAT POWER",     "HEGEMON"};
  sprintf(buf, "GLOBAL STATURE: %s", tier_names[gov->stature_tier]);
  civ_font_render_aligned(renderer, font_hud, buf, gpb_x + 30, curr_y,
                          gpb_w - 60, 30, 0x00FFFF, CIV_ALIGN_CENTER,
                          CIV_VALIGN_TOP);
  curr_y += 40;

  civ_font_render_aligned(renderer, font_hud, "IDENTITY PROFILE", gpb_x + 30,
                          curr_y, gpb_w - 60, 20, 0xAAAAAA, CIV_ALIGN_LEFT,
                          CIV_VALIGN_TOP);
  curr_y += 25;

  sprintf(buf, "Primary Language: ID %d | Faith: ID %d | Race: ID %d", 0, 0,
          0); /* Values hardcoded for demo or extracted from gov */
  civ_font_render_aligned(renderer, font_hud, buf, gpb_x + 30, curr_y,
                          gpb_w - 60, 20, 0xCCCCCC, CIV_ALIGN_LEFT,
                          CIV_VALIGN_TOP);
}

static void render_rulebook_editor(SDL_Renderer *renderer,
                                   civ_government_t *gov) {
  if (!gov)
    return;

  int win_w = last_win_w;
  int win_h = last_win_h;
  int rbe_w = 700;
  int rbe_h = 550;
  int rbe_x = (win_w - rbe_w) / 2;
  int rbe_y = (win_h - rbe_h) / 2;

  civ_render_rect_filled_alpha(renderer, rbe_x, rbe_y, rbe_w, rbe_h, 0x050A0F,
                               245);
  civ_render_rect_outline(renderer, rbe_x, rbe_y, rbe_w, rbe_h, 0x00A0FF, 2);

  civ_font_render_aligned(renderer, font_hud, "STATE RULEBOOK & INSTITUTIONS",
                          rbe_x, rbe_y + 20, rbe_w, 40, 0xFFFFFF,
                          CIV_ALIGN_CENTER, CIV_VALIGN_TOP);

  int curr_y = rbe_y + 80;

  /* Legislative Logic Section */
  civ_font_render_aligned(renderer, font_hud, "LEGISLATIVE LOGIC", rbe_x + 30,
                          curr_y, 300, 30, 0x00A0FF, CIV_ALIGN_LEFT,
                          CIV_VALIGN_TOP);
  char buf[128];
  sprintf(buf, "Voting Threshold: %.0f%% (Simple Majority)",
          gov->legislative_threshold * 100.0f);
  civ_font_render_aligned(renderer, font_hud, buf, rbe_x + 40, curr_y + 30, 300,
                          20, 0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
  curr_y += 70;

  /* Institutions Section */
  civ_font_render_aligned(renderer, font_hud, "ACTIVE INSTITUTIONS", rbe_x + 30,
                          curr_y, 300, 30, 0x00A0FF, CIV_ALIGN_LEFT,
                          CIV_VALIGN_TOP);
  curr_y += 35;

  if (gov->institution_manager) {
    for (size_t i = 0; i < gov->institution_manager->count; i++) {
      civ_institution_t *inst = &gov->institution_manager->items[i];
      civ_render_rect_filled(renderer, rbe_x + 40, curr_y, rbe_w - 80, 50,
                             0x1A2A3A);
      civ_font_render_aligned(renderer, font_hud, inst->name, rbe_x + 50,
                              curr_y + 5, 300, 20, 0xFFFFFF, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);

      sprintf(buf, "Stature: %.1f | Maint: %.1f Gold", inst->stature,
              inst->maintenance_cost);
      civ_font_render_aligned(renderer, font_hud, buf, rbe_x + 50, curr_y + 25,
                              400, 20, 0xAAAAAA, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
      curr_y += 60;
    }
  }

  /* Subdivisions Section */
  civ_font_render_aligned(renderer, font_hud, "ADMINISTRATIVE SUBDIVISIONS",
                          rbe_x + 30, curr_y, 300, 30, 0x00A0FF, CIV_ALIGN_LEFT,
                          CIV_VALIGN_TOP);
  curr_y += 35;
  if (gov->subdivision_manager) {
    for (size_t i = 0; i < gov->subdivision_manager->count; i++) {
      civ_subdivision_t *sub = &gov->subdivision_manager->items[i];
      sprintf(buf, "%s (%s) - Autonomy: %.0f%%", sub->name,
              (sub->type == CIV_SUBDIVISION_COLONY ? "COLONY" : "STATE"),
              sub->autonomy * 100.0f);
      civ_font_render_aligned(renderer, font_hud, buf, rbe_x + 40, curr_y, 500,
                              20, 0xCCCCCC, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
      curr_y += 25;
    }
  }
}

static void init(void) {
  printf("[SCENE_GAME] Initializing...\n");
  font_hud = civ_font_load_system("Inter", 18);
  if (!font_hud)
    font_hud = civ_font_load_system("Segoe UI", 18);

  selected_unit = NULL;
}

static void update(civ_game_t *game, civ_input_state_t *input) {
  if (!map_ctx)
    return;

  /* Handle Panning (Right Mouse Button) */
  if (input->mouse_right_down) {
    float move_speed = 1.0f / (map_ctx->zoom * 4.0f);
    map_ctx->view_x -= (float)input->delta_x * move_speed;
    map_ctx->view_y -= (float)input->delta_y * move_speed;
  }

  /* Handle Zooming (Scroll Wheel) */
  if (fabsf(input->scroll_delta) > 0.1f) {
    float zoom_speed = 1.15f;
    if (input->scroll_delta > 0) {
      map_ctx->zoom *= zoom_speed;
    } else {
      map_ctx->zoom /= zoom_speed;
    }

    /* Clamp Zoom */
    if (map_ctx->zoom < 0.005f)
      map_ctx->zoom = 0.005f;
    if (map_ctx->zoom > 2.0f)
      map_ctx->zoom = 2.0f;
  }

  /* Handle Selection and Interaction */
  if (input->mouse_left_pressed) {
    /* Check if "Next Turn" button clicked (Bottom-Right) */
    int btn_w = 180;
    int btn_h = 50;
    int btn_x = last_win_w - btn_w - 20;
    int btn_y = last_win_h - btn_h - 20;

    if (input->mouse_x >= btn_x && input->mouse_x <= btn_x + btn_w &&
        input->mouse_y >= btn_y && input->mouse_y <= btn_y + btn_h) {
      civ_game_end_turn(game);
      return;
    }

    /* Map Space Selection */
    float world_x = map_ctx->view_x + (float)(input->mouse_x - last_win_w / 2) /
                                          (64.0f * map_ctx->zoom);
    float world_y = map_ctx->view_y + (float)(input->mouse_y - last_win_h / 2) /
                                          (64.0f * map_ctx->zoom);

    int32_t tx = (int32_t)floorf(world_x);
    int32_t ty = (int32_t)floorf(world_y);

    /* Selection Logic */
    civ_unit_t *old_unit = selected_unit;
    civ_settlement_t *old_settlement = selected_settlement;

    selected_unit = NULL;
    selected_settlement = NULL;

    /* 1. Check for settlements first */
    if (game->settlement_manager) {
      for (size_t i = 0; i < game->settlement_manager->settlement_count; i++) {
        if ((int32_t)game->settlement_manager->settlements[i].x == tx &&
            (int32_t)game->settlement_manager->settlements[i].y == ty) {
          selected_settlement = &game->settlement_manager->settlements[i];
          printf("[SCENE_GAME] Selected settlement: %s\n",
                 selected_settlement->name);
          break;
        }
      }
    }

    /* 2. Check for units if no settlement selected */
    if (!selected_settlement && game->unit_manager) {
      for (size_t i = 0; i < game->unit_manager->unit_count; i++) {
        if (game->unit_manager->units[i].x == tx &&
            game->unit_manager->units[i].y == ty) {
          selected_unit = &game->unit_manager->units[i];
          printf("[SCENE_GAME] Selected unit: %s\n", selected_unit->name);
          break;
        }
      }
    }

    /* Handle HUD Toggles */
    if (input->mouse_x > last_win_w - 680 &&
        input->mouse_x < last_win_w - 530 && input->mouse_y < 40) {
      show_diplomacy = !show_diplomacy;
      if (show_diplomacy) {
        show_research = show_government = show_wonders = false;
      }
    }
    if (input->mouse_x > last_win_w - 530 &&
        input->mouse_x < last_win_w - 380 && input->mouse_y < 40) {
      show_wonders = !show_wonders;
      if (show_wonders) {
        show_diplomacy = show_research = show_government = false;
      }
    }
    if (input->mouse_x > last_win_w - 380 &&
        input->mouse_x < last_win_w - 230 && input->mouse_y < 40) {
      show_government = !show_government;
      if (show_government) {
        show_diplomacy = show_research = show_wonders = false;
      }
    }
    if (input->mouse_x > last_win_w - 230 && input->mouse_y < 40) {
      show_research = !show_research;
      if (show_research) {
        show_diplomacy = show_government = show_wonders = show_rulebook = false;
      }
    }
    if (input->mouse_x > last_win_w - 530 &&
        input->mouse_x < last_win_w - 380 && input->mouse_y < 40) {
      show_rulebook = !show_rulebook;
      if (show_rulebook) {
        show_diplomacy = show_research = show_government = show_wonders = false;
      }
    }

    /* Handle Research Panel Clicks */
    if (show_research && game->technology_tree) {
      int win_w = last_win_w;
      int win_h = last_win_h;
      int rpb_w = 600;
      int rpb_h = 500;
      int rpb_x = (win_w - rpb_w) / 2;
      int rpb_y = (win_h - rpb_h) / 2;

      int curr_y = rpb_y + 70;
      int tech_box_h = 60;

      for (size_t i = 0; i < game->technology_tree->tech_count; i++) {
        if (input->mouse_x >= rpb_x + 20 &&
            input->mouse_x <= rpb_x + rpb_w - 20 && input->mouse_y >= curr_y &&
            input->mouse_y <= curr_y + tech_box_h) {
          civ_innovation_system_research_tech(
              game->technology_tree, game->technology_tree->technologies[i].id);
          printf("[SCENE_GAME] Started research: %s\n",
                 game->technology_tree->technologies[i].name);
        }
        curr_y += tech_box_h + 15;
      }
    }

    /* Deselect if clicked in open space and nothing found */
    if (!selected_unit && !selected_settlement) {
      /* But only if not clicking the minimap or sidebar areas */
      bool on_ui = false;
      if (input->mouse_x < 220 && input->mouse_y > last_win_h - 170)
        on_ui = true; /* Minimap */
      if (input->mouse_x < 300)
        on_ui = true; /* Sidebar */
      if (input->mouse_x > last_win_w - 200 && input->mouse_y > last_win_h - 70)
        on_ui = true; /* End Turn */

      if (!on_ui) {
        selected_unit = NULL;
        selected_settlement = NULL;
      } else {
        /* Keep selection if clicking UI */
        selected_unit = old_unit;
        selected_settlement = old_settlement;
      }
    }

    /* Handle Button Actions (Buttons are in sidebars) */

    /* 1. FOUND CITY (Settler selected) */
    if (selected_unit && selected_unit->unit_type == CIV_UNIT_TYPE_SETTLER &&
        !selected_unit->has_moved) {
      int btn_x = 35;
      int btn_y = 310;
      int btn_w = 250;
      int btn_h = 40;

      if (input->mouse_x >= btn_x && input->mouse_x <= btn_x + btn_w &&
          input->mouse_y >= btn_y && input->mouse_y <= btn_y + btn_h) {
        civ_attempt_settlement_spawn(game->settlement_manager,
                                     (float)selected_unit->x,
                                     (float)selected_unit->y);
        selected_unit->current_strength = 0;
        selected_unit = NULL;
        update_visibility(game);
        return;
      }
    }

    /* 2. RECRUITMENT (Settlement selected) */
    if (selected_settlement) {
      int ssb_x = 20;
      int ssb_y = 60;
      int curr_y = ssb_y + 110; /* Roughly near recruitment section */

      /* Hack: the render uses curr_y which is dynamic.
         Let's use specific fixed offsets for the buttons. */
      int r1_y = ssb_y + 245;
      int r2_y = r1_y + 50;
      int r_w = 260;
      int r_h = 40;

      if (!selected_settlement->is_producing) {
        /* Option 1: Infantry */
        if (input->mouse_x >= ssb_x + 10 &&
            input->mouse_x <= ssb_x + 10 + r_w && input->mouse_y >= r1_y &&
            input->mouse_y <= r1_y + r_h) {
          selected_settlement->is_producing = true;
          selected_settlement->production_type = 0;
          selected_settlement->production_target = 30.0f;
          selected_settlement->production_progress = 0.0f;
        }
        /* Option 2: Settler */
        if (selected_settlement->population > 500) {
          if (input->mouse_x >= ssb_x + 10 &&
              input->mouse_x <= ssb_x + 10 + r_w && input->mouse_y >= r2_y &&
              input->mouse_y <= r2_y + r_h) {
            selected_settlement->is_producing = true;
            selected_settlement->production_type = 7;
            selected_settlement->production_target = 80.0f;
            selected_settlement->production_progress = 0.0f;
          }
        }
      }
    }

    /* 3. DIPLOMACY (Noting selected) */
    if (!selected_unit && !selected_settlement && show_diplomacy &&
        has_contacted_rival) {
      int dsb_x = 20;
      int dsb_y = 60;
      int t_y = dsb_y + 120;
      int n_y = t_y + 50;
      int w_y = n_y + 50;
      int d_w = 260;
      int d_h = 40;

      /* Trade Agreement */
      if (input->mouse_x >= dsb_x + 10 && input->mouse_x <= dsb_x + 10 + d_w &&
          input->mouse_y >= t_y && input->mouse_y <= t_y + d_h) {
        civ_diplomacy_system_propose_treaty(
            game->diplomacy_system, "player", "rival_kingdom",
            CIV_TREATY_TYPE_TRADE_AGREEMENT, 30);
      }
      /* Non-Aggression Pact */
      if (input->mouse_x >= dsb_x + 10 && input->mouse_x <= dsb_x + 10 + d_w &&
          input->mouse_y >= n_y && input->mouse_y <= n_y + d_h) {
        civ_diplomacy_system_propose_treaty(game->diplomacy_system, "player",
                                            "rival_kingdom",
                                            CIV_TREATY_TYPE_NON_AGGRESSION, 30);
      }
    }

    /* 4. DISCOVERY */
    if (!selected_unit && !selected_settlement && !show_diplomacy &&
        game->technology_tree) {
      /* Tech clicks handled here if we want, but let's keep it simple for now
       * or restore it. */
      civ_innovation_system_t *is = game->technology_tree;
      for (size_t i = 0; i < is->tech_count; i++) {
        int row_y = 60 + 55 + (int)i * 60;
        if (input->mouse_x >= 35 && input->mouse_x <= 265 &&
            input->mouse_y >= row_y && input->mouse_y <= row_y + 50) {
          civ_innovation_system_research_tech(is, is->technologies[i].id);
        }
      }
    }
  }

  /* Handle Movement (Right Click) */
  if (input->mouse_right_pressed && selected_unit &&
      !selected_unit->has_moved) {
    float world_x = map_ctx->view_x + (float)(input->mouse_x - last_win_w / 2) /
                                          (64.0f * map_ctx->zoom);
    float world_y = map_ctx->view_y + (float)(input->mouse_y - last_win_h / 2) /
                                          (64.0f * map_ctx->zoom);
    int32_t tx = (int32_t)floorf(world_x);
    int32_t ty = (int32_t)floorf(world_y);

    int dx = abs(tx - selected_unit->x);
    int dy = abs(ty - selected_unit->y);
    if ((dx <= 1 && dy <= 1) && (dx + dy > 0)) {
      selected_unit->x = tx;
      selected_unit->y = ty;
      selected_unit->has_moved = true;
      update_visibility(game);
    }
  }

  /* First Contact Proximity Check */
  if (game->unit_manager && game->settlement_manager && !has_contacted_rival) {
    for (size_t i = 0; i < game->unit_manager->unit_count; i++) {
      civ_unit_t *u = &game->unit_manager->units[i];
      if (strstr(u->name, "Rival"))
        continue;
      for (size_t j = 0; j < game->settlement_manager->settlement_count; j++) {
        civ_settlement_t *s = &game->settlement_manager->settlements[j];
        if (strcmp(s->id, "rival_capital") == 0) {
          float dist_sq = (float)((u->x - s->x) * (u->x - s->x) +
                                  (u->y - s->y) * (u->y - s->y));
          if (dist_sq < 64.0f) {
            has_contacted_rival = true;
            printf("[SCENE_GAME] CONTACT!\n");
          }
        }
      }
    }
  }
}

static void render(SDL_Renderer *renderer, int win_w, int win_h,
                   civ_game_t *game, civ_input_state_t *input) {
  if (!game || !game->world_map) {
    return;
  }

  /* Initialize map context if needed */
  if (!map_ctx) {
    map_ctx = civ_render_map_context_create(renderer, win_w, win_h,
                                            game->world_map->width,
                                            game->world_map->height);
    if (map_ctx) {
      map_ctx->view_x = (float)game->world_map->width / 2.0f;
      map_ctx->view_y = (float)game->world_map->height / 2.0f;
      map_ctx->zoom = 0.12f;
    }
  }

  /* 1. Map Render */
  if (map_ctx) {
    civ_render_map(renderer, map_ctx, game->world_map, win_w, win_h);

    /* 1.1. Render Borders (Contour Rendering) */
    if (game->world_map) {
      /* Only render borders within the view frustum */
      int32_t start_tx = (int32_t)floorf(
          map_ctx->view_x - (win_w / 2.0f) / (64.0f * map_ctx->zoom));
      int32_t end_tx = (int32_t)ceilf(map_ctx->view_x +
                                      (win_w / 2.0f) / (64.0f * map_ctx->zoom));
      int32_t start_ty = (int32_t)floorf(
          map_ctx->view_y - (win_h / 2.0f) / (64.0f * map_ctx->zoom));
      int32_t end_ty = (int32_t)ceilf(map_ctx->view_y +
                                      (win_h / 2.0f) / (64.0f * map_ctx->zoom));

      for (int32_t ty = start_ty; ty <= end_ty; ty++) {
        for (int32_t tx = start_tx; tx <= end_tx; tx++) {
          /* Handle map wrapping for X */
          int32_t wrapped_tx =
              (tx % game->world_map->width + game->world_map->width) %
              game->world_map->width;

          if (ty >= 0 && ty < game->world_map->height) {
            civ_map_tile_t *tile =
                civ_map_get_tile(game->world_map, wrapped_tx, ty);
            if (tile && tile->owner_id[0] != '\0') {
              /* Convert tile to screen coords */
              float screen_x = (float)win_w / 2.0f +
                               (tx - map_ctx->view_x) * 64.0f * map_ctx->zoom;
              float screen_y = (float)win_h / 2.0f +
                               (ty - map_ctx->view_y) * 64.0f * map_ctx->zoom;
              float tile_size = 64.0f * map_ctx->zoom;

              /* Choose color based on owner */
              uint32_t border_color = 0xFFFF00; /* Default: Player (Yellow) */
              if (strstr(tile->owner_id, "rival")) {
                border_color = 0xFF2200; /* Rival (Red) */
              }

              /* 1. Draw subtle alpha fill for the territory */
              civ_render_rect_filled_alpha(renderer, (int)screen_x,
                                           (int)screen_y, (int)tile_size,
                                           (int)tile_size, border_color, 40);

              /* 2. Contour Borders: Draw only if neighbors have different
               * owners */

              /* Check Right neighbor */
              int32_t r_tx = (wrapped_tx + 1) % game->world_map->width;
              civ_map_tile_t *r_tile =
                  civ_map_get_tile(game->world_map, r_tx, ty);
              if (!r_tile || strcmp(r_tile->owner_id, tile->owner_id) != 0) {
                civ_render_line(renderer, (int)(screen_x + tile_size),
                                (int)screen_y, (int)(screen_x + tile_size),
                                (int)(screen_y + tile_size), border_color);
              }

              /* Check Left neighbor */
              int32_t l_tx = (wrapped_tx - 1 + game->world_map->width) %
                             game->world_map->width;
              civ_map_tile_t *l_tile =
                  civ_map_get_tile(game->world_map, l_tx, ty);
              if (!l_tile || strcmp(l_tile->owner_id, tile->owner_id) != 0) {
                civ_render_line(renderer, (int)screen_x, (int)screen_y,
                                (int)screen_x, (int)(screen_y + tile_size),
                                border_color);
              }

              /* Check Top neighbor */
              if (ty > 0) {
                civ_map_tile_t *t_tile =
                    civ_map_get_tile(game->world_map, wrapped_tx, ty - 1);
                if (!t_tile || strcmp(t_tile->owner_id, tile->owner_id) != 0) {
                  civ_render_line(renderer, (int)screen_x, (int)screen_y,
                                  (int)(screen_x + tile_size), (int)screen_y,
                                  border_color);
                }
              } else {
                /* Top edge of the world */
                civ_render_line(renderer, (int)screen_x, (int)screen_y,
                                (int)(screen_x + tile_size), (int)screen_y,
                                border_color);
              }

              /* Check Bottom neighbor */
              if (ty < game->world_map->height - 1) {
                civ_map_tile_t *b_tile =
                    civ_map_get_tile(game->world_map, wrapped_tx, ty + 1);
                if (!b_tile || strcmp(b_tile->owner_id, tile->owner_id) != 0) {
                  civ_render_line(renderer, (int)screen_x,
                                  (int)(screen_y + tile_size),
                                  (int)(screen_x + tile_size),
                                  (int)(screen_y + tile_size), border_color);
                }
              } else {
                /* Bottom edge of the world */
                civ_render_line(renderer, (int)screen_x,
                                (int)(screen_y + tile_size),
                                (int)(screen_x + tile_size),
                                (int)(screen_y + tile_size), border_color);
              }
            }
          }
        }
      }
    }

    civ_render_settlements(renderer, map_ctx, game->settlement_manager, win_w,
                           win_h);
  }

  /* HUD Buttons: GOVERNANCE | WONDERS | RESEARCH | RULEBOOK */
  int hb_w = 120;
  int hb_h = 40;
  int hb_y = 0;

  struct {
    const char *label;
    bool *active;
    uint32_t color;
  } buttons[] = {{"RULEBOOK", &show_rulebook, 0x00A0FF},
                 {"RESEARCH", &show_research, 0x00A0FF},
                 {"GOVERNMENT", &show_government, 0xFFCC00},
                 {"WONDERS", &show_wonders, 0xFFFF00}};

  for (int i = 0; i < 4; i++) {
    int bx = win_w - (i + 1) * (hb_w + 10) - 20;
    bool hov = (input->mouse_x >= bx && input->mouse_x <= bx + hb_w &&
                input->mouse_y < hb_h);
    civ_render_rect_filled_alpha(renderer, bx, hb_y, hb_w, hb_h,
                                 hov ? 0x2A3A4A : 0x1A2A3A, 220);
    civ_render_rect_outline(renderer, bx, hb_y, hb_w, hb_h, buttons[i].color,
                            1);
    civ_font_render_aligned(renderer, font_hud, buttons[i].label, bx, hb_y,
                            hb_w, hb_h, 0xFFFFFF, CIV_ALIGN_CENTER,
                            CIV_VALIGN_MIDDLE);
  }

  /* Spawn initial unit if none exist for testing */
  if (game->unit_manager && game->unit_manager->unit_count == 0) {
    civ_unit_manager_spawn_unit(game->unit_manager, CIV_UNIT_TYPE_SETTLER,
                                "Settlers", 100, game->world_map->width / 2,
                                game->world_map->height / 2);

    /* Spawn an "Enemy" for combat testing */
    civ_unit_manager_spawn_unit(
        game->unit_manager, CIV_UNIT_TYPE_INFANTRY, "Barbarians", 80,
        game->world_map->width / 2 + 1, game->world_map->height / 2);

    update_visibility(game);
  }

  civ_render_rect_filled_alpha(renderer, 0, 0, win_w, 40, 0x050A14, 220);
  civ_render_line(renderer, 0, 40, win_w, 40, 0x1A2A3A);

  if (map_ctx && font_hud) {
    float lat = 90.0f - (map_ctx->view_y / (float)map_ctx->map_height) * 180.0f;
    float lon = (map_ctx->view_x / (float)map_ctx->map_width) * 360.0f - 180.0f;
    while (lon < -180.0f)
      lon += 360.0f;
    while (lon > 180.0f)
      lon -= 360.0f;

    char buf[128];
    sprintf(buf,
            "ATLAS POSITION: %.2f%c | %.2f%c    [POLITICAL ATLAS MODE "
            "ACTIVE]",
            fabsf(lat), lat >= 0 ? 'N' : 'S', fabsf(lon), lon >= 0 ? 'E' : 'W');

    civ_font_render_aligned(renderer, font_hud, buf, 20, 0, win_w - 40, 40,
                            CIV_COLOR_PRIMARY, CIV_ALIGN_LEFT,
                            CIV_VALIGN_MIDDLE);

    sprintf(buf, "ZOOM: %.1fx", map_ctx->zoom * 10.0f);
    civ_font_render_aligned(renderer, font_hud, buf, 20, 0, win_w - 40, 40,
                            CIV_COLOR_TEXT_DIM, CIV_ALIGN_RIGHT,
                            CIV_VALIGN_MIDDLE);

    /* 2.1. Science & Research Display */
    if (game->technology_tree) {
      char sci_buf[128];
      civ_innovation_system_t *it = game->technology_tree;

      sprintf(sci_buf, "SCIENCE: +%.1f/YR", it->research_budget);
      civ_font_render_aligned(renderer, font_hud, sci_buf, 20, 0, 200, 40,
                              0x00A0FF, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

      /* Current Research Progress */
      if (it->current_research) {
        civ_technology_node_t *current_tech = NULL;
        for (size_t i = 0; i < it->tech_count; i++) {
          if (strcmp(it->technologies[i].id, it->current_research) == 0) {
            current_tech = &it->technologies[i];
            break;
          }
        }

        if (current_tech) {
          char tech_buf[128];
          sprintf(tech_buf, "RESEARCHING: %s", current_tech->name);
          civ_font_render_aligned(renderer, font_hud, tech_buf, 230, 0, 250, 40,
                                  0xFFCC00, CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

          float progress =
              current_tech->progress / current_tech->base_research_cost;
          civ_render_rect_filled(renderer, 480, 15, 150, 10, 0x1A2A3A);
          civ_render_rect_filled(renderer, 480, 15, (int)(150 * progress), 10,
                                 0x00FF00);
        }
      } else {
        civ_font_render_aligned(renderer, font_hud, "[NO RESEARCH SELECTED]",
                                230, 0, 250, 40, 0xFF4444, CIV_ALIGN_LEFT,
                                CIV_VALIGN_MIDDLE);
      }
    }

    /* 2.2. Navigation Buttons */
    uint32_t dip_color = show_diplomacy ? 0xFFFF00 : 0xAAAAAA;
    civ_font_render_aligned(renderer, font_hud, "DIPLOMACY [M]", win_w - 680, 0,
                            150, 40, dip_color, CIV_ALIGN_LEFT,
                            CIV_VALIGN_MIDDLE);

    uint32_t won_color = show_wonders ? 0xFFFF00 : 0xAAAAAA;
    civ_font_render_aligned(renderer, font_hud, "WONDERS [W]", win_w - 530, 0,
                            150, 40, won_color, CIV_ALIGN_LEFT,
                            CIV_VALIGN_MIDDLE);

    uint32_t gov_color = show_government ? 0xFFFF00 : 0xAAAAAA;
    civ_font_render_aligned(renderer, font_hud, "GOVERNMENT [G]", win_w - 380,
                            0, 150, 40, gov_color, CIV_ALIGN_LEFT,
                            CIV_VALIGN_MIDDLE);

    uint32_t res_color = show_research ? 0xFFFF00 : 0xAAAAAA;
    civ_font_render_aligned(renderer, font_hud, "RESEARCH [T]", win_w - 200, 0,
                            150, 40, res_color, CIV_ALIGN_LEFT,
                            CIV_VALIGN_MIDDLE);

    /* 3. Render Units */
    if (game->unit_manager) {
      for (size_t i = 0; i < game->unit_manager->unit_count; i++) {
        civ_unit_t *u = &game->unit_manager->units[i];
        if (u->current_strength <= 0)
          continue;

        /* Convert tile to screen coords */
        float screen_x = (float)win_w / 2.0f +
                         (u->x - map_ctx->view_x) * 64.0f * map_ctx->zoom;
        float screen_y = (float)win_h / 2.0f +
                         (u->y - map_ctx->view_y) * 64.0f * map_ctx->zoom;
        float size = 48.0f * map_ctx->zoom;

        if (screen_x + size < 0 || screen_x - size > win_w ||
            screen_y + size < 0 || screen_y - size > win_h)
          continue;

        /* Selection highlight */
        if (selected_unit == u) {
          civ_render_rect_filled_alpha(renderer, (int)(screen_x - size / 2 - 4),
                                       (int)(screen_y - size / 2 - 4),
                                       (int)size + 8, (int)size + 8, 0x00A0FF,
                                       180);
        }

        /* Unit box */
        uint32_t color = u->has_moved ? 0x555555 : 0xFF2200;
        if (u->unit_type == CIV_UNIT_TYPE_SETTLER)
          color = 0x00FFCC;

        civ_render_rect_filled(renderer, (int)(screen_x - size / 2),
                               (int)(screen_y - size / 2), (int)size, (int)size,
                               color);
        civ_render_rect_outline(renderer, (int)(screen_x - size / 2),
                                (int)(screen_y - size / 2), (int)size,
                                (int)size, 0xFFFFFF, 1);
      }
    }

    /* 3.1. Render Settlement Labels */
    if (game->settlement_manager) {
      for (size_t i = 0; i < game->settlement_manager->settlement_count; i++) {
        civ_settlement_t *s = &game->settlement_manager->settlements[i];

        float screen_x = (float)win_w / 2.0f +
                         (s->x - map_ctx->view_x) * 64.0f * map_ctx->zoom;
        float screen_y = (float)win_h / 2.0f +
                         (s->y - map_ctx->view_y) * 64.0f * map_ctx->zoom;

        if (screen_x < 0 || screen_x > win_w || screen_y < 0 ||
            screen_y > win_h)
          continue;

        /* Draw selection highlight for settlement */
        if (selected_settlement == s) {
          float size = 52.0f * map_ctx->zoom;
          civ_render_rect_outline(renderer, (int)(screen_x - size / 2),
                                  (int)(screen_y - size / 2), (int)size,
                                  (int)size, 0xFFFF00, 2);
        }

        char label[64];
        sprintf(label, "%s (Pop: %lld)", s->name, s->population);
        civ_font_render_aligned(renderer, font_hud, label, (int)screen_x - 100,
                                (int)screen_y + 30, 200, 20, 0xFFFFFF,
                                CIV_ALIGN_CENTER, CIV_VALIGN_TOP);
      }
    }

    /* 4. "Next Turn" Button */
    int btn_w = 180;
    int btn_h = 50;
    int btn_x = win_w - btn_w - 20;
    int btn_y = win_h - btn_h - 20;

    bool hovered =
        (input->mouse_x >= btn_x && input->mouse_x <= btn_x + btn_w &&
         input->mouse_y >= btn_y && input->mouse_y <= btn_y + btn_h);

    civ_render_rect_filled_alpha(renderer, btn_x, btn_y, btn_w, btn_h,
                                 hovered ? 0x2A3A4A : 0x1A2A3A, 240);
    civ_render_rect_outline(renderer, btn_x, btn_y, btn_w, btn_h, 0x00A0FF, 1);

    char turn_buf[32];
    sprintf(turn_buf, "TURN: %d | END", game->current_turn);
    civ_font_render_aligned(renderer, font_hud, turn_buf, btn_x, btn_y, btn_w,
                            btn_h, 0xFFFFFF, CIV_ALIGN_CENTER,
                            CIV_VALIGN_MIDDLE);

    /* 5. Minimap (Bottom-Left) */
    int mm_w = 200;
    int mm_h = 150;
    int mm_x = 20;
    int mm_y = win_h - mm_h - 20;
    civ_render_minimap(renderer, mm_x, mm_y, mm_w, mm_h, game->world_map,
                       map_ctx);

    /* 6. Unit Sidebar (Left) */
    if (selected_unit) {
      int sb_w = 280;
      int sb_h = 320;
      int sb_x = 20;
      int sb_y = 60;

      civ_render_rect_filled_alpha(renderer, sb_x, sb_y, sb_w, sb_h, 0x050A14,
                                   230);
      civ_render_rect_outline(renderer, sb_x, sb_y, sb_w, sb_h, 0x00A0FF, 1);
      civ_render_line(renderer, sb_x, sb_y + 40, sb_x + sb_w, sb_y + 40,
                      0x1A2A3A);

      char name_lvl[128];
      sprintf(name_lvl, "%s [LVL %d]", selected_unit->name,
              selected_unit->level);
      civ_font_render_aligned(renderer, font_hud, name_lvl, sb_x + 15, sb_y,
                              sb_w - 30, 40, 0xFFFFFF, CIV_ALIGN_LEFT,
                              CIV_VALIGN_MIDDLE);

      char stat_buf[128];
      int curr_y = sb_y + 55;

      sprintf(stat_buf, "TYPE: %s",
              (selected_unit->unit_type == CIV_UNIT_TYPE_INFANTRY
                   ? "Infantry"
                   : (selected_unit->unit_type == CIV_UNIT_TYPE_SETTLER
                          ? "Settler"
                          : "Military")));
      civ_font_render_aligned(renderer, font_hud, stat_buf, sb_x + 15, curr_y,
                              sb_w - 30, 25, 0xCCCCCC, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
      curr_y += 30;

      sprintf(stat_buf, "STRENGTH: %d / %d", selected_unit->current_strength,
              selected_unit->max_strength);
      civ_font_render_aligned(renderer, font_hud, stat_buf, sb_x + 15, curr_y,
                              sb_w - 30, 25, 0xCCCCCC, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
      curr_y += 30;

      sprintf(stat_buf, "MORALE: %.0f%%", selected_unit->morale * 100.0f);
      civ_font_render_aligned(renderer, font_hud, stat_buf, sb_x + 15, curr_y,
                              sb_w - 30, 25, 0xCCCCCC, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
      curr_y += 30;

      sprintf(stat_buf, "POS: (%d, %d)", selected_unit->x, selected_unit->y);
      civ_font_render_aligned(renderer, font_hud, stat_buf, sb_x + 15, curr_y,
                              sb_w - 30, 25, 0xCCCCCC, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
      curr_y += 40;

      /* Movement Status */
      if (selected_unit->has_moved) {
        civ_font_render_aligned(renderer, font_hud, "MOVEMENT EXHAUSTED",
                                sb_x + 15, curr_y, sb_w - 30, 25, 0xFF4444,
                                CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
      } else {
        civ_font_render_aligned(renderer, font_hud, "READY TO MOVE", sb_x + 15,
                                curr_y, sb_w - 30, 25, 0x44FF44, CIV_ALIGN_LEFT,
                                CIV_VALIGN_TOP);
      }
      curr_y += 35;

      /* XP Bar */
      civ_font_render_aligned(renderer, font_hud, "EXPERIENCE", sb_x + 15,
                              curr_y, sb_w - 30, 20, 0xAAAAAA, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
      curr_y += 20;

      float xp_perc = selected_unit->experience / selected_unit->next_level_xp;
      civ_render_rect_filled(renderer, sb_x + 15, curr_y, sb_w - 30, 10,
                             0x1A2A3A);
      civ_render_rect_filled(renderer, sb_x + 15, curr_y,
                             (int)((sb_w - 30) * xp_perc), 10, 0xFFCC00);
      curr_y += 20;

      /* FOUND CITY BUTTON for Settlers */
      if (selected_unit->unit_type == CIV_UNIT_TYPE_SETTLER &&
          !selected_unit->has_moved) {
        int f_btn_x = sb_x + 15;
        int f_btn_y = sb_y + 250;
        int f_btn_w = sb_w - 30;
        int f_btn_h = 40;

        bool f_hov =
            (input->mouse_x >= f_btn_x && input->mouse_x <= f_btn_x + f_btn_w &&
             input->mouse_y >= f_btn_y && input->mouse_y <= f_btn_y + f_btn_h);

        civ_render_rect_filled(renderer, f_btn_x, f_btn_y, f_btn_w, f_btn_h,
                               f_hov ? 0x00A0FF : 0x005A99);
        civ_font_render_aligned(renderer, font_hud, "FOUND CITY", f_btn_x,
                                f_btn_y, f_btn_w, f_btn_h, 0xFFFFFF,
                                CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
      }

      /* Close hint */
      civ_font_render_aligned(renderer, font_hud, "(Click map to deselect)",
                              sb_x + 15, sb_y + sb_h - 25, sb_w - 30, 20,
                              0x666666, CIV_ALIGN_CENTER, CIV_VALIGN_BOTTOM);
    }

    /* 7. Settlement Sidebar (Left) */
    if (selected_settlement) {
      int ssb_w = 280;
      int ssb_h = 320;
      int ssb_x = 20;
      int ssb_y = 60;

      civ_render_rect_filled_alpha(renderer, ssb_x, ssb_y, ssb_w, ssb_h,
                                   0x0A0F1E, 240);
      civ_render_rect_outline(renderer, ssb_x, ssb_y, ssb_w, ssb_h, 0xFFFF00,
                              1);
      civ_render_line(renderer, ssb_x, ssb_y + 40, ssb_x + ssb_w, ssb_y + 40,
                      0x1A2A3A);

      civ_font_render_aligned(renderer, font_hud, selected_settlement->name,
                              ssb_x + 15, ssb_y, ssb_w - 30, 40, 0xFFFF00,
                              CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

      char stat_buf[128];
      int curr_y = ssb_y + 55;

      const char *tier_names[] = {"Hamlet", "Village",    "Town",
                                  "City",   "Metropolis", "Capital"};
      sprintf(stat_buf, "TIER: %s", tier_names[selected_settlement->tier]);
      civ_font_render_aligned(renderer, font_hud, stat_buf, ssb_x + 15, curr_y,
                              ssb_w - 30, 30, 0xFFFFFF, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
      curr_y += 25;

      sprintf(stat_buf, "POPULATION: %lld", selected_settlement->population);
      civ_font_render_aligned(renderer, font_hud, stat_buf, ssb_x + 15, curr_y,
                              ssb_w - 30, 30, 0xFFFFFF, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
      curr_y += 25;

      sprintf(stat_buf, "ATTRACTIVENESS: %.2f",
              selected_settlement->attractiveness);
      civ_font_render_aligned(renderer, font_hud, stat_buf, ssb_x + 15, curr_y,
                              ssb_w - 30, 30, 0xFFFFFF, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
      curr_y += 25;

      /* Phase 9: Loyalty & Unrest */
      civ_font_render_aligned(renderer, font_hud, "LOYALTY", ssb_x + 15, curr_y,
                              ssb_w - 30, 15, 0xAAAAAA, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
      uint32_t loyalty_color =
          (selected_settlement->loyalty > 0.4f) ? 0x00FF88 : 0xFF3300;
      civ_render_rect_filled(renderer, ssb_x + 15, curr_y + 18,
                             (int)((ssb_w - 30) * selected_settlement->loyalty),
                             6, loyalty_color);
      civ_render_rect_outline(renderer, ssb_x + 15, curr_y + 18, ssb_w - 30, 6,
                              0x444444, 1);
      curr_y += 35;

      civ_font_render_aligned(renderer, font_hud, "UNREST", ssb_x + 15, curr_y,
                              ssb_w - 30, 15, 0xAAAAAA, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
      civ_render_rect_filled(renderer, ssb_x + 15, curr_y + 18,
                             (int)((ssb_w - 30) * selected_settlement->unrest),
                             6, 0xFF6600);
      civ_render_rect_outline(renderer, ssb_x + 15, curr_y + 18, ssb_w - 30, 6,
                              0x444444, 1);
      curr_y += 35;

      /* Culture Stats */
      sprintf(stat_buf, "CULTURE: %.1f (+%.1f/YR)",
              selected_settlement->accumulated_culture,
              selected_settlement->culture_yield);
      civ_font_render_aligned(renderer, font_hud, stat_buf, ssb_x + 15, curr_y,
                              ssb_w - 30, 30, 0x00FFCC, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
      curr_y += 25;

      float next_threshold = 0;
      if (selected_settlement->territory_radius == 2)
        next_threshold = 20.0f;
      else if (selected_settlement->territory_radius == 3)
        next_threshold = 100.0f;
      else if (selected_settlement->territory_radius == 4)
        next_threshold = 500.0f;

      if (next_threshold > 0) {
        sprintf(stat_buf, "EXPANSION: %.0f%%",
                (selected_settlement->accumulated_culture / next_threshold) *
                    100.0f);
        civ_font_render_aligned(renderer, font_hud, stat_buf, ssb_x + 15,
                                curr_y, ssb_w - 30, 30, 0x00FFCC,
                                CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
      }
      curr_y += 35;

      /* Recruitment Section */
      civ_render_line(renderer, ssb_x + 10, curr_y, ssb_x + ssb_w - 10, curr_y,
                      0x336644);
      curr_y += 10;
      civ_font_render_aligned(renderer, font_hud, "RECRUIT UNITS", ssb_x + 15,
                              curr_y, ssb_w - 30, 20, 0x00FF88, CIV_ALIGN_LEFT,
                              CIV_VALIGN_TOP);
      curr_y += 25;

      if (selected_settlement->is_producing) {
        float perc = selected_settlement->production_progress /
                     selected_settlement->production_target;
        civ_render_rect_filled(renderer, ssb_x + 15, curr_y, ssb_w - 30, 25,
                               0x1A2A1A);
        civ_render_rect_filled(renderer, ssb_x + 15, curr_y,
                               (int)((ssb_w - 30) * perc), 25, 0x00A0FF);
        char prod_buf[64];
        sprintf(prod_buf, "TRAINING... %d%%", (int)(perc * 100));
        civ_font_render_aligned(renderer, font_hud, prod_buf, ssb_x + 15,
                                curr_y, ssb_w - 30, 25, 0xFFFFFF,
                                CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
      } else {
        /* Option: Infantry */
        bool h_inf =
            (input->mouse_x >= ssb_x + 10 &&
             input->mouse_x <= ssb_x + ssb_w - 10 && input->mouse_y >= curr_y &&
             input->mouse_y <= curr_y + 40);
        civ_render_rect_filled(renderer, ssb_x + 10, curr_y, ssb_w - 20, 40,
                               h_inf ? 0x2A3A2A : 0x1A2A1A);
        civ_font_render_aligned(renderer, font_hud, "RECRUIT INFANTRY (30)",
                                ssb_x + 15, curr_y, ssb_w - 30, 40, 0xFFFFFF,
                                CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
        curr_y += 50;

        /* Option: Settler */
        if (selected_settlement->population > 500) {
          bool h_set =
              (input->mouse_x >= ssb_x + 10 &&
               input->mouse_x <= ssb_x + ssb_w - 10 &&
               input->mouse_y >= curr_y && input->mouse_y <= curr_y + 40);
          civ_render_rect_filled(renderer, ssb_x + 10, curr_y, ssb_w - 20, 40,
                                 h_set ? 0x2A3A2A : 0x1A2A1A);
          civ_font_render_aligned(renderer, font_hud, "RECRUIT SETTLER (80)",
                                  ssb_x + 15, curr_y, ssb_w - 30, 40, 0xFFFFFF,
                                  CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
        } else {
          civ_font_render_aligned(renderer, font_hud, "SETTLER (Needs 500 Pop)",
                                  ssb_x + 15, curr_y, ssb_w - 30, 40, 0x666666,
                                  CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);
        }
      }
    }

    /* 8. Diplomacy Panel (Left) */
    if (!selected_unit && !selected_settlement && show_diplomacy) {
      int dsb_w = 280;
      int dsb_h = 450;
      int dsb_x = 20;
      int dsb_y = 60;

      civ_render_rect_filled_alpha(renderer, dsb_x, dsb_y, dsb_w, dsb_h,
                                   0x140505, 240);
      civ_render_rect_outline(renderer, dsb_x, dsb_y, dsb_w, dsb_h, 0xFFFF00,
                              1);
      civ_render_line(renderer, dsb_x, dsb_y + 40, dsb_x + dsb_w, dsb_y + 40,
                      0x3A1A1A);

      civ_font_render_aligned(renderer, font_hud, "DIPLOMACY & RELATIONS",
                              dsb_x + 15, dsb_y, dsb_w - 30, 40, 0xFFFF00,
                              CIV_ALIGN_LEFT, CIV_VALIGN_MIDDLE);

      int curr_y = dsb_y + 55;
      if (has_contacted_rival) {
        civ_diplomatic_relation_t *rel = civ_diplomacy_system_get_relation(
            game->diplomacy_system, "player", "rival_kingdom");

        civ_font_render_aligned(renderer, font_hud, "RIVAL KINGDOM", dsb_x + 15,
                                curr_y, dsb_w - 30, 30, 0xFFFFFF,
                                CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
        curr_y += 25;

        if (rel) {
          char status_buf[64];
          const char *lvl_str = "Neutral";
          if (rel->relation_level == CIV_RELATION_LEVEL_WAR)
            lvl_str = "WAR";
          else if (rel->relation_level == CIV_RELATION_LEVEL_HOSTILE)
            lvl_str = "Hostile";
          else if (rel->relation_level == CIV_RELATION_LEVEL_FRIENDLY)
            lvl_str = "Friendly";
          else if (rel->relation_level == CIV_RELATION_LEVEL_ALLIED)
            lvl_str = "Allied";

          sprintf(status_buf, "STATUS: %s (Opinion: %.0f)", lvl_str,
                  rel->opinion_score);
          civ_font_render_aligned(renderer, font_hud, status_buf, dsb_x + 15,
                                  curr_y, dsb_w - 30, 30, 0xAAAAAA,
                                  CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
          curr_y += 25;

          const char *stance_str = "Neutral";
          uint32_t stance_color = 0xAAAAAA;
          if (rel->current_stance == CIV_STANCE_HOSTILE) {
            stance_str = "HOSTILE";
            stance_color = 0xFF4444;
          } else if (rel->current_stance == CIV_STANCE_WARY) {
            stance_str = "WARY";
            stance_color = 0xFFCC00;
          } else if (rel->current_stance == CIV_STANCE_FRIENDLY) {
            stance_str = "FRIENDLY";
            stance_color = 0x44FF44;
          }

          sprintf(status_buf, "STANCE: %s", stance_str);
          civ_font_render_aligned(renderer, font_hud, status_buf, dsb_x + 15,
                                  curr_y, dsb_w - 30, 30, stance_color,
                                  CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
          curr_y += 25;

          const char *p_str = "Balanced";
          if (rel->personality == CIV_PERSONALITY_AGGRESSIVE)
            p_str = "Aggressive";
          else if (rel->personality == CIV_PERSONALITY_EXPANSIONIST)
            p_str = "Expansionist";
          else if (rel->personality == CIV_PERSONALITY_MERCANTILE)
            p_str = "Mercantile";

          sprintf(status_buf, "PERSONALITY: %s", p_str);
          civ_font_render_aligned(renderer, font_hud, status_buf, dsb_x + 15,
                                  curr_y, dsb_w - 30, 30, 0x00FFCC,
                                  CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
        }
        curr_y += 35;

        /* Propose Trade Action */
        bool h_trade =
            (input->mouse_x >= dsb_x + 10 &&
             input->mouse_x <= dsb_x + dsb_w - 10 && input->mouse_y >= curr_y &&
             input->mouse_y <= curr_y + 40);
        civ_render_rect_filled(renderer, dsb_x + 10, curr_y, dsb_w - 20, 40,
                               h_trade ? 0x2A3A4A : 0x1A2A3A);
        civ_font_render_aligned(renderer, font_hud, "PROPOSE TRADE AGREEMENT",
                                dsb_x + 10, curr_y, dsb_w - 20, 40, 0xFFFFFF,
                                CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
        curr_y += 50;

        /* Propose Non-Aggression Action */
        bool h_nap =
            (input->mouse_x >= dsb_x + 10 &&
             input->mouse_x <= dsb_x + dsb_w - 10 && input->mouse_y >= curr_y &&
             input->mouse_y <= curr_y + 40);
        civ_render_rect_filled(renderer, dsb_x + 10, curr_y, dsb_w - 20, 40,
                               h_nap ? 0x2A3A4A : 0x1A2A3A);
        civ_font_render_aligned(renderer, font_hud, "NON-AGGRESSION PACT",
                                dsb_x + 10, curr_y, dsb_w - 20, 40, 0xFFFFFF,
                                CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
        curr_y += 50;

        /* Declare War Action */
        bool h_war =
            (input->mouse_x >= dsb_x + 10 &&
             input->mouse_x <= dsb_x + dsb_w - 10 && input->mouse_y >= curr_y &&
             input->mouse_y <= curr_y + 40);
        civ_render_rect_filled(renderer, dsb_x + 10, curr_y, dsb_w - 20, 40,
                               h_war ? 0x660000 : 0x440000);
        civ_font_render_aligned(renderer, font_hud, "DECLARE WAR", dsb_x + 10,
                                curr_y, dsb_w - 20, 40, 0xFFFFFF,
                                CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
      } else {
        civ_font_render_aligned(renderer, font_hud, "NO NATIONS DISCOVERED",
                                dsb_x + 15, curr_y, dsb_w - 30, 30, 0x666666,
                                CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
      }
    }

    /* 9. Research Panel (Centered Overlay) */
    if (show_research && game->technology_tree) {
      int win_w = last_win_w;
      int win_h = last_win_h;
      int rpb_w = 600;
      int rpb_h = 500;
      int rpb_x = (win_w - rpb_w) / 2;
      int rpb_y = (win_h - rpb_h) / 2;

      civ_render_rect_filled_alpha(renderer, rpb_x, rpb_y, rpb_w, rpb_h,
                                   0x0A0F1A, 250);
      civ_render_rect_outline(renderer, rpb_x, rpb_y, rpb_w, rpb_h, 0x00A0FF,
                              2);
      civ_render_line(renderer, rpb_x, rpb_y + 50, rpb_x + rpb_w, rpb_y + 50,
                      0x1A2A3A);

      civ_font_render_aligned(
          renderer, font_hud, "TECHNOLOGY & INNOVATION TREE", rpb_x + 20, rpb_y,
          rpb_w - 40, 50, 0xFFFFFF, CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);

      civ_innovation_system_t *is = game->technology_tree;
      int curr_y = rpb_y + 70;
      int tech_box_h = 60;

      for (size_t i = 0; i < is->tech_count; i++) {
        civ_technology_node_t *t = &is->technologies[i];

        uint32_t bg_color = 0x1A2A3A;
        bool is_current =
            is->current_research && strcmp(is->current_research, t->id) == 0;

        if (t->researched)
          bg_color = 0x004422;
        else if (is_current)
          bg_color = 0x0055AA;

        /* Hover check for visual feedback */
        bool is_hover =
            (input->mouse_x >= rpb_x + 20 &&
             input->mouse_x <= rpb_x + rpb_w - 20 && input->mouse_y >= curr_y &&
             input->mouse_y <= curr_y + tech_box_h);
        if (is_hover)
          bg_color += 0x111111;

        civ_render_rect_filled(renderer, rpb_x + 20, curr_y, rpb_w - 40,
                               tech_box_h, bg_color);
        civ_render_rect_outline(renderer, rpb_x + 20, curr_y, rpb_w - 40,
                                tech_box_h, 0x00A0FF, 1);

        civ_font_render_aligned(renderer, font_hud, t->name, rpb_x + 40,
                                curr_y + 10, rpb_w - 80, 20, 0xFFFFFF,
                                CIV_ALIGN_LEFT, CIV_VALIGN_TOP);

        if (t->researched) {
          civ_font_render_aligned(renderer, font_hud, "COMPLETED", rpb_x + 40,
                                  curr_y + 30, rpb_w - 80, 20, 0x00FF88,
                                  CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
        } else if (is_current) {
          float perc = (t->progress / t->base_research_cost);
          civ_render_rect_filled(renderer, rpb_x + 40, curr_y + 40,
                                 (int)((rpb_w - 80) * perc), 6, 0x00FF00);
          civ_font_render_aligned(renderer, font_hud, "CURRENTLY RESEARCHING",
                                  rpb_x + 40, curr_y + 25, rpb_w - 80, 20,
                                  0x00A0FF, CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
        } else {
          char cost_buf[64];
          sprintf(cost_buf, "COST: %.1f SCIENCE", t->base_research_cost);
          civ_font_render_aligned(renderer, font_hud, cost_buf, rpb_x + 40,
                                  curr_y + 30, rpb_w - 80, 20, 0xCCCCCC,
                                  CIV_ALIGN_LEFT, CIV_VALIGN_TOP);
        }

        curr_y += tech_box_h + 15;
      }

      /* Help text */
      civ_font_render_aligned(renderer, font_hud, "[ESC/G TO CLOSE]", win_w / 2,
                              rpb_y + rpb_h - 40, 200, 30, 0x888888,
                              CIV_ALIGN_CENTER, CIV_VALIGN_MIDDLE);
    }

    if (show_government && game->government) {
      render_governance_panel(renderer, game->government);
    }

    if (show_wonders) {
      render_wonders_panel(renderer, game->wonder_manager);
    }

    if (show_rulebook && game->government) {
      render_rulebook_editor(renderer, game->government);
    }

    /* Presentation is handled by the main loop */
  }
}

static void destroy(void) {
  if (map_ctx) {
    civ_render_map_context_destroy(map_ctx);
    map_ctx = NULL;
  }
  if (font_hud) {
    civ_font_destroy(font_hud);
    font_hud = NULL;
  }
}

civ_scene_t scene_game = {.init = init,
                          .update = update,
                          .render = render,
                          .destroy = destroy,
                          .next_scene_id = -1};
