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

    // clk_use_internal(F_8MHZ);
    clk_use_external();

    gpio_t leds = {
        .port = GPIO_PORTC,
        .pin  = GPIO_3 | GPIO_4,
    };

    // gpio_config(&leds, GPIO_OUTPUT);
    gpio_config(&leds, GPIO_OUTPUT_HIGH);

    while (1) {
        gpio_toggle_pin(&leds, LED_ROJO_1 | LED_ROJO_2);
        __delay_ms(500);
    }

    return 0;
}