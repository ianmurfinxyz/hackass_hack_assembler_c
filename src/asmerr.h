#ifndef _ASMERR_H_
#define _ASMERR_H_

#define FAIL -1
#define SUCCESS 0 
#define ERROR_1 1
#define ERROR_2 2
#define ERROR_3 3
#define ERROR_4 4
#define ERROR_5 5

#define MALLOC_ERROR 0xb0000000

/*
 * global error variable used by all assembler modules; a assembler version of errno.
 *
 * note: asmerr set to error values defined above (i.e. ERROR_1 to ERROR_0) or SUCCESS.
 */ 
extern int asmerr;

/*
 * brief: converts asmerr to a string and outputs said string to stderr.
 */
void asmerrstr(int asmerr);

#endif
