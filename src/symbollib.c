#include <stdbool.h> 
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "dynpoolalloc.h"
#include "defs.h"

static const size_t LIB_NODE_POOL_SIZE_NODES = 100; /* number of LibNodes per pool allocator. */

// TODO:
// 1) ensure all functions that create a closed type accept ** arguments so they actually modify the pointer passed in 
//    rather than passing a pointer by value and thus causing a memory leak.
// 2) implement the 2 private helpers.
// 3) implement a function to print the contents of a library to the console. will require finding all unique paths
//    in the library tree.



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

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: tests if a character is a member of the set of valid symbol characters, 
 *                 valid_char_set={'a'-'z', 'A'-'Z', '0'-'9', '_', '.', '$', ':'} 
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static bool is_valid_symbol_char(char c){
  if(('a' <= c && c <= 'z') || 
     ('A' <= c && c <= 'Z') || 
     ('0' <= c && c <= '9') || 
     (c == '_') || 
     (c == '.') || 
     (c == '$') || 
     (c == ':')){
    return true;
  }
  return false;
}

/*=====================================================================================================================
 * PUBLIC INTERFACE 
 *===================================================================================================================*/
/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * SEE HEADER
 */
/*-------------------------------------------------------------------------------------------------------------------*/
int new_symbol_library(struct SymLib** pp_lib){
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
/*
 * SEE HEADER
 */
/*-------------------------------------------------------------------------------------------------------------------*/
int free_symbol_library(struct SymLib** pp_lib){
  free_dynamic_pool_alloc(&((*pp_lib)->_p_pool));
  free((*pp_lib));
  (*pp_lib) = NULL;
  return SUCCESS;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * SEE HEADER
 */
/*-------------------------------------------------------------------------------------------------------------------*/
int add_symbol(struct SymLib* p_lib, const char* sym, uint16_t address){
  int err; 

  // check all characters in the symbol are members of the set of valid characters...
  for(int i = 0; sym[i] != '\0'; ++i){
    if(is_valid_symbol_char(sym[i]) == false){
      return ERROR_1;
    }
  }

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
      return ERROR_2;
    }
  }

  // add nodes for the remaining characters...
  for(int j = i; sym[j] != '\0'; ++j){
    struct LibNode* p_child = NULL;
    err = add_child(p_lib, node, (uint16_t)sym[j], false, &p_child);
    if(err != SUCCESS){
      return ERROR_3;
    }
    node = p_child;
  }

  // finally add the terminator which stores the RAM/ROM address...
  add_child(p_lib, node, address, true, &node);

  return SUCCESS;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * SEE HEADER
 */
/*-------------------------------------------------------------------------------------------------------------------*/
int search_symbol_library(struct SymLib* p_lib, const char* sym, uint16_t* p_address){
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


























