/**
 * @file event_dispatcher.h
 * @brief Event dispatcher for efficient event handling
 */

#ifndef CIVILIZATION_EVENT_DISPATCHER_H
#define CIVILIZATION_EVENT_DISPATCHER_H

#include "../../common.h"
#include "../../types.h"

/* Event handler function pointer */
typedef void (*civ_event_handler_t)(const char* event_type, void* event_data, void* user_data);

/* Event dispatcher */
typedef struct {
    civ_event_handler_t* handlers;
    char** event_types;
    void** user_data;
    size_t handler_count;
    size_t handler_capacity;
} civ_event_dispatcher_t;

/* Function declarations */
civ_event_dispatcher_t* civ_event_dispatcher_create(void);
void civ_event_dispatcher_destroy(civ_event_dispatcher_t* ed);
void civ_event_dispatcher_init(civ_event_dispatcher_t* ed);

civ_result_t civ_event_dispatcher_register(civ_event_dispatcher_t* ed, const char* event_type,
                                          civ_event_handler_t handler, void* user_data);
void civ_event_dispatcher_unregister(civ_event_dispatcher_t* ed, const char* event_type);
civ_result_t civ_event_dispatcher_dispatch(civ_event_dispatcher_t* ed, const char* event_type, void* event_data);

#endif /* CIVILIZATION_EVENT_DISPATCHER_H */

