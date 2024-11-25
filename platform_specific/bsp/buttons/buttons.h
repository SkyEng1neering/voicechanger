#ifndef PLATFORM_SPECIFIC_BSP_BUTTON_BUTTON_LL_H_
#define PLATFORM_SPECIFIC_BSP_BUTTON_BUTTON_LL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "stm32h7xx.h"

#define BUTTON_1_Pin              GPIO_PIN_1
#define BUTTON_1_GPIO_Port        GPIOC
#define BUTTON_2_Pin              GPIO_PIN_13
#define BUTTON_2_GPIO_Port        GPIOC

#define BUTTON_INVERSED           true
#define BUTTON_NOT_INVERSED       false

enum ButtonIDs {
    BUTTON_1 = 0,
    BUTTON_2
};

struct ButtonStat {
    bool logic_inversed;
    bool last_state;
    uint32_t press_number;
};

struct ButtonConf {
    GPIO_TypeDef  *port;
    uint16_t pin;
    volatile uint32_t *rcc_reg_ptr;
    uint32_t port_en_bit_pos;
    uint32_t pull_mode;
    IRQn_Type irq_id;
    uint8_t irq_prio;
};

uint32_t get_buttons_num_ll();
bool button_init_ll(uint8_t button_ind, bool inversed);
bool get_button_state_ll(uint8_t button_ind);
uint32_t get_button_press_num(uint8_t button_ind);
void button_exti_callback(uint8_t button_ind);
void button_exti_irq_callback();

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_SPECIFIC_BSP_BUTTON_BUTTON_LL_H_ */
