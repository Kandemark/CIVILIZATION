/* events.h - Earthquake, volcano, storm, drought generation */
#ifndef EVENTS_H
#define EVENTS_H

#include "cell.h"
#include "utils.h"


typedef enum {
  EVENT_NONE,
  EVENT_EARTHQUAKE,
  EVENT_VOLCANO,
  EVENT_STORM,
  EVENT_DROUGHT
} EventType;

typedef struct {
  EventType type;
  int x;
  int y;
  int duration; // turns remaining
} Event;

typedef struct {
  Event active_events[32]; // simple fixed-size pool
  int count;
} EventsState;

/* API */
void events_initialize(EventsState *ev);
void events_update(EventsState *ev, WorldCell cells[WORLD_HEIGHT][WORLD_WIDTH],
                   UtilsState *utils);

#endif // EVENTS_H
