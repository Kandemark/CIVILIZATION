/**
 * @file time_engine.c
 * @brief Global time engine — custom calendars, eras, epochs
 *
 * Global baseline year 0 = 1 BC. Each turn advances time.
 * Calendars define their own epoch, months, eras.
 * Nothing is fixed — players with power can create new calendars.
 */
#include "../../include/core/time_engine.h"
#include "../../include/common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Default month/week names ──────────────────────────────────────── */
static const char *s_default_months[12] = {
    "First Moon", "Second Moon", "Third Moon", "Fourth Moon",
    "Fifth Moon", "Sixth Moon", "Seventh Moon", "Eighth Moon",
    "Ninth Moon", "Tenth Moon", "Eleventh Moon", "Twelfth Moon",
};
static const int32_t s_default_month_days[12] = {
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 35,
};
static const char *s_default_weekdays[7] = {
    "First Day", "Second Day", "Third Day", "Fourth Day",
    "Fifth Day", "Sixth Day", "Seventh Day",
};
static const char *s_roman_months[12] = {
    "Ianuarius", "Februarius", "Martius", "Aprilis",
    "Maius", "Iunius", "Iulius", "Augustus",
    "September", "October", "November", "December",
};
static const char *s_roman_weekdays[7] = {
    "Dies Solis", "Dies Lunae", "Dies Martis", "Dies Mercurii",
    "Dies Iovis", "Dies Veneris", "Dies Saturni",
};

/* ── Helpers ───────────────────────────────────────────────────────── */
static void setup_default_months(civ_custom_calendar_t *cal, const char **names,
                                 const int32_t *days, int count) {
  cal->month_count = count;
  for (int i = 0; i < count; i++) {
    strncpy(cal->months[i].name, names[i], CIV_MONTH_NAME_MAX - 1);
    cal->months[i].days = days ? days[i] : 30;
  }
}

static void setup_default_weekdays(civ_custom_calendar_t *cal, const char **names,
                                   int count) {
  cal->week_day_count = count;
  cal->week_length = count;
  for (int i = 0; i < count; i++)
    strncpy(cal->week_days[i], names[i], CIV_DAY_NAME_MAX - 1);
}

/* ── Engine ────────────────────────────────────────────────────────── */
civ_time_engine_t *civ_time_engine_create(void) {
  civ_time_engine_t *te = (civ_time_engine_t *)malloc(sizeof(civ_time_engine_t));
  if (!te) return NULL;
  memset(te, 0, sizeof(*te));
  te->global.global_year = 0;      /* 1 BC */
  te->global.global_day = 0;
  te->global.turn_number = 0;
  te->global.turns_per_year = 12;  /* monthly turns */
  te->global.day_progress = 0.0f;
  te->calendar_count = 0;
  te->active_calendar_index = 0;
  return te;
}

void civ_time_engine_destroy(civ_time_engine_t *te) { free(te); }

void civ_time_engine_advance_turn(civ_time_engine_t *te) {
  if (!te) return;
  te->global.turn_number++;
  te->global.day_progress = 0.0f;

  int32_t days_per_turn = 365 / te->global.turns_per_year;
  te->global.global_day += days_per_turn;

  while (te->global.global_day >= 365) {
    te->global.global_day -= 365;
    te->global.global_year++;
  }
}

int32_t civ_time_engine_create_calendar(civ_time_engine_t *te,
                                        const char *name,
                                        const char *epoch_name,
                                        int32_t epoch_global_year,
                                        const char *creator_id) {
  if (!te || te->calendar_count >= CIV_CALENDAR_MAX) return -1;

  int32_t idx = te->calendar_count++;
  civ_custom_calendar_t *cal = &te->calendars[idx];
  memset(cal, 0, sizeof(*cal));

  snprintf(cal->id, sizeof(cal->id), "cal_%d", idx);
  strncpy(cal->name, name ? name : "Unnamed Calendar", CIV_CALENDAR_NAME_MAX - 1);
  strncpy(cal->epoch_name, epoch_name ? epoch_name : "Foundation",
          CIV_EPOCH_NAME_MAX - 1);
  cal->epoch_global_year = epoch_global_year;
  cal->year_length_days = 365;
  cal->week_length = 7;

  setup_default_months(cal, s_default_months, s_default_month_days, 12);
  setup_default_weekdays(cal, s_default_weekdays, 7);

  if (creator_id)
    strncpy(cal->creator_id, creator_id, sizeof(cal->creator_id) - 1);

  /* Start with one default era */
  cal->era_count = 1;
  strncpy(cal->eras[0].name, "First Era", CIV_ERA_NAME_MAX - 1);
  cal->eras[0].start_year = 0;
  cal->current_era_index = 0;

  return idx;
}

