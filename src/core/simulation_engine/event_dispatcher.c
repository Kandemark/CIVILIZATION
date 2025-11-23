/**
 * @file event_dispatcher.c
 * @brief Implementation of event dispatcher
 */

#include "../../../include/core/simulation_engine/event_dispatcher.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>

civ_event_dispatcher_t* civ_event_dispatcher_create(void) {
    civ_event_dispatcher_t* ed = (civ_event_dispatcher_t*)CIV_MALLOC(sizeof(civ_event_dispatcher_t));
    if (!ed) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate event dispatcher");
        return NULL;
    }
    
    civ_event_dispatcher_init(ed);
    return ed;
}

void civ_event_dispatcher_destroy(civ_event_dispatcher_t* ed) {
    if (!ed) return;
    
    CIV_FREE(ed->handlers);
    if (ed->event_types) {
        for (size_t i = 0; i < ed->handler_count; i++) {
            CIV_FREE(ed->event_types[i]);
        }
        CIV_FREE(ed->event_types);
    }
    CIV_FREE(ed->user_data);
    CIV_FREE(ed);
}

void civ_event_dispatcher_init(civ_event_dispatcher_t* ed) {
    if (!ed) return;
    
    memset(ed, 0, sizeof(civ_event_dispatcher_t));
    ed->handler_capacity = 32;
    ed->handlers = (civ_event_handler_t*)CIV_CALLOC(ed->handler_capacity, sizeof(civ_event_handler_t));
    ed->event_types = (char**)CIV_CALLOC(ed->handler_capacity, sizeof(char*));
    ed->user_data = (void**)CIV_CALLOC(ed->handler_capacity, sizeof(void*));
}

civ_result_t civ_event_dispatcher_register(civ_event_dispatcher_t* ed, const char* event_type,
                                           civ_event_handler_t handler, void* user_data) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!ed || !event_type || !handler) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Expand if needed */
    if (ed->handler_count >= ed->handler_capacity) {
        ed->handler_capacity *= 2;
        ed->handlers = (civ_event_handler_t*)CIV_REALLOC(ed->handlers, ed->handler_capacity * sizeof(civ_event_handler_t));
        ed->event_types = (char**)CIV_REALLOC(ed->event_types, ed->handler_capacity * sizeof(char*));
        ed->user_data = (void**)CIV_REALLOC(ed->user_data, ed->handler_capacity * sizeof(void*));
    }
    
    /* Register handler */
    size_t name_len = strlen(event_type) + 1;
    ed->event_types[ed->handler_count] = (char*)CIV_MALLOC(name_len);
    if (ed->event_types[ed->handler_count]) {
        strcpy(ed->event_types[ed->handler_count], event_type);
        ed->handlers[ed->handler_count] = handler;
        ed->user_data[ed->handler_count] = user_data;
        ed->handler_count++;
    } else {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
    }
    
    return result;
}

void civ_event_dispatcher_unregister(civ_event_dispatcher_t* ed, const char* event_type) {
    if (!ed || !event_type) return;
    
    for (size_t i = 0; i < ed->handler_count; i++) {
        if (strcmp(ed->event_types[i], event_type) == 0) {
            CIV_FREE(ed->event_types[i]);
            memmove(&ed->event_types[i], &ed->event_types[i + 1], (ed->handler_count - i - 1) * sizeof(char*));
            memmove(&ed->handlers[i], &ed->handlers[i + 1], (ed->handler_count - i - 1) * sizeof(civ_event_handler_t));
            memmove(&ed->user_data[i], &ed->user_data[i + 1], (ed->handler_count - i - 1) * sizeof(void*));
            ed->handler_count--;
            return;
        }
    }
}

civ_result_t civ_event_dispatcher_dispatch(civ_event_dispatcher_t* ed, const char* event_type, void* event_data) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!ed || !event_type) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Dispatch to all matching handlers */
    for (size_t i = 0; i < ed->handler_count; i++) {
        if (strcmp(ed->event_types[i], event_type) == 0) {
            if (ed->handlers[i]) {
                ed->handlers[i](event_type, event_data, ed->user_data[i]);
            }
        }
    }
    
    return result;
}

