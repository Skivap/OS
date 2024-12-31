#include <8051.h>
#include "preemptive.h"

/* Share buffer between consumer and producer */
__data __at (0x30) char available;
__data __at (0x31) char globalBuffer;
__data __at (0x32) char producerChar;

void Producer(void)
{
    producerChar = 'A';
    while (1)
    {
        while(available == 1){}

        EA = 0;
            globalBuffer = producerChar;
            available = 1;
        EA = 1;

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
        while(available == 0){ }

        EA = 0;
            SBUF = globalBuffer;
            available = 0;
        EA = 1;

        while(!TI){ }

        TI = 0;
    }
}

void main(void)
{
    available = 0;
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