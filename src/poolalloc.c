/*=====================================================================================================================
 *
 * MIT License
 * 
 * This project was completed by Ian Murfin as part of the Nand2Tetris Audit course 
 * at coursera.
 *
 * It was completed as part of my personal portfolio. Nand2tetris requires submissions
 * be your own work; plagiarism is your responsibility.
 *
 * Copyright (c) 2020 Ian Murfin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in 
 * the Software without restriction, including without limitation the rights to 
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
 * of the Software, and to permit persons to whom the Software is furnished to do 
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all 
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * End license text. 
 *
 * author: Ian Murfin
 * file: poolalloc.c
 *
 *===================================================================================================================*/

#include <stddef.h>
#include <stdlib.h>
#include "poolalloc.h"
#include "asmerr.h"

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

