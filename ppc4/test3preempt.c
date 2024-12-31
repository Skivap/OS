#include <8051.h>
#include "preemptive.h"

/* Share buffer between consumer and producer */
// __data __at (0x30) char available;
// __data __at (0x31) char globalBuffer;
__data __at (0x31) char producer1Char;
__data __at (0x32) char producer2Char;

__data __at (0x20) char mutex;
__data __at (0x21) char full;
__data __at (0x22) char empty;
__data __at (0x23) char head;
__data __at (0x24) char tail;
__data __at (0x25) char p1start;
__data __at (0x26) char p2start;
__data __at (0x27) char globalBuffer[3]; // 0x25-0x27

void Producer1(void)
{
    producer1Char = 'A';
    while (1)
    {
        // while(available == 1){}
        SemaphoreWait(p1start);
        SemaphoreWait(empty);
        SemaphoreWait(mutex);

        EA = 0;
            globalBuffer[tail] = producer1Char;
            // available = 1;
        EA = 1;

        SemaphoreSignal(mutex);
        SemaphoreSignal(full);
        SemaphoreSignal(p2start);

        if (tail == 2) {
            tail = 0;
        } 
        else {
            tail = tail + 1;
        }

        if(producer1Char == 'Z') {
            producer1Char = 'A';
        }
        else {
            producer1Char = producer1Char + 1; 
        }
    }
}

void Producer2(void)
{
    producer2Char = '0';
    while (1)
    {
        // while(available == 1){}
        SemaphoreWait(p2start);
        SemaphoreWait(empty);
        SemaphoreWait(mutex);

        EA = 0;
            globalBuffer[tail] = producer2Char;
            // available = 1;
        EA = 1;

        SemaphoreSignal(mutex);
        SemaphoreSignal(full);
        SemaphoreSignal(p1start);

        if (tail == 2) {
            tail = 0;
        } 
        else {
            tail = tail + 1;
        }

        if(producer2Char == '9') {
            producer2Char = '0';
        }
        else {
            producer2Char = producer2Char + 1; 
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
    SemaphoreCreate(p1start, 1);
    SemaphoreCreate(p2start, 0);
    head = 0;
    tail = 0;

    // available = 0;
    ThreadCreate(Producer1);
    ThreadCreate(Producer2);
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