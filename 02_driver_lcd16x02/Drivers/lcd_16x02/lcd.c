
#include "lcd.h"
#include <builtins.h>
#include <stdbool.h>
#include <stdint.h>
#include <xc.h>

#define LCD_DDRAM_ADDRESS_ROW_0 0x00U
#define LCD_DDRAM_ADDRESS_ROW_1 0x40U

typedef enum {
    LCD_CMD_CLEAR_DISPLAY     = (1 << DB0),
    LCD_CMD_RETURN_HOME       = (1 << DB1),
    LCD_CMD_ENTRY_MODE_SET    = (1 << DB2),
    LCD_CMD_DISPLAY_CTRL      = (1 << DB3),
    LCD_CMD_CURSOR_SHIFT      = (1 << DB4),
    LCD_CMD_FUNCTION_SET      = (1 << DB5),
    LCD_CMD_SET_CGRAM_ADDRESS = (1 << DB6),
    LCD_CMD_SET_DDRAM_ADDRESS = (1 << DB7),
} lcd_cmd_t;

typedef enum {
    LCD_POWER_OFF = (0 << DB2),
    LCD_POWER_ON  = (1 << DB2),
} lcd_power_t;

typedef enum {
    LCD_CURSOR_VISUAL_MOVE_LEFT   = (0 << DB3 | 0 << DB2),
    LCD_CURSOR_VISUAL_MOVE_RIGHT  = (0 << DB3 | 1 << DB2),
    LCD_DISPLAY_VISUAL_MOVE_LEFT  = (1 << DB3 | 0 << DB2),
    LCD_DISPLAY_VISUAL_MOVE_RIGHT = (1 << DB3 | 1 << DB2),
} lcd_visual_movement_t;

typedef enum {
    LCD_CURSOR_MOVE_LEFT   = (0 << DB1 | 0 << DB0),
    LCD_CURSOR_MOVE_RIGHT  = (1 << DB1 | 0 << DB0),
    LCD_DISPLAY_MOVE_LEFT  = (1 << DB1 | 1 << DB0),
    LCD_DISPLAY_MOVE_RIGHT = (0 << DB1 | 1 << DB0),
} lcd_action_movement_t;

static lcd_retval_t lcd_gpio_config(lcd_gpio_t *gpio, lcd_mode_t bus_size) {

    if (gpio == NULL || gpio->EN == NULL || gpio->RS == NULL) return LCD_FAILURE;

    if (gpio_config(gpio->EN->port, gpio->EN->pin, GPIO_OUTPUT_LOW) == GPIO_FAILURE) return LCD_EN_PIN_ERROR;
    if (gpio_config(gpio->RS->port, gpio->RS->pin, GPIO_OUTPUT_LOW) == GPIO_FAILURE) return LCD_RS_PIN_ERROR;

    // The RW pin would not be necessary if RW is tied to GND for only writing
    if (gpio->RW != NULL && gpio_config(gpio->RW->port, gpio->RW->pin, GPIO_OUTPUT_LOW) != GPIO_FAILURE) return LCD_RW_PIN_ERROR;

    lcd_data_pin_t offset = bus_size == LCD_4_BIT_MODE ? DB4 : DB0;

    for (uint8_t i = offset; i <= DB7; i++)
        if (gpio->DB[i] != NULL && gpio_config(gpio->DB[i]->port, gpio->DB[i]->pin, GPIO_OUTPUT_LOW) == GPIO_FAILURE) return LCD_DB_PIN_ERROR;

    return LCD_SUCCESS;
}

static void lcd_cmd_enable_pulse(lcd_init_t *lcd) {
    gpio_write_pin(lcd->gpio.EN->port, lcd->gpio.EN->pin, GPIO_HIGH);
    __delay_us(1);
    gpio_write_pin(lcd->gpio.EN->port, lcd->gpio.EN->pin, GPIO_LOW);
}

static void lcd_gpio_write(lcd_init_t *lcd, uint8_t data) {

    if (lcd->gpio.RW) gpio_write_pin(lcd->gpio.RW->port, lcd->gpio.RW->pin, GPIO_LOW);
    gpio_write_pin(lcd->gpio.EN->port, lcd->gpio.EN->pin, GPIO_LOW);

    lcd_data_pin_t offset = lcd->bus_size == LCD_4_BIT_MODE ? DB4 : DB0;

    // Envio los 4 bits mas significativos
    for (uint8_t i = offset; i <= DB7; i++)
        gpio_write_pin(lcd->gpio.DB[i]->port, lcd->gpio.DB[i]->pin, (data >> (i - offset)) & 0x01);

    lcd_cmd_enable_pulse(lcd);
}

