#ifndef __LEDS_H__
#define __LEDS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

enum LedIDs {
    LED_1 = 0,
    LED_2
};

#define LED2_Pin                     GPIO_PIN_15
#define LED2_GPIO_Port               GPIOD
#define LED1_Pin                     GPIO_PIN_6
#define LED1_GPIO_Port               GPIOC

bool leds_init_pwm();
void led_set_val(enum LedIDs led, uint8_t brightness);

#ifdef __cplusplus
}
#endif
#endif /* __LEDS_H__ */

