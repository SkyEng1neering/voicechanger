#include "mics.h"
#include "stm32h7xx.h"

static DFSDM_Filter_HandleTypeDef         hdfsdm1_filter0;
static DFSDM_Filter_HandleTypeDef         hdfsdm1_filter1;
static DFSDM_Channel_HandleTypeDef        hdfsdm1_channel1;
static DFSDM_Channel_HandleTypeDef        hdfsdm1_channel3;
static DMA_HandleTypeDef                  hdma_dfsdm1_flt0;
static DMA_HandleTypeDef                  hdma_dfsdm1_flt1;

static int16_t                            mic_internal_buf[MIC_INTERNAL_BUF_SIZE] = {0};
static int16_t                            mic_external_buf[MIC_EXTERNAL_BUF_SIZE] = {0};
static uint32_t                           mic_ext_detects_num = 0;
static bool                               mic_ext_det_last_state = false;

bool mics_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1|RCC_PERIPHCLK_DFSDM1;
    PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL;
    PeriphClkInitStruct.Dfsdm1ClockSelection = RCC_DFSDM1CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        return false;
    }

    __HAL_RCC_DFSDM1_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /**DFSDM1 GPIO Configuration
    PE4     ------> DFSDM1_DATIN3
    PC3_C     ------> DFSDM1_DATIN1
    PB0     ------> DFSDM1_CKOUT
    */
    GPIO_InitStruct.Pin =       MIC_EXTERNAL_IN_Pin;
    GPIO_InitStruct.Mode =      GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull =      GPIO_NOPULL;
    GPIO_InitStruct.Speed =     GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_DFSDM1;
    HAL_GPIO_Init(MIC_EXTERNAL_IN_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin =       MIC_INTERNAL_IN_Pin;
    GPIO_InitStruct.Mode =      GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull =      GPIO_NOPULL;
    GPIO_InitStruct.Speed =     GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF3_DFSDM1;
    HAL_GPIO_Init(MIC_INTERNAL_IN_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin =       MIC_CLK_Pin;
    GPIO_InitStruct.Mode =      GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull =      GPIO_NOPULL;
    GPIO_InitStruct.Speed =     GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF6_DFSDM1;
    HAL_GPIO_Init(MIC_CLK_GPIO_Port, &GPIO_InitStruct);

    hdma_dfsdm1_flt0.Instance = MIC_INTERNAL_DMA_STREAM;
    hdma_dfsdm1_flt0.Init.Request = DMA_REQUEST_DFSDM1_FLT0;
    hdma_dfsdm1_flt0.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_dfsdm1_flt0.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dfsdm1_flt0.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dfsdm1_flt0.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_dfsdm1_flt0.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_dfsdm1_flt0.Init.Mode = DMA_CIRCULAR;
    hdma_dfsdm1_flt0.Init.Priority = DMA_PRIORITY_LOW;
    hdma_dfsdm1_flt0.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_dfsdm1_flt0) != HAL_OK) {
        return false;
    }

    /* Several peripheral DMA handle pointers point to the same DMA handle.
     Be aware that there is only one channel to perform all the requested DMAs. */
    __HAL_LINKDMA(&hdfsdm1_filter0,hdmaInj,hdma_dfsdm1_flt0);
    __HAL_LINKDMA(&hdfsdm1_filter0,hdmaReg,hdma_dfsdm1_flt0);

    HAL_NVIC_SetPriority(MIC_INTERNAL_DMA_STREAM_IRQ_ID, MICS_DMA_IRQ_PRIO, 0);
    HAL_NVIC_EnableIRQ(MIC_INTERNAL_DMA_STREAM_IRQ_ID);

    hdma_dfsdm1_flt1.Instance = MIC_EXTERNAL_DMA_STREAM;
    hdma_dfsdm1_flt1.Init.Request = DMA_REQUEST_DFSDM1_FLT1;
    hdma_dfsdm1_flt1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_dfsdm1_flt1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dfsdm1_flt1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dfsdm1_flt1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_dfsdm1_flt1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_dfsdm1_flt1.Init.Mode = DMA_CIRCULAR;
    hdma_dfsdm1_flt1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_dfsdm1_flt1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_dfsdm1_flt1) != HAL_OK) {
        return false;
    }

    /* Several peripheral DMA handle pointers point to the same DMA handle.
     Be aware that there is only one channel to perform all the requested DMAs. */
    __HAL_LINKDMA(&hdfsdm1_filter1,hdmaInj,hdma_dfsdm1_flt1);
    __HAL_LINKDMA(&hdfsdm1_filter1,hdmaReg,hdma_dfsdm1_flt1);

    HAL_NVIC_SetPriority(MIC_EXTERNAL_DMA_STREAM_IRQ_ID, MICS_DMA_IRQ_PRIO, 0);
    HAL_NVIC_EnableIRQ(MIC_EXTERNAL_DMA_STREAM_IRQ_ID);

    /* Init filters */
    hdfsdm1_filter0.Instance = DFSDM1_Filter0;
    hdfsdm1_filter0.Init.RegularParam.Trigger = DFSDM_FILTER_SW_TRIGGER;
    hdfsdm1_filter0.Init.RegularParam.FastMode = ENABLE;
    hdfsdm1_filter0.Init.RegularParam.DmaMode = ENABLE;
    hdfsdm1_filter0.Init.FilterParam.SincOrder = DFSDM_FILTER_SINC5_ORDER;
    hdfsdm1_filter0.Init.FilterParam.Oversampling = 64;
    hdfsdm1_filter0.Init.FilterParam.IntOversampling = 2;
    if (HAL_DFSDM_FilterInit(&hdfsdm1_filter0) != HAL_OK) {
        return false;
    }
    hdfsdm1_filter1.Instance = DFSDM1_Filter1;
    hdfsdm1_filter1.Init.RegularParam.Trigger = DFSDM_FILTER_SW_TRIGGER;
    hdfsdm1_filter1.Init.RegularParam.FastMode = ENABLE;
    hdfsdm1_filter1.Init.RegularParam.DmaMode = ENABLE;
    hdfsdm1_filter1.Init.FilterParam.SincOrder = DFSDM_FILTER_SINC5_ORDER;
    hdfsdm1_filter1.Init.FilterParam.Oversampling = 64;
    hdfsdm1_filter1.Init.FilterParam.IntOversampling = 2;
    if (HAL_DFSDM_FilterInit(&hdfsdm1_filter1) != HAL_OK) {
        return false;
    }

    /* Init channels */
    hdfsdm1_channel1.Instance = DFSDM1_Channel1;
    hdfsdm1_channel1.Init.OutputClock.Activation = ENABLE;
    hdfsdm1_channel1.Init.OutputClock.Selection = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
    hdfsdm1_channel1.Init.OutputClock.Divider = 30;
    hdfsdm1_channel1.Init.Input.Multiplexer = DFSDM_CHANNEL_EXTERNAL_INPUTS;
    hdfsdm1_channel1.Init.Input.DataPacking = DFSDM_CHANNEL_STANDARD_MODE;
    hdfsdm1_channel1.Init.Input.Pins = DFSDM_CHANNEL_SAME_CHANNEL_PINS;
    hdfsdm1_channel1.Init.SerialInterface.Type = DFSDM_CHANNEL_SPI_RISING;
    hdfsdm1_channel1.Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
    hdfsdm1_channel1.Init.Awd.FilterOrder = DFSDM_CHANNEL_FASTSINC_ORDER;
    hdfsdm1_channel1.Init.Awd.Oversampling = 1;
    hdfsdm1_channel1.Init.Offset = 0;
    hdfsdm1_channel1.Init.RightBitShift = 0x4;
    if (HAL_DFSDM_ChannelInit(&hdfsdm1_channel1) != HAL_OK) {
        return false;
    }
    hdfsdm1_channel3.Instance = DFSDM1_Channel3;
    hdfsdm1_channel3.Init.OutputClock.Activation = ENABLE;
    hdfsdm1_channel3.Init.OutputClock.Selection = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
    hdfsdm1_channel3.Init.OutputClock.Divider = 30;
    hdfsdm1_channel3.Init.Input.Multiplexer = DFSDM_CHANNEL_EXTERNAL_INPUTS;
    hdfsdm1_channel3.Init.Input.DataPacking = DFSDM_CHANNEL_STANDARD_MODE;
    hdfsdm1_channel3.Init.Input.Pins = DFSDM_CHANNEL_SAME_CHANNEL_PINS;
    hdfsdm1_channel3.Init.SerialInterface.Type = DFSDM_CHANNEL_SPI_FALLING;
    hdfsdm1_channel3.Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
    hdfsdm1_channel3.Init.Awd.FilterOrder = DFSDM_CHANNEL_FASTSINC_ORDER;
    hdfsdm1_channel3.Init.Awd.Oversampling = 1;
    hdfsdm1_channel3.Init.Offset = 0;
    hdfsdm1_channel3.Init.RightBitShift = 0x4;
    if (HAL_DFSDM_ChannelInit(&hdfsdm1_channel3) != HAL_OK) {
        return false;
    }

    if (HAL_DFSDM_FilterConfigRegChannel(&hdfsdm1_filter0,
            DFSDM_CHANNEL_1, DFSDM_CONTINUOUS_CONV_ON) != HAL_OK) {
        return false;
    }
    if (HAL_DFSDM_FilterConfigRegChannel(&hdfsdm1_filter1,
            DFSDM_CHANNEL_3, DFSDM_CONTINUOUS_CONV_ON) != HAL_OK) {
        return false;
    }
    return true;
}

