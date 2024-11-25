#ifndef __DAC_H__
#define __DAC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define DAC_FILT_SEL_Pin                    GPIO_PIN_0
#define DAC_FILT_SEL_GPIO_Port              GPIOA
#define DAC_FMT_SEL_Pin                     GPIO_PIN_1
#define DAC_FMT_SEL_GPIO_Port               GPIOA
#define DAC_MUTE_Pin                        GPIO_PIN_6
#define DAC_MUTE_GPIO_Port                  GPIOA
#define DAC_DEMP_Pin                        GPIO_PIN_2
#define DAC_DEMP_GPIO_Port                  GPIOB
#define I2S_LRCK_Pin                        GPIO_PIN_4
#define I2S_LRCK_GPIO_Port                  GPIOA
#define I2S_BCK_Pin                         GPIO_PIN_5
#define I2S_BCK_GPIO_Port                   GPIOA
#define I2S_DATA_OUT_Pin                    GPIO_PIN_7
#define I2S_DATA_OUT_GPIO_Port              GPIOA


void dac_mute();
void dac_unmute();
void dac_fmt_i2s();
void dac_fmt_left_justified();
bool dac_init();
bool dac_send_data_16(int16_t* ptr, uint16_t len);
bool dac_send_data_32(int32_t* ptr, uint16_t len);
void dac_dma_interrupt_handler();
void dac_full_sent_cb();
void dac_half_sent_cb();
void dac_resume();
void dac_stop();
void dac_pause();

#ifdef __cplusplus
}
#endif
#endif /* __DAC_H__ */

