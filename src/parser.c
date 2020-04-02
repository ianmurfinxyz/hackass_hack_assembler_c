#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include "asmerr.h"
#include "parser.h"

/*=====================================================================================================================
 * GLOBAL PARSING DATA 
 *===================================================================================================================*/

#define MAX_LINE_LENGTH 200
char g_line[MAX_LINE_LENGTH];        // buffer to store line read from translation unit/file.
char g_error_line[MAX_LINE_LENGTH];  // buffer used to compose error strings.

/*
 * parsing phases/states. (used during mnemonic extraction.)
 */
#define PHASE_DEST 0xa006
#define PHASE_COMP 0xa007
#define PHASE_JUMP 0xa008

/*
 * valid mnemonic sets for command formats (helps to detect errors; arrays initialised in 'new_parser').
 * 
 * note: the <destination>, <computation>, <jump> fields must be one of a fixed set of mnemonics defined by the 
 *  'Hack' assembly language. Further each C command format has a restricted set of these mnemonics.
 *
 * note: format CFORMAT_C2 uses the same restrictions as CFORMAT_C0 except without the destination component.
 */
#define NVDC0 1
#define NVCC0 8
#define NVJC0 7
static char g_valid_dest_c0[NVDC0][MAX_MNEMONIC_CHAR_LENGTH];
static char g_valid_comp_c0[NVCC0][MAX_MNEMONIC_CHAR_LENGTH];
static char g_valid_jump_c0[NVJC0][MAX_MNEMONIC_CHAR_LENGTH];

#define NVDC1 7
#define NVCC1 28
static char g_valid_dest_c1[NVDC1][MAX_MNEMONIC_CHAR_LENGTH];
static char g_valid_comp_c1[NVCC1][MAX_MNEMONIC_CHAR_LENGTH];

/*
 * delimiters used in C command formats.
 */
static char dest_delim = '=';
static char comp_delim = ';';
static char jump_delim = '\0';

/*=====================================================================================================================
 * TYPES
 *===================================================================================================================*/

/*
 * brief: encapsulates data output from a line parsing operation.
 */
typedef struct Parser {
  FILE* _tunit;     /* current translation unit */
  char* _filename;  /* name of current translation unit */
  int _lineno;      /* line number of current line being parsed */
} Parser_t;

/*=====================================================================================================================
 * PRIVATE HELPERS
 *===================================================================================================================*/

/*-------------------------------------------------------------------------------------------------------------------*/
void parse_module_init(){
  strcpy(g_valid_dest_c0[0], "D");

  strcpy(g_valid_comp_c0[0], "D");
  strcpy(g_valid_comp_c0[1], "!D");
  strcpy(g_valid_comp_c0[2], "-D");
  strcpy(g_valid_comp_c0[3], "D+1");
  strcpy(g_valid_comp_c0[4], "D-1");
  strcpy(g_valid_comp_c0[5], "0");
  strcpy(g_valid_comp_c0[6], "1");
  strcpy(g_valid_comp_c0[7], "-1");

  strcpy(g_valid_dest_c1[0], "M");
  strcpy(g_valid_dest_c1[1], "D");
  strcpy(g_valid_dest_c1[2], "MD");
  strcpy(g_valid_dest_c1[3], "A");
  strcpy(g_valid_dest_c1[4], "AM");
  strcpy(g_valid_dest_c1[5], "AD");
  strcpy(g_valid_dest_c1[6], "AMD");

  strcpy(g_valid_comp_c1[0], "0");
  strcpy(g_valid_comp_c1[1], "1");
  strcpy(g_valid_comp_c1[2], "-1");
  strcpy(g_valid_comp_c1[3], "D");
  strcpy(g_valid_comp_c1[4], "A");
  strcpy(g_valid_comp_c1[5], "!D");
  strcpy(g_valid_comp_c1[6], "!A");
  strcpy(g_valid_comp_c1[7], "-D");
  strcpy(g_valid_comp_c1[8], "-A");
  strcpy(g_valid_comp_c1[9], "D+1");
  strcpy(g_valid_comp_c1[10], "A+1");
  strcpy(g_valid_comp_c1[11], "D-1");
  strcpy(g_valid_comp_c1[12], "A-1");
  strcpy(g_valid_comp_c1[13], "D+A");
  strcpy(g_valid_comp_c1[14], "D-A");
  strcpy(g_valid_comp_c1[15], "A-D");
  strcpy(g_valid_comp_c1[16], "D&A");
  strcpy(g_valid_comp_c1[17], "D|A");
  strcpy(g_valid_comp_c1[18], "M");
  strcpy(g_valid_comp_c1[19], "!M");
  strcpy(g_valid_comp_c1[20], "-M");
  strcpy(g_valid_comp_c1[21], "D+M");
  strcpy(g_valid_comp_c1[22], "D-M");
  strcpy(g_valid_comp_c1[23], "M-D");
  strcpy(g_valid_comp_c1[24], "D&M");
  strcpy(g_valid_comp_c1[25], "D|M");
  strcpy(g_valid_comp_c1[26], "M+1");
  strcpy(g_valid_comp_c1[27], "M-1");

  strcpy(g_valid_jump_c0[0], "JGT");
  strcpy(g_valid_jump_c0[1], "JEQ");
  strcpy(g_valid_jump_c0[2], "JGE");
  strcpy(g_valid_jump_c0[3], "JLT");
  strcpy(g_valid_jump_c0[4], "JNE");
  strcpy(g_valid_jump_c0[5], "JLE");
  strcpy(g_valid_jump_c0[6], "JMP");
}

