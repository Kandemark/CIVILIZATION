/**
 * @file memory_pool.h
 * @brief Memory pool allocator for performance
 */

#ifndef CIVILIZATION_MEMORY_POOL_H
#define CIVILIZATION_MEMORY_POOL_H

#include "../common.h"
#include "../types.h"

/* Memory pool structure */
typedef struct civ_memory_pool {
    void* memory;
    size_t block_size;
    size_t block_count;
    size_t free_count;
    bool* used_blocks;
    struct civ_memory_pool* next;
} civ_memory_pool_t;

/* Memory pool manager */
typedef struct {
    civ_memory_pool_t* pools;
    size_t pool_count;
    size_t default_block_size;
    size_t default_block_count;
} civ_memory_pool_manager_t;

/* Function declarations */
civ_memory_pool_manager_t* civ_memory_pool_manager_create(size_t default_block_size, size_t default_block_count);
void civ_memory_pool_manager_destroy(civ_memory_pool_manager_t* manager);
void* civ_memory_pool_allocate(civ_memory_pool_manager_t* manager, size_t size);
void civ_memory_pool_free(civ_memory_pool_manager_t* manager, void* ptr);
void civ_memory_pool_reset(civ_memory_pool_manager_t* manager);

/* Convenience macros */
#define CIV_POOL_ALLOC(manager, type) ((type*)civ_memory_pool_allocate(manager, sizeof(type)))
#define CIV_POOL_ALLOC_ARRAY(manager, type, count) ((type*)civ_memory_pool_allocate(manager, sizeof(type) * (count)))

#endif /* CIVILIZATION_MEMORY_POOL_H */

