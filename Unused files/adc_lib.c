#include "xc.h"

#define NUM_SAMPLES 4

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