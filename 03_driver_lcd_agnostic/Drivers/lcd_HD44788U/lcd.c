/**
 * @file lcd.c
 * @author Guido Rodriguez (guerodriguez@fi.uba.ar)
 * @author Karla Duque (kduque@fi.uba.ar)
 * @brief LCD driver for HD44788 chip
 * @version 0.1
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025. All rights reserved.
 * 
 * Licensed under the MIT License, see LICENSE for details.
 * SPDX-License-Identifier: MIT
 * 
 */

/* ---------------------------- Private includes ---------------------------- */
#include "lcd.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* ----------------------------- Private macros ----------------------------- */
#define WRITE_CMD  0
#define WRITE_DATA 1

#define LCD_DDRAM_ADDRESS_ROW_0 0x00U
#define LCD_DDRAM_ADDRESS_ROW_1 0x40U
// #define LCD_DDRAM_ADDRESS_ROW_2 0x14U    // TODO - Check address in specific datasheet
// #define LCD_DDRAM_ADDRESS_ROW_3 0x54U    // TODO - Check address in specific datasheet

#define ASSERT_HAL_PARAMETER(x) \
    if (x.delay_ms == NULL || x.write_pin == NULL || x.init_pin == NULL) return LCD_HAL_ERROR;
#define ASSERT_GPIO(x, lcd_mode)                                                \
    do {                                                                        \
        if (x.RS == NULL) return LCD_RS_PIN_ERROR;                              \
        if (x.EN == NULL) return LCD_EN_PIN_ERROR;                              \
        for (uint8_t i = lcd_mode == LCD_4_BIT_MODE ? DB4 : DB0; i <= DB7; i++) \
            if (x.DB[i] == NULL) return LCD_DB_PIN_ERROR;                       \
    } while (0);


/* ---------------------------- Private datatypes --------------------------- */
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
    LCD_NO_CURSOR     = (0 << DB1 | 0 << DB2),
    LCD_CURSOR_STATIC = (1 << DB1 | 0 << DB2),
    LCD_CURSOR_BLINK  = (1 << DB1 | 1 << DB2),
} lcd_cursor_t;

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

/* ---------------------------- Private globals --------------------------- */
static bool lcd_use_double_enable_pulse = false;

/* ---------------------------- Private functions --------------------------- */

/**
 * @brief  Configures the GPIO pins for LCD communication
 * @pre    - The GPIO provided is valid
 * @post   - Control and data pins are set as outputs
 * @param  *lcd: Valid pointer to the LCD structure
 */
static void lcd_gpio_config(lcd_t *lcd) {

    lcd->hal.init_pin(lcd->gpio.EN, LCD_GPIO_OUTPUT);
    lcd->hal.write_pin(lcd->gpio.EN, LCD_GPIO_LOW);

    lcd->hal.init_pin(lcd->gpio.RS, LCD_GPIO_OUTPUT);
    if (lcd->gpio.RW != NULL)
        lcd->hal.init_pin(lcd->gpio.RW, LCD_GPIO_OUTPUT);

    uint8_t pin = lcd->mode == LCD_8_BIT_MODE ? DB0 : DB4;
    for (; pin <= DB7; pin++) lcd->hal.init_pin(lcd->gpio.DB[pin], LCD_GPIO_OUTPUT);
}

/**
 * @brief  Generates a pulse on the EN pin
 * @pre    - lcd_gpio_config was called before
 *         - The EN pin must be initialized by lcd_gpio_config 
 * @post   - A pulse of at least 1ms is generated on the EN pin
 * @param  *lcd: Valid pointer to the LCD structure.
 */
static void lcd_enable_pulse(lcd_t *lcd) {
    lcd->hal.write_pin(lcd->gpio.EN, LCD_GPIO_HIGH);
    lcd->hal.delay_ms(1);
    lcd->hal.write_pin(lcd->gpio.EN, LCD_GPIO_LOW);
    lcd->hal.delay_ms(1);
}

/**
 * @brief  Writes the bits of data to the LCD through GPIO
 * @pre    - The data pins must be properly initialized by lcd_gpio_config
 * @post   - If LCD is in 4 bit mode, the most significant 4 bits are transmitted followed by a pulse on EN
 *           then, the less significant 4 bits are transmitted
 * @param  *lcd: Valid pointer to the LCD structure.
 * @param  data: Data to send.
 */
static void lcd_gpio_write(lcd_t *lcd, uint8_t data) {
    
    if (lcd->gpio.RW != NULL) lcd->hal.write_pin(lcd->gpio.RW, LCD_GPIO_LOW);
    
    lcd->hal.write_pin(lcd->gpio.EN, LCD_GPIO_LOW);
    
    uint8_t offset = lcd->mode == LCD_4_BIT_MODE ? DB4 : DB0;

    // Envio los 4 bits mas significativos
    for (uint8_t i = offset; i <= DB7; i++)
    lcd->hal.write_pin(lcd->gpio.DB[i], (data >> (i - offset)) & 0x01);
    
    lcd_enable_pulse(lcd);
}

