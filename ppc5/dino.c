#include <8051.h>
#include "preemptive.h"
#include "keylib.h"
#include "buttonlib.h"
#include "lcdlib.h"

/* Share buffer between consumer and producer */
// __data __at (0x30) char available;
// __data __at (0x31) char globalBuffer;
// __data __at (0x31) char producer1Char;

__data __at (0x20) char mutex;
__data __at (0x21) char globalBuffer;
__data __at (0x22) char consumerValue;
__data __at (0x23) char producer1Char;
__data __at (0x24) char dinoPos;
__data __at (0x25) char it;
__data __at (0x26) char map00;
__data __at (0x27) char map01;
__data __at (0x28) char map10;
__data __at (0x29) char map11;
__data __at (0x2A) char map_temp;
__data __at (0x2B) char temp0;
__data __at (0x2C) char mutex1;
__data __at (0x2D) char stateDino; // 0b state = 0000_ diff = 0000
__data __at (0x2E) char timeTick;
__data __at (0x2F) char score;


const char dinosaur[] = { 0x07, 0x05, 0x06, 0x07, 0x14, 0x17, 0x0E, 0x0A };
const char cactus[] = { 0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04 }; 

void LCD_set_symbol(char code, const char symb[]) {
// or it could be defined as a macro.
// in any case, it takes the following calls:
    LCD_setCgRamAddress(code);  // code is the character generation memory 
                                // (CG RAM address for the bitmap. 
                                //  write '\1' for the dinosaur, '\2' for the cactus.
    LCD_write_char(symb[0]);
    LCD_write_char(symb[1]);
    LCD_write_char(symb[2]);
    LCD_write_char(symb[3]);
    LCD_write_char(symb[4]);
    LCD_write_char(symb[5]);
    LCD_write_char(symb[6]);
    LCD_write_char(symb[7]);
   // you need to write all 8 bytes for each bitmap.
   // of course, you could do it in a loop or do pointer arithmetic
}

void RenderDino(void) {
    while (1) {
        SemaphoreWait(mutex1);

        if((stateDino & 0x20) || (stateDino & 0x10) ){
            // Render Above
            LCD_cursorGoTo(0, 0);
            map_temp = map00;
            for (it = 0; it < 8; it++) {
                if(map_temp & 0x80) LCD_write_char('\2');
                else                LCD_write_char(' ');
                map_temp = map_temp << 1;
            }
            map_temp = map01;
            for (it = 0; it < 7; it++) {
                if(map_temp & 0x80) LCD_write_char('\2');
                else                LCD_write_char(' ');
                map_temp = map_temp << 1;
            }

            // Render Below
            LCD_cursorGoTo(1, 0);
            map_temp = map10;
            for (it = 0; it < 8; it++) {
                if(map_temp & 0x80) LCD_write_char('\2');
                else                LCD_write_char(' ');
                map_temp = map_temp << 1;
            }
            map_temp = map11;
            for (it = 0; it < 7; it++) {
                if(map_temp & 0x80) LCD_write_char('\2');
                else                LCD_write_char(' ');
                map_temp = map_temp << 1;
            }

            LCD_cursorGoTo(dinoPos, 0);
            LCD_write_char('\1');
        }
        else if (stateDino & 0x40){
            LCD_cursorGoTo(0, 0);
            LCD_write_char(' ');
            LCD_write_char(' ');
            LCD_write_char(' ');
            LCD_write_char('G');
            LCD_write_char('A');
            LCD_write_char('M');
            LCD_write_char('E');
            LCD_write_char(' ');
            LCD_write_char(' ');
            LCD_write_char('O');
            LCD_write_char('V');
            LCD_write_char('E');
            LCD_write_char('R');
            LCD_write_char(' ');
            LCD_write_char(' ');

            LCD_cursorGoTo(1, 0);
            LCD_write_char('S');
            LCD_write_char('C');
            LCD_write_char('O');
            LCD_write_char('R');
            LCD_write_char('E');
            LCD_write_char(' ');
            LCD_write_char(':');
            LCD_write_char(' ');
            LCD_write_char(score / 10 + '0');
            LCD_write_char(score % 10 + '0');
            LCD_write_char(' ');
            LCD_write_char(' ');
            LCD_write_char(' ');
            LCD_write_char(' ');
            LCD_write_char(' ');
        }
        

        SemaphoreSignal(mutex1);

        ThreadYield();


    }

}


void UpdateMap(){
    while (1) {
        SemaphoreWait(mutex1);

        if(stateDino & 0x20){
            timeTick = timeTick + 1;
            if(timeTick == 10){
                timeTick = (dinoPos & 0x0F);
                
                if(map00 & 0x80){
                    score = score + 1;
                }
                if(map10 & 0x80){
                    score = score + 1;
                }

                // Update Map
                temp0 = map00;
                map00 = (map00 << 1) + (map01 >> 7);
                map01 = (map01 << 1) + (temp0 >> 7);

                temp0 = map10;
                map10 = (map10 << 1) + (map11 >> 7);
                map11 = (map11 << 1) + (temp0 >> 7);
            }
        }


        SemaphoreSignal(mutex1);

        ThreadYield();
    }
}


void Producer1(void)
{
    producer1Char = 0;
    while (1)
    {
        while(!AnyKeyPressed()) {} // Wait for key to press
        producer1Char = KeyToChar(); // Get Pressed Key
        while(AnyKeyPressed()) {} // Release the key

        // SemaphoreWait(p2start);
        SemaphoreWait(mutex);

        EA = 0;
            globalBuffer = producer1Char;
        EA = 1;

        SemaphoreSignal(mutex);
    }
}

void Consumer (void) {
    TMOD |= 0x20;
    TH1 = (char)-6;
    SCON = 0x50;
    TR1 = 1;

    while (1)
    {
        SemaphoreWait(mutex);

        EA = 0;
            consumerValue = globalBuffer;
            // LCD_write_char(globalBuffer[head])
            // SBUF = globalBuffer[head];
        EA = 1;

        SemaphoreSignal(mutex);

        if(stateDino & 0x10) { // Main Menu
            dinoPos = 0;
            if(consumerValue <= '9' && consumerValue >= '1'){
                stateDino = (0x10) | (consumerValue - '0');
            }
            else if(consumerValue == '#'){
                stateDino = (stateDino & 0x0F) + 0x20;
            }
        }
        else if (stateDino & 0x20) { // Playing
            if(consumerValue == '2') dinoPos = 0;
            else if(consumerValue == '8') dinoPos = 1;
            
            if(dinoPos == 0){
                if(map00 >> 7){
                    stateDino = 0x41;
                }
            }
            else{
                if(map10 >> 7){
                    stateDino = 0x41;
                }
            }
        }
        else if (stateDino & 0x40) { // End Game
            
        }


        // while(!TI){ }
        // TI = 0;
    }
}

void main(void)
{
    SemaphoreCreate(mutex, 1);
    SemaphoreCreate(mutex1, 1);

    Init_Keypad();
    LCD_Init();

    LCD_set_symbol('\10', dinosaur);
    LCD_set_symbol('\20', cactus); 

    stateDino = 0x11;

    dinoPos = 0;
    score = 0;
    timeTick = 0;
    map00 = 0b00000100; map01 = 0b00000100;
    map10 = 0b01100000; map11 = 0b01100000;

    // available = 0;
    ThreadCreate(Producer1);
    ThreadCreate(RenderDino);
    ThreadCreate(UpdateMap);
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