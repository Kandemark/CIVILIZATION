/**
 * @file cli.c
 * @brief Implementation of CLI
 */

#include "../../include/ui/cli.h"
#include "../../include/common.h"
#include "../../include/core/events/game_events.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef _WIN32
#include <conio.h>
#else
#include <sys/select.h>
#include <unistd.h>
int _kbhit() {
  struct timeval tv = {0L, 0L};
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(0, &fds);
  return select(1, &fds, NULL, NULL, &tv);
}
#endif

civ_cli_t *civ_cli_create(civ_game_t *game) {
  civ_cli_t *cli = CIV_MALLOC(sizeof(civ_cli_t));
  if (cli) {
    cli->game = game;
    cli->running = true;
  }
  return cli;
}

void civ_cli_destroy(civ_cli_t *cli) {
  if (cli) {
    CIV_FREE(cli);
  }
}

void civ_cli_render_status(civ_cli_t *cli) {
  if (!cli || !cli->game)
    return;

  civ_game_t *g = cli->game;

  // Clear screen (ANSI escape)
  printf("\033[2J\033[H");

  printf("=== CIVILIZATION SIMULATION ===\n");

  // Time
  if (g->time_manager) {
    char date_str[64];
    civ_calendar_get_date_string(&g->time_manager->calendar, date_str, 64);
    printf("Date: %s\n", date_str);
  }

  // Economy
  if (g->currency_manager && g->currency_manager->currency_count > 0) {
    printf("\n[ECONOMY]\n");
    civ_currency_t *c = &g->currency_manager->currencies[0]; // Player currency
    printf("Currency: %s (%s)\n", c->name, c->symbol);
    printf("Exchange Rate: %.2f | Inflation: %.1f%%\n", c->exchange_rate,
           c->inflation_rate * 100.0f);
  }

  // Disasters
  if (g->disaster_manager) {
    printf("\n[ACTIVE DISASTERS]\n");
    int count = 0;
    for (size_t i = 0; i < g->disaster_manager->disaster_count; i++) {
      if (g->disaster_manager->active_disasters[i].active) {
        printf("! %s (Severity: %.1f)\n",
               g->disaster_manager->active_disasters[i].name,
               g->disaster_manager->active_disasters[i].severity);
        count++;
      }
    }
    if (count == 0)
      printf("None\n");
  }

  // Events
  if (g->event_log_size > 0) {
    printf("\n[LATEST EVENTS]\n");
    int start = g->event_log_size > 5 ? g->event_log_size - 5 : 0;
    for (int i = start; i < g->event_log_size; i++) {
      printf("- %s: %s\n", g->event_log[i].type, g->event_log[i].description);
    }
  }

  printf("\n[CONTROLS]\n");
  printf("Space: Pause/Resume | E: Trigger Economic Crisis | D: Trigger "
         "Disaster | Q: Quit\n");
  printf("> ");
}

void civ_cli_handle_input(civ_cli_t *cli) {
  if (!cli || !cli->game)
    return;

  if (_kbhit()) {
    char c = getchar();
    switch (c) {
    case 'q':
    case 'Q':
      cli->running = false;
      cli->game->is_running = false;
      break;
    case ' ':
      if (cli->game->is_paused)
        civ_game_resume(cli->game);
      else
        civ_game_pause(cli->game);
      break;
    case 'e':
    case 'E':
      civ_trigger_economic_crisis(cli->game);
      break;
    case 'd':
    case 'D':
      civ_trigger_natural_disaster(cli->game, CIV_DISASTER_EARTHQUAKE);
      break;
    }
  }
}

void civ_cli_update(civ_cli_t *cli) {
  if (!cli)
    return;

  civ_cli_render_status(cli);
  civ_cli_handle_input(cli);
}
