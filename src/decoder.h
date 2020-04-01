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
