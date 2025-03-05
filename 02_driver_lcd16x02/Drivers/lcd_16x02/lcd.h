#ifndef LCD_H
#define LCD_H

#include "../../board.h"
#include "../gpio/gpio.h"
#include <stdint.h>

#ifndef _XTAL_FREQ
#warning The _XTAL_FREQ must be defined before use the lcd module or 8MHz will be used
#endif

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
} lcd_bus_mode_t;

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
    lcd_gpio_t       gpio;
    lcd_bus_mode_t   bus_size;
    lcd_font_t       display_font;
    lcd_lines_t      n_lines;
    lcd_cursor_t     display_cursor;
    char            *initial_msg;
} lcd_init_t;

/**
 * @brief  Configures the LCD module with the specified parameters.
 * @note   The lcd_init_t structure must contain a valid configuration.
 *         This structure allows the software to manage the LCD state without direct hardware access.
 * @pre
 *         - The EN, RS, and RW pins (if used) must contain only one active bit (i.e., must be a power of 2).
 *         - If operating in 4-bit mode, DB0 to DB3 must be NULL.
 *         - If operating in 8-bit mode, all DB0 to DB7 must be assigned valid pins.
 *         - The initial message pointer must not be NULL if a startup message is required but can be "".
 * @post
 *         - The LCD module will be initialized and ready for operation if the function returns success.
 *         - The display will show the initial message if provided and valid.
 *         - The lcd structure will maintain relevant information for managing the LCD virtual state.
 * @param  *lcd_config: Pointer to the LCD initialization structure.
 * @retval lcd_retval_t: Return code indicating the success or failure of the configuration related to GPIOs.
 *         - LCD_EN_PIN_ERROR: Error during the EN pin configuration.
 *         - LCD_RS_PIN_ERROR: Error during the RS pin configuration.
 *         - LCD_RW_PIN_ERROR: Error during the RW pin configuration.
 *         - LCD_DB_PIN_ERROR: Error during the DB0 to DB7 pin configuration.
 *         - LCD_SUCCESS: The LCD initialization structure is valid.
 */
lcd_retval_t lcd_config(lcd_init_t *lcd_config);

/**
 * @brief  Put the display on low-power consuption state
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The lcd is putted on low power consumption state preserving the initial configuration provided
 * @param  *lcd: Pointer to the LCD structure.
 */
void lcd_sleep(lcd_init_t *lcd);

/**
 * @brief  Wake up the display from low-power consuption state
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The lcd is waked up from low power consumption state allowing the normal operation
 * @param  *lcd: Pointer to the LCD structure.
 */
void lcd_wake_up(lcd_init_t *lcd);

/**
 * @brief  Clear the display
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The display is cleared
 * @param  *lcd: Pointer to the LCD structure.
 */
void lcd_clear(lcd_init_t *lcd);

/**
 * @brief  Set the cursor position relative to the first cell of the lcd display
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @param  *lcd: Pointer to the LCD structure.
 * @param  x: Column
 * @param  y: Row
 * @retval LCD_FAILURE in case that x or y exceed the display columns or rows
 */
lcd_retval_t lcd_set_cursor(lcd_init_t *lcd, uint8_t x, uint8_t y);

/**
 * @brief  Write a single character on the display
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The character is written on the display
 * @param  *lcd: Pointer to the LCD structure.
 * @param  c: Character to be written on the display.
 * @retval None
 */
void lcd_write_char(lcd_init_t *lcd, char c);

/**
 * @brief  Write a string on the display
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The string is written on the display from x=0, y=0 to x=15, y=1
 *         - If the string is longer than the display, the string is truncated
 * @param  *lcd: Pointer to the LCD structure.
 * @param  *str: Pointer to the string to be written on the display.
 */
void lcd_write_string(lcd_init_t *lcd, char *str);

/**
 * @brief  Turn off the cursor
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The cursor is turned off
 * @param  *lcd: Pointer to the LCD structure.
 */
void lcd_turn_off_cursor(lcd_init_t *lcd);

/**
 * @brief  Turn on the cursor
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The cursor is turned on
 * @param  *lcd: Pointer to the LCD structure.
 */
void lcd_turn_on_cursor(lcd_init_t *lcd);

/**
 * @brief  Turn off the cursor blinking and leave it static
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The cursor blinking state change to static
 * @param  *lcd: Pointer to the LCD structure.
 */
void lcd_turn_off_cursor_blink(lcd_init_t *lcd);

/**
 * @brief  Turn on the cursor blinking
 * @pre    - The lcd is well initialized through the lcd_config function
 *         - The lcd_config function was called before
 * @post   - The cursor blinking state change to blinking
 * @param  *lcd: Pointer to the LCD structure.
 */
void lcd_turn_on_cursor_blink(lcd_init_t *lcd);

#endif    // LCD_H

// // lcd.h
// #ifndef LCD_H
// #define LCD_H

// #include "../gpio/gpio.h"
// #include <stdint.h>

// // Estructura para definir un pin con puerto y número de pin
// typedef struct {
//     gpio_port_t port;    // Puntero al puerto (abstracto para hacerlo agnóstico)
//     gpio_pin_t  pin;     // Número de pin en el puerto
// } lcd_gpio_pin_t;

// // Estructura de configuración del LCD
// typedef struct {
//     lcd_gpio_pin_t rs;
//     lcd_gpio_pin_t en;
//     lcd_gpio_pin_t d4;
//     lcd_gpio_pin_t d5;
//     lcd_gpio_pin_t d6;
//     lcd_gpio_pin_t d7;
// } lcd_pins_t;

// // Estructura principal del LCD
// typedef struct {
//     lcd_pins_t pins;
//     uint8_t    display_on;
//     uint8_t    cursor_on;
//     uint8_t    blink_on;
// } lcd_t;

// // Prototipos de funciones
// void lcd_init(lcd_t *lcd);
// void lcd_cmd(lcd_t *lcd, uint8_t cmd);
// void lcd_write_char(lcd_t *lcd, char c);
// void lcd_write_string(lcd_t *lcd, const char *str);
// void lcd_clear(lcd_t *lcd);
// void lcd_set_cursor(lcd_t *lcd, uint8_t x, uint8_t y);

// #endif    // LCD_H