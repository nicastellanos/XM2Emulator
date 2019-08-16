
/* 
File: psw.h
Author: Nicolas Castellanos
School: Dalhousie University
Department: Electrical and Computer Engineering
Course: ECED 3403
Purpose: Controls the PSW and Prints for User
*/

#include "psw.h"
#include "xm2.h"
psw_t psw = {0};
psw_t* pswptr = &psw;


unsigned carry[2][2][2] = { 0, 0, 1, 0, 1, 0, 1, 1 };
unsigned overflow[2][2][2] = {0, 1, 0, 0, 0, 0, 1, 0};
void update_psw(unsigned short src, unsigned short dst, unsigned short res, unsigned short wb)
{
    /* 
 - Update the PSW bits (V, N, Z, C) 
 - Using src, dst, and res values and whether word or byte 
 - ADD, ADDC, SUB, and SUBC
*/
    uint16_t status_word = 0;
    unsigned short mss, msd, msr; /* Most significant src, dst, and res bits */
    if (wb == WORD)
    {
        mss = B15(src);
        msd = B15(dst);
        msr = B15(res);
    }
    else /* Byte */
    {
        mss = B7(src);
        msd = B7(dst);
        msr = B7(res);
        res &= 0xFF; /* Mask high byte for 'z' check */
    }
    /* Carry */
    pswptr->bit.c = carry[mss][msd][msr];
    /* Zero */
    pswptr->bit.z = (res == 0);
    /* Negative */
    pswptr->bit.n = (msr == 1);
    /* oVerflow */
    pswptr->bit.v = overflow[mss][msd][msr];
    status_word = (pswptr->previous_prio<<13);
    status_word |= (pswptr->current_prio<<5);
    status_word |= ((pswptr->bit.v<<3)|(pswptr->bit.n<<2)|(pswptr->bit.c));
    bus(PSW_ADDR,&status_word,WRITE,WORD);
}



void update_psw2(unsigned short res, unsigned short wb)
{
/*
 - Update PSW bits N and Z; clear V
 - Leave C unchanged
 - XOR, AND, BIS, BIC, and BIT
*/
    uint16_t status_word;
    if (wb == WORD)
    {
        pswptr->bit.n = B15(res);
        pswptr->bit.z = (res == 0);
    }
    else /* Byte */
    {
        pswptr->bit.n = B7(res);
        pswptr->bit.z = (res & 0x00FF) == 0;
    }
    /* Clear V */
        pswptr->bit.v = 0;
    status_word = (pswptr->previous_prio<<13);
    status_word |= (pswptr->current_prio<<5);
    status_word |= ((pswptr->bit.v<<3)|(pswptr->bit.n<<2)|(pswptr->bit.c));
    bus(PSW_ADDR,&status_word,WRITE,WORD);
}


void debugger_print_psw(){
#if 0
    uint16_t status_word;
    bus(PSW_ADDR, &status_word,READ,WORD);
    printf("PSW : 0x04",status_word);
#endif  
    printf("PSW VAL\n");
    printf("CP: %d\tZ: %d\tN: %d\tC: %d\tV:%d ",pswptr->current_prio,pswptr->bit.z,pswptr->bit.n,
        pswptr->bit.c,pswptr->bit.v);
    
}
