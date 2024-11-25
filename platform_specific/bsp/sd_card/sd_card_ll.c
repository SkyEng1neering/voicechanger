#include "sd_card_ll.h"
#include "stm32h7xx_hal_sd.h"
#include "log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

static SD_HandleTypeDef hsd1;
static TaskHandle_t tx_task_to_notify = NULL;
static TaskHandle_t rx_task_to_notify = NULL;
static volatile bool tx_cplt = false;
static volatile bool rx_cplt = false;

#define SD_BLOCK_SIZE                           BLOCKSIZE

enum CardStates sd_get_state() {
    HAL_SD_CardStateTypeDef state = HAL_SD_GetCardState(&hsd1);
    enum CardStates res;
    switch (state) {
        case HAL_SD_CARD_READY:
            res =  SD_STAT_READY;
            break;
        case HAL_SD_CARD_IDENTIFICATION:
            res =  SD_STAT_IDENTIFICATION;
            break;
        case HAL_SD_CARD_STANDBY:
            res =  SD_STAT_STANDBY;
            break;
        case HAL_SD_CARD_TRANSFER:
            res =  SD_STAT_TRANSFER;
            break;
        case HAL_SD_CARD_SENDING:
            res =  SD_STAT_SENDING;
            break;
        case HAL_SD_CARD_RECEIVING:
            res =  SD_STAT_RECEIVING;
            break;
        case HAL_SD_CARD_PROGRAMMING:
            res =  SD_STAT_PROGRAMMING;
            break;
        case HAL_SD_CARD_DISCONNECTED:
            res =  SD_STAT_DISCONNECTED;
            break;
        case HAL_SD_CARD_ERROR:
            res =  SD_STAT_ERROR;
            break;
        default:
            res =  SD_STAT_READY;
            break;
    }
    return res;
}

bool sd_read_blocks(uint8_t *data_ptr, uint32_t blk_addr, uint32_t blks_num, uint32_t timeout) {
    if (HAL_SD_ReadBlocks(&hsd1, data_ptr, blk_addr, blks_num, timeout) != HAL_OK) {
        return false;
    }
    return true;
}

bool sd_write_blocks(uint8_t *data_ptr, uint32_t blk_addr, uint32_t blks_num, uint32_t timeout) {
    if (HAL_SD_WriteBlocks(&hsd1, data_ptr, blk_addr, blks_num, timeout) != HAL_OK) {
        return false;
    }
    return true;
}

bool sd_read_blocks_dma(uint8_t *data_ptr, uint32_t blk_addr, uint32_t blks_num, uint32_t timeout) {
    rx_cplt = false;
    if (HAL_SD_ReadBlocks_DMA(&hsd1, data_ptr, blk_addr, blks_num) != HAL_OK) {
        return false;
    }

    /* Sleep and wait for notification from ISR */
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        rx_task_to_notify = xTaskGetCurrentTaskHandle();
        const TickType_t max_block_time = pdMS_TO_TICKS(timeout);
        uint32_t notification_val = ulTaskNotifyTake(pdTRUE, max_block_time);
        if (notification_val != 1) {
            return false;
        }
//        sd_card_cache_invalidate(data_ptr, blks_num * SD_BLOCK_SIZE);
        return true;
    }

    while (rx_cplt != true);

//    sd_card_cache_invalidate(data_ptr, blks_num * SD_BLOCK_SIZE);
    return true;
}

bool sd_write_blocks_dma(uint8_t *data_ptr, uint32_t blk_addr, uint32_t blks_num, uint32_t timeout) {
    tx_cplt = false;
    if (HAL_SD_WriteBlocks_DMA(&hsd1, data_ptr, blk_addr, blks_num) != HAL_OK) {
        return false;
    }

    /* Sleep and wait for notification from ISR */
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        tx_task_to_notify = xTaskGetCurrentTaskHandle();
        const TickType_t max_block_time = pdMS_TO_TICKS(timeout);
        uint32_t notification_val = ulTaskNotifyTake(pdTRUE, max_block_time);
        if (notification_val != 1) {
            return false;
        }
//        sd_card_cache_invalidate(data_ptr, blks_num * SD_BLOCK_SIZE);
        return true;
    }

    while (tx_cplt != true);

