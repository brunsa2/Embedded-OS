/* lcd.h -- (c) chris kern -- Mon Feb 16 17:37:37 EST 2009  */
/* Computer programmers do it byte by byte.                 */
#ifndef LCD_H__
#define LCD_H__

#include <stdint.h>

/* lcd_init(): initialize lcd port directions
 *             enable both display chips
 *             set display line to zero */
void lcd_init();

/* lcd_clear(): clear the entire lcd */
void lcd_clear();

/* lcd_setbit(): sets the bit at (x,y) to value v
 *               may not appear until lcd_flush() is called. */
void lcd_setbit(uint8_t x, uint8_t y, uint8_t v);

/* lcd_flush(): write pending bits out to display */
void lcd_flush();

/* lcd_set_cursor(): set the cursor position to the given 6x8 cell */
void lcd_set_cursor(uint8_t row, uint8_t col);

/* lcd_putch(): write an ascii character at the cursor and advance */
void lcd_putch(uint8_t ch);

/* lcd_putstr(): write a string at the cursor */
void lcd_putstr(const char* str);

#endif  /* LCD_H__ */