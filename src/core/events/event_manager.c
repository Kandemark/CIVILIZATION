/**
 * @file event_manager.c
 * @brief Implementation of event management system
 */

#include "../../../include/core/events/event_manager.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_event_manager_t* civ_event_manager_create(void) {
    civ_event_manager_t* em = (civ_event_manager_t*)CIV_MALLOC(sizeof(civ_event_manager_t));
    if (!em) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate event manager");
        return NULL;
    }
    
    civ_event_manager_init(em);
    return em;
}

void civ_event_manager_destroy(civ_event_manager_t* em) {
    if (!em) return;
    
    /* Free all handlers */
    civ_event_handler_t* handler = em->handlers;
    while (handler) {
        civ_event_handler_t* next = handler->next;
        CIV_FREE(handler);
        handler = next;
    }
    
    /* Free events */
    if (em->events) {
        for (size_t i = 0; i < em->event_count; i++) {
            if (em->events[i].data) {
                CIV_FREE(em->events[i].data);
            }
        }
        CIV_FREE(em->events);
    }
    
    CIV_FREE(em);
}

void civ_event_manager_init(civ_event_manager_t* em) {
    if (!em) return;
    
    memset(em, 0, sizeof(civ_event_manager_t));
    em->event_capacity = 100;
    em->events = (civ_game_event_t*)CIV_CALLOC(em->event_capacity, sizeof(civ_game_event_t));
    em->last_update = time(NULL);
}

civ_result_t civ_event_manager_register_handler(civ_event_manager_t* em, civ_event_type_t type,
                                                 civ_event_handler_cb_t callback, void* user_data) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!em || !callback) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    civ_event_handler_t* handler = (civ_event_handler_t*)CIV_MALLOC(sizeof(civ_event_handler_t));
    if (!handler) {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
        return result;
    }
    
    handler->event_type = type;
    handler->callback = callback;
    handler->user_data = user_data;
    handler->next = em->handlers;
    em->handlers = handler;
    
    return result;
}

civ_result_t civ_event_manager_emit_event(civ_event_manager_t* em, const civ_game_event_t* event) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!em || !event) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Store event */
    if (em->event_count >= em->event_capacity) {
        em->event_capacity *= 2;
        em->events = (civ_game_event_t*)CIV_REALLOC(em->events, 
                                                    em->event_capacity * sizeof(civ_game_event_t));
    }
    
    if (em->events) {
        em->events[em->event_count++] = *event;
    }
    
    /* Notify handlers */
    civ_event_handler_t* handler = em->handlers;
    while (handler) {
        if (handler->event_type == event->type || handler->event_type == -1) {
            handler->callback(event, handler->user_data);
        }
        handler = handler->next;
    }
    
    return result;
}

civ_result_t civ_event_manager_create_event(civ_event_manager_t* em, civ_event_type_t type,
                                            const char* title, const char* description,
                                            civ_float_t importance) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!em || !title || !description) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    civ_game_event_t event = {0};
    snprintf(event.event_id, sizeof(event.event_id), "event_%zu", em->event_count);
    event.type = type;
    strncpy(event.title, title, sizeof(event.title) - 1);
    strncpy(event.description, description, sizeof(event.description) - 1);
    event.importance = CLAMP(importance, 0.0f, 1.0f);
    event.timestamp = time(NULL);
    event.active = true;
    
    return civ_event_manager_emit_event(em, &event);
}

void civ_event_manager_update(civ_event_manager_t* em, civ_float_t time_delta) {
    if (!em) return;
    
    /* Update event manager (could process scheduled events, etc.) */
    time_t current_time = time(NULL);
    em->last_update = current_time;
}

