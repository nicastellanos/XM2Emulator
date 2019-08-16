
/* 
File: bus.c
Author: Nicolas Castellanos
School: Dalhousie University
Department: Electrical and Computer Engineering
Course: ECED 3403
Purpose: Provides Bus functionality between system calls, is used to access memory 
*/



#include "bus.h"


extern void bus(unsigned short mar, unsigned short *mbr, enum ACTION rw, enum SIZE bw)
{
    /*
- Example of how the bus (and memory access) can be emulated
- mar - memory address - refers to a byte location (word is mar >> 1)
- mbr - pointer to data to read/write
- rw - READ|WRITE
- bw - BYTE|WORD
*/
    if (rw == READ){
        *mbr = (bw == WORD) ? memory.word[mar >> 1] : memory.byte[mar];
    }
    else /* rw = WRITE */
    {
        if (bw == WORD){
            memory.word[mar >> 1] = *mbr;
        }else{ /* BYTE */
            memory.byte[mar] = (unsigned char)(*mbr & 0xFF);
        }
    }
    /* Update device registers if necessary */
#if 0
    if (mar < DEVMEM){
        dev_mem_access(mar, *mbr, rw, bw);
    }
#endif 
    /* 
 - CPU memory (FFC0 .. FFFF) is zero-latency - inside CPU 
 - Don't increment sysclk if CPU memory
*/
    if (mar < VECTORBASE){
        sysclk += 3;
    }
}


/* Prompts user to write to memory, can only be WORD size 
*/

extern void debugger_write_mem(){
    uint16_t addr;
    uint16_t value;
    printf("Enter Address and Value: ");
    scanf("%4hx,%4hx",&addr,&value);
    bus(addr,&value,WRITE,WORD);
    printf("\n\nValue entered!");
}