#include "us_comm.h"
#include "log.h"
#include <math.h>

static const char*            tag = "US comm";
static struct UsSymbol        symbols_arr[US_SYMBOLS_NUM] = {0};

static void generate_symbol_arr(float mag, uint16_t freq_hz,
        uint32_t sample_rate, US_SAMPLE_TYPE* sampl_arr, uint32_t arr_len);

void us_generate_symbols_array() {
    float last_freq = (float)US_BASE_FREQUENCY_HZ;
    for (uint32_t i = 0; i < US_SYMBOLS_NUM; i++) {
#if US_TUNE_FREQ_FOR_CONT_PER == 1
        /* Tune frequency to have integer periods number per symbol */
        static float symb_time_slot_ms = ((float)US_SAMPLES_PER_SYMBOL * 1000.0) /
                (float)US_WAVE_SAMPLE_RATE_HZ;
        float current_symb_period_ms = 1000.0 / (float)last_freq;
        float periods_per_slot = symb_time_slot_ms / current_symb_period_ms;
        if (periods_per_slot < US_MIN_PERIODS_PER_SYMB) {
            LOGE(tag, "Too low frequency for symbol %u generation\n", i);
            return;
        }
        float ceiled_periods = ceilf(periods_per_slot);
        float diff_pt = 100.0 * (ceiled_periods - periods_per_slot) / ceiled_periods;
        last_freq += (diff_pt / 100.0) * last_freq;
#endif

        /* Fill symbol with tuned frequency */
        uint16_t last_freq_u16 = (uint16_t)last_freq;
        generate_symbol_arr(US_SYMBOL_WAVE_MAG, last_freq_u16, US_WAVE_SAMPLE_RATE_HZ,
                symbols_arr[i].samples_arr, US_SAMPLES_PER_SYMBOL);
        symbols_arr[i].freq_hz = last_freq_u16;
        last_freq += US_MIN_FREQ_STEP_HZ;
    }
}

void us_dump_symbols_array() {
    for (uint32_t i = 0; i < US_SYMBOLS_NUM; i++) {
        for (uint32_t k = 0; k < US_SAMPLES_PER_SYMBOL; k++) {
            LOGI(tag, "%d, %d\n", symbols_arr[i].freq_hz, symbols_arr[i].samples_arr[k]);
        }
    }
}

/* Static functions */
static void generate_symbol_arr(float mag, uint16_t freq_hz,
        uint32_t sample_rate, US_SAMPLE_TYPE* sampl_arr, uint32_t arr_len) {
    float phase_inc = (float)(2 * M_PI * freq_hz) / (float)sample_rate;
    float phase = 0.0;
    for (uint32_t i = 0; i < arr_len; i++) {
        float val = mag * sin(phase);
        sampl_arr[i] = (US_SAMPLE_TYPE)val;
        phase += phase_inc;
    }
}


