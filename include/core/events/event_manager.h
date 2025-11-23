/**
 * @file event_manager.h
 * @brief Event management system
 */

#ifndef CIVILIZATION_EVENT_MANAGER_H
#define CIVILIZATION_EVENT_MANAGER_H

#include "../../common.h"
#include "../../types.h"

/* Event type enumeration */
typedef enum {
    CIV_EVENT_TYPE_NATURAL = 0,
    CIV_EVENT_TYPE_POLITICAL,
    CIV_EVENT_TYPE_ECONOMIC,
    CIV_EVENT_TYPE_MILITARY,
    CIV_EVENT_TYPE_SOCIAL
} civ_event_type_t;

/* Event structure */
typedef struct {
    char event_id[STRING_SHORT_LEN];
    civ_event_type_t type;
    char title[STRING_MEDIUM_LEN];
    char description[STRING_MAX_LEN];
    civ_float_t importance;
    time_t timestamp;
    bool active;
    void* data;  /* Event-specific data */
} civ_game_event_t;

/* Event handler callback */
typedef void (*civ_event_handler_cb_t)(const civ_game_event_t* event, void* user_data);

/* Event handler structure */
typedef struct civ_event_handler {
    civ_event_type_t event_type;
    civ_event_handler_cb_t callback;
    void* user_data;
    struct civ_event_handler* next;
} civ_event_handler_t;

/* Event manager structure */
typedef struct {
    civ_game_event_t* events;
    size_t event_count;
    size_t event_capacity;
    civ_event_handler_t* handlers;
    time_t last_update;
} civ_event_manager_t;

/* Function declarations */
civ_event_manager_t* civ_event_manager_create(void);
void civ_event_manager_destroy(civ_event_manager_t* em);
void civ_event_manager_init(civ_event_manager_t* em);

civ_result_t civ_event_manager_register_handler(civ_event_manager_t* em, civ_event_type_t type,
                                                civ_event_handler_cb_t callback, void* user_data);
civ_result_t civ_event_manager_emit_event(civ_event_manager_t* em, const civ_game_event_t* event);
civ_result_t civ_event_manager_create_event(civ_event_manager_t* em, civ_event_type_t type,
                                            const char* title, const char* description,
                                            civ_float_t importance);
void civ_event_manager_update(civ_event_manager_t* em, civ_float_t time_delta);

#endif /* CIVILIZATION_EVENT_MANAGER_H */

