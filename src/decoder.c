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
 * file: decoder.c
 *
 *===================================================================================================================*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include "parser.h"
#include "asmerr.h"

/*=====================================================================================================================
 * TYPES
 *===================================================================================================================*/

/*
 * brief: maps the string representation of a mnemnic to the binary representation of the mnemonic.
 */
typedef struct Mnemonic {
  char _str[MAX_MNEMONIC_CHAR_LENGTH];
  uint16_t _bits;
} Mnemonic_t;

/*=====================================================================================================================
 * MODULE DATA
 *===================================================================================================================*/

static Mnemonic_t jump_bits[8];
static Mnemonic_t dest_bits[8];
static Mnemonic_t comp_bits[28];

/*=====================================================================================================================
 * PRIVATE INTERFACE
 *===================================================================================================================*/

void init_decoder(){
  strncpy(jump_bits[0]._str, "", MAX_MNEMONIC_CHAR_LENGTH);    jump_bits[0]._bits = 0b0000000000000000;
  strncpy(jump_bits[1]._str, "JGT", MAX_MNEMONIC_CHAR_LENGTH); jump_bits[1]._bits = 0b0000000000000001;
  strncpy(jump_bits[2]._str, "JEQ", MAX_MNEMONIC_CHAR_LENGTH); jump_bits[2]._bits = 0b0000000000000010;
  strncpy(jump_bits[3]._str, "JGE", MAX_MNEMONIC_CHAR_LENGTH); jump_bits[3]._bits = 0b0000000000000011;
  strncpy(jump_bits[4]._str, "JLT", MAX_MNEMONIC_CHAR_LENGTH); jump_bits[4]._bits = 0b0000000000000100;
  strncpy(jump_bits[5]._str, "JNE", MAX_MNEMONIC_CHAR_LENGTH); jump_bits[5]._bits = 0b0000000000000101;
  strncpy(jump_bits[6]._str, "JLE", MAX_MNEMONIC_CHAR_LENGTH); jump_bits[6]._bits = 0b0000000000000110;
  strncpy(jump_bits[7]._str, "JMP", MAX_MNEMONIC_CHAR_LENGTH); jump_bits[7]._bits = 0b0000000000000111;

  strncpy(dest_bits[0]._str, "", MAX_MNEMONIC_CHAR_LENGTH);    dest_bits[0]._bits = 0b0000000000000000;
  strncpy(dest_bits[1]._str, "M", MAX_MNEMONIC_CHAR_LENGTH);   dest_bits[1]._bits = 0b0000000000001000;
  strncpy(dest_bits[2]._str, "D", MAX_MNEMONIC_CHAR_LENGTH);   dest_bits[2]._bits = 0b0000000000010000;
  strncpy(dest_bits[3]._str, "MD", MAX_MNEMONIC_CHAR_LENGTH);  dest_bits[3]._bits = 0b0000000000011000;
  strncpy(dest_bits[4]._str, "A", MAX_MNEMONIC_CHAR_LENGTH);   dest_bits[4]._bits = 0b0000000000100000;
  strncpy(dest_bits[5]._str, "AM", MAX_MNEMONIC_CHAR_LENGTH);  dest_bits[5]._bits = 0b0000000000101000;
  strncpy(dest_bits[6]._str, "AD", MAX_MNEMONIC_CHAR_LENGTH);  dest_bits[6]._bits = 0b0000000000110000;
  strncpy(dest_bits[7]._str, "AMD", MAX_MNEMONIC_CHAR_LENGTH); dest_bits[7]._bits = 0b0000000000111000;

  strncpy(comp_bits[0]._str, "0", MAX_MNEMONIC_CHAR_LENGTH);     comp_bits[0]._bits = 0b0000101010000000;
  strncpy(comp_bits[1]._str, "1", MAX_MNEMONIC_CHAR_LENGTH);     comp_bits[1]._bits = 0b0000111111000000;
  strncpy(comp_bits[2]._str, "-1", MAX_MNEMONIC_CHAR_LENGTH);    comp_bits[2]._bits = 0b0000111010000000;
  strncpy(comp_bits[3]._str, "D", MAX_MNEMONIC_CHAR_LENGTH);     comp_bits[3]._bits = 0b0000001100000000;
  strncpy(comp_bits[4]._str, "A", MAX_MNEMONIC_CHAR_LENGTH);     comp_bits[4]._bits = 0b0000110000000000;
  strncpy(comp_bits[5]._str, "!D", MAX_MNEMONIC_CHAR_LENGTH);    comp_bits[5]._bits = 0b0000001101000000;
  strncpy(comp_bits[6]._str, "!A", MAX_MNEMONIC_CHAR_LENGTH);    comp_bits[6]._bits = 0b0000110001000000;
  strncpy(comp_bits[7]._str, "-D", MAX_MNEMONIC_CHAR_LENGTH);    comp_bits[7]._bits = 0b0000001111000000;
  strncpy(comp_bits[8]._str, "-A", MAX_MNEMONIC_CHAR_LENGTH);    comp_bits[8]._bits = 0b0000110011000000;
  strncpy(comp_bits[9]._str, "D+1", MAX_MNEMONIC_CHAR_LENGTH);   comp_bits[9]._bits = 0b0000011111000000;
  strncpy(comp_bits[10]._str, "A+1", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[10]._bits = 0b0000110111000000;
  strncpy(comp_bits[11]._str, "D-1", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[11]._bits = 0b0000001110000000;
  strncpy(comp_bits[12]._str, "A-1", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[12]._bits = 0b0000110010000000;
  strncpy(comp_bits[13]._str, "D+A", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[13]._bits = 0b0000000010000000;
  strncpy(comp_bits[14]._str, "D-A", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[14]._bits = 0b0000010011000000;
  strncpy(comp_bits[15]._str, "A-D", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[15]._bits = 0b0000000111000000;
  strncpy(comp_bits[16]._str, "D&A", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[16]._bits = 0b0000000000000000;
  strncpy(comp_bits[17]._str, "D|A", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[17]._bits = 0b0000010101000000;
  strncpy(comp_bits[18]._str, "M", MAX_MNEMONIC_CHAR_LENGTH);   comp_bits[18]._bits = 0b0001110000000000;
  strncpy(comp_bits[19]._str, "!M", MAX_MNEMONIC_CHAR_LENGTH);  comp_bits[19]._bits = 0b0001110001000000;
  strncpy(comp_bits[20]._str, "-M", MAX_MNEMONIC_CHAR_LENGTH);  comp_bits[20]._bits = 0b0001110011000000;
  strncpy(comp_bits[21]._str, "M+1", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[21]._bits = 0b0001110111000000;
  strncpy(comp_bits[22]._str, "M-1", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[22]._bits = 0b0001110010000000;
  strncpy(comp_bits[23]._str, "D+M", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[23]._bits = 0b0001000010000000;
  strncpy(comp_bits[24]._str, "D-M", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[24]._bits = 0b0001010011000000;
  strncpy(comp_bits[25]._str, "M-D", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[25]._bits = 0b0001000111000000;
  strncpy(comp_bits[26]._str, "D&M", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[26]._bits = 0b0001000000000000;
  strncpy(comp_bits[27]._str, "D|M", MAX_MNEMONIC_CHAR_LENGTH); comp_bits[27]._bits = 0b0001010101000000;
}

/*=====================================================================================================================
 * PUBLIC INTERFACE
 *===================================================================================================================*/

/*
 * brief: translate a command structure into a 'Hack' machine instruction.
 * @param p_cmd: command to translate.
 * @param p_code: output decoded 'Hack' machine instruction.
 * return: SUCCESS or FAIL; FAIL if recieves unaccepted command format.
 * note: does NOT accept CFORMAT_A0, CFORMAT_L0 or CFORMAT_L1.
 */
int decode(Command_t* p_cmd, uint16_t* p_code){
  assert(p_cmd->_type != CFORMAT_A0 && p_cmd->_type != CFORMAT_L0 && p_cmd->_type != CFORMAT_L1);
  switch(p_cmd->_type){
    case CFORMAT_A1:{
      uint16_t add = (uint16_t)strtol(p_cmd->_sym, NULL, 10); 
      (*p_code) = 0b0000000000000000 | add; 
      return SUCCESS;
    }
    case CFORMAT_C0:
    case CFORMAT_C1:
    case CFORMAT_C2:{
      int j = 0, d = 0, c = 0;
      while(j < 8 && strncmp(jump_bits[j]._str, p_cmd->_jump, MAX_MNEMONIC_CHAR_LENGTH) != 0){++j;}
      while(d < 8 && strncmp(dest_bits[d]._str, p_cmd->_dest, MAX_MNEMONIC_CHAR_LENGTH) != 0){++d;}
      while(c < 28 && strncmp(comp_bits[c]._str, p_cmd->_comp, MAX_MNEMONIC_CHAR_LENGTH) != 0){++c;}
      assert(j != 8 && d != 8 && c != 28);
      (*p_code) = (0b1110000000000000 | comp_bits[c]._bits | dest_bits[d]._bits | jump_bits[j]._bits);
      return SUCCESS;
   }
  }
  return FAIL;
}
