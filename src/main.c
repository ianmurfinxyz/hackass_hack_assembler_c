#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "symbollib.h"
#include "parser.h"
#include "asmerr.h"
#include "decoder.h"

#define MAX_ADDRESS 32768 // RAM and ROM on the Hack platform are both 15-bit addressed 32K memory.

/*
 * operation modes of the assembler.
 */
typedef enum Mode {
  MODE_STRIP_WSC,   // strips all whitespace and comments from a .asm file; outputs another .asm file.
  MODE_STRIP_ALL,   // strips whitespace, comments and symbols from a .asm file; outputs another .asm file.
  MODE_ASSEMBLE     // converts a .asm file to a .hack file containing 'Hack' machine instructions in string form.
} Mode_t;

static Mode_t g_mode;

#define RAM_START_ADDRESS 1024

//#define CHECK_SUCCESS(X)if(X != SUCCESS){exit(-1);}
//#define CHECK_VALID(X)if(X == NULL){exit(-1);}

static struct SymLib* gp_sym_lib;   // for user defined symbols.

static Parser_t* gp_parser = NULL;

static uint16_t g_line_count;  // count of number of lines in the translation unit.
static uint16_t g_ram_address; // the next ram address to store a new variable.

static Command_t* gp_cmds;     // array of command structs generated from parsing lines.

static int g_insno;            // count of number of instructions generated (note: different to sizeof hackins array).
static uint16_t* gp_hackins;   // array of hack machine instructions.

