#include <8051.h>

#include "preemptive.h"

__data __at (0x33) ThreadID currentThreadID;
__data __at (0x34) char bitmap;
__data __at (0x35) char savedStackPointers[MAXTHREADS];

__data __at (0x3A) ThreadID newThreadID;
__data __at (0X3B) char i;
__data __at (0x3C) char currentSP;


#define SAVESTATE                                   \
    {                                               \
        __asm                                       \
        PUSH ACC                                    \
        PUSH B                                      \
        PUSH DPL                                    \
        PUSH DPH                                    \
        PUSH PSW                                    \
        __endasm;                                   \
        savedStackPointers[currentThreadID] = SP;   \
    }

#define RESTORESTATE                                \
    {                                               \
        SP = savedStackPointers[currentThreadID];   \
        __asm                                       \
        POP PSW                                     \
        POP DPH                                     \
        POP DPL                                     \
        POP B                                       \
        POP ACC                                     \
        __endasm;                                   \
    }

extern void main(void);

void Bootstrap(void)
{
    bitmap = 0;
    TMOD = 0;  // Timer 0 Mode 0
    IE = 0x82; // Enable Timer 0 Interrupt -> EA & ET = 1
    TR0 = 1; // Start Timer

    currentThreadID = ThreadCreate(main);
    RESTORESTATE;
}

ThreadID ThreadCreate(FunctionPtr fp)
{
    EA = 0;

    if(bitmap == 0x0F) {
        EA = 1;;
        return -1;
    }

    if ((bitmap & 0x1) == 0)      { newThreadID = 0; }
    else if ((bitmap & 0x2) == 0) { newThreadID = 1; }
    else if ((bitmap & 0x4) == 0) { newThreadID = 2; }
    else if ((bitmap & 0x8) == 0) { newThreadID = 3; }

    bitmap = bitmap | (1 << newThreadID);
    
    currentSP = SP;
    SP = 0x3F + (0x10 * newThreadID);

    __asm
      PUSH DPL
      PUSH DPH
   __endasm;

    __asm
        MOV A, #0
        PUSH ACC
        PUSH ACC
        PUSH ACC
        PUSH ACC
    __endasm;

    PSW = (newThreadID << 3);
    __asm
      PUSH PSW
   __endasm;

    savedStackPointers[newThreadID] = SP;
    SP = currentSP;

    EA = 1;

    return newThreadID;
}

void ThreadYield(void)
{
    EA = 0;
    SAVESTATE;
    do
    {
        if (currentThreadID == 3)  {
            currentThreadID = 0;
        }
        else {
            currentThreadID = currentThreadID + 1;
        }

        if (bitmap & (1 << currentThreadID)){
            break;
        }

    } while (1);
    RESTORESTATE;
    EA = 1;
}

void ThreadExit(void)
{
    EA = 0;
        RESTORESTATE;
    EA = 1;
}

void myTimer0Handler(void){
    EA = 0;
    SAVESTATE;

    /* [FIRST METHOD]
        One solution is for you to insert code to preserve the value of 
        any such registers by copying them to registers that have been 
        saved (e.g., B, DPH, DPL, etc., or your designated memory 
        locations) after SAVESTATE, and copy them back to those 
        registers before the RESTORESTATE.   This is the quickest way 
        to get working code.

    */

    __asm
        MOV B, r0
        MOV DPL, r1
        MOV DPH, r2
        MOV A, r3
        PUSH ACC
        MOV A, r4
        PUSH ACC
        MOV A, r5
        PUSH ACC
        MOV A, r6
        PUSH ACC
        MOV A, r7
        PUSH ACC
    __endasm;

    /* THREAD YIELD FUNCTION */
    do
    {
        if (currentThreadID == 3)  {
            currentThreadID = 0;
        }
        else {
            currentThreadID = currentThreadID + 1;
        }

        if (bitmap & (1 << currentThreadID)){
            break;
        }

    } while (1);

    __asm
        POP ACC
        MOV r7, A
        POP ACC
        MOV r6, A
        POP ACC
        MOV r5, A
        POP ACC
        MOV r4, A
        POP ACC
        MOV r3, A
        MOV r2, DPH
        MOV r1, DPL
        MOV r0, B
    __endasm;

    RESTORESTATE;
    EA = 1;

    __asm
        RETI
    __endasm;
}