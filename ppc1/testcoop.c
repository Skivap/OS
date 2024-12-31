#include <8051.h>
#include "cooperative.h"

/* Share buffer between consumer and producer */
__data __at (0x30) char available;
__data __at (0x31) char globalBuffer;
__data __at (0x32) char producerChar;

/* [Producer]
 * the producer in this test program generates one characters at a
 * time from 'A' to 'Z' and starts from 'A' again. The shared buffer
 * must be empty in order for the Producer to write.
 */
void Producer(void)
{
    producerChar = 'A';
    while (1)
    {
        while(available == 1){
            ThreadYield();
        }

        globalBuffer = producerChar;
        available = 1;

        if(producerChar == 'Z') {
            producerChar = 'A';
        }
        else {
            producerChar = producerChar + 1; 
        }
    }
}

/* [Consumer]
 * the consumer in this test program gets the next item from
 * the queue and consume it and writes it to the serial port.
 * The Consumer also does not return.
 */
void Consumer(void)
{
    TMOD = 0x20;
    TH1 = (char)-6;
    SCON = 0x50;
    TR1 = 1;

    while (1)
    {
        while(available == 0){
            ThreadYield();
        }

        SBUF = globalBuffer;
        available = 0;

        while(!TI){
            ThreadYield();
        }

        TI = 0;
    }
}

/* [Main]
 * main() is started by the thread bootstrapper as thread-0.
 * It can create more thread(s) as needed:
 * one thread can act as producer and another as consumer.
 */
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
