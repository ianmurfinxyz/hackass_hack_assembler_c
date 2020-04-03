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
 * file: symbollib.c
 *
 *===================================================================================================================*/

#include <stdbool.h> 
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "dynpoolalloc.h"
#include "asmerr.h"

static const size_t LIB_NODE_POOL_SIZE_NODES = 100; /* number of LibNodes per pool allocator. */

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: Node used in Trie data structure of symbol library.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
struct LibNode {
  struct LibNode* _p_first_child;
  struct LibNode* _p_next_sibling;
  uint16_t _data;                 /* _data==character if _is_terminator==false, else _data==RAM/ROM address */
  bool _is_terminator;
};

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * SEE HEADER
 */
/*-------------------------------------------------------------------------------------------------------------------*/
struct SymLib {
  struct LibNode* _p_root;          /* root of Trie of LibNodes. */
  struct DynamicPoolAlloc* _p_pool;     /* custom pool allocator for LibNodes. */
};

/*=====================================================================================================================
 * PRIVATE HELPERS  
 *===================================================================================================================*/
/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: searches through non-terminating children of 'parent' for a child representing the 'c' character,
 *  i.e (char)child->data == c 
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static struct LibNode* find_child(struct LibNode* parent, char c){
  struct LibNode* current = parent->_p_first_child;
  while(current != NULL){
    if(current->_is_terminator == false && (char)current->_data == c){
      return current;
    }
    current = current->_p_next_sibling;
  }
  return current;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: searches through children of parent for a child that is a terminating node, i.e. child->is_terminator==true
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static struct LibNode* find_terminator(struct LibNode* parent){
  struct LibNode* current = parent->_p_first_child;
  while(current != NULL){
    if(current->_is_terminator == true){
      return current;
    }
    current = current->_p_next_sibling;
  }
  return current;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: adds a child LibNode to a LibNode in a symbol library.
 * @param p_lib: the symbol library containing the parent node; will also contain the new child.
 * @param p_parent: the node to add the child to.
 * @param data: either a character or a RAM/ROM address depending on if new child is a terminating node.
 * @param is_terminator: flag to indicate if new child is a terminating node.
 * @param <out> pp_child: used to return a pointer to the new child node.
 * returns:
 *    SUCCESS if child added.
 *    ERROR_1 if failed to allocate memory for new child.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int add_child(struct SymLib* p_lib, struct LibNode* p_parent, uint16_t data, bool is_terminator, struct LibNode** pp_child){
  // allocate memory for the new child from the symbol libraries dynamic pool allocator...
  int err;
  void* mem = NULL;
  err = dynamic_pool_malloc(p_lib->_p_pool, &mem);
  if(err != SUCCESS){
    return ERROR_1; // this will essentially cause a memory leak within the pool, can be resolved by freeing the entire pool.
  }
  struct LibNode* new_child = (struct LibNode*)mem;

  // init the new child...
  new_child->_p_first_child = NULL;
  new_child->_p_next_sibling = NULL;
  new_child->_data = data;
  new_child->_is_terminator = is_terminator;

  // link the child to the last node in the sibling list...
  if(p_parent->_p_first_child == NULL){
    p_parent->_p_first_child = new_child;
  }
  else{
    struct LibNode* current_child = p_parent->_p_first_child;
    while(true){
      if(current_child->_p_next_sibling == NULL){
        current_child->_p_next_sibling = new_child; 
        break;
      }
      current_child = current_child->_p_next_sibling;
    }
  }
  
  *pp_child = new_child;
  return SUCCESS;
}

/*=====================================================================================================================
 * PUBLIC INTERFACE 
 *===================================================================================================================*/
/*-------------------------------------------------------------------------------------------------------------------*/
int new_symlib(struct SymLib** pp_lib){
  int err;

  // allocate the symbol library...
  (*pp_lib) = NULL;
  (*pp_lib) = (struct SymLib*)malloc(sizeof(struct SymLib));
  if((*pp_lib) == NULL){
    return ERROR_1;
  }
  
  // create the custom pool allocator...
  err = new_dynamic_pool_alloc(&((*pp_lib)->_p_pool), sizeof(struct LibNode), LIB_NODE_POOL_SIZE_NODES);
  if(err != SUCCESS){
    free((*pp_lib));
  }

  // allocate the root of the symbol libraries Trie data structure from the pool...
  void* mem = NULL;
  err = dynamic_pool_malloc((*pp_lib)->_p_pool, &mem);
  if(err != SUCCESS){
    free_dynamic_pool_alloc(&((*pp_lib)->_p_pool));
    free((*pp_lib));
    pp_lib = NULL;
    return ERROR_4;
  } 
  (*pp_lib)->_p_root = (struct LibNode*)mem;
  
  // init the root of the Trie...
  (*pp_lib)->_p_root->_p_first_child = (*pp_lib)->_p_root->_p_next_sibling = NULL;
  (*pp_lib)->_p_root->_data = 0;  /* data in root is not used. */
  (*pp_lib)->_p_root->_is_terminator = false;

  return SUCCESS;
}

/*-------------------------------------------------------------------------------------------------------------------*/
int free_symlib(struct SymLib** pp_lib){
  free_dynamic_pool_alloc(&((*pp_lib)->_p_pool));
  free((*pp_lib));
  (*pp_lib) = NULL;
  return SUCCESS;
}

/*-------------------------------------------------------------------------------------------------------------------*/
int symlib_add_symbol(struct SymLib* p_lib, const char* sym, uint16_t address){
  int err; 

  // find the last character in the symbol for which a node exists, and find its node...
  struct LibNode* node = p_lib->_p_root;
  int i = 0;
  for(; sym[i]!='\0'; ++i){
    struct LibNode* child = find_child(node, sym[i]);
    if(child != NULL){
      node = child;
    }
    else{
      break;
    }
  }

  // if found a node for all characters in the symbol, determine if the last characters node has a 
  // terminating node. If it does, this symbol already exists in the library.
  if(sym[i] == '\0'){
    struct LibNode* terminator = find_terminator(node);
    if(terminator != NULL){
      return ERROR_1;
    }
  }

  // add nodes for the remaining characters...
  for(int j = i; sym[j] != '\0'; ++j){
    struct LibNode* p_child = NULL;
    err = add_child(p_lib, node, (uint16_t)sym[j], false, &p_child);
    if(err != SUCCESS){
      return ERROR_2;
    }
    node = p_child;
  }

  // finally add the terminator which stores the RAM/ROM address...
  add_child(p_lib, node, address, true, &node);

  return SUCCESS;
}

/*-------------------------------------------------------------------------------------------------------------------*/
int symlib_search_symbol(struct SymLib* p_lib, const char* sym, uint16_t* p_address){
  // find the last character in the symbol for which a node exists, and find its node...
  struct LibNode* node = p_lib->_p_root;
  int i = 0;
  for(; sym[i]!='\0'; ++i){
    struct LibNode* child = find_child(node, sym[i]);
    if(child != NULL){
      node = child;
    }
    else{
      break;
    }
  }

  // if found a node for all characters in the symbol, determine if the last characters node has a 
  // terminating node. If it does, this symbol does exists in the library.
  if(sym[i] == '\0'){
    struct LibNode* terminator = find_terminator(node);
    if(terminator != NULL){
      *p_address = terminator->_data;
    }
    else{
      return ERROR_1;
    }
  }

  return SUCCESS;
}
