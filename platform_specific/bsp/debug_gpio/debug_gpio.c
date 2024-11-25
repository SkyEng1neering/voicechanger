#include "debug_gpio.h"
#include "stm32h7xx.h"

bool tp1_init_stat = false;
bool tp2_init_stat = false;

void tp1_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    HAL_GPIO_WritePin(TESTPOINT_1_GPIO_Port, TESTPOINT_1_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = TESTPOINT_1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(TESTPOINT_1_GPIO_Port, &GPIO_InitStruct);

    tp1_init_stat = true;
}

void tp2_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    HAL_GPIO_WritePin(TESTPOINT_2_GPIO_Port, TESTPOINT_2_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = TESTPOINT_2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(TESTPOINT_2_GPIO_Port, &GPIO_InitStruct);

    tp2_init_stat = true;
}

void mco_pin_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void tp2_set() {
    if (tp2_init_stat == false)
        tp2_init();
    HAL_GPIO_WritePin(TESTPOINT_2_GPIO_Port, TESTPOINT_2_Pin, GPIO_PIN_SET);
}

void tp2_reset() {
    if (tp2_init_stat == false)
        tp2_init();
    HAL_GPIO_WritePin(TESTPOINT_2_GPIO_Port, TESTPOINT_2_Pin, GPIO_PIN_RESET);
}

void tp2_toggle() {
    if (tp2_init_stat == false)
        tp2_init();
    HAL_GPIO_TogglePin(TESTPOINT_2_GPIO_Port, TESTPOINT_2_Pin);
}

void tp1_set() {
    if (tp1_init_stat == false)
        tp1_init();
    HAL_GPIO_WritePin(TESTPOINT_1_GPIO_Port, TESTPOINT_1_Pin, GPIO_PIN_SET);
}

void tp1_reset() {
    if (tp1_init_stat == false)
        tp1_init();
    HAL_GPIO_WritePin(TESTPOINT_1_GPIO_Port, TESTPOINT_1_Pin, GPIO_PIN_RESET);
}

void tp1_toggle() {
    if (tp1_init_stat == false)
        tp1_init();
    HAL_GPIO_TogglePin(TESTPOINT_1_GPIO_Port, TESTPOINT_1_Pin);
}
