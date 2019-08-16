
/* 
File: execute.c
Author: Nicolas Castellanos
School: Dalhousie University
Department: Electrical and Computer Engineering
Course: ECED 3403
Purpose: Provides the Execution of the Instruction 
*/


#include "execute.h"
#include <signal.h>


/************************* VARIABLES & MACROS ********************************/

#define INC_BIT     (1<<1)
#define SUB_BIT     (1<<2)
#define PRPO_BIT    (1<<3)   
#define SUB_W       (0x8000)
#define SUB_B       (0x0080)   
#define SXT_BIT     (1<<8)
#define NIBBLE      (0xF)
#define MAX_VECT_SIZE (16)

enum ACTION action = READ;
enum SIZE size = WORD;
uint16_t VECT[MAX_VECT_SIZE] = { //Interrupt Vectors
    0xFFC0,0xFFC4,  0xFFC8, 
    0xFFCB, 0xFFD0, 0xFFD4, 
    0xFFD8, 0xFFDB, 0xFFE0,
    0xFFE4, 0xFFE8, 0xFFEB,
    0xFFF0, 0xFFF4, 0xFFF8,
    0xFFFB};


/****************************** FUNCTIONS  ********************************/

/* Executes branch group operands, checks the necessary PSW bits and updates
the program counter based on the outcome 
    params in: 
        @ int type
        @ uint16_t offset
 */

void execute_br(int type,uint16_t offset){
    int execute = 0; //Execution flag 

    //Condition
    switch(type){
        case 0: 
            /* BL */
            regfile[R5].value = regfile[R7].value; //set LR in regfile[R7].value
            // Add the 13 bit offset
            #if 0
            if(offset & BL_NEG_BIT ){
                regfile[R7].value = regfile[R7].value - (offset<<1);

            }else{
                regfile[R7].value = regfile[R7].value + (offset<<1);
            }
            #endif 
            regfile[R7].value = (offset & 0xFF) | ((offset & 0xD0) ? 0xFF00 : 0);
            break;
        case 1: 
            /* BEQ */
            // check the psw z bit is set
            execute = pswptr->bit.z? 1:0;
            break;
        case 2:
            /* BNE */
            // check the psw z bit it off 
            execute = !(pswptr->bit.z)?1:0;
            break;
        case 3:
            /* BC */
            // check the psw c bit is set 
            execute = pswptr->bit.c?1:0;
            break;
        case 4:
            /* BNC */
            // check the psw c bit is not set
            execute = !(pswptr->bit.c)?1:0;
            break;
        case 5:
            /* BN */
            // check the psw n bit is set
            execute = pswptr->bit.n?1:0;
            break;
        case 6:
            /* BGE */
            // check if greater or equal
            execute = !(pswptr->bit.n ^ pswptr->bit.v)? 1: 0;
            break;
        case 7:
            /* BLE */
            //check if less than
            execute = (pswptr->bit.n ^ pswptr->bit.v)?1:0;
            break;
        case 8:
            /* BRA */
            //branch always
            execute = 1;
            break;
    }
    // executes based on if positive of negative
    if(execute){
#if 0
        if(offset & BR_NEG_BIT){
   //         regfile[R7].value = (regfile[R7].value - (((0<<9)&offset)<<1))-2; 
                (offset & 0xFF) | ((offset & 0xA0) ? 0xFF00 : 0);
        }else{
            regfile[R7].value = (regfile[R7].value + (((0<<9)&offset)<<1))-2;
        }
#endif 
       regfile[R7].value+= (offset & 0xFF) | ((offset & 0xA0) ? 0xFF00 : 0);
    }
}

/* Decodes execution arithmetic group  based on type, calls the specific instruction

    params in @ group_t group
              @ int type 
              @ int SRC
              @ int DST
              @ uint16_t modifier
*/

