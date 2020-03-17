#ifndef _POOLALLOC_H_
#define _POOLALLOC_H_

/* 
 * brief: simple fixed-size generic pool allocator. 
 *
 * note: only supports allocations, NOT deallocations, i.e. once an allocation has been made it cannot be 
 *   freed short of freeing the entire pool. This greatly simplifies pool mamanagement at the cost of it
 *   not being suitable for all applications.
 */
struct PoolAlloc {
  size_t _pool_size_bytes;
  size_t _alloc_size_bytes;
  void* _p_base;             /* base address of the pool, returned from malloc. */
  void* _p_next;             /* address returned upon next allocation. */
};

/*
 * brief: initialises an instance of a Pool Allocator.
 * return: 0 on SUCCESS, 1 on FAILURE to allocate pool memory.
 * note: calls malloc.
 * note: assumes PoolAlloc instance is an unintialised pool. If not call free_pool first to avoid mem leak!
 */
int init_pool_alloc(struct PoolAlloc* p_pool, size_t alloc_size_bytes, size_t pool_size_allocs);

/*
 * brief: frees the memory used by the pool, does NOT free the PoolAlloc instance itself.
 * note: WILL invalidate all pointers to pool memory!
 */
void free_pool_alloc(struct PoolAlloc* p_pool);

/*
 * brief: allocates memory from the pool.
 * returns: pointer to pool memory on SUCCESS, NULL on FAILURE.
 * note: size of allocation set at pool init.
 * note: will fail to allocate if pool is full.
 * note: assumes the pool is initialised, if not seg fault will result upon accessing allocated memory.
 */
void* pool_malloc(struct PoolAlloc* p_pool);

/*
 * brief: calculates the remaining number of allocations the pool can make.
 */
size_t calc_free_allocs(struct PoolAlloc* p_pool);

#endif
