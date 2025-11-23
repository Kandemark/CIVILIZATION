/**
 * @file time_manager.c
 * @brief Implementation of the time management system
 */

#include "../../../include/core/simulation_engine/time_manager.h"
#include "../../../include/common.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* Time scale multipliers (days per real second) */
static const civ_float_t TIME_SCALE_MULTIPLIERS[] = {
    0.0,      /* PAUSED */
    1.0,      /* SLOW - 1 day per second */
    2.0,      /* NORMAL - 2 days per second */
    10.0,     /* FAST - 10 days per second */
    100.0,    /* VERY_FAST - 100 days per second */
    1000.0    /* ULTRA - 1000 days per second */
};

/* Month to season mapping */
static civ_season_t get_season_from_month(int32_t month) {
    if (month >= 1 && month <= 2) return CIV_SEASON_WINTER;
    if (month >= 3 && month <= 5) return CIV_SEASON_SPRING;
    if (month >= 6 && month <= 8) return CIV_SEASON_SUMMER;
    if (month >= 9 && month <= 11) return CIV_SEASON_AUTUMN;
    return CIV_SEASON_WINTER; /* December */
}

civ_time_manager_t* civ_time_manager_create(void) {
    civ_time_manager_t* tm = (civ_time_manager_t*)CIV_MALLOC(sizeof(civ_time_manager_t));
    if (!tm) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate time manager");
        return NULL;
    }
    
    civ_time_manager_init(tm);
    return tm;
}

void civ_time_manager_destroy(civ_time_manager_t* tm) {
    if (!tm) return;
    
    /* Free all listeners */
    civ_time_listener_t* listener = tm->listeners;
    while (listener) {
        civ_time_listener_t* next = listener->next;
        CIV_FREE(listener);
        listener = next;
    }
    
    CIV_FREE(tm);
}

void civ_time_manager_init(civ_time_manager_t* tm) {
    if (!tm) return;
    
    memset(tm, 0, sizeof(civ_time_manager_t));
    
    /* Initialize calendar */
    tm->calendar.year = 1;
    tm->calendar.month = 1;
    tm->calendar.day = 1;
    tm->calendar.total_days = 0;
    tm->calendar.season = CIV_SEASON_SPRING;
    
    /* Initialize time scale */
    tm->time_scale = CIV_TIME_SCALE_NORMAL;
    tm->game_speed = 1.0f;
    tm->time_delta = 0.0f;
    tm->last_update_time = time(NULL);
    tm->accumulated_time = 0.0f;
    
    /* Copy multipliers */
    memcpy(tm->scale_multipliers, TIME_SCALE_MULTIPLIERS, sizeof(TIME_SCALE_MULTIPLIERS));
    
    tm->listeners = NULL;
}

civ_float_t civ_time_manager_update(civ_time_manager_t* tm) {
    if (!tm) return 0.0f;
    
    time_t current_time = time(NULL);
    civ_float_t real_delta = (civ_float_t)difftime(current_time, tm->last_update_time);
    tm->last_update_time = current_time;
    
    if (tm->time_scale == CIV_TIME_SCALE_PAUSED) {
        tm->time_delta = 0.0f;
        return 0.0f;
    }
    
    /* Calculate game time delta */
    civ_float_t multiplier = tm->scale_multipliers[tm->time_scale] * tm->game_speed;
    tm->time_delta = real_delta * multiplier;
    tm->accumulated_time += tm->time_delta;
    
    /* Advance calendar based on accumulated time */
    int32_t days_to_advance = (int32_t)tm->accumulated_time;
    if (days_to_advance > 0) {
        for (int32_t i = 0; i < days_to_advance; i++) {
            civ_calendar_advance_day(&tm->calendar);
            
            /* Notify listeners of day advancement */
            civ_time_listener_t* listener = tm->listeners;
            while (listener) {
                listener->callback("day", &tm->calendar, listener->user_data);
                listener = listener->next;
            }
        }
        tm->accumulated_time -= (civ_float_t)days_to_advance;
        
        /* Check for month/year events */
        if (tm->calendar.day == 1) {
            civ_time_listener_t* listener = tm->listeners;
            while (listener) {
                listener->callback("month", &tm->calendar, listener->user_data);
                listener = listener->next;
            }
        }
        
        if (tm->calendar.month == 1 && tm->calendar.day == 1) {
            civ_time_listener_t* listener = tm->listeners;
            while (listener) {
                listener->callback("year", &tm->calendar, listener->user_data);
                listener = listener->next;
            }
        }
    }
    
    return tm->time_delta;
}

