#ifndef PLATFORM_SPECIFIC_BSP_SD_CARD_SD_CARD_LL_H_
#define PLATFORM_SPECIFIC_BSP_SD_CARD_SD_CARD_LL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32h7xx.h"
#include "stdbool.h"

#define SD_CARD_DET_Pin             GPIO_PIN_7
#define SD_CARD_DET_GPIO_Port       GPIOC
#define SD_CARD_D0_Pin              GPIO_PIN_8
#define SD_CARD_D0_GPIO_Port        GPIOC
#define SD_CARD_D1_Pin              GPIO_PIN_9
#define SD_CARD_D1_GPIO_Port        GPIOC
#define SD_CARD_D2_Pin              GPIO_PIN_10
#define SD_CARD_D2_GPIO_Port        GPIOC
#define SD_CARD_D3_Pin              GPIO_PIN_11
#define SD_CARD_D3_GPIO_Port        GPIOC
#define SD_CARD_CLK_Pin             GPIO_PIN_12
#define SD_CARD_CLK_GPIO_Port       GPIOC
#define SD_CARD_CMD_Pin             GPIO_PIN_2
#define SD_CARD_CMD_GPIO_Port       GPIOD

#define SD_RW_TIMEOUT_MS            5000

#define SD_IRQ_PRIO                 5

enum CardStates {
    SD_STAT_READY = 1,
    SD_STAT_IDENTIFICATION,
    SD_STAT_STANDBY,
    SD_STAT_TRANSFER,
    SD_STAT_SENDING,
    SD_STAT_RECEIVING,
    SD_STAT_PROGRAMMING,
    SD_STAT_DISCONNECTED,
    SD_STAT_ERROR = 0xFF
};

enum CardStates sd_get_state();
bool sd_read_blocks_dma(uint8_t *data_ptr, uint32_t blk_addr, uint32_t blks_num, uint32_t timeout);
bool sd_write_blocks_dma(uint8_t *data_ptr, uint32_t blk_addr, uint32_t blks_num, uint32_t timeout);
bool sd_read_blocks(uint8_t *data_ptr, uint32_t blk_addr, uint32_t blks_num, uint32_t timeout);
bool sd_write_blocks(uint8_t *data_ptr, uint32_t blk_addr, uint32_t blks_num, uint32_t timeout);
bool sd_init_ll();
bool sd_deinit_ll();
uint32_t sd_get_log_blocks();
uint32_t sd_get_log_blk_size();
void sd_card_cache_invalidate(void* mem, uint32_t size);
void sd_irq_handler();

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_SPECIFIC_BSP_SD_CARD_SD_CARD_LL_H_ */
