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
 * file: dynpoolalloc.h
 *
 *===================================================================================================================*/

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