void execute_reg_init(uint16_t type,int DST,uint16_t value){

    uint16_t holder;
    switch(type){
        
        case 0: 
                /* MOVL */
                // mask low byte into holder
                holder = 0x00FF & value;
                // mask to lb, hb unchanged
                regfile[DST].value |=  holder;
                break;
        case 1:
                /* MOVLZ */
                //mask lowbyte into holder
                 holder = 0x00FF & value;
                //set to reg hb is 0
                regfile[DST].value =  holder;
                break;
        case 2: 
                /* MOVLS */
                //mask lb of value hb is 0xFF
                holder = MSB|value;
                regfile[DST].value = holder;
                break;
        case 3: 
                /* MOVH */
                //mask lb of value lb is unchanged
                holder = MSB & value<<8;
                regfile[DST].value |=  holder;
                break;
        default: fault_handler(ILLEG_INST);
    }
    
}

/* Decodes execution s_reg group  based on type, calls the specific instruction

    params in @ group_t group
              @ int type 
              @ int SRC
              @ int DST
              @ uint16_t modifier
              @ enum SIZE size
*/

void execute_s_reg(uint16_t type, int SRC, int DST, uint16_t modifier, enum SIZE size)
{
    uint16_t DST_holder;
    uint16_t SRC_holder;
    if ((type == 7) && (modifier & REG_CONST))
    { // This is for swap
        type = 8;
    }
    switch (type)
    {
    case 0:
        /* BIT */
        //read source  & dest into holder
        SRC_holder = (modifier & REG_CONST) ? constfile[SRC] : regfile[SRC].value;
        DST_holder = regfile[DST].value;
        DST_holder = SRC_holder & DST_holder;
        regfile[DST].value = (size == WORD) ? DST_holder : regfile[DST].value | (0x00FF & DST_holder);
        update_psw2(regfile[DST].value, size);
        break;
    case 1:
        /* BIC */
        //SRC & ~DST
        SRC_holder = (modifier & REG_CONST) ? constfile[SRC] : regfile[SRC].value;
        SRC_holder = SRC_holder & ~DST_holder;
        regfile[DST].value = (size == WORD) ? DST_holder : regfile[DST].value | (0x00FF & DST_holder);
        update_psw2(regfile[DST].value, size);
        break;
    case 2:
        /* BIS */
        //SRC | DST
        SRC_holder = (modifier & REG_CONST) ? constfile[SRC] : regfile[SRC].value;
        DST_holder = SRC_holder | DST_holder;
        regfile[DST].value = (size == WORD) ? DST_holder : regfile[DST].value | (0x00FF & DST_holder);
        update_psw2(regfile[DST].value, size);
        break;
    case 3:
        /* MOV */
        //copy the value of source
        //move to dest
        SRC_holder = (modifier & REG_CONST) ? constfile[SRC] : regfile[SRC].value;
        DST_holder = SRC_holder;
        regfile[DST].value = (size == WORD) ? DST_holder : regfile[DST].value | (0x00FF & DST_holder);
        update_psw2(regfile[DST].value, size);
        break;
    case 4:
        /* SWAP */
        if (!(modifier & REG_CONST) && !(modifier & WORD_BYTE))
        {
            //taking the src and dest values 
            DST_holder = regfile[DST].value;
            SRC_holder = regfile[SRC].value;
            //swapping
            regfile[DST].value = SRC_holder;
            regfile[SRC].value = DST_holder;
        }
        else
        {
            printf("Invalid instruction");
        }
        break;
    case 5:
        /* SRA */
        DST_holder = regfile[DST].value;
        if (size == BYTE)
        {
            DST_holder = ((DST_holder & LSB) << 1);
        }
        else
        {
            DST_holder = DST_holder << 1;
        }
        regfile[DST].value = DST_holder;
        update_psw2(regfile[DST].value, size);
        break;
    case 6:
        /* RRC */
        DST_holder = regfile[DST].value;
        if (size == BYTE)
        {
            //Byte rotate
            DST_holder = ((DST_holder & LSB) >> 1);
            DST_holder += pswptr->bit.c;
        }
        else
        {
            //word rotate
            DST_holder = DST_holder >> 1;
            DST_holder += pswptr->bit.c;
        }
        regfile[DST].value = DST_holder;
        update_psw2(regfile[DST].value, size);
        break;
    case 7:
        /* SWPB */
        //read the register value into holder
        DST_holder = regfile[DST].value;
        //byte shift
        regfile[DST].value = BYTE_R_SHIFT(DST_holder);
        //read the register msb into holder
        DST_holder = BYTE_L_SHIFT(DST_holder) & MSB;
        regfile[DST].value |= DST_holder;
        update_psw2(regfile[DST].value, size);
        break;
    case 8:
        /* SXT */
        DST_holder = regfile[DST].value;
        DST_holder = (DST_holder & SXT_BIT) ? (DST_holder | MSB ) : (0x0000 | DST_holder); //Sign extend
        regfile[DST].value = DST_holder;
        update_psw2(regfile[DST].value, size);
        break;
    default:
        fault_handler(ILLEG_INST);
    }

}
/* Decodes execution reg 2 ref group based on type, calls the specific instruction

    params in @ group_t group
              @ int type 
              @ int SRC
              @ int DST
              @ uint16_t modifier
              @ enum SIZE size
*/

