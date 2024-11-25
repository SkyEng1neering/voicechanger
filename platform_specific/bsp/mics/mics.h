#ifndef __MICS_H__
#define __MICS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define EXTERNAL_MIC_DET_Pin                     GPIO_PIN_1
#define EXTERNAL_MIC_DET_GPIO_Port               GPIOB
#define MIC_CLK_Pin                              GPIO_PIN_0
#define MIC_CLK_GPIO_Port                        GPIOB
#define MIC_EXTERNAL_IN_Pin                      GPIO_PIN_4
#define MIC_EXTERNAL_IN_GPIO_Port                GPIOE
#define MIC_INTERNAL_IN_Pin                      GPIO_PIN_3
#define MIC_INTERNAL_IN_GPIO_Port                GPIOC

#define MIC_EXT_PIN_IRQ_ID                       EXTI1_IRQn
#define MIC_EXT_PIN_IRQ_PRIO                     5
#define MIC_INTERNAL_DMA_STREAM_IRQ_ID           DMA1_Stream3_IRQn
#define MIC_EXTERNAL_DMA_STREAM_IRQ_ID           DMA1_Stream4_IRQn
#define MIC_INTERNAL_DMA_STREAM                  DMA1_Stream3
#define MIC_EXTERNAL_DMA_STREAM                  DMA1_Stream4
#define MICS_DMA_IRQ_PRIO                        5
#define MIC_INTERNAL_BUF_SIZE                    64000
#define MIC_EXTERNAL_BUF_SIZE                    64000

bool mics_init();
void mic_ext_detect_gpio_init();
bool mic_ext_is_connected();
uint32_t mic_ext_get_detects_num();
bool mic_internal_start();
bool mic_external_start();

void mic_int_dma_irq_handler();
void mic_ext_dma_irq_handler();
void mic_ext_detect_pin_irq_handler();

void mic_int_data_received_cb(int16_t *data, uint32_t len);
void mic_ext_data_received_cb(int16_t *data, uint32_t len);
void mic_ext_connected_cb();
void mic_ext_disconnected_cb();

#ifdef __cplusplus
}
#endif
#endif /* __MICS_H__ */

