/* 
File: XM2.h
Author: Nicolas Castellanos
School: Dalhousie University
Department: Electrical and Computer Engineering
Course: ECED 3403
Purpose: Provides the Fetch Decode Cycles of the Machine
*/


#include "xm2.h"
#include "execute.h"
#include "debugger.h"
#include <stdio.h>



char input_buff[MAX_BUFF_LEN];
uint16_t cex_flag = 0;
uint16_t ts_flag = 0;
uint16_t true_count = 0;
uint16_t false_count = 0;
uint16_t breakpoint;
int killed;

group_t group = REG2R;

reg_t regfile[MAX_REG] = {
    {"R0", 0},
    {"R1", 0},
    {"R2", 0},
    {"R3", 0},
    {"R4", 0},
    {"R5", 0},
    {"R6", 0},
    {"R7", 0},
};

const_t constfile[MAX_CONST] = {0,1,2,4,8,16,32,-1};

uint32_t sysclk = 0;


/****************************** FUNCTIONS  ********************************/

/* fetches the corresponding instruction from memory 
    returns the machine instruction

    params in @ uint16_t address
    params out @ uint16_t value 
*/
uint16_t fetch(uint16_t address){

    uint16_t val;

    bus(address,&val,READ,WORD);
    return val;
}

/* Decodes the reg relative instructions by masking
the instruction and retrieving values such as SRC,
DST, group, type and modifier
    params @in uint16_t val    machine inst
*/
void decode_reg_rel(uint16_t val){
    int SRC = REG_REL_SRC_MASK(val); 
    int DST = REG_REL_DST_MASK(val);
    uint16_t offset = REG_REL_OFFSET_MASK(val);
    uint16_t type = REG_REL_TYP_MASK(val);
    group_t group = REGREL;
    enum SIZE size = REG_REL_CND_MASK(val) ? BYTE:WORD;
    execute(group,type,SRC,DST,offset,size);
}

/* Decodes the branch instructions by masking
the instruction and retrieving values such as group, type
and modifier
    params @in uint16_t val    machine inst
*/
void decode_br(uint16_t val){
    group_t group = BR;
    int type;
    uint16_t offset;
    if(val & (1<<13)){
        type = BR_TYPE_MASK(val);
        type +=1;
        offset = val & BR_OFFSET;
    }else{
        type = 0;
        offset = val & BL_OFFSET;
    }
    execute(group,type,0,0,offset,WORD);
}

/* Decodes the branch instructions by masking
the instruction and retrieving values such as group, type
and modifier and size
    params @in uint16_t val    machine inst
*/

void decode_r2r(uint16_t val){
    uint16_t condition = REG_2_REG_CND_MASK(val);
    uint16_t type = REG_2_REG_TYP_MASK(val);
    int SRC = REG_2_REG_SRC_MASK(val);
    int DST = REG_2_REG_DST_MASK(val);
    enum SIZE size = WORD;
    group_t group = REG2R;
    if(type < 2){
     size = (condition & WORD_BYTE)? BYTE:WORD; //checking w/b bit 
    }
    execute(group,type,SRC,DST,condition,size);
}

/* Decodes the sreg instructions by masking
the instruction and retrieving values such as group, type
and modifier and size
    params @in uint16_t val    machine inst
*/

void decode_sreg(uint16_t val){
    group_t group = SINREG;
    uint16_t type = S_REG_TYP_MASK(val);
    int DST = S_REG_DST_MASK(val);
    int SRC = S_REG_SRC_MASK(val);
    uint16_t cnd = S_REG_CND_MASK(val);
    enum SIZE size = (val & WORD_BYTE)? BYTE:WORD;
    execute(group,type,SRC,DST,cnd,size);
}

/* Decodes the logic instructions by masking
the instruction and retrieving values such as group, type
and modifier and size
    params @in uint16_t val    machine inst
*/

void decode_log(uint16_t val){
    group_t group = LOGIC;
    uint16_t type = ARI_LOG_TYP_MASK(val);
    int DST = ARI_LOG_DST_MASK(val);
    int SRC = ARI_LOG_SRC_MASK(val);
    uint16_t cnd = ARI_LOG_CND_MASK(val);
    enum SIZE size = (val & WORD_BYTE)? BYTE:WORD;
    execute(group,type,SRC,DST,cnd,size);
}