void civ_time_engine_add_era(civ_time_engine_t *te, int32_t cal_idx,
                             const char *era_name, const char *declared_by) {
  if (!te || cal_idx < 0 || cal_idx >= te->calendar_count) return;
  if (te->calendars[cal_idx].era_count >= CIV_ERAS_MAX) return;

  civ_custom_calendar_t *cal = &te->calendars[cal_idx];
  int32_t local_year = civ_calendar_global_to_local(cal, te->global.global_year);
  int32_t e = cal->era_count++;
  strncpy(cal->eras[e].name, era_name, CIV_ERA_NAME_MAX - 1);
  cal->eras[e].start_year = local_year;
  if (declared_by)
    strncpy(cal->eras[e].declared_by, declared_by, sizeof(cal->eras[e].declared_by) - 1);
  cal->current_era_index = e;
}

void civ_time_engine_set_era(civ_time_engine_t *te, int32_t cal_idx,
                             int32_t era_idx) {
  if (!te || cal_idx < 0 || cal_idx >= te->calendar_count) return;
  if (era_idx < 0 || era_idx >= te->calendars[cal_idx].era_count) return;
  te->calendars[cal_idx].current_era_index = era_idx;
}

int32_t civ_calendar_global_to_local(const civ_custom_calendar_t *cal,
                                     int32_t global_year) {
  if (!cal) return global_year;
  return global_year - cal->epoch_global_year;
}

void civ_time_engine_format_date(civ_time_engine_t *te, int32_t cal_idx,
                                 char *out_buf, size_t buf_size) {
  if (!te || !out_buf || cal_idx < 0 || cal_idx >= te->calendar_count) {
    snprintf(out_buf, buf_size, "Year 0");
    return;
  }

  civ_custom_calendar_t *cal = &te->calendars[cal_idx];
  int32_t local_year = civ_calendar_global_to_local(cal, te->global.global_year);

  /* Find current month and day */
  int32_t day_of_year = te->global.global_day;
  int32_t month_idx = 0, day_of_month = 0;
  int32_t accumulated = 0;

  for (int i = 0; i < cal->month_count; i++) {
    if (day_of_year < accumulated + cal->months[i].days) {
      month_idx = i;
      day_of_month = day_of_year - accumulated + 1;
      break;
    }
    accumulated += cal->months[i].days;
  }

  /* Week day */
  int32_t weekday = te->global.global_day % cal->week_length;

  /* Era */
  const char *era_name = (cal->current_era_index >= 0 &&
                          cal->current_era_index < cal->era_count)
                             ? cal->eras[cal->current_era_index].name
                             : "";

  /* BC/CE indicator */
  const char *era_label = "";
  if (local_year < 0) era_label = "B";

  snprintf(out_buf, buf_size,
           "%s %d, %s  |  Year %d%s of %s  |  %s",
           cal->months[month_idx].name, day_of_month,
           cal->week_days[weekday],
           local_year >= 0 ? local_year : -local_year,
           local_year < 0 ? " BC" : "",
           era_name, cal->epoch_name);
}

void civ_time_engine_format_hud(civ_time_engine_t *te, char *buf, size_t size) {
  if (!te || !buf) return;
  civ_custom_calendar_t *cal = &te->calendars[te->active_calendar_index];
  int32_t ly = civ_calendar_global_to_local(cal, te->global.global_year);
  const char *era = (cal->current_era_index < cal->era_count)
                        ? cal->eras[cal->current_era_index].name : "";
  const char *tag = ly < 0 ? "BC" : "";
  snprintf(buf, size, "Year %d %s  |  %s  |  Turn %d",
           ly < 0 ? -ly : ly, tag, era, te->global.turn_number);
}

void civ_time_engine_init_default_calendars(civ_time_engine_t *te) {
  if (!te) return;

  /* Calendar 0: Common Era — global baseline, year 0 = 1 BC */
  int32_t c0 = civ_time_engine_create_calendar(
      te, "Common Reckoning", "Global Baseline", 0, "universal");
  if (c0 >= 0) {
    setup_default_months(&te->calendars[c0], s_roman_months, s_default_month_days, 12);
    setup_default_weekdays(&te->calendars[c0], s_roman_weekdays, 7);
    te->calendars[c0].is_default = true;
    te->active_calendar_index = c0;
  }

  /* Calendar 1: Imperial — epoch at foundation of Imperial Dominion */
  int32_t c1 = civ_time_engine_create_calendar(
      te, "Imperial Chronicle", "Foundation of the Dominion", -120, "imperial_dominion");
  if (c1 >= 0) {
    civ_time_engine_add_era(te, c1, "Age of Unification", "First Chancellor");
    civ_time_engine_add_era(te, c1, "Imperial Expansion", "War Council");
  }

  /* Calendar 2: Merchant — epoch at first trade compact */
  civ_time_engine_create_calendar(
      te, "Trade Ledger", "First Compact of Commerce", -200, "mercantile_league");

  /* Calendar 3: Theocratic — epoch at first revelation */
  civ_time_engine_create_calendar(
      te, "Sacred Reckoning", "The First Revelation", -500, "theocratic_order");
}
