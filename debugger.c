
/* 
File: debugger.c
Author: Nicolas Castellanos
School: Dalhousie University
Department: Electrical and Computer Engineering
Course: ECED 3403
Purpose: Provides user with an interface to debug programs on XM2
*/


#include "debugger.h"


/************************* VARIABLES & MACROS ********************************/

uint16_t breakpoint = 0;


/****************************** FUNCTIONS  ********************************/

/* Initalizes Debugger */

extern void debugger_init(){
    printf("XM2 Debugger Init\n");
}

/*Prints device info for debugger */

extern void debugger_device_info(){
    printf("\nXMACHINA-2  DEVICE INFO");
    printf("\n16-bit ARCHITECTURE");
    printf("\n64 KiB RAM");
    printf("\nDeveloped at Dalhousie University\n");
}

/* Prints current breakpoint and prompts user for new breakpoint */
extern void debugger_set_breakpoint(){

    printf("Current BKPT: %4x \n", breakpoint);
    printf("Enter BKPT: ");
    scanf("%hx", &breakpoint);
}

/* Prints register values and prompts user to enter value
for new register */
extern void debugger_set_regs(){
    int regi; //register number
    uint16_t val; //value to place in register
    print_regs(); //prints register and values 
    printf("Enter Register to change and value: ");
    scanf("%d,%hX",&regi, &val);// reading values from user
    regfile[regi].value = val; //assigning to new register
    printf("New ");
    print_regs();
}


/* Prints Register Values */

extern void print_regs(){
    
    printf("Register Values: \n");
    printf("REG\tVAL\n");
    printf("%.*s\n",10,"**********************************");
    for(int i = 0; i< MAX_REG; i++){
        printf("%s:\t%4X\n",regfile[i].name, regfile[i].value);
    }
}

/* Prints CPU State */

extern void print_cpu_state(){
    print_regs();
    printf("\nSYSCLK  %d\n",sysclk);
    debugger_print_psw();
}

/*Prints User Options */

void print_options(){
    char option;
    printf("\nEnter Option:\n\n");
    printf("%.*s\n",30,"=================================");
    printf("C - Change a memory location\n");
    printf("D - Device Information\n");
    printf("B - Set Breakpoint\n");
    printf("L - Load\n");
    printf("R - Display Registers\n");
    printf("S - Set Register Words\n");
    printf("T - Reset XM2\n");
    printf("M - Display Memory\n");
    printf("P - Display PSW\n");
    printf("G - Go\n");
    printf("X - Exit\n");
    printf("\nOption: ");
}