/*-------------------------------------------------------------------------------------------------------------------*/
static const char* format_id_to_string(int fmt){
  switch(fmt){
    case CFORMAT_C0:
      return "C command: <dest>=<comp>;<jump>";
    case CFORMAT_C1:
      return "C command: <dest>=<comp>";
    case CFORMAT_C2:
      return "C command: <comp>;<jump>";
    case CFORMAT_A0:
      return "A command: @<symbol>";
    case CFORMAT_A1:
      return "A command: @<literal>";
    case CFORMAT_L0:
      return "L command: (<symbol>)";
    case CFORMAT_L1:
      return "L command: (<literal>)";
    default:
      return "unrecognised format";
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
static inline void print_error(const char* filename, int line, const char* errstr, const char* code){
  fprintf(stderr, "%s:%d:error:%s\n  %d |%s\n", filename, line, errstr, line, code); 
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: determines the command format of the line.
 * return: CFORMAT_C0 to CFORMAT_L1 on success, FAIL on error.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int parse_format(Parser_t* p, const char* line){
  int nsc = 0; // num ;
  int ne = 0;  // num =
  int nat = 0; // num @
  int nlb = 0;  // num (
  int nrb = 0;  // num )
  for(int i = 0; i < strlen(line); ++i){
    switch(line[i]){
      case ';':
        ++nsc;
        break;
      case '=':
        ++ne;
        break;
      case '@':
        ++nat;
        break;
      case '(':
        ++nlb;
        break;
      case ')':
        ++nrb;
        break;
    }
  }
  if((nsc==1 && ne==1) && (nat==0 && nlb==0 && nrb==0)){
    return CFORMAT_C0;  
  }
  else if((ne==1) && (nsc==0 && nat==0 && nlb==0 && nrb==0)){
    return CFORMAT_C1;  
  }
  else if((nsc==1) && (ne==0 && nat==0 && nlb==0 && nrb==0)){
    return CFORMAT_C2;  
  }
  else if((nat==1) && (ne==0 && nsc==0 && nlb==0 && nrb==0)){
    return CFORMAT_AX;  
  }
  else if((nlb==1 && nrb==1) && (ne==0 && nsc==0 && nat==0)){
    return CFORMAT_LX;  
  }
  else{ 
    print_error(p->_filename, p->_lineno, "unrecognised instruction format", line);
    return FAIL;
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: extracts all characters in 'line' between character position 'lc' and the delimiter 'delim'. Compares the
 *  extracted string against a set of valid strings, and stores the extracted string in 'p_outbuff'.
 * @param mnn: the name of the mnemonic being extracted.
 * @param fmt: string format of the command.
 * return SUCCESS if mnemonic extracted, FAIL if invalid mnemonic, i.e. not in valid set.
 * note: function guranteed to return with lc pointing to next character after delim; delim must be present!
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int extract_mnemonic(Parser_t* p, const char* line, int* lc, char delim, const char* mnn, const char* fmt, char (*p_valid_set)[MAX_MNEMONIC_CHAR_LENGTH], int num_valid, char* p_outbuff){
  int result = SUCCESS;

  // extract mnemonic into temp buffer...
  char buff[MAX_MNEMONIC_CHAR_LENGTH];
  memset((void*)buff, '\0', MAX_MNEMONIC_CHAR_LENGTH); 
  int bc = 0;
  while(line[*lc] != delim && bc < MAX_MNEMONIC_CHAR_LENGTH - 1){
    buff[bc]=line[*lc];
    ++bc;
    ++(*lc);
  }

  // if too long, cannot be a valid mnemonic.
  if(line[*lc] != delim){
    result = FAIL;
  }

  // check mnemonic is a member of the valid set...
  if(result != FAIL){
    int i = 0;
    while(i < num_valid){
      if(strncmp(p_valid_set[i], buff, MAX_MNEMONIC_CHAR_LENGTH) == 0){
        break;
      }
      ++i;
      if(i == num_valid){
        result = FAIL;
      } 
    }
  }

  if(result == FAIL){
    snprintf(g_error_line, MAX_LINE_LENGTH,"invalid %s for C command of format %s", mnn, fmt);
    print_error(p->_filename, p->_lineno, g_error_line, line);
    result = FAIL;
  }

  // save mnemonic to command output struct.
  // note: outputs invalid symbol, caller must check return code!
  strncpy(p_outbuff, buff, MAX_MNEMONIC_CHAR_LENGTH); 

  // must return with lc pointing to char after delimiter.
  while(line[*lc] != '\0' && line[*lc] != delim){
    ++(*lc);
  }
  assert(line[*lc] == delim);
  ++(*lc);

  return result;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: extract format <destination>=<computation>;<jump>
 * return: SUCCESS or FAIL.
 * note: format C0 is limited; no reference to A or M may exist in the destination or
 *  computation, thus:
 *
 *  valid destinations: {D}
 *  valid computations: {D, !D, -D, D+1, D-1, 0, 1, -1}
 *
 * note: all jumps are valid.
 * note: function will parse all mnemonics, even if an invalid mnemonic is found.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int extract_format_C0(Parser_t* p, const char* line, Command_t* p_out){
  static const char* fmt = "<dest>=<comp>;<jump>";

  p_out->_type = CFORMAT_C0;

  int result = SUCCESS; 
  int r;
  int lc = 0;
  for(int phase = PHASE_DEST; phase <= PHASE_JUMP; ++phase){
    switch(phase){
      case PHASE_DEST:
        r = extract_mnemonic(p, line, &lc, dest_delim, "destination", fmt, g_valid_dest_c0, NVDC0, p_out->_dest);
        result = (result == FAIL) ? result : r;
        break;
      case PHASE_COMP:
        r = extract_mnemonic(p, line, &lc, comp_delim, "computation", fmt, g_valid_comp_c0, NVCC0, p_out->_comp);
        result = (result == FAIL) ? result : r;
        break;
      case PHASE_JUMP:
        r = extract_mnemonic(p, line, &lc, jump_delim, "jump", fmt, g_valid_jump_c0, NVJC0, p_out->_jump);
        result = (result == FAIL) ? result : r;
    }
  }
  return result;
}

/*-------------------------------------------------------------------------------------------------------------------*/
static int extract_format_C1(Parser_t* p, const char* line, Command_t* p_out){
  static const char* fmt = "<dest>=<comp>";

  p_out->_type = CFORMAT_C1;

  int result = SUCCESS; 
  int r;
  int lc = 0;
  for(int phase = PHASE_DEST; phase <= PHASE_COMP; ++phase){
    switch(phase){
      case PHASE_DEST:
        r = extract_mnemonic(p, line, &lc, dest_delim, "destination", fmt, g_valid_dest_c1, NVDC1, p_out->_dest);
        result = (result == FAIL) ? result : r;
        break;
      case PHASE_COMP:
        r = extract_mnemonic(p, line, &lc, '\0', "computation",fmt, g_valid_comp_c1, NVCC1, p_out->_comp);
        result = (result == FAIL) ? result : r;
    }
  }
  return result;
}

/*-------------------------------------------------------------------------------------------------------------------*/
static int extract_format_C2(Parser_t* p, const char* line, Command_t* p_out){
  static const char* fmt = "<comp>;<jump>";

  p_out->_type = CFORMAT_C2;

  int result = SUCCESS; 
  int r;
  int lc = 0;
  for(int phase = PHASE_COMP; phase <= PHASE_JUMP; ++phase){
    switch(phase){
      case PHASE_COMP:
        r = extract_mnemonic(p, line, &lc, comp_delim, "computation", fmt, g_valid_comp_c0, NVCC0, p_out->_comp);
        result = (result == FAIL) ? result : r;
        break;
      case PHASE_JUMP:
        r = extract_mnemonic(p, line, &lc, jump_delim, "jump", fmt, g_valid_jump_c0, NVJC0, p_out->_jump);
        result = (result == FAIL) ? result : r;
    }
  }
  return result;
}

/*-------------------------------------------------------------------------------------------------------------------*/
static bool is_literal(const char* str, int n){
  int sc = 0;
  char c;
  while(sc < n && str[sc] != '\0'){
    if(c = str[sc], !('0' <= c && c <= '9')){
      return false;
    }
    ++sc;
  }
  return true;
}
/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: tests if a symbol character is a member of the set of valid symbol characters.
 *    valid set = {a-z, A-Z, 0-9, -, ., $, :}
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static bool is_symbol_char(char c){
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

/*-------------------------------------------------------------------------------------------------------------------*/
static bool is_symbol(const char* sym, int n){
  int sc = 0;
  if(sc >= n || ('0' <= sym[sc] && sym[sc] <= '9')){ // symbols cannot start with a number.
    return false;
  }
  ++sc;
  while(sc < n && sym[sc] != '\0'){
    if(!is_symbol_char(sym[sc])){
      return false;
    }
    ++sc;
  }
  return true;
}

/*-------------------------------------------------------------------------------------------------------------------*/
static int extract_inner(Parser_t* p, const char* line, char* out, int n, char start, char end){
  // check 'start' char is first character...
  int lc = 0;
  if(line[lc] != start){
    snprintf(g_error_line, MAX_LINE_LENGTH, "unexpected character before '%c'", start);
    print_error(p->_filename, p->_lineno, g_error_line, line);
    return FAIL;
  }
  ++lc; // skip 'start' char

  // extract inner...
  int oc = 0;
  while(line[lc] != end  && oc < (n - 1)){
    out[oc] = line[lc];
    ++oc;
    ++lc;
  }
  out[oc] = '\0';
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: extracts an A or L command.
 * @param line: line to extract command from.
 * @param ds: start delimiter of symbol -> '@' for A command, '(' for L command.
 * @param de: end delimiter of symbol -> '\0' for A command, ')' for L command.
 * @param fmt0: format of command if symbol -> CFORMAT_L0 or CFORMAT_A0
 * @param fmt1: format of command if literal -> CFORMAT_L1 or CFORMAT_A1
 * @param p_out: output command struct.
 * returns: SUCCESS if command extracted, FAIL if not extracted.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int extract_format_ALX(Parser_t* p, const char* line, char ds, char de, int fmt0, int fmt1, Command_t* p_out){
  extract_inner(p, line, p_out->_sym, MAX_SYM_LENGTH, ds, de);
  p_out->_type = (is_literal(p_out->_sym, MAX_SYM_LENGTH)) ? fmt1 : (is_symbol(p_out->_sym, MAX_SYM_LENGTH)) ? fmt0 : CFORMAT_XX;
  if(p_out->_type == fmt1 && (strlen(p_out->_sym) > 5)){
    snprintf(g_error_line, MAX_LINE_LENGTH, "literal '%s' too large for 15-bit address", p_out->_sym);
    print_error(p->_filename, p->_lineno, g_error_line, line);
    return FAIL;
  }
  else if(p_out->_type == CFORMAT_XX){
    snprintf(g_error_line, MAX_LINE_LENGTH, "expected symbol or literal after '%c', recieved: %s", ds,  p_out->_sym);
    print_error(p->_filename, p->_lineno, g_error_line, line);
    return FAIL;
  }
  return SUCCESS;
}

/*-------------------------------------------------------------------------------------------------------------------*/
static void clear_command(Command_t* p_cmd){
  p_cmd->_type = CFORMAT_XX;
  memset((void*)p_cmd->_sym, '\0', MAX_SYM_LENGTH);
  memset((void*)p_cmd->_dest, '\0', MAX_MNEMONIC_CHAR_LENGTH);
  memset((void*)p_cmd->_comp, '\0', MAX_MNEMONIC_CHAR_LENGTH);
  memset((void*)p_cmd->_jump, '\0', MAX_MNEMONIC_CHAR_LENGTH);
}

/*-------------------------------------------------------------------------------------------------------------------*/
static void clear_symbol(Symbol_t* p_sym){
  p_sym->_type = SYMBOL_X;
  memset((void*)p_sym->_sym, '\0', MAX_SYM_LENGTH); 
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: parses an assmebly instruction into a command struct.
 * return: SUCCESS if no parsing errors, else FAIL; p_out garbage if FAIL.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int parse_command(Parser_t* p, const char* line, int n, Command_t* p_out){
  clear_command(p_out);
  int fmt = parse_format(p, line);
  int result;
  switch(fmt){
    case CFORMAT_C0:
      result = extract_format_C0(p, line, p_out);
      break;
    case CFORMAT_C1:
      result = extract_format_C1(p, line, p_out);
      break;
    case CFORMAT_C2:
      result = extract_format_C2(p, line, p_out);
      break;
    case CFORMAT_AX:
      result = extract_format_ALX(p, line, '@', '\0', CFORMAT_A0, CFORMAT_A1, p_out);
      break;
    case CFORMAT_LX:
      result = extract_format_ALX(p, line, '(', ')', CFORMAT_L0, CFORMAT_L1, p_out);
      break;
    default: // command format error.
      result = FAIL;
  }
  return result;
}

/*-------------------------------------------------------------------------------------------------------------------*/
static int parse_symbol(Parser_t* p, char* line, int n, Symbol_t* p_out){
  clear_symbol(p_out);
  int fmt = parse_format(p, line);
  int result = FAIL;
  Command_t cmd;
  clear_command(&cmd);
  switch(fmt){
    case CFORMAT_AX:
      result = extract_format_ALX(p, line, '@', '\0', CFORMAT_A0, CFORMAT_A1, &cmd);
      if(result == SUCCESS && cmd._type == CFORMAT_A0){
        p_out->_type = SYMBOL_A;
      }
      else{
        result = FAIL;
      }
      break;
    case CFORMAT_LX:
      result = extract_format_ALX(p, line, '(', ')', CFORMAT_L0, CFORMAT_L1, &cmd);
      if(result == SUCCESS && cmd._type == CFORMAT_L0){
        p_out->_type = SYMBOL_L;
      }
      else{
        result = FAIL;
      }
      break;
    default: 
      result = FAIL;
  }
  if(result == SUCCESS){
    strncpy(p_out->_sym, cmd._sym, MAX_SYM_LENGTH);
  }
  return result;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: strips a line of all whitespace and comments.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static void strip_line(char* line, int n){
  int t = 0, f = 0; // t=copy to, f=copy from
  // strip whitespace...
  while(t < n && !isspace(line[t])){
    ++t;
  }
  f = t + 1;
  while(f < n && isspace(line[f])) {
    ++f;
  }
  while(f < n && line[f] != '\0'){
    if(!isspace(line[f])){
      line[t] = line[f];
      ++t;
    }
    ++f;
  }
  // strip comments...
  f = 0;
  while((f + 1) < n){
    if(line[f] == '/' && line[f + 1] == '/'){
      t = f;
      break;
    }
    ++f;
  } 
  // clear the remainder of the buffer...
  while(t < n){
    line[t] = '\0';
    ++t;
  }
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: gets the next code line from the file, skipping lines with only whitespace or comments.
 * return: SUCCESS if line extracted, FAIL if end of file.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
static int get_next_line(Parser_t* p, char* line, int n){
  memset((void*)line, '\0', n); 
  while(fgets(line, n, p->_tunit) != NULL){ 
    ++p->_lineno;
    strip_line(line, n);
    if(!strlen(line) == 0){
      return SUCCESS;
    }
  }
  return FAIL;
}

/*=====================================================================================================================
 * PUBLIC INTERFACE
 *===================================================================================================================*/

/*-------------------------------------------------------------------------------------------------------------------*/
/* brief: instantiates a new parser.
 * @param filename: the .asm file the parser will parse.
 * return: pointer to the new parser or NULL on error.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
Parser_t* new_parser(const char* filename){
  static bool is_initialised = false;
  if(!is_initialised){
    parse_module_init();
    is_initialised = true;
  }

  Parser_t* p = NULL;
  p = (Parser_t*)malloc(sizeof(Parser_t));
  if(p == NULL){
    return NULL;
  }

  p->_tunit = fopen(filename, "r");
  if(p->_tunit == NULL){
    perror("fopen");
    free(p);
    return NULL;
  }

  size_t fns = (sizeof(char) * strlen(filename)) + 1;
  p->_filename = (char*)malloc(fns);
  memset((void*)p->_filename, '\0', fns); 
  strncpy(p->_filename, filename, fns - 1);

  p->_lineno = 0;

  return p;
}

/*-------------------------------------------------------------------------------------------------------------------*/
void free_parser(Parser_t** p){
  fclose((*p)->_tunit);
  free((*p)->_filename);
  free(*p);
}

/*-------------------------------------------------------------------------------------------------------------------*/
/* brief: parses the next line in the translation unit and returns the result in a Command_t struct.
 * return: 
 *      SUCCESS if line parsed into command.
 *      FAIL if parser error.
 *      CMD_EOF if end of commands.
 *
 * note: output command is invalid in all case except return success.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
int parser_next_command(Parser_t* p, Command_t* p_out){
  return (get_next_line(p, g_line, MAX_LINE_LENGTH) == SUCCESS) ? parse_command(p, g_line, MAX_LINE_LENGTH, p_out) : CMD_EOF;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: parses the next line in the translation unit and, if the line is an A or L command, and contains a valid
 *  symbol, returns the symbol.
 * return: SUCCESS if symbol extracted
 *         FAIL if symbol not extracted for any reason.
 *         CMD_EOF if end of commands.
 *
 * note: the parser will NOT return an invalid symbol, rather it prints an error and returns FAIL.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
int parser_next_symbol(Parser_t* p, Symbol_t* p_out){
  return (get_next_line(p, g_line, MAX_LINE_LENGTH) == SUCCESS) ? parse_symbol(p, g_line, MAX_LINE_LENGTH, p_out) : CMD_EOF;
}

/*-------------------------------------------------------------------------------------------------------------------*/
bool parser_has_next(Parser_t* p){
  return (feof(p->_tunit) == 0) ? true : false;
}

/*-------------------------------------------------------------------------------------------------------------------*/
/*
 * brief: moves the parser back to the start of the file/translation unit.
 */
/*-------------------------------------------------------------------------------------------------------------------*/
void parser_rewind(Parser_t* p){
  rewind(p->_tunit);
  p->_lineno = 0;
}

/*-------------------------------------------------------------------------------------------------------------------*/
void print_command(FILE* stream, Command_t* c){
  fprintf(stream, "------------- COMMAND -------------\ntype:%s\nsym :%s\ndest:%s\ncomp:%s\njump:%s\n----------------"
      "-------------------\n", format_id_to_string(c->_type), c->_sym, c->_dest, c->_comp, c->_jump);
}

