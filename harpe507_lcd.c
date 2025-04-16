/*
 * File:   harpe507_lcd.c
 * Author: mrcarrot
 *
 * Created on April 14, 2025, 11:51 AM
 */

#include "harpe507_lcd.h"

#include "xc.h"

#include "harpe507_lab5_asm.h"

#define LCD_ADDR ((uint8_t)0x78)
#define LCD_ADDR_W ((uint8_t)((LCD_ADDR) & 0b11111110))
#define LCD_ADDR_R ((uint8_t)((LCD_ADDR) | 0b00000001))
#define LCD_CMD ((uint8_t)0x00)
#define LCD_DATA ((uint8_t)0x40)

static inline void i2c_start(void)
{
    I2C1CONbits.SEN = 1;
    while (I2C1CONbits.SEN);
}

static inline void i2c_end(void)
{
    I2C1CONbits.PEN = 1;
    while (I2C1CONbits.PEN);
}

void lcd_write_1(uint8_t byte)
{
    i2c_start();
    
    _MI2C1IF = 0;
    I2C1TRN = byte;
    while (!_MI2C1IF);
    
    i2c_end();
}

int lcd_write_3(uint8_t bytes[3])
{
    int error = 0;
    
    i2c_start();
    
    for (int i = 0; i < 3; i++)
    {
        _MI2C1IF = 0;
        I2C1TRN = bytes[i];
        while (!(_MI2C1IF));
        if (I2C1STATbits.ACKSTAT)
        {
            _MI2C1IF = 0;
            i2c_end();
            return lcd_write_3(bytes);
        }
    }
    
    i2c_end();
    
    return error;
}

int lcd_write_command(uint8_t command)
{
    return lcd_write_3((uint8_t[]){ LCD_ADDR_W, LCD_CMD, command });
}

int lcd_write_data(uint8_t data)
{
    return lcd_write_3((uint8_t[]){ LCD_ADDR_W, LCD_DATA, data });
}

void lcd_init(void)
{
    I2C1CONbits.I2CEN = 0;
    
    TRISBbits.TRISB6 = 0;
    
    // Reset LCD
    LATB |= (1 << 6);
    for (int i = 0; i < 50; i++) delay_1ms();
    LATB &= (0xffff ^ (1 << 6));
    delay_200us();
    LATB |= (1 << 6);
    delay_1ms();
    
    // Initialize I²C pin pull-up resistors
    _CN21PUE = 1;
    _CN22PUE = 1;
    
    // Initialize I²C Properties
    I2C1BRG = 0x9d;
    I2C1CONbits.I2CEN = 1;
    
    // Initialize LCD with proper commands
    uint8_t lcdInitCmds[] = 
    {
        0x3a, 0x09, 0x06, 0x1e, 0x39, 0x1b, 0x6e, 0x56, 0x7a, 0x38, 0x0f, 0x01,
        0x3a, 0x09, 0x1a, 0x3c
    };
    
    for (int i = 0; i < sizeof(lcdInitCmds); i++)
    {
        lcd_write_command(lcdInitCmds[i]);
    }
    lcd_set_cursor(0, 0);

    uint8_t message[] = {
        0xba, 0xdd, 0xc6, 0xc1, 0xca, 0x00
    };

    lcd_print_str((char*) message);
}

void lcd_cursor_right(void)
{
    lcd_write_command(0x14);
}

int cursor_x = 0;
int cursor_y = 0;

void lcd_set_cursor(int x, int y)
{
    lcd_write_command(0x02);
    int positions = y * 10 + x;
    for (int i = 0; i < positions; i++)
    {
        lcd_cursor_right();
    }
    cursor_x = x;
    cursor_y = y;
}

void lcd_print_char(char c)
{
    lcd_write_data(c);
    int newX = cursor_x == 9 ? 0 : cursor_x + 1;
    int newY = cursor_x < 9 ? cursor_y : cursor_y + 1;
    if (newY == 4)
        newY = 0;
    lcd_set_cursor(newX, newY);
}

void lcd_clear(void)
{
    lcd_write_command(0x01);
    lcd_set_cursor(0, 0);
}

void lcd_print_str(char *str)
{
    for (int i = 0; str[i]; i++)
        lcd_print_char(str[i]);
}
