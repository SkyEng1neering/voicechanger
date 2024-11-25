#ifndef US_COMM_US_COMM_H_
#define US_COMM_US_COMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define US_SAMPLE_TYPE               int16_t
//#define SAMPLE_RESOLUTION_BITS    (sizeof(SAMPLE_TYPE) * 8)
#define US_SAMPLES_PER_SYMBOL        1500
#define US_SYMBOLS_NUM               256
#define US_MIN_PERIODS_PER_SYMB      2
#define US_TUNE_FREQ_FOR_CONT_PER    0

#define US_BASE_FREQUENCY_HZ         15000
#define US_MIN_FREQ_STEP_HZ          150
#define US_SYMBOL_WAVE_MAG           10000
#define US_WAVE_SAMPLE_RATE_HZ       150000

struct UsSymbol {
    US_SAMPLE_TYPE samples_arr[US_SAMPLES_PER_SYMBOL];
    uint16_t freq_hz;
};

void us_generate_symbols_array();
void us_dump_symbols_array();

#ifdef __cplusplus
}
#endif

#endif /* US_COMM_US_COMM_H_ */
