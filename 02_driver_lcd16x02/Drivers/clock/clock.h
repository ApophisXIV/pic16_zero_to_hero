#ifndef CLOCK_H
#define CLOCK_H

typedef enum {
    F_8MHZ   = 0b111,
    F_4MHZ   = 0b110,
    F_2MHZ   = 0b101,
    F_1MHZ   = 0b100,
    F_500KHZ = 0b011,
    F_250KHZ = 0b010,
    F_125KHZ = 0b001,
    F_31KHZ  = 0b000,
} intosc_freq_t;

// TODO: Add a clock_status_t enum to indicate the clock source and monitor protections
// typedef enum {
//     CLK_IS_RUNNING_INTOSC,
//     CLK_IS_RUNNING_EXTOSC,
//     CLK_FAILSAFE_TRIGGERED,
// } clk_status_t;

// TODO: Opaque pointer to clock_t struct
// struct _clock;
// typedef struct _clock clock_t;

/**
 * @brief Configure the clock system to use the internal clock
 * @param  freq: The freq. values are bounded by intosc_freq_t enum
 * @retval None
 */
void clk_use_internal(intosc_freq_t freq);

/**
 * @brief  Configure the clock system to use an external clock
 * @param  None
 * @retval None
 */
void clk_use_external(void);

#endif    // CLOCK_H