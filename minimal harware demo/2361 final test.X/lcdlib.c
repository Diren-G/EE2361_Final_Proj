#include "xc.h"

#define contrast 0x7A

const int LCDaddy = 0b0111100; // 7-bit base I2C address with SA0 line pulled low 
int LCDaddy_writ = 0b01111000;
int cursor_loc = 0b10000000;

void delay(unsigned int ms){
    while(ms-- > 0){
        asm("repeat #15998");
        asm("nop");
    }
}

void lcd_cmd(char command){
    _SEN = 1; //start sequence
    while(_SEN == 1); //wait for start sequence to finish
    
    _MI2C1IF = 0; //clear flag
    I2C1TRN = 0b01111000; //address frame/byte and R/nW bit
    while(_MI2C1IF == 0 || _TRSTAT == 1); //wait for flag and !TRSTAT
    
    _MI2C1IF = 0; //reset flag
    I2C1TRN = 0x00; //control frame/byte
    while(_MI2C1IF == 0 || _TRSTAT == 1);//wait for flag and !TRSTAT
    
    _MI2C1IF = 0;//reset flag
    I2C1TRN = command; //command data
    while(_MI2C1IF == 0 || _TRSTAT == 1);//wait for flag and !TRSTAT
    
    _PEN = 1; //stop sequence
    while (_PEN == 1); //wait for stop sequence to finish
}

void clearLCD(void){
    lcd_cmd(0x01); //clear lcd cmd
}

void initLCD(void){
    //set up I2C
    I2C1CON = 0;
    I2C1BRG = 157; //set to 100kHz
    _MI2C1IF = 0; //clear interrupt
    I2C1CONbits.I2CEN = 1;
    
    delay(40);
    PORTBbits.RB6 = 0;
    delay(40); //wait 40ms
    PORTBbits.RB6 = 1;
    delay(40);
    
    
    lcd_cmd(0x3A); //8bit data length extension
    lcd_cmd(0x09); // 4 line display
    lcd_cmd(0x06); //bottom view
    lcd_cmd(0x1E); //BS1 =1
    lcd_cmd(0x39); //8bit data length extension
    lcd_cmd(0x1B); 
    lcd_cmd(0x6E);
    lcd_cmd(0x56);
    lcd_cmd(contrast); //set contrast
    lcd_cmd(0x38); //function set
    lcd_cmd(0x0F); //display on, cursor on, blink on
    
    delay(200); //wait 200 ms
    //set double height
    lcd_cmd(0x3A); 
    lcd_cmd(0x09);
    lcd_cmd(0x1A);
    lcd_cmd(0x3C);
    
    delay(1);
    clearLCD(); //clear display
}

void setcursor(int x, int y){
    int cursor_loc = 0;
    cursor_loc = (0x20*x + y) | 0b10000000; //determine location of cursor
    lcd_cmd(cursor_loc); //send info to lcd
}

void printChar(char myChar){
    _SEN = 1; //start sequence
    while(_SEN == 1); //wait for start sequence to finish
    
    _MI2C1IF = 0; //clear flag
    I2C1TRN = LCDaddy_writ; //address frame/byte and R/nW bit
    while(_MI2C1IF == 0 || _TRSTAT == 1); //wait for flag and !TRSTAT
    
    _MI2C1IF = 0; //reset flag
    I2C1TRN = 0x40; //control frame/byte
    while(_MI2C1IF == 0 || _TRSTAT == 1);//wait for flag and !TRSTAT
    
    _MI2C1IF = 0;//reset flag
    I2C1TRN = myChar; //command data
    while(_MI2C1IF == 0 || _TRSTAT == 1);//wait for flag and !TRSTAT
    
    _PEN = 1; //stop sequence
    while (_PEN == 1); //wait for stop sequence to finish
}

void printString(const char * str){
    int i=0;
    setcursor(0,0); //start at top left
    while(str[i] != 0){ //check that data entry isn't null
        printChar(str[i]); //print
        i++;
    }
}