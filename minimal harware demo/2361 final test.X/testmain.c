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

#define buffer_size 4
#define numsamples 128
#define SCALE_FACTOR (3.3/1023.0) //use in outputting value
#define threshold 310

int buffer1[buffer_size]; //declare buffer
int buffer_index = 0;

void setup(void){
    CLKDIVbits.RCDIV = 0;  //Set RCDIV=1:1 (default 2:1) 32MHz or FCY/2=16M
    AD1PCFG = 0x9fff;            //sets all pins to digital I/O
    TRISA = 0;  //set port A to outputs, 
    TRISB = 0;  //and port B to outputs 
    TRISBbits.TRISB10 = 1;
    TRISBbits.TRISB11 = 1;

    
//    IC1CONbits.ICM = 0b010;
//    IC2CONbits.ICM = 0b010;
//    _TRISB10 = 1; //set RB10 & 11 to inputs
//    _TRISB11 = 1;
    CNPU1bits.CN15PUE = 1; //turn on pullup resistor for CN22 = RB8
    CNPU2bits.CN16PUE = 1; //turn on pullup resistor for CN22 = RB8
//    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
//    RPINR7bits.IC1R = 10;  // Use Pin RP10 = "10", for Input Capture 1 (Table 10-2)
//    RPINR7bits.IC2R = 11;  // Use Pin RP11 for Incput Capture 2 = "11" (Table 10-3)
//    __builtin_write_OSCCONL(OSCCON | 0x40); // lock   PPS
}

void putVal(int ADCval){
    buffer1[buffer_index++] = ADCval;
    if(buffer_index >= buffer_size) //make circular
        buffer_index = 0;
}

void adc_init(void){
    AD1CON1bits.ADON = 0; //turn off ADC
    _TRISA0 = 1; //A0 is input
    _TRISA1 = 1; //A1 is input
    _TRISB2 = 1; //B2 is input
    _TRISB3 = 1; //B3 is input
    _TRISB12 = 1; //B12 is input
    _TRISB13 = 1; //B13 is input
    _TRISB14 = 1; //B14 is input
    _TRISB15 = 1; //B15 is input
    AD1PCFGbits.PCFG0 = 0; //analog mode
    
    AD1CON2bits.VCFG = 0b000; //use 3.3V max and 0V min (0-1023)
    AD1CON3bits.ADCS = 0b00000010; //use 2 * 62.5ns = 125ns
    AD1CON1bits.SSRC = 0b010; //use timer 3 to end sample and start conversion
    AD1CON3bits.SAMC = 0b10; // use 2 auto sample time bits
    AD1CON1bits.FORM = 0b00; //output as integer (0b0000000000 - 0b1111111111)
    
    AD1CON1bits.ASAM = 0b1; //enable auto sample
    AD1CON2bits.SMPI = 0b0000; //call interrupt on each conversion completion
    AD1CON1bits.ADON = 1; //turn on ADC
    
    _AD1IF = 0; //clear interrupt
    _AD1IE = 1; //enable interrupt
    
    TMR3 = 0; //setup timer 3
    T3CON = 0;
    T3CONbits.TCKPS = 0b00;
    PR3 = 15624;
    T3CONbits.TON = 1; //enable timer
}

void loop(void){
    int avgval = 0;
    char testchar[2] = {0};
    while(_T1IF == 0); //wait 100ms
    _T1IF = 0; //clear flag
    avgval = getAvg(); //save average value
    testchar[0] = '0'+ (avgval > threshold);
    if(_RB11 == 0)
        clearLCD();
    if(_RB10 == 0)
        printString(testchar);
}

void __attribute__((__interrupt__,__auto_psv__))_ADC1Interrupt(void){
    _AD1IF = 0; //clear interrupt
    putVal(ADC1BUF0);
}

//void __attribute__((__interrupt__,__auto_psv__))_IC2Interrupt(void){
//    _IC2IF = 0; //clear interrupt
//    delay(20); //debounce
//    while(_RB10 == 0); //wait until button is let up
//    clearLCD(); //clear LCD
//}
//void __attribute__((__interrupt__,__auto_psv__))_IC1Interrupt(void){
//    _IC1IF = 0; //clear interrupt
//    delay(20); //debounce
//    while(_RB11== 0); //wait until button is let up
//    loop();
//}

void timer1_init(void){
    //timer 1
    T1CON = 0; //turn off timer
    PR1 = 6249; //(6249+1)*256 = 100ms delay
    _T1IF = 0;
    T1CONbits.TCKPS = 0b11; //prescalar 1:256
    _T1IE = 0; //disable T2 interrupt
    TMR1 = 0; //clear timer
    T1CONbits.TON = 1; //start timer
}

void initbuffer(void){
    for(int i=0; i< buffer_size; i++){
        buffer1[i]=0;//clear all entries in buffer
    }
}

int getAvg(void){
    unsigned long int tempsum = 0;
    for(int i=0; i<buffer_size; i++){
        tempsum += buffer1[i]; //sum all entries
    }
    tempsum /= buffer_size; //divide by number of entries
    return tempsum;
}

int main(void){
    setup();
    initLCD();
    adc_init();
    timer1_init();
    initbuffer();
    
    printString("hello");
    while(1){
        loop();
    }
    return 0;
}

