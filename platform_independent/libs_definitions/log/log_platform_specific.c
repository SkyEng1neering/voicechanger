#include "usart_io.h"
#include "log.h"
#if LOG_USE_RTOS == 1
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define LOG_UART_INCOME_BUFFER_LEN     64

static SemaphoreHandle_t               log_mutex = NULL;
static StaticSemaphore_t               log_mutex_buffer;
static uint8_t                         rx_buf[LOG_UART_INCOME_BUFFER_LEN];

const char* get_active_task_name() {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        return NULL;
    }
    return pcTaskGetName(xTaskGetCurrentTaskHandle());
}

void log_lock() {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        return;
    }
    if (log_mutex == NULL) {
        log_mutex = xSemaphoreCreateMutexStatic(&log_mutex_buffer);
    }
    xSemaphoreTake(log_mutex, portMAX_DELAY);
}

void log_unlock() {
    if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
        return;
    }
    xSemaphoreGive(log_mutex);
}
#endif

void log_data_income_cb(uint32_t data_size) {
//    printf("%lu\n", data_size);
    (void)data_size;
}

void log_platform_init() {
    /* Place here any actions that are needed for logging, maybe open file or init some peripheral */
    uart_init(UART_1, UART_8N1, UART_BAUDRATE_115200, false,
            rx_buf, LOG_UART_INCOME_BUFFER_LEN, NULL);
}

void log_print_arr(char *arr, uint32_t len) {
    uart_write(UART_1, (uint8_t*)arr, (int)len);
}

uint32_t log_get_timestamp() {
    return HAL_GetTick() / 1000;
}

void _putchar(char character) {
    uart_write(UART_1, (uint8_t*)&character, 1);
}
