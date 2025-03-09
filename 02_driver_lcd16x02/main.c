/**
 * @file main.c
 * @author guido
 * @date 2025-02-09
 * @brief Main function
 */

/* -------------------------------------------------------------------------- */
#include "Drivers/clock/clock.h"
#include "Drivers/gpio/gpio.h"
#include "Drivers/lcd_16x02/lcd.h"
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

int main() {

    clk_use_internal(F_8MHZ);

    // clang-format off
    lcd_init_t lcd_cfg = {
        .bus_size  = LCD_4_BIT_MODE,
        .display_font   = LCD_5x8_FONT,
        // .display_font   = LCD_5x10_FONT,
        .n_lines = LCD_2_LINES,
        .display_cursor = LCD_CURSOR_BLINK,
        .initial_msg    = "Bienvenido Guido",
        .gpio           = {
            .EN = &(gpio_t) { .pin =GPIO_4, .port = GPIO_PORTB, },
            .RS = &(gpio_t) { .pin =GPIO_5, .port = GPIO_PORTB, },
            .RW = NULL,
            // .RW = &(gpio_t) { .pin =GPIO_7, .port = GPIO_PORTC, },
            .DB = {
                [DB0] = NULL,[DB1] = NULL,[DB2] = NULL,[DB3] = NULL,
                [DB4]= &(gpio_t){.pin = GPIO_1, .port = GPIO_PORTB},
                [DB5]= &(gpio_t){.pin = GPIO_0, .port = GPIO_PORTB},
                [DB6]= &(gpio_t){.pin = GPIO_5, .port = GPIO_PORTC},
                [DB7]= &(gpio_t){.pin = GPIO_4, .port = GPIO_PORTC},
            },
            // .DB = {
            //     [DB0] = &(gpio_t){.pin = GPIO_3, .port = GPIO_PORTA},
            //     [DB1] = &(gpio_t){.pin = GPIO_2, .port = GPIO_PORTA},
            //     [DB2] = &(gpio_t){.pin = GPIO_1, .port = GPIO_PORTA},
            //     [DB3] = &(gpio_t){.pin = GPIO_0, .port = GPIO_PORTA},
            //     [DB4] = &(gpio_t){.pin = GPIO_1, .port = GPIO_PORTB},
            //     [DB5] = &(gpio_t){.pin = GPIO_0, .port = GPIO_PORTB},
            //     [DB6] = &(gpio_t){.pin = GPIO_5, .port = GPIO_PORTC},
            //     [DB7] = &(gpio_t){.pin = GPIO_4, .port = GPIO_PORTC},
            // },
        }
    };
    // clang-format on

    lcd_config(&lcd_cfg);
    __delay_ms(2000);
    gpio_config(GPIO_PORTC, GPIO_6, GPIO_OUTPUT);

    int i = 0;
    lcd_set_cursor(&lcd_cfg, 0, 1);
    lcd_write_string(&lcd_cfg, "saludos...");
    __delay_ms(2000);
    lcd_clear(&lcd_cfg);

    while (1) {
        lcd_set_cursor(&lcd_cfg, 0, 0);
        lcd_write_string(&lcd_cfg, int_to_string(i++));
        gpio_toggle_pin(GPIO_PORTC, GPIO_6);
        __delay_ms(300);
    }

    return 0;
}