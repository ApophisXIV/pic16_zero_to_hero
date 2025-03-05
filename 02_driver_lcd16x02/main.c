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
        .n_lines = LCD_2_LINES,
        .display_cursor = LCD_CURSOR_BLINK,
        .initial_msg    = "Bienvenido Guido",
        .gpio           = {
            .EN = &(gpio_t) { .pin =GPIO_4, .port = GPIO_PORTB, },
            .RS = &(gpio_t) { .pin =GPIO_5, .port = GPIO_PORTB, },
            .RW = &(gpio_t) { .pin =GPIO_7, .port = GPIO_PORTC, },
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

    lcd_config(&lcd_cfg);
    __delay_ms(2000);
    gpio_config(GPIO_PORTC, GPIO_6, GPIO_OUTPUT);

    int i = 0;
    lcd_write_char(&lcd_cfg,'G');

    while (1) {
        // lcd_set_cursor(&lcd_cfg, 0, 0);
        // lcd_write_string(&lcd_cfg, int_to_string(i++));
        gpio_toggle_pin(GPIO_PORTC, GPIO_6);
        __delay_ms(300);
    }

    return 0;
}

// ch1 RB4 -> EN
// ch3 RB5 -> RS
// ch2 RB1 -> DB4
// ch4 RB0 -> DB5
// ch6 RC5 -> DB6
// ch8 RC4 -> DB7

// primer cmd -> 32 + 16 + 8 = 56 -> 0011 1000
// segundo cmd -> 32 + 16 + 8 = 56-> 0011 1000
// tercer cmd ->9 + 6 = 15 -> 0000 1111
// cuarta cmd -> 1 -> 0000 0001

// cmd 5 -> 4 + 2 = 6 -> 0000 0110

// int main(void) {

//     clk_use_internal(F_8MHZ);

//     // Configuración de pines (simulación con punteros)
//     lcd_gpio_pin_t rs = {GPIO_PORTB, GPIO_5};    // RS en PORTA, PIN1
//     lcd_gpio_pin_t en = {GPIO_PORTB, GPIO_4};    // EN en PORTA, PIN3

//     lcd_gpio_pin_t d4 = {GPIO_PORTB, GPIO_1};    // DB4 en PORTA, PIN2
//     lcd_gpio_pin_t d5 = {GPIO_PORTB, GPIO_0};    // DB5 en PORTA, PIN3
//     lcd_gpio_pin_t d6 = {GPIO_PORTC, GPIO_5};    // DB6 en PORTA, PIN4
//     lcd_gpio_pin_t d7 = {GPIO_PORTC, GPIO_4};    // DB7 en PORTA, PIN5

//     // clang-format off
//     lcd_t lcd = {
//         .pins = {
//             .rs = rs, .en = en,
//             .d4 = d4, .d5 = d5, .d6 = d6, .d7 = d7,
//         },
//     };
//     // clang-format on

//     // Inicializar LCD
//     lcd_init(&lcd);

//     // Escribir mensaje
//     lcd_set_cursor(&lcd, 0, 0);
//     lcd_write_string(&lcd, "Bienvenido!");

//     // Mover cursor y escribir en segunda línea
//     lcd_set_cursor(&lcd, 0, 1);
//     lcd_write_string(&lcd, "LCD 16x2 OK!");

//     while (1) {
//         // Esperar 1 segundo
//         __delay_ms(500);
//         // Mover cursor y escribir en segunda línea
//         lcd_clear(&lcd);
//         lcd_set_cursor(&lcd, 0, 1);
//         lcd_write_string(&lcd, "                ");

//         __delay_ms(500);
//         lcd_clear(&lcd);
//         lcd_set_cursor(&lcd, 0, 0);
//         lcd_write_string(&lcd, "Bienvenido!");
//         // Esperar 1 segundo (2 segundos_t *lcd);
//         __delay_ms(500);
//         lcd_clear(&lcd);
//         lcd_set_cursor(&lcd, 0, 1);
//         lcd_write_string(&lcd, "LCD 16x2 OK!");
//     }
// }