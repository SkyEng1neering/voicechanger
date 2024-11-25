#ifndef __POWER_H__
#define __POWER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define POW_AMP_EN_Pin                  GPIO_PIN_2
#define POW_AMP_EN_GPIO_Port            GPIOA
#define POW_16V8_DIS_Pin                GPIO_PIN_3
#define POW_16V8_DIS_GPIO_Port          GPIOA
#define POW_CHARGE_EN_Pin               GPIO_PIN_3
#define POW_CHARGE_EN_GPIO_Port         GPIOB

void power_gpio_init();
void power_amplifier_en();
void power_amplifier_dis();
void power_16v8_en();
void power_16v8_dis();
void power_charger_en();
void power_charger_dis();

#ifdef __cplusplus
}
#endif
#endif /* __POWER_H__ */

