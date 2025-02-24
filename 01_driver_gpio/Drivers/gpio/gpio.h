#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>

typedef enum {
    GPIO_SUCCESS,
    GPIO_FAILURE,
} gpio_retval_t;

typedef enum {
    GPIO_PORTA,
    GPIO_PORTB,
    GPIO_PORTC,
} gpio_port_t;

typedef enum {
    GPIO_OUTPUT,
    GPIO_OUTPUT_HIGH,
    GPIO_OUTPUT_LOW,
    GPIO_INPUT,
    GPIO_INPUT_PULLUP
} gpio_mode_t;

typedef enum {
    GPIO_LOW  = 0,
    GPIO_HIGH = 1,
} gpio_pin_state_t;

typedef enum {
    GPIO_0 = (1 << 0),
    GPIO_1 = (1 << 1),
    GPIO_2 = (1 << 2),
    GPIO_3 = (1 << 3),
    GPIO_4 = (1 << 4),
    GPIO_5 = (1 << 5),
    GPIO_6 = (1 << 6),
    GPIO_7 = (1 << 7),
} gpio_pin_t;

typedef struct {
    gpio_port_t port;
    gpio_pin_t  pin;
} gpio_t;

typedef void (*callback_t)(void);

gpio_retval_t gpio_config(gpio_t *gpio, gpio_mode_t mode);

gpio_retval_t gpio_write_pin(gpio_t *gpio, gpio_pin_t pin, gpio_pin_state_t state);

bool gpio_read_pin(gpio_t *gpio, gpio_pin_t pin, gpio_retval_t *err_val);

gpio_retval_t gpio_toggle_pin(gpio_t *gpio, gpio_pin_t pin);

gpio_retval_t gpio_attach_interrupt(gpio_t *gpio, gpio_pin_t pin, callback_t fn);

#endif    // GPIO_H