static int g_asm_fail = false; // flag indicates if assembly failed.

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: callback registered with 'atexit' to clean up all dynamic memory.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
void clean_exit(){
  if(gp_sym_lib != NULL){
    free_symlib(&gp_sym_lib); 
  }
  if(gp_parser != NULL){
    free_parser(&gp_parser);
  }
  if(gp_cmds != NULL){
    free(gp_cmds);
  }
  if(gp_hackins != NULL){
    free(gp_hackins);
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
static void next_ram(){
  ++g_ram_address;
  if(g_ram_address > MAX_ADDRESS){
    fprintf(stderr, "exceeded RAM size, variable with address '%x' cannot fit in 32K memory", g_ram_address);
    g_asm_fail = FAIL;
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
static void next_line(){
  ++g_line_count;
  if(g_line_count > MAX_ADDRESS){
    fprintf(stderr, "exceeded ROM size, instruction '%d' cannot fit in 32K memory", g_line_count);
    g_asm_fail = FAIL;
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: attemps to add the symbol to the appropriate library.
 * returns: SUCCESS if symbol added, FAIL if symbol already exists in the library.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int add_symbol(Symbol_t* p_sym){
  assert(p_sym->_type == SYMBOL_A || p_sym->_type == SYMBOL_L);
  int result;
  switch(p_sym->_type){
    case SYMBOL_A:
      result = symlib_add_symbol(gp_sym_lib, p_sym->_sym, g_ram_address);
      assert(result == SUCCESS || result == ERROR_1);
      return (result == SUCCESS) ? next_ram(), SUCCESS : FAIL;
    case SYMBOL_L:
      result = symlib_add_symbol(gp_sym_lib, p_sym->_sym, g_line_count + 1);
      assert(result == SUCCESS || result == ERROR_1);
      return (result == SUCCESS) ? SUCCESS : FAIL;
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
static void populate_predefined_symbols(){

}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: searches the translation unit for symbols and adds all unique symbols to the library.
 *
 * note: this operation must be done in 2 phases because an '@' assembly instruction is ambiguous; it is not 
 *  possible to know if the symbol after the '@' refers to a variable or a label without first knowing what labels
 *  exist.
 *
 * note: this function also counts the number of lines in the file, and for this reason MUST be the first operation
 *  performed by the assembler; this is convenient for initialising labels.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int populate_symbols(){
  Symbol_t sym;
  int result;

  // first populate all labels...

  while((result = parser_next_symbol(gp_parser, &sym)) != CMD_EOF){
    next_line();
    if(result == FAIL || sym._type != SYMBOL_L){
      continue;
    }
    if(add_symbol(&sym) != SUCCESS){
      fprintf(stderr, "multiple declerations of label %s - labels must be unique.", sym._sym);
      g_asm_fail = FAIL; 
    }
  }
  //while(parser_has_next(gp_parser)){
  //  if((parser_next_symbol(gp_parser, &sym) == SUCCESS) && sym._type == SYMBOL_L){
  //    if(add_symbol(&sym) != SUCCESS){
  //      fprintf(stderr, "multiple declerations of label %s - labels must be unique.", sym._sym);
  //      g_asm_fail = FAIL; 
  //    }
  //  }
  //  next_line();
  //}
  parser_rewind(gp_parser); 

  // then populate all variables...
  while((result = parser_next_symbol(gp_parser, &sym)) != CMD_EOF){
    if(result == FAIL){
      continue;
    }
    if(sym._type != SYMBOL_A){
      continue;
    }
    add_symbol(&sym);
  }
  //while(parser_has_next(gp_parser)){
  //  if((parser_next_symbol(gp_parser, &sym) == SUCCESS) && sym._type == SYMBOL_A){
  //    add_symbol(&sym);
  //  }
  //}
  parser_rewind(gp_parser); 
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: 
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int parse_asm(){
  gp_cmds = (Command_t*)calloc(g_line_count, sizeof(Command_t));
  int cmdno = 0;
  int result; 
  while((result = parser_next_command(gp_parser, &gp_cmds[cmdno])) != CMD_EOF && cmdno < g_line_count){
    if(result == FAIL){
      g_asm_fail = FAIL;
    }
    //print_command(stderr, &command);
    ++cmdno;
  }

  //while(parser_has_next(gp_parser) && cmdno < g_line_count){
  //  if(parser_next_command(gp_parser, &gp_cmds[cmdno]) != SUCCESS){
  //     g_asm_fail = FAIL;
  //  }
  //  ++cmdno;
  //}
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: operates on the global gp_cmds command array; if command has a symbol, substitutes it for the literal.
 * @param mode: if mode=0, subs A and L command symbols, if mode!=0 subs only A command symbols.
 * note: expects the symbol library to contain ALL symbols encountered; should be guaranteed by the symbol populating
 *  phase.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int substitute_symbols(int mode){
  char addstr[6]; // max 5-digits in a 15-bit literal.
  for(int cn = 0; cn < g_line_count; ++cn){
    if(gp_cmds[cn]._type == CFORMAT_A0 || (gp_cmds[cn]._type == CFORMAT_L0 && mode == 0)){
      uint16_t add;
      assert(symlib_search_symbol(gp_sym_lib, gp_cmds[cn]._sym, &add) == SUCCESS);
      snprintf(addstr, 6, "%d", add); 
      ++gp_cmds[cn]._type; // change to CFORMAT_A/L1
      memset((void*)gp_cmds[cn]._sym, '\0', MAX_SYM_LENGTH);
      strncpy(gp_cmds[cn]._sym, addstr, 6);
    }
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
static int generate_asm(){

}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: operates on the global gp_cmds command array; translates commands into hack machine instructions.
 * note: command array MUST first have all symbols substituted for their literal values.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int generate_instructions(){
  gp_hackins = (uint16_t*)calloc(g_line_count, sizeof(uint16_t));
  int in = 0;
  for(int cn = 0; cn < g_line_count; ++cn){
    int type = gp_cmds[cn]._type;
    if(type == CFORMAT_A1 || type == CFORMAT_C0 || type == CFORMAT_C1 || type == CFORMAT_C2){
      decode(&gp_cmds[cn], &gp_hackins[in]);
      ++in;
    }
  }
  g_insno = in; // can be smaller than hackins array as L commands dont generate instructions.
}

/*-------------------------------------------------------------------------------------------------------------------*/
static int print_instructions(FILE* stream){
  char insstr[17];
  for(int in = 0; in < g_insno; ++in){
    snprintf(insstr, 17, "%x", gp_hackins[in]);
    fprintf(stream, "%s\n", insstr);
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char* argv[]){
  if(atexit(clean_exit)){
    exit(-1);
  }

  if(new_symlib(&gp_sym_lib) != SUCCESS){exit(-1);}
  if((gp_parser = new_parser("test.asm")) == NULL){exit(-1);}

  populate_predefined_symbols();
  populate_symbols();
  if(g_asm_fail){
    exit(FAIL);
  }

  parse_asm();
  if(g_asm_fail){
    exit(FAIL);
  }

  g_mode = MODE_ASSEMBLE;     // temp

  // these operations depend on the .asm input file being parsed without errors.
  switch(g_mode){
    case MODE_STRIP_WSC:
      // generate the .asm file from the command array.
    case MODE_STRIP_ALL:
      // sub all symbols
      // generate the .asm file from the subbed command array.
      substitute_symbols(0);
    case MODE_ASSEMBLE:
      substitute_symbols(1);
      init_decoder();
      generate_instructions();
      print_instructions(stdout);
  }
  
  return SUCCESS;
}

