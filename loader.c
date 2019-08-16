/* 
File: loader.c
Author: Nicolas Castellanos
School: Dalhousie University
Department: Electrical and Computer Engineering
Course: ECED 3403
Purpose: Loads Program Into Memory and Provides Debugger with Options for Loading
*/
#include "loader.h"
#include "memory.h"

/************************* VARIABLES & MACROS ********************************/

FILE* fin;
mem_t memory;
char buffer[MAX_BUFF_LEN];
char path_buffer[MAX_BUFF_LEN];


/* Loads the XME specified by path into memory, returns the starting address of the program*/

extern uint16_t debugger_load(){
    // reading path 
    char * s = path_buffer;
    printf("Enter Location of File: ");
    scanf("%s",s);
    // loading xme 
    return load_xme(s);
}


/* Loads the File specified by path into memory, returns the starting address of the program*/

extern uint16_t load_xme(char* string){ // loads .xme file into memory
    char data[MAX_SREC_BYTE_LEN]; //data buffer
    char srec[3]; //Srec directive buffer 
    uint16_t mem_data[MAX_SREC_WORD_LEN];
    uint16_t mem_holder; //holds the memory for byte loading
    char* data_str = data; //pointer to data string
    char* srec_type = srec;  // pointer to srec
    uint16_t start; 
    uint16_t end;
    int j = 0;
    uint16_t address; //value to hold address
    unsigned int size; //value to hold size
    unsigned int checksum; // value to hold checksum
    memory_init((mem_t*)&memory); //Cleaning memory
    fin = fopen(string,"r"); //opening file
    fgets(buffer,MAX_BUFF_LEN,fin); // reading file for S0 rec
   while(fgets(buffer,MAX_BUFF_LEN,fin)){ // reading S1 REC
        sscanf(buffer,"%2s%02x%4hX%s",srec_type,&size,&address,data_str);
        if(strcmp(srec_type,"S1") == 0){
            while (j<((size-2)>>1))
            {
                // reading into mem data 
                sscanf(data_str, "%4hX", &mem_data[j]);
                j++; // incremending mem data array 
                data_str += 4; // moving data str to next word 
            }

            j = 0;

            // loading record into memory for bytes
            for (int i = 0; i < ((size-2)); i+=2)
            {   //msb first
                memory.byte[address+i]= mem_data[i]>>8;
                //lsb next 
                memory.byte[address+i+1] = mem_data[i];
            }      

            address = address>>1;
            //loading recotrd into memory for words 
            for (int i = 0; i < ((size-2)>>1); i++) //word size 
            {   // msb first
                mem_holder = (0x00FF & (mem_data[i])>>8);
                //lsb next 
                mem_holder |= (0xFF00 & (mem_data[i])<<8);
                memory.word[address+i]= mem_holder;
            }

        }else{
            break;
        }
        data_str = data; // pointer ptr to head of biffer
    }
    fclose(fin); // closing .xme
    //returning starting address
    return address;
}