void execute_reg_2_reg(uint16_t type, int SRC, int DST, uint16_t condition, enum SIZE size)
{
    int prpo = 0;
    int dec = 0;
    int inc = 0;
    uint16_t val;
    uint16_t address;
    enum ACTION action = READ;
    switch (type)
    {
    case 0:
        /* LD */
        //reading pre increment bit
        prpo = (condition & PRPO_BIT) ? 1 : 0;
        action = READ;
        /* If increment bit is set */
        if (condition & INC_BIT)
        {
            inc = 1;
        }
        /* If deincrement bit is set */
        else if (condition & SUB_BIT)
        {
            dec = 1;
        }
        else
        {
            break;
        }
        //assigning the address value from source
        address = regfile[SRC].value;
        //if pre increment we must apply before accessing mem
        if (prpo)
        {
            if (inc)
            {
                address += 1;
            }
            if (dec)
            {
                address -= 1;
            }
        }
        break;
    case 1:
        /* ST*/
        //reading pre increment bit
        prpo = (condition & PRPO_BIT) ? 1 : 0;
        action = WRITE;
        /* If increment bit is set */
        if (condition & INC_BIT)
        {
            inc = 1;
        }
        /* If decrement bit is set */
        else if (condition & SUB_BIT)
        {
            dec = 1;
        }
        else
        {
            break;
        }
        address = regfile[DST].value;
        val = regfile[SRC].value;
        if (prpo)
        {
            if (inc)
            {
                address += 1;
            }
            if (dec)
            {
                address -= 1;
            }
        }
        break;
    case 2:
        val = NIBBLE & (SRC|DST);
        regfile[R7].value = VECT[val];
        bus((regfile[R7].value + 2),&address,READ,WORD);
        if((address & NIBBLE) > pswptr->current_prio){ //priority is a nibble
            fault_handler(PRIO_FLT);
        }
        break;
    case 3:
        /*CEX*/
        cex_flag = 1;
        //checking if true state met 
        ts_flag = check_true_state(condition) ? 1: 0;
        //setting the true count as SRC
        true_count = SRC;
        //setting false count as DST
        false_count = DST;
        break;
    default:
        fault_handler(ILLEG_INST);
    }

    if (type < 2) //if LD or ST
    {
        //rw from memory
        bus(address, &val, action, size);
        if(type){
            //if ld and inc bit set (post increment DST)
            if (inc)
            {
                regfile[DST].value += 1;
            }
             //if ld and dec bit set (post increment DST)
            if (dec)
            {
                regfile[DST].value -= 1;
            }
        }else{
            // st and inc bit set (post increment SRC)
            regfile[DST].value = val;
            if (inc)
            {
                regfile[SRC].value += 1;
            }
            // st and dec bit set (post increment SRC)
            if (dec)
            {
                regfile[SRC].value -= 1;
            }
        }
    }
}
/* Decodes execution register relative group based on type, calls the specific instruction

    params in @ group_t group
              @ int type 
              @ int SRC
              @ int DST
              @ uint16_t modifier
              @ enum SIZE size
*/

