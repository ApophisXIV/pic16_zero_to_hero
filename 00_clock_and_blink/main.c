/**
 * @file main.c
 * @author guido
 * @date 2025-02-09
 * @brief Main function
 */

/* -------------------------------------------------------------------------- */
#include "config.h"
#include "Drivers/clock/clock.h"
/* -------------------------------------------------------------------------- */

#define _XTAL_FREQ 4e6

int main() {

    // clk_use_internal(F_8MHZ);
    clk_use_external();

    TRISCbits.TRISC4 = 0;    // Set RC4 as output

    while (1) {
        PORTCbits.RC4 ^= 1;
        __delay_ms(500);
    }

    return 0;
}