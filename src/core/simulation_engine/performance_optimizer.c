/**
 * @file performance_optimizer.c
 * @brief Implementation of performance optimizer
 */

#include "../../../include/core/simulation_engine/performance_optimizer.h"
#include "../../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

civ_performance_optimizer_t* civ_performance_optimizer_create(void) {
    civ_performance_optimizer_t* po = (civ_performance_optimizer_t*)CIV_MALLOC(sizeof(civ_performance_optimizer_t));
    if (!po) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate performance optimizer");
        return NULL;
    }
    
    civ_performance_optimizer_init(po);
    return po;
}

void civ_performance_optimizer_destroy(civ_performance_optimizer_t* po) {
    if (!po) return;
    CIV_FREE(po->metrics);
    CIV_FREE(po);
}

void civ_performance_optimizer_init(civ_performance_optimizer_t* po) {
    if (!po) return;
    
    memset(po, 0, sizeof(civ_performance_optimizer_t));
    po->metric_capacity = 100;
    po->metrics = (civ_performance_metric_t*)CIV_CALLOC(po->metric_capacity, sizeof(civ_performance_metric_t));
    po->profiling_enabled = false;
    po->optimization_threshold = 100.0f;  /* 100ms */
}

void civ_performance_optimizer_enable_profiling(civ_performance_optimizer_t* po, bool enabled) {
    if (!po) return;
    po->profiling_enabled = enabled;
    civ_log(CIV_LOG_INFO, "Performance profiling %s", enabled ? "enabled" : "disabled");
}

void civ_performance_optimizer_record_metric(civ_performance_optimizer_t* po, const char* name,
                                              civ_float_t execution_time, civ_float_t memory_delta) {
    if (!po || !name) return;
    
    /* Find or create metric */
    civ_performance_metric_t* metric = NULL;
    for (size_t i = 0; i < po->metric_count; i++) {
        if (strcmp(po->metrics[i].name, name) == 0) {
            metric = &po->metrics[i];
            break;
        }
    }
    
    if (!metric) {
        /* Create new metric */
        if (po->metric_count >= po->metric_capacity) {
            po->metric_capacity *= 2;
            po->metrics = (civ_performance_metric_t*)CIV_REALLOC(po->metrics,
                                                                  po->metric_capacity * sizeof(civ_performance_metric_t));
        }
        
        if (po->metrics) {
            metric = &po->metrics[po->metric_count++];
            memset(metric, 0, sizeof(civ_performance_metric_t));
            strncpy(metric->name, name, sizeof(metric->name) - 1);
            metric->min_time = execution_time;
            metric->max_time = execution_time;
        }
    }
    
    if (metric) {
        metric->execution_time += execution_time;
        metric->call_count++;
        metric->memory_usage += memory_delta;
        metric->avg_time = metric->execution_time / (civ_float_t)metric->call_count;
        metric->min_time = MIN(metric->min_time, execution_time);
        metric->max_time = MAX(metric->max_time, execution_time);
        
        po->total_calls++;
        po->total_execution_time += execution_time;
    }
}

civ_performance_metric_t* civ_performance_optimizer_get_metric(civ_performance_optimizer_t* po, const char* name) {
    if (!po || !name) return NULL;
    
    for (size_t i = 0; i < po->metric_count; i++) {
        if (strcmp(po->metrics[i].name, name) == 0) {
            return &po->metrics[i];
        }
    }
    
    return NULL;
}

char* civ_performance_optimizer_generate_report(const civ_performance_optimizer_t* po) {
    if (!po) return NULL;
    
    char* report = (char*)CIV_MALLOC(4096);
    if (!report) return NULL;
    
    size_t offset = 0;
    offset += snprintf(report + offset, 4096 - offset, "Performance Report\n");
    offset += snprintf(report + offset, 4096 - offset, "==================\n\n");
    offset += snprintf(report + offset, 4096 - offset, "Total Calls: %llu\n", (unsigned long long)po->total_calls);
    offset += snprintf(report + offset, 4096 - offset, "Total Time: %.2f ms\n\n", po->total_execution_time);
    
    offset += snprintf(report + offset, 4096 - offset, "Metrics:\n");
    offset += snprintf(report + offset, 4096 - offset, "%-20s %10s %10s %10s %10s %10s\n",
                      "Name", "Calls", "Total(ms)", "Avg(ms)", "Min(ms)", "Max(ms)");
    offset += snprintf(report + offset, 4096 - offset, "%-20s %10s %10s %10s %10s %10s\n",
                      "--------------------", "----------", "----------", "----------", "----------", "----------");
    
    for (size_t i = 0; i < po->metric_count && offset < 4000; i++) {
        const civ_performance_metric_t* m = &po->metrics[i];
        offset += snprintf(report + offset, 4096 - offset,
                          "%-20s %10llu %10.2f %10.2f %10.2f %10.2f\n",
                          m->name, (unsigned long long)m->call_count, m->execution_time,
                          m->avg_time, m->min_time, m->max_time);
    }
    
    return report;
}

void civ_performance_optimizer_reset(civ_performance_optimizer_t* po) {
    if (!po) return;
    
    for (size_t i = 0; i < po->metric_count; i++) {
        memset(&po->metrics[i], 0, sizeof(civ_performance_metric_t));
    }
    po->metric_count = 0;
    po->total_calls = 0;
    po->total_execution_time = 0.0f;
}

