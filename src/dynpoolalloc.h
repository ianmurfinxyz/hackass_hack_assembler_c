#ifndef _DYN_POOL_ALLOC_H_
#define _DYN_POOL_ALLOC_H_

/*
 * brief: simple expandable-size generic pool allocator.
 *
 */
struct DynamicPoolAlloc;

/*
 * brief: creates a new instance of a dynamic pool allocator.
 *
 */
int new_dynamic_pool_alloc(struct DynamicPoolAlloc** p_dynpool, size_t alloc_size_bytes, size_t sub_pool_size_allocs);

/*
 * brief: frees all memory used by the dynamic pool allocator, leaves p_dynpool equal to NULL.
 */
void free_dynamic_pool_alloc(struct DynamicPoolAlloc** p_dynpool);

/*
 * brief: allocates a memory chunk from the dynamic pool.
 *
 * note: size of allocation set when pool is created.
 */
int dynamic_pool_malloc(struct DynamicPoolAlloc* p_dynpool, void** mem);

#endif
