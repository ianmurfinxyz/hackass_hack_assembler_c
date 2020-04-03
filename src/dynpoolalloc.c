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
 * file: dynpoolalloc.c
 *
 *===================================================================================================================*/

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include "poolalloc.h"
#include "dynpoolalloc.h"
#include "asmerr.h"

/*
 * brief: node in the linked list of pool allocators.
 */
struct PoolListNode {
  struct PoolListNode* _p_next;
  struct PoolAlloc _pool;
};

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: simple expandable-size generic pool allocator.
 *
 * note: to ensure resizing doesn't invalidate pointers to memory allocated from the pool, resizing is implemented
 *  via a linked list of pool allocators, i.e. resized by making more sub-pools.
 *
 * note: the 'features' of this pool are somewhat restricted to those of the sub-pools, thus the dynamic pool allocator
 *  also does NOT support deallocations; same as PoolAlloc.
 *
 */
/*-------------------------------------------------------------------------------------------------------------------*/
struct DynamicPoolAlloc {
  struct PoolListNode* _p_head; 
  size_t _alloc_size_bytes;
  size_t _sub_pool_size_allocs;
};

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * SEE HEADER
 */
/*-------------------------------------------------------------------------------------------------------------------*/
int new_dynamic_pool_alloc(struct DynamicPoolAlloc** pp_dynpool, size_t alloc_size_bytes, size_t sub_pool_size_allocs){
  // create a new instance of a dynamic pool allocator...
  (*pp_dynpool) = NULL;
  (*pp_dynpool) = (struct DynamicPoolAlloc*)malloc(sizeof(struct DynamicPoolAlloc));
  if((*pp_dynpool) == NULL){
    return ERROR_1;
  }

  // allocate the first node/pool (i.e the head) of the pool alloc linked list...
  (*pp_dynpool)->_p_head = NULL;
  (*pp_dynpool)->_p_head = (struct PoolListNode*)malloc(sizeof(struct PoolListNode));
  if((*pp_dynpool)->_p_head == NULL){
    free((*pp_dynpool));
    return ERROR_2;
  }

  // initialise the first pool in the list...
  (*pp_dynpool)->_p_head->_p_next = NULL;
  int r = init_pool_alloc(&((*pp_dynpool)->_p_head->_pool), alloc_size_bytes, sub_pool_size_allocs);
  if(r != SUCCESS){
    free((*pp_dynpool)->_p_head);
    free((*pp_dynpool));
    return ERROR_3;
  }

  (*pp_dynpool)->_alloc_size_bytes = alloc_size_bytes;
  (*pp_dynpool)->_sub_pool_size_allocs = sub_pool_size_allocs;

  return SUCCESS;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * SEE HEADER
 */
/*-------------------------------------------------------------------------------------------------------------------*/
void free_dynamic_pool_alloc(struct DynamicPoolAlloc** pp_dynpool){
  struct PoolListNode* current = (*pp_dynpool)->_p_head;
  while(true){
    if(current->_p_next == NULL){ // if current node is last node...
      break;
    }
    if(current->_p_next->_p_next == NULL){ // if next node is last node...
      free_pool_alloc(&(current->_p_next->_pool));
      free(current->_p_next);
      current->_p_next = NULL;
      current = (*pp_dynpool)->_p_head; // return to start of list; restart search for last node. 
      continue;
    }
    current = current->_p_next;
  }
  free_pool_alloc(&((*pp_dynpool)->_p_head->_pool));
  free((*pp_dynpool)->_p_head);
  free((*pp_dynpool));
  (*pp_dynpool) = NULL;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * SEE HEADER
 */
/*-------------------------------------------------------------------------------------------------------------------*/
int dynamic_pool_malloc(struct DynamicPoolAlloc* p_dynpool, void** mem){
  // find last pool in the list; the 'active' pool...
  struct PoolListNode* active = p_dynpool->_p_head;
  while(true){
    if(active->_p_next != NULL){
      active = active->_p_next;
    }
    else {
      break;
    }
  }

  // allocate a chunk of memory...
  (*mem) = pool_malloc(&active->_pool);

  // if successfully allocated the memory, return early...
  if((*mem) != NULL){
    return SUCCESS;
  }

  // if failed to allocate from active pool then pool is full; so create another one...
  struct PoolListNode* new_node = (struct PoolListNode*)malloc(sizeof(struct PoolListNode));
  new_node->_p_next = NULL;
  int err = init_pool_alloc(&(new_node->_pool), p_dynpool->_alloc_size_bytes, p_dynpool->_sub_pool_size_allocs);
  if(err != SUCCESS){
    free(new_node);
    return ERROR_2;
  }
  active->_p_next = new_node;
  active = active->_p_next; 

  // attempt to allocate from the new pool...
  (*mem) = pool_malloc(&active->_pool);

  // if failed to allocate from newly created pool.
  if((*mem) == NULL){
    return ERROR_3;
  }

  return SUCCESS;
}
