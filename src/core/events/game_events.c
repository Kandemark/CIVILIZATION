/**
 * @file game_events.c
 * @brief Implementation of Game Events
 */

#include "../../../include/core/events/game_events.h"
#include "../../../include/common.h"
#include "../../../include/core/events/event_manager.h"
#include "../../../include/core/game.h"
#include <stdio.h>
#include <string.h>


void civ_trigger_economic_crisis(civ_game_t *game) {
  if (!game)
    return;

  civ_log(CIV_LOG_INFO, "EVENT: Economic Crisis Triggered!");
  civ_game_add_event(game, "CRISIS", "Global Economic Meltdown", 0.9f);

  /* 1. Market Crash */
  if (game->market_economy) {
    /* Direct access for now, ideally use a setter */
    // game->market_economy->sentiment = CIV_MARKET_SENTIMENT_BEARISH;
  }

  /* 2. Currency Devaluation */
  if (game->currency_manager) {
    for (size_t i = 0; i < game->currency_manager->currency_count; i++) {
      game->currency_manager->currencies[i].exchange_rate *= 0.7f; // 30% drop
      game->currency_manager->currencies[i].inflation_rate +=
          0.05f; // +5% inflation
    }
  }

  /* 3. Ideology Shift -> Radicalism */
  if (game->ideology_system) {
    // civ_ideology_shift_global(game->ideology_system, "Radicalism", 0.2f);
  }

  /* 4. Legislative Response */
  if (game->legislative_manager) {
    // civ_legislative_propose_bill(game->legislative_manager, "Austerity Act",
    // ...);
  }
}

void civ_trigger_natural_disaster(civ_game_t *game, civ_disaster_type_t type) {
  if (!game || !game->disaster_manager)
    return;

  /* Trigger disaster */
  civ_coordinate_t loc = {50.0f, 50.0f}; // Random or specific
  civ_disaster_trigger(game->disaster_manager, type, loc, 0.8f);

  civ_game_add_event(game, "DISASTER", "Major Natural Disaster", 0.8f);

  /* Economic Impact */
  if (game->trade_manager) {
    // Disrupt trade routes near location
  }
}