bool mic_internal_start() {
    if (HAL_DFSDM_FilterRegularMsbStart_DMA(&hdfsdm1_filter0,
            mic_internal_buf, MIC_INTERNAL_BUF_SIZE) != HAL_OK) {
        return false;
    }
    return true;
}

bool mic_external_start() {
    if (HAL_DFSDM_FilterRegularMsbStart_DMA(&hdfsdm1_filter1,
            mic_external_buf, MIC_EXTERNAL_BUF_SIZE) != HAL_OK) {
        return false;
    }
    return true;
}

void mic_ext_detect_gpio_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = EXTERNAL_MIC_DET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(EXTERNAL_MIC_DET_GPIO_Port, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(MIC_EXT_PIN_IRQ_ID, MIC_EXT_PIN_IRQ_PRIO, 0);
    HAL_NVIC_EnableIRQ(MIC_EXT_PIN_IRQ_ID);
}

bool mic_ext_is_connected() {
    if (HAL_GPIO_ReadPin(EXTERNAL_MIC_DET_GPIO_Port,
            EXTERNAL_MIC_DET_Pin) == GPIO_PIN_SET) {
        return false;
    }
    return true;
}

void mic_ext_detect_pin_irq_handler() {
    if (mic_ext_is_connected() != mic_ext_det_last_state) {
        __HAL_GPIO_EXTI_CLEAR_IT(MIC_EXT_PIN_IRQ_ID);
        mic_ext_det_last_state = mic_ext_is_connected();

        if (mic_ext_det_last_state == true) {
            mic_ext_detects_num++;
            mic_ext_connected_cb();
        } else {
            mic_ext_disconnected_cb();
        }
    }
}

