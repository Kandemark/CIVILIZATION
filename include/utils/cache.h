/**
 * @file cache.h
 * @brief Caching system for performance
 */

#ifndef CIVILIZATION_CACHE_H
#define CIVILIZATION_CACHE_H

#include "../common.h"
#include "../types.h"

/* Cache entry */
typedef struct civ_cache_entry {
    char key[STRING_SHORT_LEN];
    void* data;
    size_t data_size;
    time_t timestamp;
    time_t expiry;
    struct civ_cache_entry* next;
} civ_cache_entry_t;

/* Cache structure */
typedef struct {
    civ_cache_entry_t* entries;
    size_t entry_count;
    size_t max_entries;
    size_t max_size;
    size_t current_size;
    time_t default_ttl;  /* Time to live in seconds */
} civ_cache_t;

/* Function declarations */
civ_cache_t* civ_cache_create(size_t max_entries, size_t max_size, time_t default_ttl);
void civ_cache_destroy(civ_cache_t* cache);
void civ_cache_init(civ_cache_t* cache, size_t max_entries, size_t max_size, time_t default_ttl);

civ_result_t civ_cache_set(civ_cache_t* cache, const char* key, const void* data, size_t data_size, time_t ttl);
civ_result_t civ_cache_get(civ_cache_t* cache, const char* key, void* out, size_t* out_size);
void civ_cache_remove(civ_cache_t* cache, const char* key);
void civ_cache_clear(civ_cache_t* cache);
void civ_cache_cleanup_expired(civ_cache_t* cache);
size_t civ_cache_get_size(const civ_cache_t* cache);

#endif /* CIVILIZATION_CACHE_H */

