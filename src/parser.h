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
 * file: parser.h
 *
 *===================================================================================================================*/

#ifndef _PARSER_H_
#define _PARSER_H_

#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>

#define CMD_EOF 0xc001 

/*
 * ids for command formats; 5 different valid formats of assembly instructions.
 */
#define CFORMAT_XX 0xa0 // unkown command format.
#define CFORMAT_C0 0xa1 // C command: <destination>=<computation>;<jump> 
#define CFORMAT_C1 0xa2 // C command: <destination>=<compuation>
#define CFORMAT_C2 0xa3 // C command: <computation>;<jump>
#define CFORMAT_AX 0xa4 // A command: unknown if A0 or A1.
#define CFORMAT_A0 0xa5 // A command: @<symbol>
#define CFORMAT_A1 0xa6 // A command: @<literal>
#define CFORMAT_LX 0xa7 // L command: unknown if L0 or L1
#define CFORMAT_L0 0xa8 // L command: (<symbol>)
#define CFORMAT_L1 0xa9 // L command: (<literal>)

/*
 * ids for symbols format; used for Symbol_t::_type member.
 */
#define SYMBOL_X 0xaa   // unknown symbol type
#define SYMBOL_A 0xab   // A command symbol
#define SYMBOL_L 0xac   // L command symbol

#define MAX_MNEMONIC_CHAR_LENGTH 4 // no language mnemonic in 'Hack' assembly is longer than this.
#define MAX_SYM_LENGTH 63  // this determines the maximum length of user-defined assembly symbols.

/*
 * brief: closed parser type; instatiate with 'new_parser' to use this module.
 */
typedef struct Parser Parser_t;

/*
 * brief: Parsed assembly instruction data.
 *
 * @member _type: the format type of the command (CFORMAT_C0, CFORMAT_C1 ...).
 * @member _sym: buffer to store symbol string.
 * @member _dest: destination mnemonic extracted from a C command of format C0 or C1.
 * @member _comp: computation mnemonic extracted from a C command.
 * @member _jump: jump mnemonic extracted from a C command of format C0 or C2.
 *
 * note: check _type before reading other members; members are only set if the command type has the member. Garbage 
 *  values will reside in the members the command doesn't have. For example, only the _literal member if set if the
 *  _type if CFORMAT_A0 | CFORMAT_L0.
 */
typedef struct Command {
  uint8_t _type;
  char _sym[MAX_SYM_LENGTH];
  char _dest[MAX_MNEMONIC_CHAR_LENGTH]; 
  char _comp[MAX_MNEMONIC_CHAR_LENGTH];
  char _jump[MAX_MNEMONIC_CHAR_LENGTH];
} Command_t;

/*
 * brief: parsed symbol data. Used in parsing phase that only extracts symbol data.
 *
 * @member _type: either SYMBOL_X | SYMBOL_A | SYMBOL_L
 * @member _sym: buffer to store symbol string.
 */
typedef struct Symbol {
  uint8_t _type;
  char _sym[MAX_SYM_LENGTH];
} Symbol_t;

Parser_t* new_parser(const char* filename);
void free_parser(Parser_t** p_parser);
int parser_next_command(Parser_t* p_parser, Command_t* p_out);
int parser_next_symbol(Parser_t* p, Symbol_t* p_out);
bool parser_has_next(Parser_t* p_parser);
void parser_rewind(Parser_t* p_parser);
void parser_print_cmdrep(FILE* stream, Command_t* c);
int parser_print_cmdasm(FILE* stream, Command_t* c);

#endif
