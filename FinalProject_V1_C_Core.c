#include "xc.h"
#include "stdio.h"
#include "stdint.h"
#include "lcdlib.h"
#include <string.h>
#include "adc_lib.h"
#include "bttn_lib.h"

#define FCY 16000000UL
#define contrast 0x7A
#define LCDaddy 0b0111100
#define LCDaddy_writ 0b01111000
#define NUM_SAMPLES 4
#define DEBOUNCE_MS 2

const int channels[8] = {0, 1, 4, 5, 9, 10, 11, 12}; // MSB to LSB: AN0 to AN12
char displayStr[17] = "";  // Holds characters to print

// --- Delay ---
void delay(int ms){
    while(ms-- > 0){
        asm("repeat #15998");
        asm("nop");
    }
}

// --- Read punch card into ASCII char ---
char readPunchCard(void){
    uint8_t bits = 0;
    for (int i = 0; i < 8; i++) {
        float v = readAvgVoltage(channels[i]);
        bits = (bits << 1) | (v > 0.5 ? 1 : 0);
    }
    return (char)bits;
}

//interrupt for clearing lcd
void __attribute__((__interrupt__,__auto_psv__))_IC1Interrupt(void){
    _IC1IF = 0; //clear interrupt
    delay(20); //debounce
    clearLCD();
}

//interrupt for reading data
void __attribute__((__interrupt__,__auto_psv__))_IC2Interrupt(void){
    _IC2IF = 0; //clear interrupt
   delay(DEBOUNCE_MS);
    if (!PORTBbits.RB10){
        char c = readPunchCard();
        int len = strlen(displayStr);
        if (len < 16) {
            displayStr[len] = c;
            displayStr[len+1] = '\0';
            clearLCD();
            printString(displayStr);
        }
    }
}

// --- Main ---
int main(void){
    CLKDIVbits.RCDIV = 0;
    setupADC();
    initLCD();
    setupButtons();
    

    while(1){
    }
    return 0;
}

