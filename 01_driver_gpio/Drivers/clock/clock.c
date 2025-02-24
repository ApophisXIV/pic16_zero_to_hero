#include "clock.h"
#include <xc.h>

// TODO: Opaque pointer to clock_t struct
// struct _clock {
//     bool is_intosc;
//     uint16_t freq;
// };

// clk_status_t clk_is_running(clock_t *clk) {
//     return clk->is_intosc;
// }

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
