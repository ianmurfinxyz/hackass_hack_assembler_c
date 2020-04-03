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
 * file: symbollib.h
 *
 *===================================================================================================================*/

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
 * brief: performs a RAM/ROM address lookup in the symbol library.
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
