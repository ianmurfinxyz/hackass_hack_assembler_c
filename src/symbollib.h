#ifndef _SYMBOL_LIB_H_
#define _SYMBOL_LIB_H_

#include <stdint.h>

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: a library of symbols; maps symbols to RAM/ROM addresses of the Hack computer.
 *
 * note: the SymLib is a Trie data structure with a custom memory allocator; all LibNodes are allocated from the pools.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
struct SymLib;

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: creates and returns a new and initialised SymLib instance.
 *
 * @param <out> p_lib: pointer to reference the new instance to be allocated.
 *
 * return: SUCCESS or ERROR_1 to ERROR_4, errors indicate malloc error.
 *
 * note: calls malloc.
 * note: assumes p_lib is a hanging pointer, if not will result in mem leak!
 * note: guarantees pp_lib == NULL if return ERROR of any kind.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
int new_symlib(struct SymLib** pp_lib);

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: deallocates all memory used by a symbol library.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
int free_symlib(struct SymLib** pp_lib);

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: adds a symbol to the symbol library, mapping said symbol to a RAM/ROM address.
 * @param p_lib: pointer to the symbol library to add the symbol to.
 * @param sym: the symbol to add.
 * @param address: the RAM/ROM address to map to the symbol.
 * return: 
 *        SUCCESS if symbol added.
 *        ERROR_1 if symbol already in the symbol library.
 *        ERROR_2 if failed to modify the underlying data structure, thus cannot add symbol.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
int symlib_add_symbol(struct SymLib* p_lib, const char* sym, uint16_t address);

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: performs an address lookup in the symbol library.
 * @param p_lib: symbol library to search.
 * @param sym: the symbol to search for.
 * @param <out> p_address: used to return the found address.
 * return: 
 *    SUCCESS if symbol found.
 *    ERROR_1 if symbol not found.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
int symlib_search_symbol(struct SymLib* p_lib, const char* sym, uint16_t* p_address);

#endif
