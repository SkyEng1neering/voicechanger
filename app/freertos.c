#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "leds.h"
#include "log.h"
#include "display_ll.h"
#include "audio_tasks.h"

static const char* tag = "TEST";

osThreadId defaultTaskHandle;
uint32_t defaultTaskBuffer[ 512 ];
osStaticThreadDef_t defaultTaskControlBlock;

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

__attribute__((weak)) void configureTimerForRunTimeStats(void) {

}

__attribute__((weak)) unsigned long getRunTimeCounterValue(void) {
    return 0;
}

static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
        StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
    /* place for user code */
}

void MX_FREERTOS_Init(void) {
    osThreadStaticDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512, defaultTaskBuffer, &defaultTaskControlBlock);
    defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);
}

void StartDefaultTask(void const * argument) {
    LOGD(tag, "Test task started\n");

//    display_tasks_init();
    audio_tasks_init();

    if (leds_init_pwm() != true) {
        LOGD(tag, "leds_init_pwm() error\n");
    }

    uint8_t color_value = 0;
    bool val_change_dir = true;

    for (;;) {
        if (color_value == 0) {
            val_change_dir = true;
        }
        if (color_value == 255) {
            val_change_dir = false;
        }

        led_set_val(LED_1, color_value);
        led_set_val(LED_2, 255 - color_value);

        color_value = (val_change_dir == true) ?
                        (color_value + 1) : (color_value - 1);

        osDelay(2);
    }
}
