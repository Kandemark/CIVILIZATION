/* events.c - Simple random natural events handling */
#include "../../include/core/environment/events.h"
#include "../../include/core/environment/cell.h"
#include "../../include/utils/config.h"
#include "../../include/utils/utils.h"
#include <stdio.h>
#include <stdlib.h>

/* Helper to spawn a single event at (x,y) */
static void spawn_event(EventsState *ev, EventType type, int x, int y,
                        int duration) {
  if (!ev)
    return;
  if (ev->count >= 32)
    return; // pool full
  Event *e = &ev->active_events[ev->count++];
  e->type = type;
  e->x = x;
  e->y = y;
  e->duration = duration;
}

void events_initialize(EventsState *ev) {
  if (!ev)
    return;
  ev->count = 0;
}

/* Simple event logic:
   - Earthquake: random chance based on tectonic stress (here we use
   utils_rand_float)
   - Volcano: rare, random location on land
   - Storm: more likely over water and with high wind speed (not modeled here)
   - Drought: occurs on desert cells when rainfall is low (handled elsewhere)
   For demo we only generate earthquakes and occasional storms.
*/
void events_update(EventsState *ev, WorldCell cells[WORLD_HEIGHT][WORLD_WIDTH],
                   UtilsState *utils) {
  if (!ev || !utils)
    return;
  // Decrease duration of existing events and remove expired ones
  int write = 0;
  for (int i = 0; i < ev->count; ++i) {
    Event *e = &ev->active_events[i];
    e->duration--;
    if (e->duration > 0) {
      ev->active_events[write++] = *e; // keep alive
    }
  }
  ev->count = write;

  // Random earthquake generation based on tectonic stress
  for (int y = 0; y < WORLD_HEIGHT; ++y) {
    for (int x = 0; x < WORLD_WIDTH; ++x) {
      float stress = cells[y][x].tectonic_stress;
      // Simple probability: higher stress -> higher chance
      if (utils_rand_float(utils) < stress * 0.001f) { // scale down
        spawn_event(ev, EVENT_EARTHQUAKE, x, y, 2);
        // Reset stress after quake
        cells[y][x].tectonic_stress = 0.0f;
      }
    }
  }

  // Random storm generation over water cells
  for (int y = 1; y < WORLD_HEIGHT - 1; ++y) {
    for (int x = 1; x < WORLD_WIDTH - 1; ++x) {
      if (cells[y][x].terrain == TERRAIN_OCEAN) {
        if (utils_rand_float(utils) < 0.0005f) { // rare storm
          spawn_event(ev, EVENT_STORM, x, y, 3);
        }
      }
    }
  }
}
