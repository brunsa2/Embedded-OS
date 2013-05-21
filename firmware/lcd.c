/* lcd.c -- (c) chris kern -- Mon Feb 16 17:36:34 EST 2009  */
/* Swap read error.  You lose your mind.                    */
#include "lcd.h"
#include <avr/io.h>
#include <avr/pgmspace.h>

// LCD <=> AVR connections:
//
// D:   PORTB
// D/I: PD7
// R/W: PD6
// E:   PD5
// CS1: PD2
// CS2: PD3
//
// If you change them you will need to change the following #define's:

#define LCD_SELECT_INST (PORTD &= ~(1<<7))
#define LCD_SELECT_DATA (PORTD |= (1<<7))
#define LCD_SELECT_READ (DDRB = 0x00, PORTD |= (1<<6))
#define LCD_SELECT_WRITE (PORTD &= ~(1<<6), DDRB = 0xFF)
#define LCD_SELECT_CS1 (PORTD |= (1<<3), PORTD &= ~(1<<2))
#define LCD_SELECT_CS2 (PORTD |= (1<<2), PORTD &= ~(1<<3))
#define LCD_E_HIGH (PORTD |= (1<<5))
#define LCD_E_LOW (PORTD &= ~(1<<5))

#define LCD_SELECT_CHIP(A) ((A) ? LCD_SELECT_CS2 : LCD_SELECT_CS1)
#define LCD_SELECT_REG(A) ((A) ? LCD_SELECT_DATA : LCD_SELECT_INST);
#define LCD_INST 0
#define LCD_DATA 1
#define LCD_YADDR(Y) (((Y) & 0x3F) | 0x40)
#define LCD_XADDR(X) (((X) & 0x07) | 0xB8)
#define LCD_POWERON(P) (((P)?1:0) | 0x3E)
#define LCD_STARTLINE(L) (((L) & 0x3F) | 0xC0)
#define LCD_BUSY (1<<7)
#define LCD_OFF (1<<5)
#define LCD_RESET (1<<4)

// These might need to be adjusted.  If they're too low you'll get garbled
// data; too high and updates will be slow.
static uint8_t lcd_dl;
#define LCD_DELAY1 for(lcd_dl=0; lcd_dl<2; ++lcd_dl){ asm volatile("nop\n"); }
#define LCD_DELAY2 for(lcd_dl=0; lcd_dl<8; ++lcd_dl){ asm volatile("nop\n"); }

#include "font.h"



void lcd_write(uint8_t chip, uint8_t reg, uint8_t data) {
    LCD_SELECT_WRITE;
    LCD_SELECT_CHIP(chip);
    LCD_SELECT_REG(reg);
    PORTB = data;
    LCD_DELAY2;
    LCD_E_HIGH;
    LCD_DELAY1;
    LCD_E_LOW;
}

uint8_t lcd_read(uint8_t chip, uint8_t reg) {
    uint8_t d;
    LCD_SELECT_READ;
    LCD_SELECT_CHIP(chip);
    LCD_SELECT_REG(reg);
    LCD_DELAY2;
    LCD_E_HIGH;
    LCD_DELAY1;
    d = PINB;
    LCD_E_LOW;
    return d;
}

void lcd_wait(uint8_t chip) {
    while(lcd_read(chip, LCD_INST) & (LCD_BUSY | LCD_RESET)) { }
}

void lcd_write_wait(uint8_t chip, uint8_t reg, uint8_t data) {
    lcd_write(chip, reg, data);
    lcd_wait(chip);
}

void lcd_init() {
    uint16_t d;
    DDRB   =  0x00;  // PORTB inputs for now.
    DDRD  |=  0xEC;  // 5 outputs on PORTD
    PORTD &= ~0xE0;  // R/W, D/I, E low
    PORTD |=  0x0C;  // CS1, CS2 high
    
    for(d=0; d<50000; ++d);  // let the above sink in a bit.
    
    lcd_wait(0);
    lcd_wait(1);
    lcd_write_wait(0, LCD_INST, LCD_POWERON(1));
    lcd_write_wait(1, LCD_INST, LCD_POWERON(1));
    lcd_write_wait(0, LCD_INST, LCD_STARTLINE(0));
    lcd_write_wait(1, LCD_INST, LCD_STARTLINE(0));
    
    lcd_clear();
}


