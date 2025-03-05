#include "gpio.h"
#include <xc.h>

static volatile uint8_t *port_reg[] = {(uint8_t *)(&PORTA), (uint8_t *)(&PORTB), (uint8_t *)(&PORTC)};
static volatile uint8_t *tris_reg[] = {(uint8_t *)(&TRISA), (uint8_t *)(&TRISB), (uint8_t *)(&TRISC)};

static uint8_t port_status[] = {[GPIO_PORTA] = 0, [GPIO_PORTB] = 0, [GPIO_PORTC] = 0};

gpio_retval_t gpio_config(gpio_port_t port, uint8_t pin, gpio_mode_t mode) {

    if (port > GPIO_PORTC) return GPIO_FAILURE;

    uint8_t tris_value = *tris_reg[port];

    if (mode == GPIO_INPUT || mode == GPIO_INPUT_PULLUP)
        tris_value |= pin;
    else
        tris_value &= ~pin;

    if (mode == GPIO_OUTPUT_LOW || mode == GPIO_OUTPUT_HIGH)
        gpio_write_pin(port, pin, mode == GPIO_OUTPUT_HIGH);

    *tris_reg[port] = tris_value;

    return GPIO_SUCCESS;
}

gpio_retval_t gpio_write_pin(gpio_port_t port, uint8_t pin, gpio_pin_state_t state) {

    if (state == GPIO_HIGH)
        port_status[port] |= pin;
    else
        port_status[port] &= ~pin;

    *port_reg[port] = port_status[port];

    return GPIO_SUCCESS;
}

gpio_retval_t gpio_toggle_pin(gpio_port_t port, uint8_t pin) {
    port_status[port] ^= pin;
    *port_reg[port] = port_status[port];
    return GPIO_SUCCESS;
}

bool gpio_read_pin(gpio_port_t port, uint8_t pin, gpio_retval_t *err_val) {
    return *port_reg[port] & pin;
}
