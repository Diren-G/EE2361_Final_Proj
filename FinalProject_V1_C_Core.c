#include "xc.h"
#include "stdio.h"
#include "stdint.h"
#include "harpe507_lcd.h"
#include <string.h>

#define FCY 16000000UL
#define contrast 0x7A
#define LCDaddy 0b0111100
#define LCDaddy_writ 0b01111000
#define NUM_SAMPLES 3
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

// --- ADC setup ---
void setupADC(void){
    AD1PCFG = 0x0000; // All ANx analog mode
    AD1CON1 = 0x00E0;
    AD1CON2 = 0;
    AD1CON3 = 0x1F02;
    AD1CON1bits.ADON = 1;
}

int readADC(int ch){
    AD1CHS = ch;
    AD1CON1bits.SAMP = 1;
    while(!AD1CON1bits.DONE);
    return ADC1BUF0;
}

float readAvgVoltage(int ch){
    long sum = 0;
    for(int i = 0; i < NUM_SAMPLES; i++) {
        sum += readADC(ch);
        delay(1);  // small pause
    }
    float avg = (float)sum / NUM_SAMPLES;
    return (3.3 * avg) / 1024.0;
}


// --- Button setup ---
void setupButtons(void){
        //configure OC1 to RP6
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
    RPINR7bits.IC1R = 11;  // Use Pin RP11 = "11", for Input Capture 1 (Table 10-2)
    RPINR7bits.IC2R = 10;  // Use Pin RP110 = "10", for Input Capture 2 (Table 10-2)
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock   PPS
    
    TRISBbits.TRISB10 = 1; // Set as input
    TRISBbits.TRISB11 = 1;

    AD1PCFGbits.PCFG10 = 1; // RB10 digital
    AD1PCFGbits.PCFG11 = 1; // RB11 digital

    CNPU1bits.CN15PUE = 1;  // Enable pull-up on RB11 (CN15)
    CNPU2bits.CN16PUE = 1;  // Enable pull-up on RB10 (CN17)
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
void __attribute__((__interrupt__,__auto_psv__))IC1Interrupt(void){
    _IC1IF = 0; //clear interrupt
    delay(20); //debounce
    lcd_clear();
}

//interrupt for reading data
void __attribute__((__interrupt__,__auto_psv__))IC2Interrupt(void){
    _IC2IF = 0; //clear interrupt
   delay(DEBOUNCE_MS);
    if (!PORTBbits.RB10){
        char c = readPunchCard();
        int len = strlen(displayStr);
        if (len < 16) {
            displayStr[len] = c;
            displayStr[len+1] = '\0';
            lcd_clear();
            lcd_print_str(displayStr);
        }
    }
}

// --- Main ---
int main(void){
    CLKDIVbits.RCDIV = 0;
    setupADC();
    lcd_init();
    setupButtons();

    lcd_clear();
    lcd_print_str(displayStr);

    while(1){
    }
    return 0;
}

