#include "gpio.h"
#include <stdbool.h>
#include <stdint.h>
#include <xc.h>

static gpio_retval_t get_pin_registers(gpio_port_t pin_port, uint8_t **port,
                                       uint8_t **tris) {
    switch (pin_port) {
    case GPIO_PORTA: {
        *port = (uint8_t *)(&PORTA);
        *tris = (uint8_t *)(&TRISA);
    } break;
    case GPIO_PORTB:
        *port = (uint8_t *)(&PORTB);
        *tris = (uint8_t *)(&TRISB);
        break;
    case GPIO_PORTC:
        *port = (uint8_t *)(&PORTC);
        *tris = (uint8_t *)(&TRISC);
        break;
    default:
        return GPIO_FAILURE;
    }
    return GPIO_SUCCESS;
}

static bool is_output(uint8_t *tris_reg, gpio_pin_t pin_msk) {
    return (*tris_reg & pin_msk) == 0;
}

gpio_retval_t gpio_config(gpio_t *gpio, gpio_mode_t mode) {

    if (gpio == NULL) return GPIO_FAILURE;

    uint8_t *port_aux, *tris_aux;

    if (get_pin_registers(gpio->port, &port_aux, &tris_aux) == GPIO_FAILURE)
        return GPIO_FAILURE;

    // Set OUTPUT or INPUT
    if (mode == GPIO_OUTPUT || mode == GPIO_OUTPUT_HIGH || mode == GPIO_OUTPUT_LOW)
        *tris_aux &= ~gpio->pin;
    else
        *tris_aux |= gpio->pin;

    // In case of OUTPUT will be setted the initial state if required
    if (mode == GPIO_OUTPUT_LOW)
        *port_aux &= ~gpio->pin;
    else if (mode == GPIO_OUTPUT_HIGH)
        *port_aux |= gpio->pin;

    return GPIO_SUCCESS;
}

gpio_retval_t gpio_write_pin(gpio_t *gpio, gpio_pin_t pin, gpio_pin_state_t state) {

    uint8_t *port_aux, *tris_aux;

    if (get_pin_registers(gpio->port, &port_aux, &tris_aux) == GPIO_FAILURE)
        return GPIO_FAILURE;

    if (!is_output(tris_aux, pin))
        return GPIO_FAILURE;

    if (state == GPIO_LOW)
        *port_aux &= ~pin;
    else
        *port_aux |= pin;

    return GPIO_SUCCESS;
}

bool gpio_read_pin(gpio_t *gpio, gpio_pin_t pin, gpio_retval_t *err_val) {

    uint8_t *port_aux, *tris_aux;
    if (get_pin_registers(gpio->port, &port_aux, &tris_aux) == GPIO_FAILURE)
        return 0;

    if (is_output(tris_aux, pin)) {
        *err_val = GPIO_FAILURE;
        return 0;
    }

    *err_val = GPIO_SUCCESS;
    return (*port_aux & pin) == 0;
}

gpio_retval_t gpio_toggle_pin(gpio_t *gpio, gpio_pin_t pin) {

    uint8_t *port_aux, *tris_aux;
    if (get_pin_registers(gpio->port, &port_aux, &tris_aux) == GPIO_FAILURE)
        return GPIO_FAILURE;

    if (!is_output(tris_aux, pin))
        return GPIO_FAILURE;

    *port_aux ^= pin;

    return GPIO_SUCCESS;
}

gpio_retval_t gpio_attach_interrupt(gpio_t *gpio, gpio_pin_t pin, callback_t fn) {

    uint8_t *port_aux, *tris_aux;
    if (get_pin_registers(gpio->port, &port_aux, &tris_aux) == GPIO_FAILURE)
        return GPIO_FAILURE;

    if (!is_output(tris_aux, pin))
        return GPIO_FAILURE;

    return GPIO_SUCCESS;
}
