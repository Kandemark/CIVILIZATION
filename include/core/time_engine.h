/**
 * @file time_engine.h
 * @brief Global time engine with custom per-nation/per-religion calendars
 *
 * Global baseline: year 0 = 1 BC. All calendars define their epoch
 * relative to this global baseline. Nothing is fixed — calendars,
 * eras, month structures can all be created and modified through
 * gameplay decisions (founding religions, declaring new eras, etc.).
 */
#ifndef CIV_CORE_TIME_ENGINE_H
#define CIV_CORE_TIME_ENGINE_H

#include "../common.h"
#include "../types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CIV_CALENDAR_NAME_MAX   64
#define CIV_EPOCH_NAME_MAX      64
#define CIV_ERA_NAME_MAX        48
#define CIV_MONTH_NAME_MAX      24
#define CIV_DAY_NAME_MAX        16
#define CIV_CALENDAR_MAX        16
#define CIV_MONTHS_MAX          20
#define CIV_WEEK_DAYS_MAX       10
#define CIV_ERAS_MAX            12

/* ── Global time ──────────────────────────────────────────────────── */
typedef struct {
  int32_t  global_year;      /* 0 = 1 BC; negative = BC; positive = CE */
  int32_t  global_day;       /* day within current year (0-indexed) */
  uint32_t turn_number;      /* total turns elapsed */
  int32_t  turns_per_year;   /* how many turns per year (default 12) */
  float    day_progress;     /* 0.0–1.0 within current turn */
} civ_global_time_t;

/* ── Month definition ──────────────────────────────────────────────── */
typedef struct {
  char    name[CIV_MONTH_NAME_MAX];
  int32_t days;              /* days in this month */
} civ_month_def_t;

/* ── Era definition ────────────────────────────────────────────────── */
typedef struct {
  char    name[CIV_ERA_NAME_MAX];
  int32_t start_year;        /* year in this calendar when era began */
  char    declared_by[64];   /* who declared this era */
} civ_era_def_t;

/* ── Calendar ──────────────────────────────────────────────────────── */
typedef struct {
  char             id[32];
  char             name[CIV_CALENDAR_NAME_MAX];
  char             epoch_name[CIV_EPOCH_NAME_MAX];  /* "Foundation", "Birth of X" */
  int32_t          epoch_global_year;  /* global year when this calendar starts */
  int32_t          year_length_days;   /* default 365 */
  int32_t          week_length;        /* default 7 */

  /* Months */
  civ_month_def_t  months[CIV_MONTHS_MAX];
  int32_t          month_count;

  /* Week day names */
  char             week_days[CIV_WEEK_DAYS_MAX][CIV_DAY_NAME_MAX];
  int32_t          week_day_count;

  /* Eras */
  civ_era_def_t    eras[CIV_ERAS_MAX];
  int32_t          era_count;
  int32_t          current_era_index;

  /* Creator */
  char             creator_id[32];     /* nation or entity that created this */
  bool             is_default;         /* the baseline common calendar */
} civ_custom_calendar_t;

/* ── Time engine ───────────────────────────────────────────────────── */
typedef struct {
  civ_global_time_t  global;
  civ_custom_calendar_t     calendars[CIV_CALENDAR_MAX];
  int32_t            calendar_count;
  int32_t            active_calendar_index;  /* which calendar the player views */
} civ_time_engine_t;

/* ── API ────────────────────────────────────────────────────────────── */
civ_time_engine_t *civ_time_engine_create(void);
void               civ_time_engine_destroy(civ_time_engine_t *te);

/* Advance time by one turn. Returns the new global year/day. */
void civ_time_engine_advance_turn(civ_time_engine_t *te);

/* Create a new calendar. Returns index or -1 on failure. */
int32_t civ_time_engine_create_calendar(civ_time_engine_t *te,
                                        const char *name,
                                        const char *epoch_name,
                                        int32_t epoch_global_year,
                                        const char *creator_id);

/* Add an era to a calendar */
void civ_time_engine_add_era(civ_time_engine_t *te, int32_t cal_idx,
                             const char *era_name, const char *declared_by);

/* Switch to a new era */
void civ_time_engine_set_era(civ_time_engine_t *te, int32_t cal_idx,
                             int32_t era_idx);

/* Format the current date for a calendar into a display string */
void civ_time_engine_format_date(civ_time_engine_t *te, int32_t cal_idx,
                                 char *out_buf, size_t buf_size);

/* Get calendar year from global year */
int32_t civ_calendar_global_to_local(const civ_custom_calendar_t *cal,
                                     int32_t global_year);

/* Simple formatted date for HUD */
void civ_time_engine_format_hud(civ_time_engine_t *te, char *buf, size_t size);

/* Default calendars (Common Era, Imperial, Religious, etc.) */
void civ_time_engine_init_default_calendars(civ_time_engine_t *te);

#ifdef __cplusplus
}
#endif
#endif
