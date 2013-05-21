#include "os.h"
#include "usart.h"
#include "lcd.h"
#include "adc.h"
#include "i2c.h"

#include <stdint.h>

#define STACK_SIZE 64

os_semaphore lcd_sem;
os_semaphore btn_sem;
os_semaphore stt_sem;
os_semaphore tck_sem;

uint8_t state = 0;
uint8_t ticks = 0;

volatile uint8_t button_task_stack[STACK_SIZE + 64];
volatile uint8_t uart_task_stack[STACK_SIZE + 64];
volatile uint8_t adc_task_stack[STACK_SIZE + 64];

void uint8_t_to_ascii(uint8_t num, char *buffer) {
    uint8_t hundreds = 0, tens = 0, ones = 0;
    char hundreds_digit, tens_digit, ones_digit;
    while (num >= 100) {
        hundreds++;
        num -= 100;
    }
    
    while (num >= 10) {
        tens++;
        num -= 10;
    }
    
    while (num >= 1) {
        ones++;
        num -= 1;
    }
    
    hundreds_digit = hundreds + '0';
    if (hundreds_digit == '0') {
        hundreds_digit = ' ';
    }
    tens_digit = tens + '0';
    if (hundreds_digit != '0' && tens_digit == '0') {
        tens_digit = ' ';
    }
    ones_digit = ones + '0';
    
    *(buffer) = hundreds_digit;
    *(buffer + 1) = tens_digit;
    *(buffer + 2) = ones_digit;
    *(buffer + 3) = '\0';
}

void uart_task(void);
void adc_task(void);
void button_task(void);

void uart_task(void) {
	/*usart_init(USART_TRANSMIT | USART_RECEIVE);
	usart_putc('\f');
	while (1) {
        usart_puts("Hello World!\r\n");
        os_delay(os_get_current_pid(), 1000);
    }*/
}

void adc_task(void) {
    uint8_t update_time = 0;
    adc_init();
    while (1) {
        char light_str[6], temp_str[6], time_str[6];
        uint8_t light = adc_acquire(0);
        uint8_t temperature = adc_acquire(1);
        uint8_t_to_ascii(light, &(light_str[0]));
        uint8_t_to_ascii(temperature, &(temp_str[0]));
        uint8_t_to_ascii(ticks, &(time_str[0]));
        os_semaphore_wait(&lcd_sem);
        lcd_set_cursor(0, 0);
        lcd_putstr("Time: ");
        lcd_putstr(time_str);
        lcd_set_cursor(1, 0);
        lcd_putstr("Light: ");
        lcd_putstr(light_str);
        lcd_set_cursor(2, 0);
        lcd_putstr("Temp: ");
        lcd_putstr(temp_str);
        os_semaphore_signal(&lcd_sem);
        
        os_semaphore_wait(&stt_sem);
        if (state) {
            update_time = 1;
        } else {
            update_time = 0;
        }
        os_semaphore_signal(&stt_sem);
        os_semaphore_wait(&tck_sem);
        if (update_time) {
            ticks++;
        }
        os_semaphore_signal(&tck_sem);
        
        if (state == 1) {
            usart_puts("Time: ");
            usart_puts(time_str);
            usart_puts("\r\n");
            usart_puts("Light: ");
            usart_puts(light_str);
            usart_puts("\r\n");
            usart_puts("Temperature: ");
            usart_puts(temp_str);
            usart_puts("\r\n");
        }
        
        os_delay(os_get_current_pid(), 1000);
    }
}

void button_task(void) {
    uint8_t start = 0, stop = 0, pause = 0;
    os_semaphore_wait(&btn_sem);
    // Port A with no pull ups for buttons, input
    DDRA &= ~(0b01110000);
    os_semaphore_signal(&btn_sem);
    os_semaphore_wait(&lcd_sem);
    lcd_set_cursor(4, 0);
    lcd_putstr("Stopped");
    os_semaphore_signal(&lcd_sem);
    while (1) {
        os_semaphore_wait(&btn_sem);
        if (!(PINA & 0b00010000)) {
            start = 1;
        } else {
            start = 0;
        }
        if (!(PINA & 0b00100000)) {
            pause = 1;
        } else {
            pause = 0;
        }
        if (!(PINA & 0b01000000)) {
            stop = 1;
        } else {
            stop = 0;
        }
        os_semaphore_signal(&btn_sem);
        
        if (state == 1 && pause == 1) {
            os_semaphore_wait(&btn_sem);
            pause = 0;
            os_semaphore_signal(&btn_sem);
            os_semaphore_wait(&stt_sem);
            state = 2;
            os_semaphore_signal(&stt_sem);
            os_semaphore_wait(&lcd_sem);
            lcd_set_cursor(4, 0);
            lcd_putstr("Paused ");
            os_semaphore_signal(&lcd_sem);
        }
        
        if (start) {
            os_semaphore_wait(&btn_sem);
            start = 0;
            os_semaphore_signal(&btn_sem);
            os_semaphore_wait(&stt_sem);
            state = 1;
            os_semaphore_signal(&stt_sem);
            os_semaphore_wait(&lcd_sem);
            lcd_set_cursor(4, 0);
            lcd_putstr("Running");
            os_semaphore_signal(&lcd_sem);
        }
        
        if (stop) {
            os_semaphore_wait(&btn_sem);
            stop = 0;
            os_semaphore_signal(&btn_sem);
            os_semaphore_wait(&stt_sem);
            state = 0;
            os_semaphore_signal(&stt_sem);
            os_semaphore_wait(&lcd_sem);
            lcd_set_cursor(4, 0);
            lcd_putstr("Stopped");
            os_semaphore_signal(&lcd_sem);
            os_semaphore_wait(&tck_sem);
            ticks = 0;
            os_semaphore_signal(&tck_sem);
        }
        
        os_delay(os_get_current_pid(), 50);
    }
}

int main(void) {
    os_init();
    os_semaphore_init(&lcd_sem, 1);
    os_semaphore_init(&btn_sem, 1);
    os_semaphore_init(&stt_sem, 1);
    os_semaphore_init(&tck_sem, 1);
    os_add_task(uart_task, &uart_task_stack[STACK_SIZE + 64], 1, "uart");
    os_add_task(adc_task, &adc_task_stack[STACK_SIZE + 64], 0, "adc");
    os_add_task(button_task, &button_task_stack[STACK_SIZE + 64], 2, "btn");
    os_start_ticker();
    
    os_semaphore_wait(&lcd_sem);
    lcd_init();
    os_semaphore_signal(&lcd_sem);
    
    i2c_init();
    
    usart_init(USART_TRANSMIT | USART_RECEIVE);
    usart_putc('\f');
    
    while(1) {
        char buff[6];
        os_delay(os_get_current_pid(), 2000);
        /*usart_puts("Start I2C\r\n");
        int8_t start_ = i2c_start();
        int8_t send_ = i2c_send_address(0xa0);
        i2c_stop();
        if (start_ != 0) {
            usart_puts("Start error\r\n");
            uint8_t_to_ascii((uint8_t) start_, &(buff[0]));
            usart_puts(buff);
            usart_puts("\r\n");
            
        }
        if (send_ != 0) {
            usart_puts("Send error\r\n");
            uint8_t_to_ascii((uint8_t) send_, &(buff[0]));
            usart_puts(buff);
            usart_puts("\r\n");
        }
        usart_puts("Stop I2C\r\n\r\n");*/
    }
    
    return 0;
}
