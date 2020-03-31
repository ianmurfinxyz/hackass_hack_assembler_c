#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "symbollib.h"
#include "parser.h"
#include "asmerr.h"

#define RAM_START_ADDRESS 1024

//#define CHECK_SUCCESS(X)if(X != SUCCESS){exit(-1);}
//#define CHECK_VALID(X)if(X == NULL){exit(-1);}

struct SymLib* gp_label_lib; // for goto labels
struct SymLib* gp_var_lib;   // for user defined variables.

Parser_t* gp_parser = NULL;

uint16_t g_line_count;  // count of number of lines in the translation unit.
uint16_t g_ram_address; // the next ram address to store a new variable.

uint16_t* gp_hackins;   // pointer to array of hack machine instructions.

/*
 * brief: callback registered with 'atexit' to clean up all dynamic memory.
 */
void clean_exit(){
  if(gp_label_lib != NULL){
    free_symlib(&gp_label_lib); 
  }
  if(gp_var_lib != NULL){
    free_symlib(&gp_var_lib); 
  }
  if(gp_parser != NULL){
    free_parser(&gp_parser);
  }
}

/*
 * brief: attemps to add the symbol to the appropriate library.
 * returns: SUCCESS if symbol added, FAIL if symbol already exists in the library.
 */
static int add_symbol(Symbol_t* p_sym){
  assert(p_sym->_type == SYMBOL_A || p_sym->_type == SYMBOL_L);
  int result;
  if(p_sym->_type == SYMBOL_A){
    result = symlib_add_symbol(gp_var_lib, p_sym->_sym, g_ram_address);
    assert(result == SUCCESS || result == ERROR_1);
    return (result == SUCCESS) ? ++g_ram_address, SUCCESS : FAIL;
  }
  else{  
    result = symlib_add_symbol(gp_label_lib, p_sym->_sym, g_line_count + 1);
    assert(result == SUCCESS || result == ERROR_1);
    if(result == ERROR_1){
       fprintf(stderr, "multiple declerations of label %s - labels must be unique.", p_sym->_sym);
       g_asm_fail = FAIL; 
       result = FAIL;
    }
    return result;
  }
}

/*
 * brief: loops through every line in the translation unit and extracts all unique symbols, storing them in the
 *  symbols libraries.
 */
static int populate_symbols(){
  Symbol_t sym;
  while(parser_has_next(gp_parser)){
    if(parser_next_symbol(gp_parser, &sym) == SUCCESS){
      add_symbol(&sym);
    }
    ++g_line_count;
  }
  parser_rewind(gp_parser); 
}

/*
 * brief: loops through every line in the translation unit and converts them into 16-bit 'Hack' machine instructions.
 */
static int assemble_instructions(){
  gp_hackins = (uint16_t*)calloc(g_line_count, sizeof(uint16_t));
  Command_t command;
  while(parser_has_next(gp_parser)){
    if(parser_next_command(gp_parser, &command) == SUCCESS){
       print_command(stderr, &command);
    }
  }
}

static int stringify_instruction(uint16_t ins, char* strout, int n){

}

static int print_instructions(FILE* stream){

}

int main(int argc, char* argv[]){
  if(atexit(clean_exit)){
    exit(-1);
  }


  // phase 1 - populate symbol libraries
  // 1) for each line:
  //      if(L command)
  //        extract symbol
  //        if(symbol is new)
  //          calculate ROM address for symbol
  //          add symbol to label symbol library
  //      if(A command)
  //        extract symbol
  //        if(symbol is new)
  //          calculate RAM address for symbol
  //          add symbol to variable symbol library
  //
  // note: also count the number of lines in file for use later.
  //
  // phase 2 - parse commands into hack machine instruction.
  // create array of uint16_t of size 'number of lines in .asm file'
  // for each line:
  //   parse line into command
  //   convert command into uint16_t machine instruction
  //   store instruction
  //
  // phase 3 - stringify the instruction array and output to file
  

  if(new_symlib(&gp_label_lib) != SUCCESS){exit(-1);}
  if(new_symlib(&gp_var_lib) != SUCCESS){exit(-1);}
  if((gp_parser = new_parser("test.asm")) == NULL){exit(-1);}
  //CHECK_SUCCESS(new_symlib(&gp_label_lib));
  //CHECK_SUCCESS(new_symlib(&gp_var_lib));
  //CHECK_VALID((gp_parser = new_parser("test.asm")));

  // run phase 1 of assembling...
  populate_symbols();

  // run phase 2 of assembling...
  if(!g_asm_fail){
    assemble_instructions();
  }
  
  return SUCCESS;
}


/*
int main(int argc, char* argv[]){
  struct SymLib* p_lib = NULL;
  int err;
  err = new_symbol_library(&p_lib);

  // add symbols to the library...
  printf("adding symbol: THAT:0x0b01\n");
  printf("adding symbol: that:0x0c71\n");
  printf("adding symbol: moonCat:0xdb25\n");

  add_symbol(p_lib, "THAT", 0x0b01);
  add_symbol(p_lib, "that", 0x0c71);
  add_symbol(p_lib, "moonCat", 0xdb25);

  // search the symbols in the library...
  printf("retrieving symbols from library...");
  uint16_t address = 0;

  search_symbol_library(p_lib, "THAT", &address);
  printf("symbol THAT address = %#04x\n", address);

  search_symbol_library(p_lib, "that", &address);
  printf("symbol that address = %#04x\n", address);

  search_symbol_library(p_lib, "moonCat", &address);
  printf("symbol moonCat address = %#04x\n", address);

  free_symbol_library(&p_lib);

  return 0;
}*/
