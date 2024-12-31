#
# makefile for testing cooperative multithreading
#
CC = sdcc
CFLAGS = -c  --model-small
LDFLAGS = 
LCD_OBJ = testlcd.rel preemptive.rel lcdlib.rel buttonlib.rel keylib.rel
DINO_OBJ = dino.rel preemptive.rel lcdlib.rel buttonlib.rel keylib.rel

all: testlcd.hex dino.hex

testlcd.hex:   $(LCD_OBJ) 
	$(CC) $(LDFLAGS) -o testlcd.hex $(LCD_OBJ)

dino.hex:	$(DINO_OBJ) 
	$(CC) $(LDFLAGS) -o dino.hex $(DINO_OBJ)

clean:
	del /Q *.hex *.ihx *.lnk *.lst *.map *.mem *.rel *.rst *.sym *.asm *.lk

%.rel:  %.c    preemptive.h Makefile
	$(CC) $(CFLAGS) $<
