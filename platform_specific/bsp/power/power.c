#include "power.h"
#include "stm32h7xx.h"

void power_gpio_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(POW_AMP_EN_GPIO_Port, POW_AMP_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(POW_16V8_DIS_GPIO_Port, POW_16V8_DIS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(POW_CHARGE_EN_GPIO_Port, POW_CHARGE_EN_Pin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = POW_AMP_EN_Pin|POW_16V8_DIS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = POW_CHARGE_EN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void power_amplifier_en() {
    HAL_GPIO_WritePin(POW_AMP_EN_GPIO_Port, POW_AMP_EN_Pin, GPIO_PIN_SET);
}

void power_amplifier_dis() {
    HAL_GPIO_WritePin(POW_AMP_EN_GPIO_Port, POW_AMP_EN_Pin, GPIO_PIN_RESET);
}

void power_16v8_en() {
    HAL_GPIO_WritePin(POW_16V8_DIS_GPIO_Port, POW_16V8_DIS_Pin, GPIO_PIN_RESET);
}

void power_16v8_dis() {
    HAL_GPIO_WritePin(POW_16V8_DIS_GPIO_Port, POW_16V8_DIS_Pin, GPIO_PIN_SET);
}

void power_charger_en() {
    HAL_GPIO_WritePin(POW_CHARGE_EN_GPIO_Port, POW_CHARGE_EN_Pin, GPIO_PIN_SET);
}

void power_charger_dis() {
    HAL_GPIO_WritePin(POW_CHARGE_EN_GPIO_Port, POW_CHARGE_EN_Pin, GPIO_PIN_RESET);
}
