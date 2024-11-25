#include "dac.h"
#include "stm32h7xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#define DAC_DMA_STREAM                  DMA1_Stream2
#define DAC_DMA_STREAM_IRQ_ID           DMA1_Stream2_IRQn
#define DAC_DMA_IRQ_PRIO                5
#define DAC_DATA_SEND_TIMEOUT           2000

static I2S_HandleTypeDef hi2s1;
static DMA_HandleTypeDef hdma_spi1_tx;
static TaskHandle_t task_to_notify = NULL;

void dac_mute(){
    HAL_GPIO_WritePin(DAC_MUTE_GPIO_Port, DAC_MUTE_Pin, GPIO_PIN_RESET);
}

void dac_unmute(){
    HAL_GPIO_WritePin(DAC_MUTE_GPIO_Port, DAC_MUTE_Pin, GPIO_PIN_SET);
}

void dac_fmt_i2s(){
    HAL_GPIO_WritePin(DAC_FMT_SEL_GPIO_Port, DAC_FMT_SEL_Pin, GPIO_PIN_RESET);
}

void dac_fmt_left_justified(){
    HAL_GPIO_WritePin(DAC_FMT_SEL_GPIO_Port, DAC_FMT_SEL_Pin, GPIO_PIN_SET);
}

bool dac_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* Init DAC GPIO */
    HAL_GPIO_WritePin(GPIOA, DAC_FILT_SEL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, DAC_FMT_SEL_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, DAC_MUTE_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(GPIOB, DAC_DEMP_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = DAC_FILT_SEL_Pin|DAC_FMT_SEL_Pin|DAC_MUTE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DAC_DEMP_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Init interface */
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI1;
    PeriphClkInitStruct.PLL2.PLL2M = 1;
    PeriphClkInitStruct.PLL2.PLL2N = 64;
    PeriphClkInitStruct.PLL2.PLL2P = 4;
    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    PeriphClkInitStruct.PLL2.PLL2R = 2;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        return false;
    }

    /**I2S1 GPIO Configuration
    PA4     ------> I2S1_WS
    PA5     ------> I2S1_CK
    PA7     ------> I2S1_SDO
    */
    GPIO_InitStruct.Pin = I2S_LRCK_Pin|I2S_BCK_Pin|I2S_DATA_OUT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    hdma_spi1_tx.Instance = DAC_DMA_STREAM;
    hdma_spi1_tx.Init.Request = DMA_REQUEST_SPI1_TX;
    hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_spi1_tx.Init.MemDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_spi1_tx.Init.Mode = DMA_CIRCULAR;
    hdma_spi1_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_spi1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK) {
        return false;
    }
    __HAL_LINKDMA(&hi2s1,hdmatx,hdma_spi1_tx);

    HAL_NVIC_SetPriority(DAC_DMA_STREAM_IRQ_ID, DAC_DMA_IRQ_PRIO, 0);
    HAL_NVIC_EnableIRQ(DAC_DMA_STREAM_IRQ_ID);

    hi2s1.Instance = SPI1;
    hi2s1.Init.Mode = I2S_MODE_MASTER_TX;
    hi2s1.Init.Standard = I2S_STANDARD_MSB;
    hi2s1.Init.DataFormat = I2S_DATAFORMAT_16B;
    hi2s1.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
    hi2s1.Init.AudioFreq = 96000;
    hi2s1.Init.CPOL = I2S_CPOL_LOW;
    hi2s1.Init.FirstBit = I2S_FIRSTBIT_MSB;
    hi2s1.Init.WSInversion = I2S_WS_INVERSION_DISABLE;
    hi2s1.Init.Data24BitAlignment = I2S_DATA_24BIT_ALIGNMENT_RIGHT;
    hi2s1.Init.MasterKeepIOState = I2S_MASTER_KEEP_IO_STATE_DISABLE;
    if (HAL_I2S_Init(&hi2s1) != HAL_OK) {
        return false;
    }
    return true;
}

bool dac_send_data_16(int16_t* ptr, uint16_t len) {
    if (HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t*)ptr, len) != HAL_OK) {
        return false;
    }

//    /* Sleep and wait for notification from ISR */
//    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
//        task_to_notify = xTaskGetCurrentTaskHandle();
//        const TickType_t max_block_time = pdMS_TO_TICKS(DAC_DATA_SEND_TIMEOUT);
//        uint32_t notification_val = ulTaskNotifyTake(pdTRUE, max_block_time);
//        if (notification_val != 1) {
//            return true;
//        }
//        return false;
//    }
    return true;
}

bool dac_send_data_32(int32_t* ptr, uint16_t len) {
    HAL_StatusTypeDef stat = HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t*)ptr, len * 2);
    if (stat != HAL_OK) {
        return false;
    }

//    /* Sleep and wait for notification from ISR */
//    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
//        task_to_notify = xTaskGetCurrentTaskHandle();
//        const TickType_t max_block_time = pdMS_TO_TICKS(DAC_DATA_SEND_TIMEOUT);
//        uint32_t notification_val = ulTaskNotifyTake(pdTRUE, max_block_time);
//        if (notification_val != 1) {
//            return true;
//        }
//        return false;
//    }
    return true;
}

void dac_stop() {
    HAL_I2S_DMAStop(&hi2s1);
}

void dac_pause() {
    HAL_I2S_DMAPause(&hi2s1);
}

void dac_resume() {
    HAL_I2S_DMAResume(&hi2s1);
}

void dac_dma_interrupt_handler() {
    HAL_DMA_IRQHandler(&hdma_spi1_tx);
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
//    HAL_I2S_DMAStop(&hi2s1);
//    hi2s->State = HAL_I2S_STATE_READY;
//    if (task_to_notify == NULL) {
//        return;
//    }
//    BaseType_t higher_prio_task_woken = pdFALSE;
//
//    /* Notify the task that the transmission is complete. */
//    vTaskNotifyGiveFromISR(task_to_notify, &higher_prio_task_woken);
//    task_to_notify = NULL;
//    portYIELD_FROM_ISR(higher_prio_task_woken);
    dac_full_sent_cb();
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
    dac_half_sent_cb();
}

__attribute__((weak)) void dac_half_sent_cb() {

}

__attribute__((weak)) void dac_full_sent_cb() {

}
