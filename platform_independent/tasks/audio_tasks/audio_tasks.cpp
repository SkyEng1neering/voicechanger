#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "log.h"
#include "audio_tasks.h"
#include "dac.h"
#include "pcm_file.h"
#include "timebase_us.h"
#include "debug_gpio.h"
#include "us_comm.h"

#define DAC_EVENT_TASK_MAIN_STACK_SIZE          1024
#define DAC_EVENT_TASK_SLEEP_MAX                0xFFFFFFFFU
#define AUDIO_TASK_MAIN_STACK_SIZE              4096
#define AUDIO_FILE_SAMPLES_ARR_LEN              64000

static osThreadId                               mainTaskHandle;
static uint32_t                                 mainTaskBuffer[AUDIO_TASK_MAIN_STACK_SIZE];
static osStaticThreadDef_t                      mainTaskControlBlock;

static osThreadId                               dacEventTaskHandle;
static uint32_t                                 dacEventTaskBuffer[DAC_EVENT_TASK_MAIN_STACK_SIZE];
static osStaticThreadDef_t                      dacEventTaskControlBlock;
static TaskHandle_t                             task_to_notify = NULL;

static const char*                              main_tag = "AUDIO";
static const char*                              dac_event_tag = "DAC EVENT";

//static float                                    audio_samples_array_fl[AUDIO_FILE_SAMPLES_ARR_LEN];
static int16_t                                  audio_samples_array_i16[AUDIO_FILE_SAMPLES_ARR_LEN];
//static int32_t                                  audio_samples_array_i32[AUDIO_FILE_SAMPLES_ARR_LEN];
enum DacState                                   dac_state;

PCMfile                                         audio_file("test96khz_16bit.wav");

static void audio_main_task(void const * argument) {
    LOGI(main_tag, "%s task started\n", main_tag);
//    us_generate_symbols_array();
//    us_dump_symbols_array();
//    /* Init HW DAC */
    if (dac_init() != true) {
        LOGI(main_tag, "DAC init error, task %s won't be started\n", main_tag);
        return;
    }
//
//    if (audio_file.open() != true) {
//        LOGI(main_tag, "Audio file open error\n");
//    }
//
//    int len = audio_file.readMono(audio_samples_array_i16, AUDIO_FILE_SAMPLES_ARR_LEN);
//    if (len < AUDIO_FILE_SAMPLES_ARR_LEN) {
//        dac_state = DAC_STOPPED_STATE;
//    }
//    dac_send_data_16(audio_samples_array_i16, len);

    for (;;) {
//        dac_send_data_u32((uint32_t*)audio_samples_array_i32, 100);
        osDelay(100);
    }
}

static void dac_event_task(void const * argument) {
    LOGI(dac_event_tag, "%s task started\n", dac_event_tag);

    for (;;) {
        /* Sleep and wait for notification from ISR */
        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            task_to_notify = xTaskGetCurrentTaskHandle();
            const TickType_t max_block_time = pdMS_TO_TICKS(DAC_EVENT_TASK_SLEEP_MAX);
            uint32_t notification_val = ulTaskNotifyTake(pdTRUE, max_block_time);
            if (notification_val != 1) {
                continue;
            } else {
                /* Preload data from file to DAC buffer */
                int len = 0;
                if (dac_state == DAC_HALF_SENT_STATE) {
                    tp2_set();
                    len = audio_file.readMono(&audio_samples_array_i16[0],
                            AUDIO_FILE_SAMPLES_ARR_LEN / 2);

                    if (len < AUDIO_FILE_SAMPLES_ARR_LEN / 2) {
                        LOGI(dac_event_tag, "Audio file ended\n");
                        dac_stop();
                    }
                } else if (dac_state == DAC_FULL_SENT_STATE) {
                    tp2_reset();
                    len = audio_file.readMono(&audio_samples_array_i16[AUDIO_FILE_SAMPLES_ARR_LEN / 2],
                            AUDIO_FILE_SAMPLES_ARR_LEN / 2);

                    if (len < AUDIO_FILE_SAMPLES_ARR_LEN / 2) {
                        LOGI(dac_event_tag, "Audio file ended\n");
                        dac_stop();
                    }
                }
            }
        }
    }
}

void audio_tasks_init() {
    osThreadStaticDef(audio_main_task, audio_main_task, osPriorityNormal, 0,
            AUDIO_TASK_MAIN_STACK_SIZE, mainTaskBuffer, &mainTaskControlBlock);
    mainTaskHandle = osThreadCreate(osThread(audio_main_task), NULL);

    osThreadStaticDef(dac_event_task, dac_event_task, osPriorityAboveNormal, 0,
            DAC_EVENT_TASK_MAIN_STACK_SIZE, dacEventTaskBuffer, &dacEventTaskControlBlock);
    dacEventTaskHandle = osThreadCreate(osThread(dac_event_task), NULL);
}

void dac_half_sent_cb() {
    if (dac_state == DAC_STOPPED_STATE) {
        dac_stop();
    }

    if (task_to_notify == NULL) {
        return;
    }
    BaseType_t higher_prio_task_woken = pdFALSE;

    /* Notify the task that the transmission is complete. */
    dac_state = DAC_HALF_SENT_STATE;
    vTaskNotifyGiveFromISR(task_to_notify, &higher_prio_task_woken);
    task_to_notify = NULL;
    portYIELD_FROM_ISR(higher_prio_task_woken);
    tp1_set();
}

void dac_full_sent_cb() {
    if (dac_state == DAC_STOPPED_STATE) {
        dac_stop();
    }

    if (task_to_notify == NULL) {
        return;
    }
    BaseType_t higher_prio_task_woken = pdFALSE;

    /* Notify the task that the transmission is complete. */
    dac_state = DAC_FULL_SENT_STATE;
    vTaskNotifyGiveFromISR(task_to_notify, &higher_prio_task_woken);
    task_to_notify = NULL;
    portYIELD_FROM_ISR(higher_prio_task_woken);
    tp1_reset();
}
