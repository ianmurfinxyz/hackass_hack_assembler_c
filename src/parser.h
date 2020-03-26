#ifndef _PARSER_H_
#define _PARSER_H_

#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>

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

#define MAX_MNEMONIC_CHAR_LENGTH 4 // no language mnemonic in 'Hack' assembly is longer than this.
#define MAX_SYMBOL_CHAR_LENGTH 63  // this determines the maximum length of user-defined assembly symbols.

/*
 * brief: closed parser type; instatiate with 'new_parser' to use this module.
 */
typedef struct Parser Parser_t;

/*
 * brief: Parsed assembly instruction data.
 *
 * @member _type: the format type of the command (CFORMAT_C0, CFORMAT_C1 ...).
 * @member _literal: numeric value extracted from an A or L command.
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
  char _sym[MAX_SYMBOL_CHAR_LENGTH];
  char _dest[MAX_MNEMONIC_CHAR_LENGTH]; 
  char _comp[MAX_MNEMONIC_CHAR_LENGTH];
  char _jump[MAX_MNEMONIC_CHAR_LENGTH];
} Command_t;


/*
 *
 */
Parser_t* new_parser(const char* filename);
/*
 *
 */

void free_parser(Parser_t* p_parser);

/*
 *
 */
int parser_next_command(Parser_t* p_parser, Command_t* p_out);
  
/*
 *
 */
bool parser_has_next(Parser_t* p_parser);

/*
 *
 */
void print_command(FILE* stream, Command_t* c);

#endif
