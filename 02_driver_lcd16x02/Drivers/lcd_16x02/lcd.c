
#include "lcd.h"
#include <builtins.h>
#include <stdbool.h>
#include <stdint.h>
#include <xc.h>

#define LCD_BUS_SIZE 8U
#define LCD_OFFSET_4 4U
#define LCD_OFFSET_8 0U

#define LCD_TRANSFER_BYTE   1
#define LCD_TRANSFER_NIBBLE 2

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
    LCD_POWER_OFF = (0 << DB0),
    LCD_POWER_ON  = (1 << DB0),
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

static lcd_retval_t lcd_gpio_config(lcd_gpio_t *gpio, lcd_bus_mode_t bus_size) {

    if (gpio_config(gpio->EN->port, gpio->EN->pin, GPIO_OUTPUT_LOW) == GPIO_FAILURE) return LCD_EN_PIN_ERROR;
    if (gpio_config(gpio->RS->port, gpio->RS->pin, GPIO_OUTPUT_LOW) == GPIO_FAILURE) return LCD_RS_PIN_ERROR;

    // The RW pin would not be necessary if RW is tied to GND for only writing
    // if (gpio_config(gpio->RW->port, gpio->RW->pin, GPIO_OUTPUT_LOW) == GPIO_FAILURE) return LCD_RW_PIN_ERROR;

    for (uint8_t i = 0; i < LCD_BUS_SIZE; i++) {
        if (gpio->DB[i] != NULL)
            if (gpio_config(gpio->DB[i]->port, gpio->DB[i]->pin, GPIO_OUTPUT_LOW) == GPIO_FAILURE) return LCD_DB_PIN_ERROR;
    }

    return LCD_SUCCESS;
}

static void lcd_write_nibble(lcd_init_t *lcd, uint8_t data) {

    // gpio_write_pin(lcd->gpio.RW->port, lcd->gpio.RW->pin, GPIO_LOW);
    gpio_write_pin(lcd->gpio.EN->port, lcd->gpio.EN->pin, GPIO_LOW);

    // Envio los 4 bits mas significativos
    for (uint8_t i = 0; i < 4; i++) {
        gpio_write_pin(lcd->gpio.DB[i + DB4]->port, lcd->gpio.DB[i + DB4]->pin, (data >> i) & 0x01);
    }

    gpio_write_pin(lcd->gpio.EN->port, lcd->gpio.EN->pin, GPIO_HIGH);
    __delay_us(3);
    gpio_write_pin(lcd->gpio.EN->port, lcd->gpio.EN->pin, GPIO_LOW);
}

static void lcd_cmd_write(lcd_init_t *lcd, uint8_t cmd) {
    gpio_write_pin(lcd->gpio.RS->port, lcd->gpio.RS->pin, GPIO_LOW);
    lcd_write_nibble(lcd, cmd >> 4);
    lcd_write_nibble(lcd, cmd & 0x0F);
}

static void lcd_data_write(lcd_init_t *lcd, uint8_t data) {
    gpio_write_pin(lcd->gpio.RS->port, lcd->gpio.RS->pin, GPIO_HIGH);
    lcd_write_nibble(lcd, data >> 4);
    lcd_write_nibble(lcd, data & 0x0F);
}

static void lcd_start_sequence(lcd_init_t *lcd) {

    // LCD Start-up
    __delay_ms(20);    // Wait for PSU stabilization
    lcd_write_nibble(lcd, LCD_CMD_FUNCTION_SET | LCD_8_BIT_MODE);
    __delay_ms(5);
    lcd_write_nibble(lcd, LCD_CMD_FUNCTION_SET | LCD_8_BIT_MODE);
    __delay_us(150);
    lcd_write_nibble(lcd, LCD_CMD_FUNCTION_SET | LCD_8_BIT_MODE);

    lcd_write_nibble(lcd, (uint8_t)(LCD_CMD_FUNCTION_SET | lcd->bus_size));    // TODO: Revisar en caso de 8bits porque no se si es necesario
    lcd_cmd_write(lcd, (uint8_t)(LCD_CMD_FUNCTION_SET | lcd->bus_size | lcd->n_lines | lcd->display_font));
    lcd_cmd_write(lcd, LCD_CMD_DISPLAY_CTRL | LCD_POWER_OFF);
    lcd_cmd_write(lcd, LCD_CMD_CLEAR_DISPLAY);
    lcd_cmd_write(lcd, LCD_CMD_ENTRY_MODE_SET | LCD_CURSOR_MOVE_RIGHT);
}


lcd_retval_t lcd_set_cursor(lcd_init_t *lcd, uint8_t x, uint8_t y) {
    if (x >= LCD_COLUMN_LIMIT || y >= LCD_ROW_LIMIT) return LCD_FAILURE;

    lcd_cmd_write(lcd, LCD_CMD_SET_DDRAM_ADDRESS | ((y == 0 ? LCD_DDRAM_ADDRESS_ROW_0 : LCD_DDRAM_ADDRESS_ROW_1) + x));

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
    // https://panda-bg.com/resources/prod_2424_2134-091834-lcd-module-tc1602d-02wa0-16x2-stn.pdf
    lcd_start_sequence(lcd);

    lcd_clear(lcd);

    lcd_write_string(lcd, lcd->initial_msg);

    return LCD_SUCCESS;
}

// // void lcd_sleep(lcd_init_t *lcd) {
// //     lcd_cmd_write(lcd, LCD_CMD_DISPLAY_ON_OFF | LCD_POWER_OFF);
// //     __delay_us(50);
// // }