uint32_t mic_ext_get_detects_num() {
    uint32_t ret = mic_ext_detects_num;
    mic_ext_detects_num = 0;
    return ret;
}

void mic_int_dma_irq_handler() {
    HAL_DMA_IRQHandler(&hdma_dfsdm1_flt0);
}

void mic_ext_dma_irq_handler() {
    HAL_DMA_IRQHandler(&hdma_dfsdm1_flt1);
}

__attribute__((weak)) void mic_ext_connected_cb() {

}

__attribute__((weak)) void mic_ext_disconnected_cb() {

}

__attribute__((weak)) void mic_int_data_received_cb(int16_t *data, uint32_t len) {

}

__attribute__((weak)) void mic_ext_data_received_cb(int16_t *data, uint32_t len) {

}

void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter){
    if (hdfsdm_filter == &hdfsdm1_filter0) {
        /* Internal mic data received */
        mic_int_data_received_cb(mic_internal_buf, MIC_INTERNAL_BUF_SIZE / 2);
    } else {
        /* External mic data received */
        mic_ext_data_received_cb(mic_external_buf, MIC_EXTERNAL_BUF_SIZE / 2);
    }
}

void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter){
    if (hdfsdm_filter == &hdfsdm1_filter0) {
        /* Internal mic data received */
        mic_int_data_received_cb(&mic_internal_buf[MIC_INTERNAL_BUF_SIZE / 2],
                MIC_INTERNAL_BUF_SIZE / 2);
    } else {
        /* External mic data received */
        mic_ext_data_received_cb(&mic_external_buf[MIC_INTERNAL_BUF_SIZE / 2],
                MIC_EXTERNAL_BUF_SIZE / 2);
    }
}

void HAL_DFSDM_FilterErrorCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter){

}