void civ_time_manager_set_time_scale(civ_time_manager_t* tm, civ_time_scale_t scale) {
    if (!tm) return;
    if (scale >= 0 && scale < 6) {
        tm->time_scale = scale;
    }
}

void civ_time_manager_adjust_speed(civ_time_manager_t* tm, civ_float_t multiplier) {
    if (!tm) return;
    tm->game_speed = CLAMP(multiplier, 0.1f, 10.0f);
}

void civ_time_manager_add_listener(civ_time_manager_t* tm, civ_time_listener_cb_t callback, void* user_data) {
    if (!tm || !callback) return;
    
    civ_time_listener_t* listener = (civ_time_listener_t*)CIV_MALLOC(sizeof(civ_time_listener_t));
    if (!listener) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate time listener");
        return;
    }
    
    listener->callback = callback;
    listener->user_data = user_data;
    listener->next = tm->listeners;
    tm->listeners = listener;
}

void civ_time_manager_remove_listener(civ_time_manager_t* tm, civ_time_listener_cb_t callback) {
    if (!tm || !callback) return;
    
    civ_time_listener_t* prev = NULL;
    civ_time_listener_t* current = tm->listeners;
    
    while (current) {
        if (current->callback == callback) {
            if (prev) {
                prev->next = current->next;
            } else {
                tm->listeners = current->next;
            }
            CIV_FREE(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

civ_season_t civ_calendar_get_season(const civ_calendar_t* cal) {
    if (!cal) return CIV_SEASON_SPRING;
    return get_season_from_month(cal->month);
}

void civ_calendar_get_date_string(const civ_calendar_t* cal, char* buffer, size_t buffer_size) {
    if (!cal || !buffer || buffer_size == 0) return;
    
    const char* season_names[] = { "Winter", "Spring", "Summer", "Autumn" };
    civ_season_t season = civ_calendar_get_season(cal);
    
    snprintf(buffer, buffer_size, "%s, Day %d, Month %d, Year %d",
             season_names[season], cal->day, cal->month, cal->year);
}

void civ_calendar_advance_day(civ_calendar_t* cal) {
    if (!cal) return;
    
    cal->day++;
    cal->total_days++;
    
    /* Handle month/year rollover (30-day months) */
    if (cal->day > 30) {
        cal->day = 1;
        cal->month++;
        if (cal->month > 12) {
            cal->month = 1;
            cal->year++;
        }
    }
    
    cal->season = get_season_from_month(cal->month);
}

char* civ_time_manager_to_json(const civ_time_manager_t* tm) {
    if (!tm) return NULL;
    
    char* json = (char*)CIV_MALLOC(512);
    if (!json) return NULL;
    
    snprintf(json, 512,
        "{\"year\":%d,\"month\":%d,\"day\":%d,\"total_days\":%lld,"
        "\"time_scale\":%d,\"game_speed\":%.2f}",
        tm->calendar.year, tm->calendar.month, tm->calendar.day,
        (long long)tm->calendar.total_days, tm->time_scale, tm->game_speed);
    
    return json;
}

civ_result_t civ_time_manager_from_json(civ_time_manager_t* tm, const char* json) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!tm || !json) {
        result.error = CIV_ERROR_NULL_POINTER;
        result.message = "Null pointer argument";
        return result;
    }
    
    /* Simple JSON parsing (in production, use a proper JSON library) */
    /* For now, just reinitialize */
    civ_time_manager_init(tm);
    
    return result;
}

/* Logging is implemented in common.c */