//    sd_card_cache_invalidate(data_ptr, blks_num * SD_BLOCK_SIZE);
    return true;
}

uint32_t sd_get_log_blocks() {
    HAL_SD_CardInfoTypeDef info;
    HAL_SD_GetCardInfo(&hsd1, &info);
    return info.LogBlockNbr;
}

uint32_t sd_get_log_blk_size() {
    HAL_SD_CardInfoTypeDef info;
    HAL_SD_GetCardInfo(&hsd1, &info);
    return info.BlockSize;
}

void sd_card_cache_invalidate(void* mem, uint32_t size) {
    SCB_InvalidateDCache_by_Addr(mem, (int32_t)size);
}

bool sd_init_ll() {
    hsd1.Instance = SDMMC1;
    hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
    hsd1.Init.ClockDiv = 2;
    if (HAL_SD_Init(&hsd1) != HAL_OK) {
        return false;
    }
    return true;
}

bool sd_deinit_ll() {
    if (HAL_SD_DeInit(&hsd1) != HAL_OK) {
        return false;
    }
    return true;
}

void HAL_SD_MspInit(SD_HandleTypeDef* sdHandle) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (sdHandle->Instance == SDMMC1) {
        /** Initializes the peripherals clock
        */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC;
        PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            return;
        }

        /* SDMMC1 clock enable */
        __HAL_RCC_SDMMC1_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**SDMMC1 GPIO Configuration
        PC8     ------> SDMMC1_D0
        PC9     ------> SDMMC1_D1
        PC10     ------> SDMMC1_D2
        PC11     ------> SDMMC1_D3
        PC12     ------> SDMMC1_CK
        PD2     ------> SDMMC1_CMD
        */
        GPIO_InitStruct.Pin = SD_CARD_D0_Pin|SD_CARD_D1_Pin|SD_CARD_D2_Pin
                |SD_CARD_D3_Pin|SD_CARD_CLK_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = SD_CARD_CMD_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
        HAL_GPIO_Init(SD_CARD_CMD_GPIO_Port, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(SDMMC1_IRQn, SD_IRQ_PRIO, 0);
        HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
    }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef* sdHandle) {
    if (sdHandle->Instance == SDMMC1) {
        /* Peripheral clock disable */
        __HAL_RCC_SDMMC1_CLK_DISABLE();

        /**SDMMC1 GPIO Configuration
        PC8     ------> SDMMC1_D0
        PC9     ------> SDMMC1_D1
        PC10     ------> SDMMC1_D2
        PC11     ------> SDMMC1_D3
        PC12     ------> SDMMC1_CK
        PD2     ------> SDMMC1_CMD
        */
        HAL_GPIO_DeInit(GPIOC, SD_CARD_D0_Pin|SD_CARD_D1_Pin|SD_CARD_D2_Pin|SD_CARD_D3_Pin
                                  |SD_CARD_CLK_Pin);
        HAL_GPIO_DeInit(SD_CARD_CMD_GPIO_Port, SD_CARD_CMD_Pin);
    }
}

void sd_irq_handler() {
    HAL_SD_IRQHandler(&hsd1);
}

void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd) {
    tx_cplt = true;
    if (tx_task_to_notify == NULL) {
        return;
    }
    BaseType_t higher_prio_task_woken = pdFALSE;

    /* Notify the task that the transmission is complete. */
    vTaskNotifyGiveFromISR(tx_task_to_notify, &higher_prio_task_woken);
    tx_task_to_notify = NULL;
    portYIELD_FROM_ISR(higher_prio_task_woken);
}

void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd) {
    rx_cplt = true;
    if (rx_task_to_notify == NULL) {
        return;
    }
    BaseType_t higher_prio_task_woken = pdFALSE;

    /* Notify the task that the transmission is complete. */
    vTaskNotifyGiveFromISR(rx_task_to_notify, &higher_prio_task_woken);
    rx_task_to_notify = NULL;
    portYIELD_FROM_ISR(higher_prio_task_woken);
}