// // void lcd_wakeup(lcd_init_t *lcd) {
// //     lcd_cmd_write(lcd, LCD_CMD_DISPLAY_ON_OFF | LCD_POWER_ON);
// //     __delay_us(50);
// // }

void lcd_clear(lcd_init_t *lcd) {
    lcd_cmd_write(lcd, LCD_CMD_CLEAR_DISPLAY);
    __delay_ms(4);
}

// // void lcd_turn_off_cursor(lcd_init_t *lcd) {
// //     lcd_cmd_write(lcd, LCD_CMD_DISPLAY_ON_OFF | LCD_NO_CURSOR);
// //     __delay_us(50);
// // }

// // void lcd_turn_on_cursor(lcd_init_t *lcd) {
// //     lcd_cmd_write(lcd, LCD_CMD_DISPLAY_ON_OFF | LCD_CURSOR_STATIC);
// //     __delay_us(50);
// // }

// // void lcd_turn_off_cursor_blink(lcd_init_t *lcd) {
// //     lcd_turn_on_cursor(lcd);
// // }

// // void lcd_turn_on_cursor_blink(lcd_init_t *lcd) {
// //     lcd_cmd_write(lcd, LCD_CMD_DISPLAY_ON_OFF | LCD_CURSOR_BLINK);
// //     __delay_us(50);
// // }

// // #include "lcd.h"
// // #include "../../board.h"
// // #include <xc.h>

// // // Función auxiliar para escribir en un pin
// // static void lcd_write_pin(lcd_gpio_pin_t pin, uint8_t value) {
// //     gpio_write_pin(pin.port, pin.pin, value);
// // }

// // static void lcd_enable_pulse(lcd_t *lcd) {
// //     lcd_write_pin(lcd->pins.en, 1);
// //     __delay_ms(1);
// //     lcd_write_pin(lcd->pins.en, 0);
// //     __delay_ms(1);
// // }

// // // Enviar comando al LCD
// // void lcd_write_4_bit_mode(lcd_t *lcd, uint8_t cmd) {

// //     // Enviar parte alta
// //     lcd_write_pin(lcd->pins.d4, (cmd >> DB4) & 1);
// //     lcd_write_pin(lcd->pins.d5, (cmd >> DB5) & 1);
// //     lcd_write_pin(lcd->pins.d6, (cmd >> DB6) & 1);
// //     lcd_write_pin(lcd->pins.d7, (cmd >> DB7) & 1);
// //     lcd_enable_pulse(lcd);

// //     // Enviar parte baja
// //     lcd_write_pin(lcd->pins.d4, (cmd >> DB0) & 1);
// //     lcd_write_pin(lcd->pins.d5, (cmd >> DB1) & 1);
// //     lcd_write_pin(lcd->pins.d6, (cmd >> DB2) & 1);
// //     lcd_write_pin(lcd->pins.d7, (cmd >> DB3) & 1);
// //     lcd_enable_pulse(lcd);
// // }

// // static void lcd_init_gpio(lcd_t *lcd) {
// //     gpio_config(lcd->pins.rs.port, lcd->pins.rs.pin, GPIO_OUTPUT_LOW);
// //     gpio_config(lcd->pins.en.port, lcd->pins.en.pin, GPIO_OUTPUT_LOW);
// //     gpio_config(lcd->pins.d4.port, lcd->pins.d4.pin, GPIO_OUTPUT_LOW);
// //     gpio_config(lcd->pins.d5.port, lcd->pins.d5.pin, GPIO_OUTPUT_LOW);
// //     gpio_config(lcd->pins.d6.port, lcd->pins.d6.pin, GPIO_OUTPUT_LOW);
// //     gpio_config(lcd->pins.d7.port, lcd->pins.d7.pin, GPIO_OUTPUT_LOW);
// // }

// // void lcd_cmd(lcd_t *lcd, uint8_t cmd) {
// //     lcd_write_pin(lcd->pins.rs, 0);    // Modo comando
// //     lcd_write_4_bit_mode(lcd, cmd);
// // }

// // // Inicializar LCD
// // void lcd_init(lcd_t *lcd) {

// //     lcd_init_gpio(lcd);

// //     __delay_ms(100);
// //     lcd_cmd(lcd, 0x30);
// //     __delay_ms(5);
// //     lcd_cmd(lcd, 0x30);
// //     __delay_ms(1);
// //     lcd_cmd(lcd, 0x32);
// //     lcd_cmd(lcd, 0x28);    // 4 bits, 2 líneas
// //     lcd_cmd(lcd, 0x0C);    // Display ON
// //     lcd_cmd(lcd, 0x01);    // Clear
// //     lcd_cmd(lcd, 0x06);    // Entry mode

// //     lcd_clear(lcd);
// // }

// // // Escribir un carácter
// // void lcd_write_char(lcd_t *lcd, char c) {
// //     lcd_write_pin(lcd->pins.rs, 1);
// //     lcd_write_4_bit_mode(lcd, c);
// // }

// // // Escribir una cadena
// // void lcd_write_string(lcd_t *lcd, const char *str) {
// //     while (*str) {
// //         lcd_write_char(lcd, *str++);
// //     }
// // }

// // // Borrar pantalla
// // void lcd_clear(lcd_t *lcd) {
// //     lcd_cmd(lcd, 0x01);
// //     __delay_ms(2);
// // }

// // // Posicionar cursor
// // void lcd_set_cursor(lcd_t *lcd, uint8_t x, uint8_t y) {
// //     lcd_cmd(lcd, x > 0 ? 0xC0 + y : 0x80 + y);
// // }
