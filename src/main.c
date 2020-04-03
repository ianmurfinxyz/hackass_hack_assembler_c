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
 * file: main.c
 *
 *===================================================================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "symbollib.h"
#include "parser.h"
#include "asmerr.h"
#include "decoder.h"

#define VERBOSE(X)if(g_is_verbose){fprintf(stdout, X);}
#define VERBOSE2(X, Y)if(g_is_verbose){fprintf(stdout, X, Y);}

#define MAX_ADDRESS 32768        // RAM and ROM on the Hack platform are both 15-bit addressed 32K memory.
#define RAM_START_ADDRESS 1024
#define MAX_FILENAME_CHAR 256    // filename have 256 character max on linux.
#define MAX_FILEPATH_CHAR 4096   // file paths have max 4K bytes on linux.

/*
 * Data used in solution to print binary representation of 16-bit instructions. Adapted from:
 *  source: https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format?page=1&tab=votes#tab-top
 */
static const char* g_bitstr[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

/*
 * operation modes of the assembler.
 */
typedef enum Mode {
  MODE_HELP,        // outputs a help message.
  MODE_STRIP,       // strips whitespace, comments and symbols from a .asm file; outputs another .asm file.
  MODE_ASSEMBLE     // converts a .asm file to a .hack file containing 'Hack' machine instructions in string form.
} Mode_t;

static Mode_t g_mode;
static struct SymLib* gp_sym_lib;                  // for user defined symbols.
static Parser_t* gp_parser;
static char* g_ifpath;                             // file path string to input .asm file (dynamically allocated).
static char g_ofname[MAX_FILENAME_CHAR];           // name of output file.
static FILE* g_ofstream;                           // output file stream to print results.
static uint16_t g_line_count;                      // number of non-whitespace/comment lines in the translation unit.
static uint16_t g_ins_count;                       // number of instructions to generate (= num_line - num_L_commands).
static uint16_t g_ram_address = RAM_START_ADDRESS; // the next ram address to store a new variable.
static Command_t* gp_cmds;                         // array of command structs generated from parsing lines.
static uint16_t* gp_hackins;                       // array of hack machine instructions.
static int g_asm_fail;                             // flag indicates if assembly failed.
static bool g_is_verbose;                          // flag to control verbose output. 

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: callback registered with 'atexit' to clean up all dynamic memory.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
void clean_exit(){
  if(gp_sym_lib){
    free_symlib(&gp_sym_lib); 
  }
  if(gp_parser){
    free_parser(&gp_parser);
  }
  if(gp_cmds){
    free(gp_cmds);
  }
  if(gp_hackins){
    free(gp_hackins);
  }
  if(g_ifpath){
    free(g_ifpath);
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
static void next_ram(){
  ++g_ram_address;
  if(g_ram_address > MAX_ADDRESS){
    fprintf(stderr, "exceeded RAM size, variable with address '%x' cannot fit in 32K memory/n", g_ram_address);
    g_asm_fail = FAIL;
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
static inline void next_line(){
  ++g_line_count;
}

/*-------------------------------------------------------------------------------------------------------------------*/
static void next_instruction(){
  ++g_ins_count;
  if(g_line_count > MAX_ADDRESS){
    fprintf(stderr, "exceeded ROM size, instruction '%d' cannot fit in 32K memory\n", g_line_count);
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
      result = symlib_add_symbol(gp_sym_lib, p_sym->_sym, g_ins_count);
      assert(result == SUCCESS || result == ERROR_1);
      return (result == SUCCESS) ? SUCCESS : FAIL;
  }
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
static int parse_symbols(){
  Symbol_t sym;
  int result;

  // first add all predefined symbols...
  strcpy(sym._sym, "SP");
  assert(symlib_add_symbol(gp_sym_lib, sym._sym, 0) == SUCCESS);
  strcpy(sym._sym, "LCL");
  assert(symlib_add_symbol(gp_sym_lib, sym._sym, 1) == SUCCESS);
  strcpy(sym._sym, "ARG");
  assert(symlib_add_symbol(gp_sym_lib, sym._sym, 2) == SUCCESS);
  strcpy(sym._sym, "THIS");
  assert(symlib_add_symbol(gp_sym_lib, sym._sym, 3) == SUCCESS);
  strcpy(sym._sym, "THAT");
  assert(symlib_add_symbol(gp_sym_lib, sym._sym, 4) == SUCCESS);
  char rx[4];
  for(int r = 0; r <= 15; ++r){
    snprintf(rx, 4, "R%d", r);
    strcpy(sym._sym, rx);
    assert(symlib_add_symbol(gp_sym_lib, sym._sym, r) == SUCCESS);
  }
  strcpy(sym._sym, "SCREEN");
  assert(symlib_add_symbol(gp_sym_lib, sym._sym, 16384) == SUCCESS);
  strcpy(sym._sym, "KBD");
  assert(symlib_add_symbol(gp_sym_lib, sym._sym, 24576) == SUCCESS);

  // parse all labels...
  while((result = parser_next_symbol(gp_parser, &sym)) != CMD_EOF){
    if(result != FAIL && sym._type == SYMBOL_L){
      VERBOSE2("found label symbol '%s', adding to symbol library...\n", sym._sym);
      if(add_symbol(&sym) != SUCCESS){
        fprintf(stderr, "multiple declerations of label %s - labels must be unique\n", sym._sym);
        g_asm_fail = FAIL; 
      }
    }
    else{
      next_instruction(); // dont count L commands; they dont generate instructions.
    }
    next_line();
  }
  parser_rewind(gp_parser); 

  // then parse all variables...
  while((result = parser_next_symbol(gp_parser, &sym)) != CMD_EOF){
    if(result == FAIL){
      continue;
    }
    if(sym._type != SYMBOL_A){
      continue;
    }
    if(add_symbol(&sym) == SUCCESS){
      VERBOSE2("found variable symbol '%s', adding to symbol library...\n", sym._sym);
    }
  }
  parser_rewind(gp_parser); 
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: parse assembly instructions into command structs.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int parse_commands(){
  gp_cmds = (Command_t*)calloc(g_line_count, sizeof(Command_t));
  int cmdno = 0;
  int result; 
  while((result = parser_next_command(gp_parser, &gp_cmds[cmdno])) != CMD_EOF && cmdno < g_line_count){
    if(result == FAIL){
      g_asm_fail = FAIL;
    }
    if(g_is_verbose && result == SUCCESS){
      parser_print_cmdrep(stderr, &gp_cmds[cmdno]);
    }
    ++cmdno;
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: operates on the global gp_cmds command array; if command has a symbol, substitutes it for mapped  literal.
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
/*
 * brief: operates on the global gp_cmds command array; translates commands into hack machine instructions.
 * note: command array MUST first have all symbols substituted for their literal values.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int generate_hackins(){
  VERBOSE("generating Hack instructions...\n");
  gp_hackins = (uint16_t*)calloc(g_line_count, sizeof(uint16_t));
  int in = 0;
  for(int cn = 0; cn < g_line_count; ++cn){
    int type = gp_cmds[cn]._type;
    if(type == CFORMAT_A1 || type == CFORMAT_C0 || type == CFORMAT_C1 || type == CFORMAT_C2){
      decode(&gp_cmds[cn], &gp_hackins[in]);
      ++in;
    }
  }
  assert(in == g_ins_count);
}

/*-------------------------------------------------------------------------------------------------------------------*/
static int print_hackins(FILE* stream){
  VERBOSE2("printing Hack instructions to file '%s'...\n", g_ofname);
  for(int in = 0; in < g_ins_count; ++in){
    uint16_t i = gp_hackins[in];
    fprintf(stream, "%s%s%s%s\n", g_bitstr[i >> 12], g_bitstr[(i >> 8) & 0x000F], g_bitstr[(i >> 4) & 0x000F], g_bitstr[i & 0x000F]); 
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
static int print_stripped_assembly(FILE* stream){
  VERBOSE2("printing assembly commands to file '%s'...\n", g_ofname);
  for(int cn = 0; cn < g_line_count; ++cn){
    if(gp_cmds[cn]._type != CFORMAT_L0 && gp_cmds[cn]._type != CFORMAT_L1){
      parser_print_cmdasm(stream, &gp_cmds[cn]);
    }
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
static void print_help(){
  printf("USAGE\n  hackass infile [-o outfile] [-a|-s|-h] [-v]\n\n"
          "OPTIONS\n"
          "  -a    Assemble .asm infile to .hack outfile (default mode).\n"
          "  -s    Strip .asm infile of whitespace, comments and symbols.\n"
          "  -h    Print this help message.\n"
          "  -v    Print verbose assembler output to stdout.\n"
          "  -o    Specify name of outfile, default is a.out\n\n"
          "For more detailed help, please see,\n"
          "<https://github.com/imurf/hack-assembler-c>\n");
}

/*-------------------------------------------------------------------------------------------------------------------*/
static void parse_args(int argc, char* argv[]){
  bool is_error = false;

  // parse switches...
  int oi = -1;
  bool s = false, h = false, a = false, o = false, v = false;
  for(int i = 1; i < argc; ++i){
    if(argv[i][0] == '-'){
      for(int j = 1; j < strlen(argv[i]); ++j){
        switch(argv[i][j]){
          case 's':
            s = true;
            break;
          case 'h':
            h = true;
            break;
          case 'a':
            a = true;
            break;
          case 'o':
            oi = i;
            o = true;
            break;
          case 'v':
            v = true;
            break;
          default:
            fprintf(stderr, "fatal error: unrecognised command line option '-%c'\n", argv[i][j]);
            is_error = true;
        }
      }
    }
  }

  if(v){
    g_is_verbose = true;
  }

  if(h && !s && !a){
    g_mode = MODE_HELP;
    return;
  }
  else if(s && !h && !a){
    VERBOSE("started MODE_STRIP, stripping comments, whitespace and symbols from .asm input...\n");
    g_mode = MODE_STRIP;
  }
  else if(!h && !s){
    VERBOSE("started MODE_ASSEMBLE, beginning assembly of .asm input to .hack file...\n");
    g_mode = MODE_ASSEMBLE;
  }
  else{
    fprintf(stderr, "fatal error: conflicting operation modes; -h,-a,-s are mutually exclusive\n");
    is_error = true;
  }

  // search for .asm file input...
  for(int i = 1; i < argc; ++i){
    if(argv[i][0] == '-'){
      continue;
    }
    if(oi == (i - 1)){ // if string follows -o switch then this is not the input file you are looking for.
      continue;
    }
    int l = strlen(argv[i]);
    if(l < MAX_FILEPATH_CHAR && strstr(argv[i], ".asm") != NULL){
      g_ifpath = (char*)calloc(l, sizeof(char)); 
      strcpy(g_ifpath, argv[i]);
    }
  }
  if(g_ifpath == NULL){
    fprintf(stderr, "fatal error: no input file\n");
    is_error = true;
  }



  strncpy(g_ofname, "a.out", MAX_FILENAME_CHAR);
  if(o){
    if(oi + 1 >= argc || argv[oi + 1][0] =='-'){
      fprintf(stderr, "fatal error: specified '-o' option but provided no file name\n"); 
      is_error = true;
    }
    else{
      strncpy(g_ofname, argv[oi + 1], MAX_FILENAME_CHAR);
    }
  }

  if(is_error){
    exit(FAIL);
  }

  if(!h){
    g_ofstream = fopen(g_ofname, "w");
    if(g_ofstream == NULL){
      perror(g_ofname);
      exit(FAIL);
    }
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
static void parse_file(){
  VERBOSE2("parsing symbols from input file '%s'...\n", g_ifpath);
  parse_symbols();
  if(g_asm_fail){
    VERBOSE("terminating assembly: symbol errors occured\n");
    exit(FAIL);
  }
  VERBOSE2("parsing assembly commands from input file '%s'...\n", g_ifpath);
  parse_commands();
  if(g_asm_fail){
    VERBOSE("terminating assembly: assembly command errors occured\n");
    exit(FAIL);
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
static void init_assembler(){
  assert(atexit(clean_exit) == SUCCESS);
  assert(new_symlib(&gp_sym_lib) == SUCCESS);
  if((gp_parser = new_parser(g_ifpath)) == NULL){
    exit(FAIL);
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
int main(int argc, char* argv[]){
  parse_args(argc, argv);
  switch(g_mode){
    case MODE_HELP:
      print_help();
      break;
    case MODE_STRIP:
      init_assembler();
      parse_file();
      substitute_symbols(0);
      print_stripped_assembly(g_ofstream);
      fclose(g_ofstream);
      break;
    case MODE_ASSEMBLE:
      init_assembler();
      parse_file();
      substitute_symbols(1);
      init_decoder();
      generate_hackins();
      print_hackins(g_ofstream);
      fclose(g_ofstream);
  }
  return SUCCESS;
}

