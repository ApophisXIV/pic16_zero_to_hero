/**
 * @file main.c
 * @author guido
 * @date 2025-02-09
 * @brief Main function
 */

/* -------------------------------------------------------------------------- */
#include "Drivers/clock/clock.h"
#include "Drivers/gpio/gpio.h"
#include "Drivers/lcd_HD44788U/lcd.h"
#include "board.h"
#include "config_bits.h"
#include <stdio.h>
/* -------------------------------------------------------------------------- */

char *int_to_string(int value) {
    static char buffer[10];
    sprintf(buffer, "%d", value);
    buffer[9] = '\0';
    return buffer;
}

/* -------------------------------------------------------------------------- */
// LCD hal functions
void lcd_delay(uint8_t time_ms) {
    while (time_ms--) __delay_ms(1);
}

void lcd_gpio_init(void *gpio, uint8_t mode) {
    gpio_t _gpio = *(gpio_t *)gpio;
    gpio_config(_gpio.port, _gpio.pin, mode);
}

void lcd_gpio_write(void *gpio, uint8_t value) {
    gpio_t _gpio = *(gpio_t *)gpio;
    gpio_write_pin(_gpio.port, _gpio.pin, value);
}
/* -------------------------------------------------------------------------- */

int main() {

    clk_use_internal(F_8MHZ);

    // clang-format off
    lcd_t lcd = {
        .cols = 16, .rows = 2,
        .initial_msg = "Buen dia karla!",
        .lines = LCD_2_LINES, .mode = LCD_4_BIT_MODE, .font = LCD_5x8_FONT,
        .hal = {.delay_ms = lcd_delay, .init_pin = lcd_gpio_init, .write_pin = lcd_gpio_write},
        .gpio = {
            .EN = &(gpio_t) { .pin =GPIO_4, .port = GPIO_PORTB, },
            .RS = &(gpio_t) { .pin =GPIO_5, .port = GPIO_PORTB, },
            .RW = NULL,
            .DB = {
                [DB0] = NULL,[DB1] = NULL,[DB2] = NULL,[DB3] = NULL,
                [DB4]= &(gpio_t){.pin = GPIO_1, .port = GPIO_PORTB},
                [DB5]= &(gpio_t){.pin = GPIO_0, .port = GPIO_PORTB},
                [DB6]= &(gpio_t){.pin = GPIO_5, .port = GPIO_PORTC},
                [DB7]= &(gpio_t){.pin = GPIO_4, .port = GPIO_PORTC},
            },
        }
    };
    // clang-format on

    lcd_config(&lcd);
    __delay_ms(2000);
    gpio_config(GPIO_PORTC, GPIO_6, GPIO_OUTPUT);

    int i = 0;
    lcd_set_cursor(&lcd, 0, 1);
    lcd_write_string(&lcd, "y guido...");
    __delay_ms(2000);
    lcd_clear(&lcd);

    while (1) {
        lcd_set_cursor(&lcd, 0, 0);
        lcd_write_string(&lcd, int_to_string(i++));
        gpio_toggle_pin(GPIO_PORTC, GPIO_6);
        __delay_ms(300);
    }

    return 0;
}
