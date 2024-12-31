#include <8051.h>
#include "preemptive.h"

/* Share buffer between consumer and producer */
// __data __at (0x30) char available;
// __data __at (0x31) char globalBuffer;
__data __at (0x32) char producerChar;

__data __at (0x20) char mutex;
__data __at (0x21) char full;
__data __at (0x22) char empty;
__data __at (0x23) char head;
__data __at (0x24) char tail;
__data __at (0x25) char globalBuffer[3]; // 0x25-0x27

void Producer(void)
{
    producerChar = 'A';
    while (1)
    {
        // while(available == 1){}
        SemaphoreWait(empty);
        SemaphoreWait(mutex);

        EA = 0;
            globalBuffer[tail] = producerChar;
            // available = 1;
        EA = 1;

        SemaphoreSignal(mutex);
        SemaphoreSignal(full);

        if (tail == 2) {
            tail = 0;
        } 
        else {
            tail = tail + 1;
        }

        if(producerChar == 'Z') {
            producerChar = 'A';
        }
        else {
            producerChar = producerChar + 1; 
        }
    }
}

void Consumer(void)
{
    TMOD |= 0x20;
    TH1 = (char)-6;
    SCON = 0x50;
    TR1 = 1;

    while (1)
    {
        // while(available == 0){ }
        SemaphoreWait(full);
        SemaphoreWait(mutex);

        EA = 0;
            SBUF = globalBuffer[head];
            // available = 0;
        EA = 1;

        SemaphoreSignal(mutex);
        SemaphoreSignal(empty);

        if (head == 2) {
            head = 0;
        } 
        else {
            head = head + 1;
        }

        while(!TI){ }

        TI = 0;
    }
}

void main(void)
{
    SemaphoreCreate(mutex, 1);
    SemaphoreCreate(full, 0);
    SemaphoreCreate(empty, 3);
    head = 0;
    tail = 0;

    // available = 0;
    ThreadCreate(Producer);
    Consumer();
}

void _sdcc_gsinit_startup(void)
{
    __asm
        LJMP _Bootstrap
    __endasm;
}

void _mcs51_genRAMCLEAR(void) {}
void _mcs51_genXINIT(void) {}
void _mcs51_genXRAMCLEAR(void) {}

void timer0_ISR(void) __interrupt(1) {
    __asm
        ljmp _myTimer0Handler
    __endasm;
}