#include "buttons.h"

/* Buttons array */
static struct ButtonConf buttons[] = {
        {
                /* Button with index 0 */
                BUTTON_1_GPIO_Port,
                BUTTON_1_Pin,
                &RCC->AHB4ENR,
                RCC_AHB4ENR_GPIOCEN_Pos,
                GPIO_NOPULL,
                EXTI1_IRQn,
                5
        },
        {
                /* Button with index 1 */
                BUTTON_2_GPIO_Port,
                BUTTON_2_Pin,
                &RCC->AHB4ENR,
                RCC_AHB4ENR_GPIOCEN_Pos,
                GPIO_NOPULL,
                EXTI15_10_IRQn,
                5
        },
};

static struct ButtonStat buttons_stats_arr[sizeof(buttons) / sizeof(struct ButtonConf)] = {0};

uint32_t get_buttons_num_ll() {
    return sizeof(buttons) / sizeof(struct ButtonConf);
}

bool button_init_ll(uint8_t button_ind, bool inversed) {
    /* Check button index */
    if (button_ind >= get_buttons_num_ll())
        return false;

    /* Init button */
    struct ButtonConf *button_ptr = &buttons[button_ind];

    /* Enable clocking */
    *button_ptr->rcc_reg_ptr |= (1 << button_ptr->port_en_bit_pos);

    /* Init GPIO as input with external interrupt */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = button_ptr->pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = button_ptr->pull_mode;
    HAL_GPIO_Init(button_ptr->port, &GPIO_InitStruct);

    buttons_stats_arr[button_ind].logic_inversed = inversed;

    NVIC_SetPriority(button_ptr->irq_id, (uint32_t)button_ptr->irq_prio);
    NVIC_EnableIRQ(button_ptr->irq_id);

    return true;
}

bool get_button_state_ll(uint8_t button_ind) {
    /* Check button index */
    if (button_ind >= get_buttons_num_ll())
        return false;

    struct ButtonStat *button_stat_ptr = &buttons_stats_arr[button_ind];
    struct ButtonConf *button_ptr = &buttons[button_ind];
    if (HAL_GPIO_ReadPin(button_ptr->port, button_ptr->pin) == GPIO_PIN_SET) {
        if (button_stat_ptr->logic_inversed == false) {
            button_stat_ptr->last_state = false;
            return false;
        } else {
            button_stat_ptr->last_state = true;
            return true;
        }
    }
    if (button_stat_ptr->logic_inversed == false) {
        button_stat_ptr->last_state = true;
        return true;
    } else {
        button_stat_ptr->last_state = false;
        return false;
    }
}

uint32_t get_button_press_num(uint8_t button_ind) {
    /* Check button index */
    if (button_ind >= get_buttons_num_ll())
        return 0;

    struct ButtonStat *button_stat_ptr = &buttons_stats_arr[button_ind];
    uint32_t press_num = button_stat_ptr->press_number;
    button_stat_ptr->press_number = 0;
    return press_num;
}

void button_exti_callback(uint8_t button_ind) {
    struct ButtonStat *button_stat_ptr = &buttons_stats_arr[button_ind];
    /* Check EXTI front */
    if (get_button_state_ll(button_ind) == false) {
        /* Falling edge */
        if (button_stat_ptr->logic_inversed == false) {
            button_stat_ptr->press_number++;
            button_stat_ptr->last_state = false;
        }
    } else {
        /* Rising edge */
        if (button_stat_ptr->logic_inversed == true) {
            button_stat_ptr->press_number++;
            button_stat_ptr->last_state = true;
        }
    }
}

void button_exti_irq_callback() {
    uint8_t buttons_num = (uint8_t)get_buttons_num_ll();
    for (uint8_t i = 0; i < buttons_num; i++) {
        if (__HAL_GPIO_EXTI_GET_IT(buttons[i].pin) != 0x00U) {
            /* IRQ line detected */
            if (get_button_state_ll(i) != buttons_stats_arr[i].last_state) {
                __HAL_GPIO_EXTI_CLEAR_IT(buttons[i].pin);
                button_exti_callback(i);
            }
        }
    }
}
