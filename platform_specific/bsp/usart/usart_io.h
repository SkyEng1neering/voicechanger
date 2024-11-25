#ifndef BSP_USART_USART_IO_H_
#define BSP_USART_USART_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "usart_ll.h"

enum UartBaudrates {
    UART_BAUDRATE_9600 = 0,
    UART_BAUDRATE_19200,
    UART_BAUDRATE_38400,
    UART_BAUDRATE_57600,
    UART_BAUDRATE_115200,
    UART_BAUDRATE_230400,
    UART_BAUDRATE_460800,
    UART_BAUDRATE_921600,
    UART_BAUDRATE_420000,
    UART_BAUDRATE_2000000,
    UART_BAUDRATE_100000
};

bool uart_init(enum UartIDs uart_id, enum UartModes mode, enum UartBaudrates baudrate,
        bool logic_inv, uint8_t *rx_buf_ptr, uint32_t rx_buf_len, void (*idle_cb_ptr)(uint32_t));
bool uart_deinit(enum UartIDs uart_id);
uint32_t uart_read(enum UartIDs uart_id, uint8_t *rx_arr, uint32_t len);
uint32_t uart_read_blocking(enum UartIDs uart_id, uint8_t *rx_arr, uint32_t len, uint32_t timeout_ms);
void uart_write(enum UartIDs uart_id, uint8_t *tx_arr, int len);
void send_uart_str(enum UartIDs uart_id, uint8_t *tx_arr, int len);
void send_uart_str_dma(enum UartIDs uart_id, uint8_t *tx_arr, int len);
void uart_tx_dma_irq_callback(enum UartIDs uart_id);
void uart_irq_callback(enum UartIDs uart_id);
uint32_t get_rx_data_tail(enum UartIDs uart_id);
uint32_t uart_get_data_pending(enum UartIDs uart_id);
void uart_idle_line_callback(enum UartIDs uart_id);

#ifdef __cplusplus
}
#endif

#endif /* BSP_USART_USART_IO_H_ */
