/**
 * @file lcd.h
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

#ifndef LCD_H
#define LCD_H

/* ----------------------------- Public includes ---------------------------- */
#include <stdint.h>

/* ------------------------------ Public macros ----------------------------- */
// Define with the equivalent value that represent an output in the system
#define LCD_GPIO_OUTPUT 0
#ifndef LCD_GPIO_OUTPUT
#error Check the lcd.h file and define LCD_GPIO_OUTPUT or uncomment the defined one
#endif

// Define with the equivalent electrical state that represent a low state in the system
#define LCD_GPIO_LOW 0
#ifndef LCD_GPIO_LOW
#error Check the lcd.h file and define LCD_GPIO_LOW or uncomment the defined one
#endif

// Define with the equivalent electrical state that represent a high state in the system
#define LCD_GPIO_HIGH 1
#ifndef LCD_GPIO_HIGH
#error Check the lcd.h file and define LCD_GPIO_HIGH or uncomment the defined one
#endif

/* ---------------------------- Public datatypes ---------------------------- */
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
    LCD_SUCCESS,
    LCD_FAILURE,
    LCD_HAL_ERROR,
    LCD_ROW_COLS_ERROR,
    LCD_EN_PIN_ERROR,
    LCD_RS_PIN_ERROR,
    LCD_RW_PIN_ERROR,
    LCD_DB_PIN_ERROR,
} lcd_retval_t;

typedef struct {
    void *RS;
    void *RW;
    void *EN;
    void *DB[8];
} lcd_gpio_t;

typedef struct {
    void (*init_pin)(void *gpio, uint8_t mode);
    void (*write_pin)(void *gpio, uint8_t state);
    void (*delay_ms)(uint8_t time);
} lcd_hal_t;

typedef struct {
    char       *initial_msg;
    uint8_t     rows, cols;
    lcd_mode_t  mode;
    lcd_font_t  font;
    lcd_lines_t lines;
    lcd_hal_t   hal;
    lcd_gpio_t  gpio;
} lcd_t;


/* ---------------------------- Public prototypes --------------------------- */
/**
 * @brief  Configures the LCD module with the specified parameters 
 * @pre    - The lcd_init_t structure must contain a valid configuration
 *         - The EN, RS, and RW pins (if used) must contain only one active bit (must be a power of 2) 
 *         - If operating in 4-bit mode, DB0 to DB3 should be NULL 
 *         - If operating in 8-bit mode, all DB0 to DB7 must be assigned valid pins 
 *         - The initial message pointer must not be NULL if a startup message is required but can be "" 
 * @post   - The LCD module will be initialized and ready for operation if the function returns success
 *         - The display will show the initial message if provided and valid
 * @param  *lcd: Valid pointer to the LCD initialization structure
 * @retval lcd_retval_t: Return code indicating the success or failure of the configuration related to GPIOs 
 *         - LCD_EN_PIN_ERROR: Error during the EN pin configuration 
 *         - LCD_RS_PIN_ERROR: Error during the RS pin configuration 
 *         - LCD_RW_PIN_ERROR: Error during the RW pin configuration 
 *         - LCD_DB_PIN_ERROR: Error during the DB0 to DB7 pin configuration 
 *         - LCD_ROW_COLS_ERROR: Error if the specified rows or columns exceed the display limits
 *         - LCD_HAL_ERROR: Error related to the HAL functions
 *         - LCD_SUCCESS: The LCD initialization structure is valid 
 */
lcd_retval_t lcd_config(lcd_t *lcd);

/**
 * @brief  Put the display on low-power consuption state
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The lcd is putted on low power consumption state preserving the initial configuration provided
 * @param  *lcd: Valid pointer to the LCD structure 
 */
void lcd_sleep(lcd_t *lcd);

/**
 * @brief  Wake up the display from low-power consuption state
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The lcd is waked up from low power consumption state allowing the normal operation
 * @param  *lcd: Valid pointer to the LCD structure 
 */
void lcd_wake_up(lcd_t *lcd);

/**
 * @brief  Clear the display
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The display is cleared
 * @param  *lcd: Valid pointer to the LCD structure 
 */
void lcd_clear(lcd_t *lcd);

/**
 * @brief  Set the cursor position relative to the first cell of the lcd display
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @param  *lcd: Valid pointer to the LCD structure 
 * @param  x: Column
 * @param  y: Row
 * @retval LCD_FAILURE in case that x or y exceed the display columns or rows
 */
lcd_retval_t lcd_set_cursor(lcd_t *lcd, uint8_t x, uint8_t y);

/**
 * @brief  Write a single character on the display
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The character is written on the display
 * @param  *lcd: Valid pointer to the LCD structure 
 * @param  c: Character to be written on the display 
 * @retval None
 */
void lcd_write_char(lcd_t *lcd, char c);

/**
 * @brief  Write a string on the display
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The string is written on the display from x=0, y=0 to x=15, y=1
 *         - If the string is longer than the display, the string is truncated
 * @param  *lcd: Valid pointer to the LCD structure 
 * @param  *str: Valid pointer to the string to be written on the display 
 */
void lcd_write_string(lcd_t *lcd, char *str);

/**
 * @brief  Turn off the cursor
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The cursor is turned off
 * @param  *lcd: Valid pointer to the LCD structure 
 */
void lcd_turn_off_cursor(lcd_t *lcd);

/**
 * @brief  Turn on the cursor
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The cursor is turned on
 * @param  *lcd: Valid pointer to the LCD structure 
 */
void lcd_turn_on_cursor(lcd_t *lcd);

/**
 * @brief  Turn off the cursor blinking and leave it static
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The cursor blinking state change to static
 * @param  *lcd: Valid pointer to the LCD structure 
 */
void lcd_turn_off_cursor_blink(lcd_t *lcd);

/**
 * @brief  Turn on the cursor blinking
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The cursor blinking state change to blinking
 * @param  *lcd: Valid pointer to the LCD structure 
 */
void lcd_turn_on_cursor_blink(lcd_t *lcd);

#endif    // LCD_H