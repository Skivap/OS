#include <8051.h>

#include "cooperative.h"

/*
 * [TODO]
 * declare the static globals here using
 *        __data __at (address) type name; syntax
 * manually allocate the addresses of these variables, for
 * - saved stack pointers (MAXTHREADS)
 * - current thread ID
 * - a bitmap for which thread ID is a valid thread;
 *   maybe also a count, but strictly speaking not necessary
 * - plus any temporaries that you need.
 */
__data __at (0x33) ThreadID currentThreadID;
__data __at (0x34) char bitmap;
__data __at (0x35) char savedStackPointers[MAXTHREADS];

__data __at (0x3A) ThreadID newThreadID;
__data __at (0X3B) char i;
__data __at (0x3C) char currentSP;

/*
 * [TODO]
 * define a macro for saving the context of the current thread by
 * 1) push ACC, B register, Data pointer registers (DPL, DPH), PSW
 * 2) save SP into the saved Stack Pointers array
 *   as indexed by the current thread ID.
 * Note that 1) should be written in assembly,
 *     while 2) can be written in either assembly or C
 */
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

/*
 * [TODO]
 * define a macro for restoring the context of the current thread by
 * essentially doing the reverse of SAVESTATE:
 * 1) assign SP to the saved SP from the saved stack pointer array
 * 2) pop the registers PSW, data pointer registers, B reg, and ACC
 * Again, popping must be done in assembly but restoring SP can be
 * done in either C or assembly.
 */
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

/*
 * we declare main() as an extern so we can reference its symbol
 * when creating a thread for it.
 */

extern void main(void);

/*
 * Bootstrap is jumped to by the startup code to make the thread for
 * main, and restore its context so the thread can run.
 */

void Bootstrap(void)
{
    /*
     * [TODO]
     * initialize data structures for threads (e.g., mask)
     *
     * optional: move the stack pointer to some known location
     * only during bootstrapping. by default, SP is 0x07.
     *
     * [TODO]
     *     create a thread for main; be sure current thread is
     *     set to this thread ID, and restore its context,
     *     so that it starts running main().
     */

    bitmap = 0;
    currentThreadID = ThreadCreate(main);
    RESTORESTATE;
}

/*
 * ThreadCreate() creates a thread data structure so it is ready
 * to be restored (context switched in).
 * The function pointer itself should take no argument and should
 * return no argument.
 */
ThreadID ThreadCreate(FunctionPtr fp)
{
    

    if(bitmap == 0x0F) {
        return -1;
    }

    if ((bitmap & 0x1) == 0)      { newThreadID = 0; }
    else if ((bitmap & 0x2) == 0) { newThreadID = 1; }
    else if ((bitmap & 0x4) == 0) { newThreadID = 2; }
    else if ((bitmap & 0x8) == 0) { newThreadID = 3; }
    
    bitmap = bitmap | (1 << newThreadID);

    // 2. Calculate the starting stack location for new thread
        /*
            Base 0x3F
            Each thread difference is 0x10
        */
    currentSP = SP;
    SP = 0x3F + (0x10 * newThreadID); // Base + Offset

    // 3. Push RA FP -> DPL & DPH
    __asm
      PUSH DPL
      PUSH DPH
   __endasm;

   // 4. Clear and Push 4 zeros
    __asm
        MOV A, #0
        PUSH ACC
        PUSH ACC
        PUSH ACC
        PUSH ACC
    __endasm;

    // 5. Set PSW
    PSW = (newThreadID << 3);
    __asm
      PUSH PSW
   __endasm;

    // 6. Current stack pointer to the saved stack pointer array for this newly created thread ID
    savedStackPointers[newThreadID] = SP;
    SP = currentSP;

    // 7. return thread ID
    return newThreadID;
}

/*
 * this is called by a running thread to yield control to another
 * thread.  ThreadYield() saves the context of the current
 * running thread, picks another thread (and set the current thread
 * ID to it), if any, and then restores its state.
 */

void ThreadYield(void)
{
    SAVESTATE;
    do
    {
        /*
         * [TODO]
         * do round-robin policy for now.
         * find the next thread that can run and
         * set the current thread ID to it,
         * so that it can be restored (by the last line of
         * this function).
         * there should be at least one thread, so this loop
         * will always terminate.
         */
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
}

/*
 * ThreadExit() is called by the thread's own code to terminate
 * itself.  It will never return; instead, it switches context
 * to another thread.
 */
void ThreadExit(void)
{
    /*
     * clear the bit for the current thread from the
     * bit mask, decrement thread count (if any),
     * and set current thread to another valid ID.
     * Q: What happens if there are no more valid threads?
     */

    RESTORESTATE;
}
