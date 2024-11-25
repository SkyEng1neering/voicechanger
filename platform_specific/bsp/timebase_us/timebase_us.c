#include "FreeRTOS.h"
#include "task.h"
#include "stm32h7xx.h"
#include "timebase_us.h"
#include "log.h"

#define TIMEBASE_US_INTERRUPT_PRIO                      2

static const char* tag = "TIMBASE_US";

/* Counter that increments on every 2-bytes timer overflow */
static volatile uint64_t overflow_counter;
TIM_HandleTypeDef htim7;

bool timebase_us_init() {
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    __HAL_RCC_TIM7_CLK_ENABLE();

    HAL_NVIC_SetPriority(TIM7_IRQn, TIMEBASE_US_INTERRUPT_PRIO, 0);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);

    htim7.Instance = TIM7;
    htim7.Init.Prescaler = (SystemCoreClock/2000000) - 1;//Prescaler for counting every processor tick
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = 1000 - 1;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if(HAL_TIM_Base_Init(&htim7) != HAL_OK){
        LOGE(tag, "%s(): Time base init error\n", __func__);
        return false;
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK) {
        LOGE(tag, "%s(): Time base init error\n", __func__);
        return false;
    }
    if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK) {
        LOGE(tag, "%s(): Time base start error\n", __func__);
        return false;
    }
    return true;
}

uint16_t get_tick_us_lo16() {
    return TIM7->CNT;
}

uint64_t get_tick_us_overflow_cnt() {
    return overflow_counter;
}

uint64_t get_tick_us() {
    uint16_t lo16_cached = get_tick_us_lo16();
    uint64_t hi32_cached = overflow_counter;
    uint64_t res = hi32_cached*1000 + lo16_cached;
    return res;
}

uint32_t get_tick_ms() {
    return (uint32_t)overflow_counter;
}

void set_tick_us(uint64_t tick) {
    overflow_counter = tick/1000;
    TIM7->CNT = tick % 1000;
}

void timebase_us_callback() {
    if ((overflow_counter % 1000) == 0) {
//        toggle_debug_pin();
    }

    overflow_counter++;
}

void tb_delay_us(uint32_t us) {
    uint64_t start_us = get_tick_us();
    while (get_tick_us() - start_us <= us) {
        /* Do nothing */
    }
}
