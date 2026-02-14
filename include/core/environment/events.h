#ifndef CIVILIZATION_EVENTS_H
#define CIVILIZATION_EVENTS_H

#include "../../utils/utils.h"
#include "cell.h"
#include "world.h" // For dimensions


typedef enum {
  EVENT_NONE,
  EVENT_EARTHQUAKE,
  EVENT_VOLCANO,
  EVENT_STORM,
  EVENT_DROUGHT,
  EVENT_PLAGUE,
  EVENT_BOOM
} EventType;

typedef struct {
  EventType type;
  int x, y;
  int duration;
} Event;

typedef struct {
  Event active_events[32];
  int count;
} EventsState;

void events_initialize(EventsState *events);
void events_update(EventsState *ev, WorldCell cells[WORLD_HEIGHT][WORLD_WIDTH],
                   UtilsState *utils);

#endif /* CIVILIZATION_EVENTS_H */
