/**
 * @file cultural_assimilation.c
 * @brief Implementation of cultural assimilation tracking
 */

#include "../../../include/core/culture/cultural_assimilation.h"
#include "../../../include/common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


civ_assimilation_tracker_t *civ_assimilation_tracker_create(void) {
  civ_assimilation_tracker_t *tracker =
      (civ_assimilation_tracker_t *)CIV_MALLOC(
          sizeof(civ_assimilation_tracker_t));
  if (!tracker) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate assimilation tracker");
    return NULL;
  }

  civ_assimilation_tracker_init(tracker);
  return tracker;
}

void civ_assimilation_tracker_destroy(civ_assimilation_tracker_t *tracker) {
  if (!tracker)
    return;
  CIV_FREE(tracker->events);
  CIV_FREE(tracker);
}

void civ_assimilation_tracker_init(civ_assimilation_tracker_t *tracker) {
  if (!tracker)
    return;

  memset(tracker, 0, sizeof(civ_assimilation_tracker_t));
  tracker->base_assimilation_rate = 0.01f;
  tracker->forced_assimilation_multiplier = 2.0f;
  tracker->event_capacity = 100;
  tracker->events = (civ_assimilation_event_t *)CIV_CALLOC(
      tracker->event_capacity, sizeof(civ_assimilation_event_t));
}

civ_result_t
civ_assimilation_tracker_add_event(civ_assimilation_tracker_t *tracker,
                                   const char *source_id, const char *target_id,
                                   const char *region_id,
                                   civ_assimilation_type_t type) {
  civ_result_t result = {CIV_OK, NULL};

  if (!tracker || !source_id || !target_id) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  /* Check if event already exists */
  for (size_t i = 0; i < tracker->event_count; i++) {
    if (strcmp(tracker->events[i].source_culture_id, source_id) == 0 &&
        strcmp(tracker->events[i].target_culture_id, target_id) == 0) {
      result.error = CIV_ERROR_INVALID_STATE;
      result.message = "Assimilation event already exists";
      return result;
    }
  }

  /* Expand if needed */
  if (tracker->event_count >= tracker->event_capacity) {
    tracker->event_capacity *= 2;
    tracker->events = (civ_assimilation_event_t *)CIV_REALLOC(
        tracker->events,
        tracker->event_capacity * sizeof(civ_assimilation_event_t));
  }

  if (tracker->events) {
    civ_assimilation_event_t *event = &tracker->events[tracker->event_count++];
    memset(event, 0, sizeof(civ_assimilation_event_t));
    strncpy(event->source_culture_id, source_id,
            sizeof(event->source_culture_id) - 1);
    strncpy(event->target_culture_id, target_id,
            sizeof(event->target_culture_id) - 1);
    if (region_id) {
      strncpy(event->region_id, region_id, sizeof(event->region_id) - 1);
    }
    event->type = type;
    event->stage = CIV_INTEGRATION_DISSENT;
    event->progress = 0.0f;
    event->rate = tracker->base_assimilation_rate;
    event->resistance = 0.7f; /* Starting resistance is higher for dissent */
    event->tool_count = 0;
    event->adoption_level = 0.0f;
    event->start_time = time(NULL);
    event->last_update = event->start_time;
  } else {
    result.error = CIV_ERROR_OUT_OF_MEMORY;
  }

  return result;
}

