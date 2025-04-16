
#include "xc.h"
#include "stdio.h"
#include "stdint.h"

#pragma config ICS = PGx1
#pragma config FWDTEN = OFF
#pragma config GWRP = OFF
#pragma config GCP = OFF
#pragma config JTAGEN = OFF
#pragma config I2C1SEL = PRI
#pragma config IOL1WAY = OFF
#pragma config OSCIOFNC = ON
#pragma config FCKSM = CSECME
#pragma config FNOSC = FRCPLL

void setup(void){
    CLKDIVbits.RCDIV = 0;
    AD1PCFG = 0x0000;    // AN0 is analog (RA0)
    TRISB = 0;
    TRISA = 0;


    AD1CON1bits.ADON = 1; // turn on ADC
}

int main(void){
    while(1){
    
    }
    return 0;
}