void execute_reg_rel(uint16_t type, int SRC, int DST, uint16_t offset, enum SIZE size)
{

    uint16_t address;
    enum ACTION action;
    switch (type)
    { 
    case 0:
        /* LDR */
        address = (offset & 1 << 13) ? regfile[SRC].value - offset : regfile[SRC].value + offset;
        action = READ;
        bus(address, &(regfile[DST].value), action, size);
        break;
    case 1:
        /* STR */
        address = (offset & 1 << 13) ? regfile[DST].value - offset : regfile[DST].value + offset;
        action = WRITE;
        bus(address, &(regfile[SRC].value), action, size);
        break;
    default:
        fault_handler(ILLEG_INST);
        break;
    }
}

/* Decodes execution logic group based on type, calls the specific instruction

    params in @ group_t group
              @ int type 
              @ int SRC
              @ int DST
              @ uint16_t modifier
              @ enum SIZE size
*/

void execute_logic(uint16_t type, int SRC, int DST, uint16_t modifier, enum SIZE size){
    uint16_t DST_holder = regfile[SRC].value;
    uint16_t SRC_holder = (modifier & REG_CONST) ? constfile[SRC] : regfile[SRC].value;
    switch(type){
        case 0: 
            break;
        case 1:
            /* CMP */
            regfile[DST].value = (size == BYTE) ? (DST_holder & LSB) - (SRC_holder & LSB) : DST_holder - SRC_holder;
            break;
        case 2:
            /* XOR */
            regfile[DST].value = (size == BYTE) ? (DST_holder & LSB) ^ (SRC_holder & LSB) : DST_holder ^ SRC_holder;
            break;
        case 3:
            /* AND */
            regfile[DST].value = (size == BYTE) ? (DST_holder & LSB) & (SRC_holder & LSB) : DST_holder & SRC_holder;
            break;
        case 5:
            /* CMP */
            regfile[DST].value = (size == BYTE) ? (DST_holder & LSB) - (SRC_holder & LSB) : DST_holder - SRC_holder;
            break;
        default:
            fault_handler(ILLEG_INST);
            break;

    }
    update_psw(SRC_holder, DST_holder,regfile[DST].value,size);
}

/* Decodes execution arithmetic group  based on type, calls the specific instruction

    params in @ group_t group
              @ int type 
              @ int SRC
              @ int DST
              @ uint16_t modifier
              @ enum SIZE size
*/
void execute_ari(uint16_t type, int SRC, int DST, uint16_t modifier, enum SIZE size){

    uint16_t src_holder = (modifier & REG_CONST)? constfile[SRC] : regfile[SRC].value;
    uint16_t dst_holder = regfile[DST].value;
    switch(type){
        case 0: 
            /* ADD */  
            dst_holder = (size == BYTE)? (dst_holder & LSB)+(src_holder & LSB): (dst_holder + src_holder);
            update_psw(regfile[DST].value, regfile[SRC].value, dst_holder,size);
            break;
        case 1:
            /* ADDC */
            dst_holder = (size == BYTE)? (dst_holder & LSB)+(src_holder & MSB): (dst_holder += src_holder);
            update_psw(regfile[DST].value, regfile[SRC].value, dst_holder,size);
            dst_holder += pswptr->bit.c;
            break;
        case 2:
            /* SUB */
            //taking 2s complement of src
            src_holder = (size == BYTE)? ~(src_holder & LSB): ~src_holder;
            src_holder += 1;
            //adding to dst
            dst_holder = (size == BYTE)? ((dst_holder +src_holder) & LSB): (dst_holder + src_holder);
            update_psw(regfile[DST].value, regfile[SRC].value, dst_holder,size);
            break;
        case 3:
            /* SUBC */
            //taking 2;s complement of srxc
            src_holder = (size == BYTE)? ~(src_holder & LSB): ~src_holder;
            src_holder += 1;
            //adding to dst + carry
            dst_holder = (size == BYTE)? ((dst_holder +src_holder) & LSB): (dst_holder + src_holder);
            update_psw(regfile[DST].value, regfile[SRC].value, dst_holder,size);
            dst_holder -= pswptr->bit.c;
            break;
        default:
        fault_handler(ILLEG_INST);
    }
}

