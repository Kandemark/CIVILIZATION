/**
 * @file performance_optimizer.h
 * @brief Performance optimization and profiling system
 */

#ifndef CIVILIZATION_PERFORMANCE_OPTIMIZER_H
#define CIVILIZATION_PERFORMANCE_OPTIMIZER_H

#include "../../common.h"
#include "../../types.h"

/* Performance metric structure */
typedef struct {
    char name[STRING_SHORT_LEN];
    civ_float_t execution_time;
    uint64_t call_count;
    civ_float_t memory_usage;
    civ_float_t avg_time;
    civ_float_t min_time;
    civ_float_t max_time;
} civ_performance_metric_t;

/* Performance optimizer */
typedef struct {
    civ_performance_metric_t* metrics;
    size_t metric_count;
    size_t metric_capacity;
    bool profiling_enabled;
    civ_float_t optimization_threshold;  /* ms */
    uint64_t total_calls;
    civ_float_t total_execution_time;
} civ_performance_optimizer_t;

/* Function declarations */
civ_performance_optimizer_t* civ_performance_optimizer_create(void);
void civ_performance_optimizer_destroy(civ_performance_optimizer_t* po);
void civ_performance_optimizer_init(civ_performance_optimizer_t* po);

void civ_performance_optimizer_enable_profiling(civ_performance_optimizer_t* po, bool enabled);
void civ_performance_optimizer_record_metric(civ_performance_optimizer_t* po, const char* name,
                                            civ_float_t execution_time, civ_float_t memory_delta);
civ_performance_metric_t* civ_performance_optimizer_get_metric(civ_performance_optimizer_t* po, const char* name);
char* civ_performance_optimizer_generate_report(const civ_performance_optimizer_t* po);
void civ_performance_optimizer_reset(civ_performance_optimizer_t* po);

/* Profiling macros */
#ifdef DEBUG
#define CIV_PROFILE_START(po, name) \
    clock_t _profile_start_##name = clock(); \
    const char* _profile_name_##name = #name;

#define CIV_PROFILE_END(po, name) \
    do { \
        clock_t _profile_end_##name = clock(); \
        civ_float_t _profile_time_##name = ((civ_float_t)(_profile_end_##name - _profile_start_##name)) / CLOCKS_PER_SEC * 1000.0f; \
        if (po && po->profiling_enabled) { \
            civ_performance_optimizer_record_metric(po, _profile_name_##name, _profile_time_##name, 0.0f); \
        } \
    } while(0)
#else
#define CIV_PROFILE_START(po, name) ((void)0)
#define CIV_PROFILE_END(po, name) ((void)0)
#endif

#endif /* CIVILIZATION_PERFORMANCE_OPTIMIZER_H */