civ_result_t civ_assimilation_tracker_update(
    civ_assimilation_tracker_t *tracker,
    civ_cultural_identity_manager_t *identity_manager, civ_float_t time_delta) {
  civ_result_t result = {CIV_OK, NULL};

  if (!tracker || !identity_manager) {
    result.error = CIV_ERROR_NULL_POINTER;
    return result;
  }

  time_t now = time(NULL);

  for (size_t i = 0; i < tracker->event_count; i++) {
    civ_assimilation_event_t *event = &tracker->events[i];

    /* Get source and target cultures */
    civ_cultural_identity_t *source = civ_cultural_identity_manager_find(
        identity_manager, event->source_culture_id);
    civ_cultural_identity_t *target = civ_cultural_identity_manager_find(
        identity_manager, event->target_culture_id);

    if (!source || !target) {
      /* Remove invalid event */
      memmove(&tracker->events[i], &tracker->events[i + 1],
              (tracker->event_count - i - 1) *
                  sizeof(civ_assimilation_event_t));
      tracker->event_count--;
      i--;
      continue;
    }

    /* Calculate assimilation rate */
    event->rate =
        civ_assimilation_calculate_rate(tracker, source, target, event->type);

    /* Update progress */
    civ_float_t progress_delta =
        event->rate * (1.0f - event->resistance) * time_delta;
    event->progress = CLAMP(event->progress + progress_delta, 0.0f, 1.0f);

    /* Update adoption level (how much of target culture adopts source traits)
     */
    event->adoption_level = event->progress * (1.0f - event->resistance);

    /* Apply assimilation to target culture */
    if (event->progress > 0.1f) {
      /* Diffuse traits from source to target */
      for (size_t j = 0; j < source->trait_count; j++) {
        const char *trait_name = source->traits[j].name;
        civ_float_t source_strength = source->traits[j].strength;

        /* Find or create trait in target */
        bool found = false;
        for (size_t k = 0; k < target->trait_count; k++) {
          if (strcmp(target->traits[k].name, trait_name) == 0) {
            found = true;
            /* Increase trait strength based on assimilation */
            civ_float_t adoption =
                source_strength * event->adoption_level * time_delta * 0.1f;
            target->traits[k].strength =
                CLAMP(target->traits[k].strength + adoption, 0.0f, 1.0f);
            break;
          }
        }

        if (!found && event->adoption_level > 0.2f) {
          /* Create new trait if adoption is significant */
          civ_cultural_identity_add_trait(target, trait_name,
                                          source_strength *
                                              event->adoption_level * 0.5f);
        }
      }
    }

    /* Update stage based on progress */
    civ_assimilation_update_stage(event);

    event->last_update = now;

    /* Remove completed events */
    if (event->progress >= 1.0f) {
      memmove(&tracker->events[i], &tracker->events[i + 1],
              (tracker->event_count - i - 1) *
                  sizeof(civ_assimilation_event_t));
      tracker->event_count--;
      i--;
    }
  }

  return result;
}

civ_assimilation_event_t *
civ_assimilation_tracker_get_event(const civ_assimilation_tracker_t *tracker,
                                   const char *source_id,
                                   const char *target_id) {
  if (!tracker || !source_id || !target_id)
    return NULL;

  for (size_t i = 0; i < tracker->event_count; i++) {
    if (strcmp(tracker->events[i].source_culture_id, source_id) == 0 &&
        strcmp(tracker->events[i].target_culture_id, target_id) == 0) {
      return (civ_assimilation_event_t *)&tracker->events[i];
    }
  }

  return NULL;
}

civ_float_t civ_assimilation_calculate_rate(
    civ_assimilation_tracker_t *tracker, const civ_cultural_identity_t *source,
    const civ_cultural_identity_t *target, civ_assimilation_type_t type) {
  if (!tracker || !source || !target)
    return 0.0f;

  civ_float_t base_rate = tracker->base_assimilation_rate;

  /* Adjust based on type */
  if (type == CIV_ASSIMILATION_FORCED) {
    base_rate *= tracker->forced_assimilation_multiplier;
  }

  /* Adjust based on source influence */
  base_rate *= source->influence_radius * 0.1f;

  /* Adjust based on target cohesion (lower cohesion = easier assimilation) */
  base_rate *= (1.0f - target->cohesion);

  /* Adjust based on similarity (more similar = easier) */
  civ_float_t similarity =
      civ_cultural_identity_calculate_similarity(source, target);
  base_rate *= (1.0f + similarity);

  return base_rate;
}

civ_result_t civ_assimilation_apply_tool(civ_assimilation_event_t *event,
                                         civ_assimilation_tool_t tool) {
  if (!event)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null event"};

  if (event->tool_count >= 4)
    return (civ_result_t){CIV_ERROR_INVALID_STATE, "Too many tools"};

  event->active_tools[event->tool_count++] = tool;

  /* Policy shift: reduce resistance or increase rate */
  event->resistance = CLAMP(event->resistance - 0.1f, 0.0f, 1.0f);
  event->rate *= 1.2f;

  return (civ_result_t){CIV_OK, NULL};
}

civ_result_t civ_assimilation_update_stage(civ_assimilation_event_t *event) {
  if (!event)
    return (civ_result_t){CIV_ERROR_NULL_POINTER, "Null event"};

  civ_integration_stage_t old_stage = event->stage;

  if (event->progress > 0.9f)
    event->stage = CIV_INTEGRATION_UNIFICATION;
  else if (event->progress > 0.7f)
    event->stage = CIV_INTEGRATION_ABSORPTION;
  else if (event->progress > 0.4f)
    event->stage = CIV_INTEGRATION_ADOPTION;
  else if (event->progress > 0.2f)
    event->stage = CIV_INTEGRATION_ACCEPTANCE;

  if (event->stage != old_stage) {
    civ_log(CIV_LOG_INFO, "Cultural event in %s moved to stage %d",
            event->region_id, event->stage);
    /* Resistance drops as integration increases */
    event->resistance *= 0.8f;
  }

  return (civ_result_t){CIV_OK, NULL};
}
