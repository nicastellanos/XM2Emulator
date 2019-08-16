/* 
File: loader.c
Author: Nicolas Castellanos
School: Dalhousie University
Department: Electrical and Computer Engineering
Course: ECED 3403
Purpose: Loads Program Into Memory and Provides Debugger with Options for Loading
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

/************************* VARIABLES & MACROS ********************************/

#define MAX_BUFF_LEN (400)
#define MAX_SREC_WORD_LEN (40)
#define MAX_SREC_BYTE_LEN (80)



/* Loads the XME specified by path into memory, returns the starting address of the program*/
extern uint16_t debugger_load();

/* Loads the File specified by path into memory, returns the starting address of the program*/
extern uint16_t load_xme(char* string);