/**
 * @file main.c
 * @author guido
 * @date 2025-02-09
 * @brief Main function
 */

/* -------------------------------------------------------------------------- */
#include "Drivers/clock/clock.h"
#include "Drivers/gpio/gpio.h"
#include "config.h"
/* -------------------------------------------------------------------------- */

#define _XTAL_FREQ 4e6

#define LED_ROJO_1 GPIO_3
#define LED_ROJO_2 GPIO_4

int main() {

    clk_use_external();

    gpio_config(GPIO_PORTB, GPIO_5, GPIO_OUTPUT_HIGH);
    gpio_config(GPIO_PORTB, GPIO_4, GPIO_OUTPUT_LOW);
    gpio_config(GPIO_PORTB, GPIO_3, GPIO_OUTPUT_HIGH);
    gpio_config(GPIO_PORTB, GPIO_2, GPIO_OUTPUT_LOW);

    gpio_write_pin(GPIO_PORTB, GPIO_4, GPIO_HIGH);
    gpio_write_pin(GPIO_PORTB, GPIO_3, GPIO_LOW);

    while (1) {
        gpio_write_pin(GPIO_PORTB, GPIO_4, !gpio_read_pin(GPIO_PORTB, GPIO_4, NULL));
        __delay_ms(500);
        gpio_write_pin(GPIO_PORTB, GPIO_4, !gpio_read_pin(GPIO_PORTB, GPIO_4, NULL));
        __delay_ms(500);
    }

    return 0;
}