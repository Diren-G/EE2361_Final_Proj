#ifndef LCDLIB_H
#define	LCDLIB_H

#include <xc.h> 
#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
void lcd_cmd(char command);
void delay(unsigned int ms);
void initLCD(void);
void clearLCD(void);
void setcursor(int x, int y);
void printChar(char myChar);
void printString(const char * str);
    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

