
/* 
File: memory.c
Author: Nicolas Castellanos
School: Dalhousie University
Department: Electrical and Computer Engineering
Course: ECED 3403
Purpose: Provides the Execution of the Instruction 
*/



#include "memory.h"


/************************* VARIABLES & MACROS ********************************/

#define BYTE_MASK(x)    ((x)<<1)
#define WORD_MASK(x)    ((x)>>1)



/* Initializes memory by filling with 0xFFFF 
    params in @ mem_t* memory 
*/
void memory_init(mem_t* memory){
    //filling memory with 0xFFFF to init
    for(int i = 0; i< WORDMAXMEM;i++){
        memory->word[i] = 0xFFFF;
    }
}

/* Prints the memory range to the debugger screen */

extern void debugger_mem_print(){
    uint16_t start, end;
    //prompting user for memory range
    printf("Enter Memory Range: ");
    scanf("%4hx,%4hx", &start, &end);
    memory_print(start, end,(mem_t*)&memory); //Printing Memory
}

/* Prints the ascii character for each byte of memory within a range
    params in: @ int start 
               @ int end     
               @ mem_t* memory 
*/

void char_print(int start, int end, mem_t* memory){
    if( start > 4){ //first iteration shouldnt
        printf("\t\t"); //Space out hex and ascii
        //printing ascii values 
        for(int i = BYTE_MASK(start); i<= BYTE_MASK(end); i++){
            //if 0xFF just print . instead 
            if(memory->byte[i] != 0xFF){
                printf("%c", memory->byte[i]);
            }else{
                printf(".");
            }
        }
    }
}

/* Prints the memory range set by start and end calls char_print
    to print the corresponding ascii values 

    params in @ uint16_t start
              @ uint16_t end 
              @ mem_t* memory 
*/
void memory_print(uint16_t start, uint16_t end, mem_t* memory){
    int i;
    int start_holder;
    int end_holder;
    for(i = WORD_MASK(start); (i<= WORDMAXMEM)^(i == WORD_MASK(end));i++){
        if(!(i%8)){
            //8 Words per line
            end_holder = start_holder + 8;
            char_print(start_holder,end_holder, memory);
            printf("\n");
            start_holder = i;
        }
        printf("%04X ", memory->word[i]);
    }
    //printing last string of 8 words
    end_holder = i+8;
    char_print(i,end_holder, memory);
}