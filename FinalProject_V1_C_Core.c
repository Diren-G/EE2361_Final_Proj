#include "xc.h"
#include "stdio.h"
#include "stdint.h"
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

//interrupt for clearing lcd
void __attribute__((__interrupt__,__auto_psv__))IC1Interrupt(void){
    _IC1IF = 0; //clear interrupt
    delay(20); //debounce
    clear_lcd();
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

// --- LCD (reuse from original) ---
void lcd_cmd(char command){
    _SEN = 1; while(_SEN == 1);
    _MI2C1IF = 0; I2C1TRN = LCDaddy_writ; while(!_MI2C1IF || _TRSTAT);
    _MI2C1IF = 0; I2C1TRN = 0x00;         while(!_MI2C1IF || _TRSTAT);
    _MI2C1IF = 0; I2C1TRN = command;      while(!_MI2C1IF || _TRSTAT);
    _PEN = 1; while(_PEN == 1);
}

void clear_lcd(void){
    lcd_cmd(0x01);
}

void initLCD(void){
    I2C1CON = 0; I2C1BRG = 157;
    _MI2C1IF = 0; I2C1CONbits.I2CEN = 1;
    delay(40);
    PORTBbits.RB6 = 0; delay(40);
    PORTBbits.RB6 = 1; delay(40);
    lcd_cmd(0x3A); lcd_cmd(0x09); lcd_cmd(0x06); lcd_cmd(0x1E); lcd_cmd(0x39);
    lcd_cmd(0x1B); lcd_cmd(0x6E); lcd_cmd(0x56); lcd_cmd(contrast);
    lcd_cmd(0x38); lcd_cmd(0x0F);
    delay(200);
    lcd_cmd(0x3A); lcd_cmd(0x09); lcd_cmd(0x1A); lcd_cmd(0x3C);
    delay(2);
    clear_lcd();
}

void set_cursor(int x, int y){
    int cursor_loc = (0x20 * x + y) | 0b10000000;
    lcd_cmd(cursor_loc);
}

void printChar(char myChar){
    _SEN = 1; while(_SEN == 1);
    _MI2C1IF = 0; I2C1TRN = LCDaddy_writ; while(!_MI2C1IF || _TRSTAT);
    _MI2C1IF = 0; I2C1TRN = 0x40;         while(!_MI2C1IF || _TRSTAT);
    _MI2C1IF = 0; I2C1TRN = myChar;       while(!_MI2C1IF || _TRSTAT);
    _PEN = 1; while(_PEN == 1);
}

void printString(const char* str){
    set_cursor(0,0);
    for(int i = 0; str[i] != 0; i++){
        printChar(str[i]);
    }
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

// --- Main ---
int main(void){
    CLKDIVbits.RCDIV = 0;
    setupADC();
    initLCD();
    setupButtons();

    clear_lcd();
    printString(displayStr);

    while(1){
        if (!PORTBbits.RB10){ // Read button
            delay(DEBOUNCE_MS);
            if (!PORTBbits.RB10){
                char c = readPunchCard();
                int len = strlen(displayStr);
                if (len < 16) {
                    displayStr[len] = c;
                    displayStr[len+1] = '\0';
                    clear_lcd();
                    printString(displayStr);
                }
                while (!PORTBbits.RB10); // wait release
            }
        }

        if (!PORTBbits.RB11){ // Reset button
            delay(DEBOUNCE_MS);
            if (!PORTBbits.RB11){
                displayStr[0] = '\0';
                clear_lcd();
                printString(displayStr);
                while (!PORTBbits.RB11);
            }
        }
    }
    return 0;
}

