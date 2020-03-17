#include <stddef.h>
#include <stdlib.h>
#include "poolalloc.h"
#include "defs.h"

/*--------------------------------------------------------------------------------------------------------------------*/
int init_pool_alloc(struct PoolAlloc* p_pool, size_t alloc_size_bytes, size_t pool_size_allocs){
  p_pool->_alloc_size_bytes = alloc_size_bytes;
  p_pool->_pool_size_bytes = pool_size_allocs * alloc_size_bytes;
  p_pool->_p_base = (struct PoolAlloc*)malloc(p_pool->_pool_size_bytes);
  if(p_pool->_p_base == NULL){
    return 1;
  }
  p_pool->_p_next = p_pool->_p_base;
  return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
void free_pool_alloc(struct PoolAlloc* p_pool){
  free(p_pool->_p_base);
  p_pool->_p_base = NULL;
  p_pool->_p_next = NULL;
  p_pool->_pool_size_bytes = p_pool->_alloc_size_bytes = 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
void* pool_malloc(struct PoolAlloc* p_pool){
  if(p_pool->_p_next == (p_pool->_p_base + p_pool->_pool_size_bytes)){
    return NULL;
  }
  void* radd = p_pool->_p_next;
  p_pool->_p_next += p_pool->_alloc_size_bytes;
  return radd;
}

/*--------------------------------------------------------------------------------------------------------------------*/
size_t calc_free_allocs(struct PoolAlloc* p_pool){
  return ((p_pool->_p_base + p_pool->_pool_size_bytes) - p_pool->_p_next) / p_pool->_alloc_size_bytes;
}
/*--------------------------------------------------------------------------------------------------------------------*/

