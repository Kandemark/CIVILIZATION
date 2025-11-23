/**
 * @file time_manager.h
 * @brief Time management system for the Civilization simulation
 * 
 * Handles different time scales, game calendar, and time-based events.
 */

#ifndef CIVILIZATION_TIME_MANAGER_H
#define CIVILIZATION_TIME_MANAGER_H

#include "../../common.h"
#include "../../types.h"

/* Time scale enumeration */
typedef enum {
    CIV_TIME_SCALE_PAUSED = 0,
    CIV_TIME_SCALE_SLOW = 1,
    CIV_TIME_SCALE_NORMAL = 2,
    CIV_TIME_SCALE_FAST = 3,
    CIV_TIME_SCALE_VERY_FAST = 4,
    CIV_TIME_SCALE_ULTRA = 5
} civ_time_scale_t;

/* Season enumeration */
typedef enum {
    CIV_SEASON_WINTER = 0,
    CIV_SEASON_SPRING = 1,
    CIV_SEASON_SUMMER = 2,
    CIV_SEASON_AUTUMN = 3
} civ_season_t;

/* Game calendar structure */
typedef struct {
    int32_t year;
    int32_t month;
    int32_t day;
    int64_t total_days;
    civ_season_t season;
} civ_calendar_t;

/* Time listener callback function type */
typedef void (*civ_time_listener_cb_t)(const char* event_type, void* calendar, void* user_data);

/* Time listener structure */
typedef struct civ_time_listener {
    civ_time_listener_cb_t callback;
    void* user_data;
    struct civ_time_listener* next;
} civ_time_listener_t;

/* Time manager structure */
typedef struct {
    civ_calendar_t calendar;
    civ_time_scale_t time_scale;
    civ_float_t game_speed;
    civ_float_t time_delta;
    time_t last_update_time;
    civ_float_t accumulated_time;
    
    /* Time listeners */
    civ_time_listener_t* listeners;
    
    /* Time scale multipliers */
    civ_float_t scale_multipliers[6];
} civ_time_manager_t;

/* Function declarations */

/**
 * Create a new time manager
 */
civ_time_manager_t* civ_time_manager_create(void);

/**
 * Destroy a time manager
 */
void civ_time_manager_destroy(civ_time_manager_t* tm);

/**
 * Initialize time manager with default values
 */
void civ_time_manager_init(civ_time_manager_t* tm);

/**
 * Update the time manager (call each frame)
 * @return Time delta in game days
 */
civ_float_t civ_time_manager_update(civ_time_manager_t* tm);

/**
 * Set the time scale
 */
void civ_time_manager_set_time_scale(civ_time_manager_t* tm, civ_time_scale_t scale);

/**
 * Adjust game speed multiplier
 */
void civ_time_manager_adjust_speed(civ_time_manager_t* tm, civ_float_t multiplier);

/**
 * Add a time event listener
 */
void civ_time_manager_add_listener(civ_time_manager_t* tm, civ_time_listener_cb_t callback, void* user_data);

/**
 * Remove a time event listener
 */
void civ_time_manager_remove_listener(civ_time_manager_t* tm, civ_time_listener_cb_t callback);

/**
 * Get current season
 */
civ_season_t civ_calendar_get_season(const civ_calendar_t* cal);

/**
 * Get formatted date string
 */
void civ_calendar_get_date_string(const civ_calendar_t* cal, char* buffer, size_t buffer_size);

/**
 * Advance calendar by one day
 */
void civ_calendar_advance_day(civ_calendar_t* cal);

/**
 * Serialize time manager to JSON (returns allocated string, caller must free)
 */
char* civ_time_manager_to_json(const civ_time_manager_t* tm);

/**
 * Deserialize time manager from JSON
 */
civ_result_t civ_time_manager_from_json(civ_time_manager_t* tm, const char* json);

#endif /* CIVILIZATION_TIME_MANAGER_H */