/**
 * @brief  Sends a command or data to the LCD
 * @note   Using the global flag lcd_use_double_enable_pulse the function 
 *         will know if treats of initialization sequence or "normal" one
 * @pre    - The data pins must be properly initialized by lcd_gpio_config
 * @post   - The specified value is transmitted using the proper sequence
 * @param  *lcd: Valid pointer to the LCD structure.
 * @param  is_data: Set to true to send data or false to send a command
 * @param  data: The value to be transmitted
 */
static void lcd_write(lcd_t *lcd, bool is_data, uint8_t data) {
    lcd->hal.write_pin(lcd->gpio.RS, is_data);

    if (lcd->mode == LCD_4_BIT_MODE)
        lcd_gpio_write(lcd, data >> 4);

    if (lcd->mode == LCD_8_BIT_MODE || lcd_use_double_enable_pulse)
        lcd_gpio_write(lcd, data);
}

/**
 * @brief   LCD initialization sequence
 * @pre     - The lcd_config function must be called before this function.
 * @post    - The LCD is initialized and ready to receive commands or data.
 * @param   *lcd: Valid pointer to the LCD structure.
 */
static void lcd_start_sequence(lcd_t *lcd) {

    lcd->hal.delay_ms(50);    // Wait for PSU stabilization
    lcd_write(lcd, WRITE_CMD, LCD_CMD_FUNCTION_SET | LCD_8_BIT_MODE);
    lcd->hal.delay_ms(5);    // More than 4.1 ms
    lcd_write(lcd, WRITE_CMD, LCD_CMD_FUNCTION_SET | LCD_8_BIT_MODE);
    lcd->hal.delay_ms(1);    // More than 100 us
    lcd_write(lcd, WRITE_CMD, LCD_CMD_FUNCTION_SET | LCD_8_BIT_MODE);

    if (lcd->mode == LCD_4_BIT_MODE) {
        lcd_write(lcd, WRITE_CMD, LCD_CMD_FUNCTION_SET | LCD_4_BIT_MODE);
        lcd_use_double_enable_pulse = true;
    }

    lcd_write(lcd, WRITE_CMD, (uint8_t)(LCD_CMD_FUNCTION_SET | lcd->mode | lcd->lines | lcd->font));
    lcd_write(lcd, WRITE_CMD, LCD_CMD_DISPLAY_CTRL | LCD_POWER_OFF);

    lcd_write(lcd, WRITE_CMD, LCD_CMD_CLEAR_DISPLAY);
    lcd->hal.delay_ms(3);

    lcd_write(lcd, WRITE_CMD, LCD_CMD_ENTRY_MODE_SET | LCD_CURSOR_MOVE_RIGHT);
    lcd_write(lcd, WRITE_CMD, LCD_CMD_DISPLAY_CTRL | LCD_POWER_ON);
}

/* -------------------- Private prototypes implementation ------------------- */
void lcd_write_char(lcd_t *lcd, char c) {
    lcd_write(lcd, WRITE_DATA, c);
    lcd->hal.delay_ms(1);
}

void lcd_write_string(lcd_t *lcd, char *str) {
    while (*str) lcd_write_char(lcd, *str++);
}

lcd_retval_t lcd_set_cursor(lcd_t *lcd, uint8_t x, uint8_t y) {

    if (x >= lcd->cols || y >= lcd->rows) return LCD_ROW_COLS_ERROR;

    uint8_t address = x;

    switch (y) {
    case 0:  address += LCD_DDRAM_ADDRESS_ROW_0; break;
    case 1:  address += LCD_DDRAM_ADDRESS_ROW_1; break;
    case 2:  address += LCD_DDRAM_ADDRESS_ROW_2; break;
    case 3:  address += LCD_DDRAM_ADDRESS_ROW_3; break;
    default: return LCD_FAILURE;
    }

    lcd_write(lcd, WRITE_CMD, LCD_CMD_SET_DDRAM_ADDRESS | address);

    return LCD_SUCCESS;
}

lcd_retval_t lcd_config(lcd_t *lcd) {

    if (lcd == NULL) return LCD_FAILURE;

    ASSERT_HAL_PARAMETER(lcd->hal);
    ASSERT_GPIO(lcd->gpio, lcd->mode);

    if (lcd->rows == 0 || lcd->cols == 0) return LCD_ROW_COLS_ERROR;

    lcd_gpio_config(lcd);

    lcd_start_sequence(lcd);    // https://cdn.sparkfun.com/assets/9/5/f/7/b/HD44780.pdf

    lcd_write_string(lcd, lcd->initial_msg);

    return LCD_SUCCESS;
}

void lcd_clear(lcd_t *lcd) {
    lcd_write(lcd, WRITE_CMD, LCD_CMD_CLEAR_DISPLAY);
    lcd->hal.delay_ms(4);
}