static void lcd_cmd_write_init(lcd_init_t *lcd, uint8_t cmd) {
    gpio_write_pin(lcd->gpio.RS->port, lcd->gpio.RS->pin, GPIO_LOW);
    if (lcd->bus_size == LCD_4_BIT_MODE)
        lcd_gpio_write(lcd, cmd >> 4);
    else
        lcd_gpio_write(lcd, cmd);
}

static void lcd_write(lcd_init_t *lcd, uint8_t cmd) {
    gpio_write_pin(lcd->gpio.RS->port, lcd->gpio.RS->pin, GPIO_LOW);
    if (lcd->bus_size == LCD_4_BIT_MODE)
        lcd_gpio_write(lcd, cmd >> 4);
    lcd_gpio_write(lcd, cmd);
}

static void lcd_data_write(lcd_init_t *lcd, uint8_t data) {
    gpio_write_pin(lcd->gpio.RS->port, lcd->gpio.RS->pin, GPIO_HIGH);
    if (lcd->bus_size == LCD_4_BIT_MODE)
        lcd_gpio_write(lcd, data >> 4);
    lcd_gpio_write(lcd, data);
}

static void lcd_start_sequence(lcd_init_t *lcd) {

    // LCD Start-up
    __delay_ms(50);    // Wait for PSU stabilization
    lcd_cmd_write_init(lcd, LCD_CMD_FUNCTION_SET | LCD_8_BIT_MODE);
    __delay_ms(5);
    lcd_cmd_write_init(lcd, LCD_CMD_FUNCTION_SET | LCD_8_BIT_MODE);
    __delay_us(150);
    lcd_cmd_write_init(lcd, LCD_CMD_FUNCTION_SET | LCD_8_BIT_MODE);

    if (lcd->bus_size == LCD_4_BIT_MODE)
        lcd_cmd_write_init(lcd, (uint8_t)(LCD_CMD_FUNCTION_SET | lcd->bus_size));

    lcd_write(lcd, (uint8_t)(LCD_CMD_FUNCTION_SET | lcd->bus_size | lcd->n_lines | lcd->display_font));
    lcd_write(lcd, LCD_CMD_DISPLAY_CTRL | LCD_POWER_OFF);
    lcd_write(lcd, LCD_CMD_CLEAR_DISPLAY);
    __delay_ms(3);
    lcd_write(lcd, LCD_CMD_ENTRY_MODE_SET | LCD_CURSOR_MOVE_RIGHT);
    lcd_write(lcd, LCD_CMD_DISPLAY_CTRL | LCD_POWER_ON);
}

lcd_retval_t lcd_set_cursor(lcd_init_t *lcd, uint8_t x, uint8_t y) {
    if (x >= LCD_COLUMN_LIMIT || y >= LCD_ROW_LIMIT) return LCD_FAILURE;

    lcd_write(lcd, LCD_CMD_SET_DDRAM_ADDRESS | ((y == 0 ? LCD_DDRAM_ADDRESS_ROW_0 : LCD_DDRAM_ADDRESS_ROW_1) + x));

    return LCD_SUCCESS;
}

void lcd_write_char(lcd_init_t *lcd, char c) {
    lcd_data_write(lcd, c);
    __delay_us(50);
}

void lcd_write_string(lcd_init_t *lcd, char *str) {
    while (*str) lcd_write_char(lcd, *str++);
}

lcd_retval_t lcd_config(lcd_init_t *lcd) {

    if (lcd == NULL) return LCD_FAILURE;

    lcd_retval_t err = lcd_gpio_config(&lcd->gpio, lcd->bus_size);

    if (err != LCD_SUCCESS) return err;

    // Start-up sequence
    // https://cdn.sparkfun.com/assets/9/5/f/7/b/HD44780.pdf
    lcd_start_sequence(lcd);
    lcd_write_string(lcd, lcd->initial_msg);

    return LCD_SUCCESS;
}

void lcd_clear(lcd_init_t *lcd) {
    lcd_write(lcd, LCD_CMD_CLEAR_DISPLAY);
    __delay_ms(3);
}