/* Decodes execution type based on group, calls the group specific execute to handle
individual instructions

    params in @ group_t group
              @ int type 
              @ int SRC
              @ int DST
              @ uint16_t modifier
              @ enum SIZE size
*/

void execute(group_t group, int type, int SRC, int DST, uint16_t modifier, enum SIZE size)
{
    uint16_t prev_ts;
    uint16_t execute = 0;
    //Checking CEX before execution
    if(cex_flag){
        //if True state achieve
        if(ts_flag){
            //execute true count until 0
            true_count --;
            execute = 1;
            if(true_count == 0){
                //stop execution once weve finished true state
                execute = 0;
                //store ts flag and assing as 0
                prev_ts = ts_flag;
                ts_flag = 0;
            }
        }else{
            //if we have already completed ts 
            if((true_count == 0) && prev_ts){
                //decrement false count and dont execute
                false_count --;
                execute = 0;
            }else if(true_count == 0){
                //true state did not execute
                //decrement false count
                false_count --;
                //execute
                execute = 1;
            }else{
                //true state executed
                //dec false count
                true_count --;
                //dont execute
                execute = 0;
            }
        }

        //if both true state and false state are completed
        if((true_count == 0) & (false_count == 0)){
            //cex is complete execute can occur normally
            cex_flag = 0;
            execute = 1;
        }
    }else{
        //cex has not occureed normally 
        execute = 1;
    }


    if(execute){
        //switch execute type based on group 
        switch (group)
        {
        case REGREL:
            execute_reg_rel(type, SRC, DST, modifier, size);
            break;
        case BR:
            execute_br(type, modifier);
            break;
        case REG_INI:
            execute_reg_init(type, DST, modifier);
            break;
        case REG2R:
            execute_reg_2_reg(type, SRC, DST, modifier, size);
            break;
        case SINREG:
            execute_s_reg(type, SRC, DST, modifier, size);
            break;
        case LOGIC:
            execute_logic(type, SRC, DST, modifier, size);
            break;
        case ARITH:
            execute_ari(type, SRC, DST, modifier, size);
            break;
        default:
            //illegal instruciton detected in decode 
            fault_handler(ILLEG_INST);
        }
    }
}

/* Sign extend the value and returns the sign extended value
    params in: @ uint16_t value
    params out: @ uint16_t sign_ext_val */

uint16_t sign_ext(uint16_t value){
    return   (value & 0xFF) | ((value & 0x80) ? 0xFF00 : 0);
}



/* 
    Handles faults by determining fault type and pushing necessary information to stack]
    then setting PC to fault vector address

    params in: @ enum FAULT fault
*/

void fault_handler(enum FAULT fault){
    execute_reg_2_reg(1,7,6,PRPO_BIT|SUB_BIT,WORD); //Pushing PC
    execute_reg_2_reg(1,5,6,PRPO_BIT|SUB_BIT,WORD); //Pushing SP
    regfile[R5].value = PSW_ADDR;
    execute_reg_2_reg(1,5,6,PRPO_BIT|SUB_BIT,WORD); //Pushing PSW
    regfile[R5].value = 0xFFFF; //LR = 0xFFFF
    switch(fault){ //PC equals fault vector
        case ILLEG_INST:
            regfile[R7].value = 0xFFD0; 
        case INV_ADDR:
            regfile[R7].value = 0xFFD4;
        case PRIO_FLT:
            regfile[R7].value = 0xFFD8;
    }

}

