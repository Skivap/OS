#ifndef __PREEMPTIVE_H__
#define __PREEMPTIVE_H__

#define MAXTHREADS 4 /* not including the scheduler */
/* the scheduler does not take up a thread of its own */

// #define EA = 0; EA = 0;
// #define EA = 1; EA = 1;

typedef char ThreadID;
typedef void (*FunctionPtr)(void);

ThreadID ThreadCreate(FunctionPtr);
void ThreadYield(void);
void ThreadExit(void);

void myTimer0Handler(void);

#define CNAME(S) _ ## S
#define LABEL(label) label ## $

// create a counting semaphore s that is initialized to n
#define SemaphoreCreate(s, n) s = n;

// do (busy-)wait() on semaphore s
#define SemaphoreWaitBody(s, label) \
    __asm \
        LABEL(label): \
            MOV A, CNAME(s) \
            JZ LABEL(label) \
            JB ACC.7, LABEL(label) \
            DEC CNAME(s) \
    __endasm; \

#define SemaphoreWait(s) SemaphoreWaitBody(s, __COUNTER__)

// signal() semaphore s
#define SemaphoreSignal(s) \
    __asm \
        INC CNAME(s) \
    __endasm; \

#endif // __COOPERATIVE_H__