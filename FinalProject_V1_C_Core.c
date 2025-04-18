#include "xc.h"
#include "stdlib.h"
#include "stdio.h"
#include "lcdlib.h"

// CW1: FLASH CONFIGURATION WORD 1 (see PIC24 Family Reference Manual 24.1)
#pragma config ICS = PGx1          // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config FWDTEN = OFF        // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config GWRP = OFF          // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF           // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF        // JTAG Port Enable (JTAG port is disabled)


// CW2: FLASH CONFIGURATION WORD 2 (see PIC24 Family Reference Manual 24.1)
#pragma config I2C1SEL = PRI       // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF       // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON       // Primary Oscillator I/O Function (CLKO/RC15 functions as I/O pin)
#pragma config FCKSM = CSECME      // Clock Switching and Monitor (Clock switching is enabled, 
                                       // Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL      // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))

void setup(void){
    CLKDIVbits.RCDIV = 0;  //Set RCDIV=1:1 (default 2:1) 32MHz or FCY/2=16M
    AD1PCFG = 0x9fff;            //sets all pins to digital I/O
    TRISA = 0;  //set port A to outputs, 
    TRISB = 0;  //and port B to outputs 
    TRISBbits.TRISB10 = 1;
    TRISBbits.TRISB11 = 1;
    _TRISA0 = 1; //A0 is input
    _TRISA1 = 1; //A1 is input
    _TRISB2 = 1; //B2 is input
    _TRISB3 = 1; //B3 is input
    _TRISB12 = 1; //B12 is input
    _TRISB13 = 1; //B13 is input
    _TRISB14 = 1; //B14 is input
    _TRISB15 = 1; //B15 is input

    
    CNPU1bits.CN15PUE = 1; //turn on pullup resistor for CN22 = RB8
    CNPU2bits.CN16PUE = 1; //turn on pullup resistor for CN22 = RB8
}

void loop(void)
{
    union 
    {
        char c;
        struct
        {
            unsigned int bit0 : 1;
            unsigned int bit1 : 1;
            unsigned int bit2 : 1;
            unsigned int bit3 : 1;
            unsigned int bit4 : 1;
            unsigned int bit5 : 1;
            unsigned int bit6 : 1;
            unsigned int bit7 : 1;
        } bits;
    } c = {0};

    c.bits.bit0 = _RA0;
    c.bits.bit1 = _RA1;
    c.bits.bit2 = _RB2;
    c.bits.bit3 = _RB3;
    c.bits.bit4 = _RB15;
    c.bits.bit5 = _RB14;
    c.bits.bit6 = _RB13;
    c.bits.bit7 = _RB12;
    

    if(_RB11 == 0)
        clearLCD();
    if(_RB10 == 0)
    {
        delay(2);
        printChar(c.c);
        while (!_RB10);
    }
}

int main(void){
    setup();
    initLCD();
    
    printString("hello");
    while(1){
        loop();
    }
    return 0;
}
