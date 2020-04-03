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
 * file: decoder.h
 *
 *===================================================================================================================*/

#ifndef _DECODER_H_
#define _DECODER_H_

void init_decoder();

/*
 * brief: translate a command structure into a 'Hack' machine instruction.
 * @param p_cmd: command to translate.
 * @param p_code: output decoded 'Hack' machine instruction.
 * return: SUCCESS or FAIL; FAIL if recieves unaccepted command format.
 * note: does NOT accept CFORMAT_A0, CFORMAT_L0 or CFORMAT_L1.
 */
int decode(Command_t* p_cmd, uint16_t* p_code);

#endif
