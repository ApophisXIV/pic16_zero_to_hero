#ifndef CONFIG_H
#define CONFIG_H

/* -------------------------------------------------------------------------- */
// config bits - register CONFIG1
#pragma config DEBUG = OFF              // We utilize the debug lines RB6 and RB7
#pragma config LVP   = OFF             // We don't want to use the low voltage programming because is tendious to fail
#pragma config FCMEN = ON              // We will check if any external clock fail and we will act acording to that
#pragma config IESO  = ON              // During the start-up the MCU will decide to use the internal clock to accelerate the program boot process
#pragma config BOREN = ON              // We will check if the voltage is below the desired threshold. In that case the MCU will be restarted (See BOR4V config)
#pragma config CPD   = OFF             // Data protection
#pragma config CP    = OFF             // Code protection
#pragma config MCLRE = ON              // MCLR pin is used to reset the MCU
#pragma config PWRTE = ON              // Delay on power up until the PSU is stable
#pragma config WDTE  = OFF             // No watchdog is used
// #pragma config FOSC  = INTRC_CLKOUT    // We use the internal clock with the CLOCKOUT function on the pin RA6 to measure the real freq.
#pragma config FOSC  = XT   // We use the external clock with the CLOCKOUT function on the pin RA6 to measure the real freq.

// config bits - register CONFIG2
#pragma config WRT   = OFF       // We don't want to single-write use on the MCU
#pragma config BOR4V = BOR40V    // We set the BOR threshold at 4.0V
/* -------------------------------------------------------------------------- */

#include <xc.h>

#endif    // CONFIG_H