/**
 * @file cache.c
 * @brief Implementation of caching system
 */

#include "../../include/utils/cache.h"
#include "../../include/common.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

civ_cache_t* civ_cache_create(size_t max_entries, size_t max_size, time_t default_ttl) {
    civ_cache_t* cache = (civ_cache_t*)CIV_MALLOC(sizeof(civ_cache_t));
    if (!cache) {
        civ_log(CIV_LOG_ERROR, "Failed to allocate cache");
        return NULL;
    }
    
    civ_cache_init(cache, max_entries, max_size, default_ttl);
    return cache;
}

void civ_cache_destroy(civ_cache_t* cache) {
    if (!cache) return;
    
    civ_cache_clear(cache);
    CIV_FREE(cache);
}

void civ_cache_init(civ_cache_t* cache, size_t max_entries, size_t max_size, time_t default_ttl) {
    if (!cache) return;
    
    memset(cache, 0, sizeof(civ_cache_t));
    cache->max_entries = max_entries > 0 ? max_entries : 1000;
    cache->max_size = max_size > 0 ? max_size : 10 * 1024 * 1024;  /* 10MB default */
    cache->default_ttl = default_ttl > 0 ? default_ttl : 3600;  /* 1 hour default */
}

civ_result_t civ_cache_set(civ_cache_t* cache, const char* key, const void* data, size_t data_size, time_t ttl) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!cache || !key || !data) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    /* Check if entry exists */
    civ_cache_entry_t* entry = cache->entries;
    civ_cache_entry_t* prev = NULL;
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            /* Update existing entry */
            if (entry->data_size != data_size) {
                CIV_FREE(entry->data);
                entry->data = CIV_MALLOC(data_size);
                if (!entry->data) {
                    result.error = CIV_ERROR_OUT_OF_MEMORY;
                    return result;
                }
            }
            memcpy(entry->data, data, data_size);
            entry->data_size = data_size;
            entry->timestamp = time(NULL);
            entry->expiry = entry->timestamp + (ttl > 0 ? ttl : cache->default_ttl);
            return result;
        }
        prev = entry;
        entry = entry->next;
    }
    
    /* Check limits */
    if (cache->entry_count >= cache->max_entries) {
        /* Remove oldest entry */
        civ_cache_cleanup_expired(cache);
        if (cache->entry_count >= cache->max_entries && cache->entries) {
            /* Remove first entry */
            civ_cache_entry_t* oldest = cache->entries;
            cache->entries = oldest->next;
            cache->current_size -= oldest->data_size;
            CIV_FREE(oldest->data);
            CIV_FREE(oldest);
            cache->entry_count--;
        }
    }
    
    if (cache->current_size + data_size > cache->max_size) {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
        result.message = "Cache size limit exceeded";
        return result;
    }
    
    /* Create new entry */
    entry = (civ_cache_entry_t*)CIV_MALLOC(sizeof(civ_cache_entry_t));
    if (!entry) {
        result.error = CIV_ERROR_OUT_OF_MEMORY;
        return result;
    }
    
    memset(entry, 0, sizeof(civ_cache_entry_t));
    strncpy(entry->key, key, sizeof(entry->key) - 1);
    entry->data = CIV_MALLOC(data_size);
    if (!entry->data) {
        CIV_FREE(entry);
        result.error = CIV_ERROR_OUT_OF_MEMORY;
        return result;
    }
    
    memcpy(entry->data, data, data_size);
    entry->data_size = data_size;
    entry->timestamp = time(NULL);
    entry->expiry = entry->timestamp + (ttl > 0 ? ttl : cache->default_ttl);
    
    entry->next = cache->entries;
    cache->entries = entry;
    cache->entry_count++;
    cache->current_size += data_size;
    
    return result;
}

civ_result_t civ_cache_get(civ_cache_t* cache, const char* key, void* out, size_t* out_size) {
    civ_result_t result = {CIV_OK, NULL};
    
    if (!cache || !key || !out || !out_size) {
        result.error = CIV_ERROR_NULL_POINTER;
        return result;
    }
    
    time_t now = time(NULL);
    civ_cache_entry_t* entry = cache->entries;
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            /* Check if expired */
            if (entry->expiry > 0 && now > entry->expiry) {
                result.error = CIV_ERROR_NOT_FOUND;
                result.message = "Cache entry expired";
                return result;
            }
            
            if (*out_size < entry->data_size) {
                result.error = CIV_ERROR_INVALID_ARGUMENT;
                result.message = "Output buffer too small";
                return result;
            }
            
            memcpy(out, entry->data, entry->data_size);
            *out_size = entry->data_size;
            return result;
        }
        entry = entry->next;
    }
    
    result.error = CIV_ERROR_NOT_FOUND;
    return result;
}

void civ_cache_remove(civ_cache_t* cache, const char* key) {
    if (!cache || !key) return;
    
    civ_cache_entry_t* entry = cache->entries;
    civ_cache_entry_t* prev = NULL;
    
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (prev) {
                prev->next = entry->next;
            } else {
                cache->entries = entry->next;
            }
            
            cache->current_size -= entry->data_size;
            cache->entry_count--;
            CIV_FREE(entry->data);
            CIV_FREE(entry);
            return;
        }
        prev = entry;
        entry = entry->next;
    }
}

void civ_cache_clear(civ_cache_t* cache) {
    if (!cache) return;
    
    civ_cache_entry_t* entry = cache->entries;
    while (entry) {
        civ_cache_entry_t* next = entry->next;
        CIV_FREE(entry->data);
        CIV_FREE(entry);
        entry = next;
    }
    
    cache->entries = NULL;
    cache->entry_count = 0;
    cache->current_size = 0;
}

void civ_cache_cleanup_expired(civ_cache_t* cache) {
    if (!cache) return;
    
    time_t now = time(NULL);
    civ_cache_entry_t* entry = cache->entries;
    civ_cache_entry_t* prev = NULL;
    
    while (entry) {
        if (entry->expiry > 0 && now > entry->expiry) {
            /* Remove expired entry */
            civ_cache_entry_t* next = entry->next;
            if (prev) {
                prev->next = next;
            } else {
                cache->entries = next;
            }
            
            cache->current_size -= entry->data_size;
            cache->entry_count--;
            CIV_FREE(entry->data);
            CIV_FREE(entry);
            entry = next;
        } else {
            prev = entry;
            entry = entry->next;
        }
    }
}

size_t civ_cache_get_size(const civ_cache_t* cache) {
    if (!cache) return 0;
    return cache->current_size;
}

