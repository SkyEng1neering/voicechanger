#include <string.h>
#include "usart_io.h"
#include "timebase_us.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

uint32_t baudrate_values[11] = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600, 420000, 2000000, 100000};

/* Initialize uart
 */
bool uart_init(enum UartIDs uart_id, enum UartModes mode, enum UartBaudrates baudrate,
        bool logic_inv, uint8_t *rx_buf_ptr, uint32_t rx_buf_len, void (*idle_cb_ptr)(uint32_t)) {
    return uart_init_ll(uart_id, mode, baudrate_values[baudrate], logic_inv, rx_buf_ptr, rx_buf_len, idle_cb_ptr);
}

/* Deinitialize uart
 */
bool uart_deinit(enum UartIDs uart_id) {
    return uart_deinit_ll(uart_id);
}

/* Non blocking function that returns length of received data that was read from rx buffer
 * at the time, but not more then 'len' parameter
 */
uint32_t uart_read(enum UartIDs uart_id, uint8_t *rx_arr, uint32_t len) {
    struct UartInstance *uart_instance = get_uart_instance(uart_id);
    if (uart_instance == NULL) {
        return 0;
    }

    uint32_t data_was_read = 0;
    uint32_t rx_data_tail_buffered = get_rx_data_tail(uart_id);
    if (uart_instance->rx_buf_read_pos < rx_data_tail_buffered) {
        /* Just read data between read_pos and rx_data_tail */
        data_was_read = (len <= rx_data_tail_buffered - uart_instance->rx_buf_read_pos) ? len : rx_data_tail_buffered - uart_instance->rx_buf_read_pos;
        memcpy(rx_arr, &uart_instance->rx_buf_ptr[uart_instance->rx_buf_read_pos], data_was_read);
        uart_instance->rx_buf_read_pos += data_was_read;
    } else if(uart_instance->rx_buf_read_pos > rx_data_tail_buffered) {
        /* Read data from read_pos to end of usart_rx_buf */
        data_was_read = (len <= uart_instance->rx_buf_size - uart_instance->rx_buf_read_pos) ? len : uart_instance->rx_buf_size - uart_instance->rx_buf_read_pos;
        memcpy(rx_arr, &uart_instance->rx_buf_ptr[uart_instance->rx_buf_read_pos], data_was_read);
        uart_instance->rx_buf_read_pos += data_was_read;

        if (data_was_read < len) {
            /* Append to rx_arr part of data from start circular buffer */
            uint32_t data_to_read = (len <= data_was_read + rx_data_tail_buffered) ? len - data_was_read : rx_data_tail_buffered;
            memcpy(&rx_arr[data_was_read], &uart_instance->rx_buf_ptr[0], data_to_read);
            data_was_read += data_to_read;
            uart_instance->rx_buf_read_pos = data_to_read;
        }
    }
    return data_was_read;
}

/* Blocks until until all requested data copied to rx_arr, or timeout occured
 */
uint32_t uart_read_blocking(enum UartIDs uart_id, uint8_t *rx_arr, uint32_t len, uint32_t timeout_ms) {
    uint32_t start_tick = get_tick_ms();
    uint32_t data_received_total = 0;
    while((data_received_total < len) && (get_tick_ms() - start_tick) < timeout_ms){
        data_received_total += uart_read(uart_id, &rx_arr[data_received_total], len - data_received_total);
    }
    return data_received_total;
}

void uart_write(enum UartIDs uart_id, uint8_t *tx_arr, int len) {
    send_uart_str(uart_id, tx_arr, len);
//    send_uart_str_dma(uart_id, tx_arr, len);
}

void send_uart_str(enum UartIDs uart_id, uint8_t *tx_arr, int len) {
    struct UartInstance *uart_instance = get_uart_instance(uart_id);
    if (uart_instance == NULL) {
        return;
    }
    for (int i = 0; i < len; i++) {
        HAL_UART_Transmit(&uart_instance->hal_handler, &tx_arr[i], 1, USART_SEND_TIMEOUT_MS);
    }
}

void send_uart_str_dma(enum UartIDs uart_id, uint8_t *tx_arr, int len) {
    struct UartInstance *uart_instance = get_uart_instance(uart_id);
    if (uart_instance == NULL) {
        return;
    }

    HAL_UART_Transmit_DMA(&uart_instance->hal_handler, tx_arr, len);
    while (uart_instance->hal_dma_tx_handler.State == HAL_DMA_STATE_BUSY) {
    //        tb_delay_us(10);
    }
    uart_instance->hal_handler.gState = HAL_UART_STATE_READY;
}

void uart_tx_dma_irq_callback(enum UartIDs uart_id) {
    struct UartInstance *uart_instance = get_uart_instance(uart_id);
    if (uart_instance == NULL) {
        return;
    }

    HAL_DMA_IRQHandler(&uart_instance->hal_dma_tx_handler);
}

void uart_irq_callback(enum UartIDs uart_id) {
    struct UartInstance *uart_instance = get_uart_instance(uart_id);
    if (uart_instance == NULL) {
        return;
    }

    if (uart_instance->hal_handler.Instance->ISR & USART_ISR_ORE) {
        /* Rx overrun detected */
        volatile uint8_t data = (uint8_t)uart_instance->hal_handler.Instance->RDR;
        UNUSED(data);
        /* Clear Rx overrun flag */
        SET_BIT(uart_instance->hal_handler.Instance->ICR, USART_ICR_ORECF);
    }
    if (uart_instance->hal_handler.Instance->ISR & USART_ISR_IDLE) {
        /* Idle line detected */
        /* Clear idle line irq flag */
        SET_BIT(uart_instance->hal_handler.Instance->ICR, USART_ICR_IDLECF);
        if (uart_instance->idle_cb_ptr != NULL) {
            uart_instance->idle_cb_ptr(uart_get_data_pending(uart_id));
        }
    }
    if ((uart_instance->hal_handler.Instance->ISR & USART_ISR_NE)
            || (uart_instance->hal_handler.Instance->ISR & USART_ISR_FE)) {
        /* Noise detected */
        SET_BIT(uart_instance->hal_handler.Instance->ICR, USART_ICR_NECF);
        /* Framing error */
        SET_BIT(uart_instance->hal_handler.Instance->ICR, USART_ICR_FECF);
    }
    if (uart_instance->hal_handler.Instance->ISR & USART_ISR_TC) {
        SET_BIT(uart_instance->hal_handler.Instance->ICR, USART_ICR_TCCF);
    }
}

uint32_t uart_get_data_pending(enum UartIDs uart_id) {
    struct UartInstance *uart_instance = get_uart_instance(uart_id);
    if (uart_instance == NULL) {
        return 0;
    }

    uint32_t rx_data_tail_buffered = get_rx_data_tail(uart_id);
    if (rx_data_tail_buffered >= uart_instance->rx_buf_read_pos) {
        return rx_data_tail_buffered - uart_instance->rx_buf_read_pos;
    }
    return uart_instance->rx_buf_size - uart_instance->rx_buf_read_pos + rx_data_tail_buffered;
}

uint32_t get_rx_data_tail(enum UartIDs uart_id) {
    struct UartInstance *uart_instance = get_uart_instance(uart_id);
    if (uart_instance == NULL) {
        return 0;
    }
    return uart_instance->rx_buf_size - ((DMA_Stream_TypeDef*)(uart_instance->hal_dma_rx_handler.Instance))->NDTR;
}

void uart_idle_line_callback(enum UartIDs uart_id) {
    (void)(uart_id);
}


