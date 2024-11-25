#include "current_meas_ll.h"
#include "stm32h7xx_hal.h"

static ADC_HandleTypeDef hadc3;
static DMA_HandleTypeDef hdma_adc3;

static union adc_data_t adc_data;

static uint32_t adc_devide_coefs_arr[] = {
    1,      //ADC_CUR_MEAS
};

/* Get ADC voltage in mV */
uint32_t get_adc_data_ll(enum AdcChannels channel_ind) {
    if (channel_ind >= ADC_CHANNELS_NUM) {
        return 0.0;
    }

    uint16_t adc_raw = adc_data.data_arr[channel_ind];
    uint32_t result = ((uint32_t)adc_raw * ADC_REFERENCE_VOLTAGE_MV) / (uint32_t)ADC_RAW_DATA_MAX;
    return result / adc_devide_coefs_arr[channel_ind];
}

bool current_meas_init_ll() {
    ADC_ChannelConfTypeDef sConfig = {0};

    __HAL_RCC_ADC3_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**ADC3 GPIO Configuration
    PC2_C     ------> ADC3_INP0
    */
    HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PC2, SYSCFG_SWITCH_PC2_OPEN);

    /* ADC3 DMA Init */
    /* ADC3 Init */
    hdma_adc3.Instance = CURRENT_MEAS_DMA_STREAM;
    hdma_adc3.Init.Request = DMA_REQUEST_ADC3;
    hdma_adc3.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc3.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc3.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc3.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc3.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc3.Init.Mode = DMA_CIRCULAR;
    hdma_adc3.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc3.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc3) != HAL_OK) {
        return false;
    }

    __HAL_LINKDMA(&hadc3,DMA_Handle,hdma_adc3);

//    HAL_NVIC_SetPriority(CURRENT_MEAS_DMA_STREAM_IRQ_ID, CURRENT_MEAS_DMA_STREAM_IRQ_PRIO, 0);
//    HAL_NVIC_EnableIRQ(CURRENT_MEAS_DMA_STREAM_IRQ_ID);

    hadc3.Instance = ADC3;
    hadc3.Init.Resolution = ADC_RESOLUTION_16B;
    hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc3.Init.LowPowerAutoWait = DISABLE;
    hadc3.Init.ContinuousConvMode = ENABLE;
    hadc3.Init.NbrOfConversion = 1;
    hadc3.Init.DiscontinuousConvMode = DISABLE;
    hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
    hadc3.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
    hadc3.Init.OversamplingMode = DISABLE;
    if (HAL_ADC_Init(&hadc3) != HAL_OK) {
        return false;
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    sConfig.OffsetSignedSaturation = DISABLE;
    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
        return false;
    }

    /* Start conversions */
    if (HAL_ADC_Start_DMA(&hadc3, (uint32_t *)adc_data.data_arr,
            ADC_CHANNELS_NUM) != HAL_OK) {
        return false;
    }
    return true;
}


