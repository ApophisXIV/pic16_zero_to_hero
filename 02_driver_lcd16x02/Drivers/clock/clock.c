#include "clock.h"
#include <xc.h>

void clk_use_internal(intosc_freq_t freq) {

    OSCCONbits.SCS  = 1;                      // Set the system clock to internal
    OSCCONbits.IRCF = (unsigned char)freq;    // Set the oscillator frequency

    // Wait until the internal oscillator stabilize
    if (freq == F_31KHZ)
        while (!OSCCONbits.LTS);
    else
        while (!OSCCONbits.HTS);
}

void clk_use_external() {
    OSCCONbits.SCS = 0;    // Set the system clock to external
    while (!OSCCONbits.OSTS);
}
