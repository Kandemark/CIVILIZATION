/**
 * @file memory_pool.c
 * @brief Implementation of memory pool allocator
 */

#include "../../include/utils/memory_pool.h"
#include "../../include/common.h"
#include <stdlib.h>
#include <string.h>

civ_memory_pool_manager_t *
civ_memory_pool_manager_create(size_t default_block_size,
                               size_t default_block_count) {
  civ_memory_pool_manager_t *manager = (civ_memory_pool_manager_t *)CIV_MALLOC(
      sizeof(civ_memory_pool_manager_t));
  if (!manager) {
    civ_log(CIV_LOG_ERROR, "Failed to allocate memory pool manager");
    return NULL;
  }

  memset(manager, 0, sizeof(civ_memory_pool_manager_t));
  manager->default_block_size =
      default_block_size > 0 ? default_block_size : 1024;
  manager->default_block_count =
      default_block_count > 0 ? default_block_count : 100;

  return manager;
}

void civ_memory_pool_manager_destroy(civ_memory_pool_manager_t *manager) {
  if (!manager)
    return;

  civ_memory_pool_t *pool = manager->pools;
  while (pool) {
    civ_memory_pool_t *next = pool->next;
    CIV_FREE(pool->used_blocks);
    CIV_FREE(pool->memory);
    CIV_FREE(pool);
    pool = next;
  }

  CIV_FREE(manager);
}

static civ_memory_pool_t *create_pool(size_t block_size, size_t block_count) {
  civ_memory_pool_t *pool =
      (civ_memory_pool_t *)CIV_MALLOC(sizeof(civ_memory_pool_t));
  if (!pool)
    return NULL;

  memset(pool, 0, sizeof(civ_memory_pool_t));
  pool->block_size = block_size;
  pool->block_count = block_count;
  pool->free_count = block_count;

  pool->memory = CIV_MALLOC(block_size * block_count);
  pool->used_blocks = (bool *)CIV_CALLOC(block_count, sizeof(bool));

  if (!pool->memory || !pool->used_blocks) {
    CIV_FREE(pool->memory);
    CIV_FREE(pool->used_blocks);
    CIV_FREE(pool);
    return NULL;
  }

  return pool;
}

void *civ_memory_pool_allocate(civ_memory_pool_manager_t *manager,
                               size_t size) {
  if (!manager)
    return NULL;

  /* Find pool with appropriate block size */
  civ_memory_pool_t *pool = manager->pools;
  while (pool) {
    if (pool->block_size >= size && pool->free_count > 0) {
      /* Find free block */
      for (size_t i = 0; i < pool->block_count; i++) {
        if (!pool->used_blocks[i]) {
          pool->used_blocks[i] = true;
          pool->free_count--;
          return (char *)pool->memory + (i * pool->block_size);
        }
      }
    }
    pool = pool->next;
  }

  /* Create new pool if needed */
  size_t block_size =
      size > manager->default_block_size ? size : manager->default_block_size;
  pool = create_pool(block_size, manager->default_block_count);
  if (!pool) {
    /* Fallback to regular malloc */
    return CIV_MALLOC(size);
  }

  pool->next = manager->pools;
  manager->pools = pool;
  manager->pool_count++;

  /* Allocate from new pool */
  pool->used_blocks[0] = true;
  pool->free_count--;
  return pool->memory;
}

void civ_memory_pool_free(civ_memory_pool_manager_t *manager, void *ptr) {
  if (!manager || !ptr)
    return;

  /* Find which pool this pointer belongs to */
  civ_memory_pool_t *pool = manager->pools;
  while (pool) {
    if ((char *)ptr >= (char *)pool->memory &&
        (char *)ptr <
            (char *)pool->memory + (pool->block_size * pool->block_count)) {
      size_t block_index =
          ((char *)ptr - (char *)pool->memory) / pool->block_size;
      if (block_index < pool->block_count && pool->used_blocks[block_index]) {
        pool->used_blocks[block_index] = false;
        pool->free_count++;
      }
      return;
    }
    pool = pool->next;
  }

  /* Not found in pool, use regular free */
  CIV_FREE(ptr);
}

void civ_memory_pool_reset(civ_memory_pool_manager_t *manager) {
  if (!manager)
    return;

  civ_memory_pool_t *pool = manager->pools;
  while (pool) {
    memset(pool->used_blocks, 0, pool->block_count * sizeof(bool));
    pool->free_count = pool->block_count;
    pool = pool->next;
  }
}
