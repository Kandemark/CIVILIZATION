/**
 * @file politics.c
 * @brief Implementation of main politics system
 */

#include "../../../include/core/politics/politics.h"
#include "../../../include/common.h"

civ_politics_system_t* civ_politics_system_create(void) {
    civ_politics_system_t* politics = (civ_politics_system_t*)CIV_MALLOC(sizeof(civ_politics_system_t));
    if (!politics) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate politics system");
        return NULL;
    }
    
    civ_politics_system_init(politics);
    return politics;
}

void civ_politics_system_destroy(civ_politics_system_t* politics) {
    if (!politics) return;
    
    if (politics->faction_system) {
        civ_faction_system_destroy(politics->faction_system);
    }
    CIV_FREE(politics);
}

void civ_politics_system_init(civ_politics_system_t* politics) {
    if (!politics) return;
    
    memset(politics, 0, sizeof(civ_politics_system_t));
    politics->faction_system = civ_faction_system_create();
    politics->legitimacy = 0.7f;
    politics->stability = 0.7f;
}

civ_result_t civ_politics_system_update(civ_politics_system_t* politics, civ_float_t time_delta) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!politics) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Update faction system */
    if (politics->faction_system) {
        civ_faction_system_update(politics->faction_system, time_delta);
        politics->stability = civ_faction_system_calculate_stability(politics->faction_system);
    }
    
    /* Stability affects legitimacy */
    civ_float_t legitimacy_change = (politics->stability - politics->legitimacy) * time_delta * 0.01f;
    politics->legitimacy = CLAMP(politics->legitimacy + legitimacy_change, 0.0f, 1.0f);
    
    return result;
}

void civ_politics_system_set_legitimacy(civ_politics_system_t* politics, civ_float_t legitimacy) {
    if (!politics) return;
    politics->legitimacy = CLAMP(legitimacy, 0.0f, 1.0f);
}

civ_float_t civ_politics_system_get_stability(const civ_politics_system_t* politics) {
    if (!politics) return 0.0f;
    return politics->stability;
}

