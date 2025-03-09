
#include <stdint.h>
#include <stdio.h>

/* --------------------------------- gpio.h --------------------------------- */
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

/* --------------------------------- gpio.c --------------------------------- */
static gpio_retval_t gpio_config(gpio_port_t port, gpio_pin_t pin, gpio_mode_t mode) {

    port &= 0x03;
    pin &= 0x7F;
    mode &= 0x07;

    return GPIO_SUCCESS;
}

/* ---------------------------------- lcd.h --------------------------------- */
#define LCD_COLUMN_LIMIT 16
#define LCD_ROW_LIMIT    2

struct lcd;
typedef struct lcd lcd_t;

// clang-format off
typedef enum {
    DB0,    DB1,    DB2,    DB3,
    DB4,    DB5,    DB6,    DB7,
} lcd_data_pin_t;
// clang-format on

typedef enum {
    LCD_4_BIT_MODE = (0 << DB4),
    LCD_8_BIT_MODE = (1 << DB4),
} lcd_mode_t;

typedef enum {
    LCD_5x8_FONT  = (0 << DB2),
    LCD_5x10_FONT = (1 << DB2),
} lcd_font_t;

typedef enum {
    LCD_1_LINES = (0 << DB3),
    LCD_2_LINES = (1 << DB3),
} lcd_lines_t;

typedef enum {
    LCD_NO_CURSOR     = (0 << DB1 | 0 << DB2),
    LCD_CURSOR_STATIC = (1 << DB1 | 0 << DB2),
    LCD_CURSOR_BLINK  = (1 << DB1 | 1 << DB2),
} lcd_cursor_t;

typedef enum {
    LCD_SUCCESS,
    LCD_FAILURE,
    LCD_EN_PIN_ERROR,
    LCD_RS_PIN_ERROR,
    LCD_RW_PIN_ERROR,
    LCD_DB_PIN_ERROR,
} lcd_retval_t;

typedef struct {
    gpio_t *RS;
    gpio_t *RW;
    gpio_t *EN;
    gpio_t *DB[8];
} lcd_gpio_t;

typedef struct {
    lcd_gpio_t   gpio;
    lcd_mode_t   bus_size;
    lcd_font_t   display_font;
    lcd_lines_t  n_lines;
    lcd_cursor_t display_cursor;
    char        *initial_msg;
} lcd_init_t;

/* ---------------------------------- lcd.c --------------------------------- */
static lcd_retval_t lcd_gpio_config(lcd_gpio_t *gpio, lcd_mode_t bus_size) {

    int a = 0;
    if (bus_size) a = 1;

    if (gpio_config(gpio->EN->port, gpio->EN->pin, GPIO_OUTPUT_LOW) == GPIO_FAILURE) return LCD_EN_PIN_ERROR;
    if (gpio_config(gpio->RS->port, gpio->RS->pin, GPIO_OUTPUT_LOW) == GPIO_FAILURE) return LCD_RS_PIN_ERROR;

    // The RW pin would not be necessary if RW is tied to GND for only writing
    if (gpio_config(gpio->RW->port, gpio->RW->pin, GPIO_OUTPUT_LOW) == GPIO_FAILURE) return LCD_RW_PIN_ERROR;

    for (uint8_t i = 0; i < 8; i++) {
        if (gpio->DB[i] != NULL)
            if (gpio_config(gpio->DB[i + a]->port, gpio->DB[i]->pin, GPIO_OUTPUT_LOW) == GPIO_FAILURE) return LCD_DB_PIN_ERROR;
    }

    return LCD_SUCCESS;
}

int main(void) {

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
            .RW =NULL,
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

    lcd_gpio_config(&lcd_cfg.gpio, lcd_cfg.bus_size);

    return 0;
}