/* Decodes the arith instructions by masking
the instruction and retrieving values such as group, type
and modifier and size
    params @in uint16_t val    machine inst
*/
void decode_ari(uint16_t val){
    group_t group = ARITH;
    uint16_t type = ARI_LOG_TYP_MASK(val);
    int DST = ARI_LOG_DST_MASK(val);
    int SRC = ARI_LOG_SRC_MASK(val);
    uint16_t cnd = ARI_LOG_CND_MASK(val);
    enum SIZE size = (val & WORD_BYTE)? BYTE:WORD;
    execute(group,type,SRC,DST,cnd,size);
}

/* Decodes the register init instructions by masking
the instruction and retrieving values such as group, type
and modifier and size
    params @in uint16_t val    machine inst
*/
void decode_reginit(uint16_t val){

    group_t group = REG_INI;
    int DST = REG_INIT_DST_MASK(val);
    uint16_t type = REG_INIT_TYP_MASK(val);
    uint16_t value = REG_INIT_VAL_MASK(val);
    execute(group,type,0,DST,value,BYTE);
}

/* Takes the machine instruction and decodes it based on op groups
    params @in val   machine instruction
*/

void decode(uint16_t val){
    uint16_t check;
    uint16_t holder;
    if(val & REG_REL){
        decode_reg_rel(val);
    }else{
        if(val & NON_BR){
            if(val & REG_INIT){
                decode_reginit(val);
            }else{
                if(val & R2R){
                    decode_r2r(val);
                }else{
                    if(val & SREG){
                        decode_sreg(val);
                    }else{
                        if(val & LOG){
                            decode_log(val);
                        }else{
                            decode_ari(val);
                        }
                    }

                }
            }
        }else{
            decode_br(val);
        }
    }
}

/* Checks the true state for CEX condition and returns a boolean indicating if true state is met or not
    params @in uint16_t condition   cex condition
    params @out uint16_t bool_value 
*/

uint16_t check_true_state(uint16_t condition){
    switch(condition){
        case 0:
        return pswptr->bit.z;
        case 1:
        return !(pswptr->bit.z);
        case 2:
        return pswptr->bit.c;
        case 3:
        return !(pswptr->bit.c);
        case 4:
        return pswptr->bit.n;
        case 5:
        return !(pswptr->bit.n);
        case 6:
        return pswptr->bit.v;
        case 7: 
        return !(pswptr->bit.v);
        case 8:
        return pswptr->bit.c & !(pswptr->bit.z);
        case 9:
        return !(pswptr->bit.c) | pswptr->bit.c;
        case 10:
        return (pswptr->bit.n == pswptr->bit.z);
        case 11:
        return (pswptr->bit.n != pswptr->bit.z);
        case 12:
        return (!(pswptr->bit.z) & (pswptr->bit.n == pswptr->bit.z));
        case 13:
        return ((pswptr->bit.z) &(pswptr->bit.n != pswptr->bit.z));
        case 14:
        return 1;
    }
}

/* Allows the debugger to begin execution of the program */

void debug_go(){
    uint16_t val;
    pswptr->current_prio = 0;
    pswptr->previous_prio = 0;
    while(1){
        val = fetch(regfile[R7].value);
        regfile[R7].value += 2;
        if(breakpoint == regfile[R7].value){
            return;
        }
        if(killed){
            return;
        }
        decode(val);
    }
}

void sigint_hndlr(){
    killed = 1;
    signal(SIGINT,sigint_hndlr);
}

/* Initializes debugger and runs forever until user exits program */

void debugger(){
    char option;
    char flush_buffer[MAX_BUFF_LEN];
    char * flush = flush_buffer;
    while(1){
    print_options();
    scanf("%s",flush);
    option = *flush;
    option = toupper(option);
        switch(option){
            case 'C':
                debugger_write_mem();
                break;
            case 'D':
                debugger_device_info();
                break;
            case 'B':
                debugger_set_breakpoint();
                break;
            case 'L':
                regfile[R7].value = debugger_load();
                break;
            case 'R':
                print_regs();
                break;
            case 'S':
                debugger_set_regs();
                break;
            case 'T':
                break;
            case 'M':
                debugger_mem_print();
                break;
            case 'P':
                debugger_print_psw();
                break;
            case 'G':
                debug_go();
                break;
            case 'X':
                return;
            default:
                printf("\nInvalid Option!");
        }
        killed = 0;
    }
}

int main(void){

    signal(SIGINT,sigint_hndlr);
    debugger();
    printf("\nExiting XM2 Debugger.....");
    return 0;
}