#define CACHE_EMPTY 255
static uint8_t cache_chip = CACHE_EMPTY;
static uint8_t cache_x;
static uint8_t cache_y;
static uint8_t cache_d;

void lcd_flush() {
    if (cache_chip == CACHE_EMPTY) return;
    lcd_write_wait(cache_chip, LCD_INST, LCD_YADDR(cache_y));
    lcd_write_wait(cache_chip, LCD_INST, LCD_XADDR(cache_x));
    lcd_write_wait(cache_chip, LCD_DATA, cache_d);
    cache_chip = CACHE_EMPTY;
}

void lcd_load(uint8_t chip, uint8_t x, uint8_t y) {
    if (cache_chip == chip &&
        cache_x == x &&
        cache_y == y) {
        return;
    }
    lcd_flush();
    cache_x = x;
    cache_y = y;
    cache_chip = chip;
    
    lcd_write_wait(cache_chip, LCD_INST, LCD_YADDR(cache_y));
    lcd_write_wait(cache_chip, LCD_INST, LCD_XADDR(cache_x));
    
    // the lcd has a read pipeline; each read gets you the last's result.
    lcd_read(cache_chip, LCD_DATA);
    cache_d = lcd_read(cache_chip, LCD_DATA);
}

void lcd_clear() {
    uint8_t x, y;
    for(x = 0; x < 8; ++x) {
        lcd_write_wait(0, LCD_INST, LCD_YADDR(0));
        lcd_write_wait(0, LCD_INST, LCD_XADDR(x));
        for(y = 0; y < 64; ++y) {
            lcd_write_wait(0, LCD_DATA, 0);
        }
    }
    
    for(x = 0; x < 8; ++x) {
        lcd_write_wait(1, LCD_INST, LCD_YADDR(0));
        lcd_write_wait(1, LCD_INST, LCD_XADDR(x));
        for(y = 0; y < 64; ++y) {
            lcd_write_wait(1, LCD_DATA, 0);
        }
    }
    cache_chip = CACHE_EMPTY;
}

void lcd_setbit(uint8_t x, uint8_t y, uint8_t v) {
    uint8_t lcd_chip = (x & 0x40) ? 1 : 0;
    uint8_t lcd_x = (y & 0x3F) >> 3;
    uint8_t lcd_y = (x & 0x3F);
    uint8_t lcd_bit = y & 0x07;
    lcd_load(lcd_chip, lcd_x, lcd_y);
    if (v) {
        cache_d |= 1 << lcd_bit;
    } else {
        cache_d &= ~(1 << lcd_bit);
    }
}

static uint8_t cursor_x;
static uint8_t cursor_y;

void lcd_set_cursor(uint8_t row, uint8_t col) {
    cursor_x = 6 * col;
    cursor_y = 8 * row;
}

void lcd_putch(uint8_t ch) {
    uint8_t x, y;
    const uint8_t* chp;
    uint8_t b;
    if (ch < 32) ch = 32;
    if (ch > 128) ch = 128;
    chp = font_5x7_data + 5 * (ch-32);
    for(x = 0; x < 6; ++x) {
        //b = pgm_read_byte(chp + x);
        b = *(chp + x);
        for(y = 0; y < 8; ++y) {
            if (x < 5 && y < 7) {
                lcd_setbit(cursor_x + x, cursor_y +y, b & (1<<y));
            } else {
                lcd_setbit(cursor_x + x, cursor_y +y, 0);
            }
        }
    }
    cursor_x += 6;
    lcd_flush();
}

void lcd_putstr(const char* str) {
    while(*str) {
        lcd_putch(*str++);
    }
}