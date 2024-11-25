#ifndef __DEBUG_GPIO_H__
#define __DEBUG_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define TESTPOINT_2_Pin             GPIO_PIN_0
#define TESTPOINT_2_GPIO_Port       GPIOC

#define TESTPOINT_1_Pin             GPIO_PIN_8
#define TESTPOINT_1_GPIO_Port       GPIOA

void mco_pin_init();
void tp1_init();
void tp2_init();
void tp2_set();
void tp2_reset();
void tp2_toggle();
void tp1_set();
void tp1_reset();
void tp1_toggle();

#ifdef __cplusplus
}
#endif
#endif /* __DEBUG_GPIO_H__ */

