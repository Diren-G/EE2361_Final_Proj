#ifndef HARPE507_LCD_H
#define HARPE507_LCD_H

void lcd_init(void);
void lcd_set_cursor(int x, int y);
void lcd_print_char(char c);
void lcd_print_str(char *str);
void lcd_clear(void);

#endif