#ifndef BSP_CURRENT_MEAS_CURRENT_MEAS_LL_H_
#define BSP_CURRENT_MEAS_CURRENT_MEAS_LL_H_

#include <stdbool.h>
#include <stdint.h>

#define CURRENT_MEAS_Pin                    GPIO_PIN_2
#define CURRENT_MEAS_GPIO_Port              GPIOC
#define CURRENT_MEAS_DMA_STREAM             DMA1_Stream5
#define CURRENT_MEAS_DMA_STREAM_IRQ_ID      DMA1_Stream5_IRQn
#define CURRENT_MEAS_DMA_STREAM_IRQ_PRIO    5

#define ADC_CHANNELS_NUM                    1
#define ADC_REFERENCE_VOLTAGE_MV            3300UL
#define ADC_RAW_DATA_MAX                    0xFFF

enum AdcChannels {
    ADC_CURRENT_MEAS = 0,
};

struct adc_channels {
    uint16_t ch1;
};

union adc_data_t {
    uint16_t data_arr[ADC_CHANNELS_NUM];
    struct adc_channels data_ch;
};

uint32_t get_adc_data_ll(enum AdcChannels channel_ind);
bool current_meas_init_ll();

#endif /* BSP_CURRENT_MEAS_CURRENT_MEAS_LL_H_ */
