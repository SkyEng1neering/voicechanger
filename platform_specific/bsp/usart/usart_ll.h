#ifndef BSP_USART_USART_LL_H_
#define BSP_USART_USART_LL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "stm32h7xx.h"

#define USART1_BAUDRATE             (921600U)
#define USART1_RX_BUF_SIZE          128

#define USART_IRQ_PRIO              4
#define USART_SEND_TIMEOUT_MS       1000
#define UARTS_PLATFORM_NUM_MAX      10

enum UartIDs {
    UART_1 = 0,
    UART_2,
    UART_3,
    UART_4,
    UART_6,
    UART_7,
    UART_8
};

enum UartModes {
    UART_8N1 = 0,
    UART_8N2,
    UART_8E1,
    UART_8E2,
    UART_8O1,
    UART_8O2
};

struct UartPinInfo {
    GPIO_TypeDef *port;
    volatile uint32_t *rcc_reg_ptr;
    uint32_t port_en_bit_pos;
    uint32_t pin;
    uint8_t alternate_function;
};

struct UartInfo {
    USART_TypeDef *uart_instance;
    DMA_Stream_TypeDef *dma_rx_instance;
    uint32_t dma_rx_request;
    DMA_Stream_TypeDef *dma_tx_instance;
    uint32_t dma_tx_request;
    volatile uint32_t *dma_clk_reg_ptr;
    uint32_t dma_clk_en_bit_pos;
    volatile uint32_t *uart_clk_reg_ptr;
    uint32_t uart_clk_en_bit_pos;
    IRQn_Type irq_id;
    uint8_t irq_prio;
    IRQn_Type dma_rx_irq_id;
    uint8_t dma_rx_irq_prio;
    IRQn_Type dma_tx_irq_id;
    uint8_t dma_tx_irq_prio;
};

struct UartHwConf {
    struct UartPinInfo pin_rx;
    struct UartPinInfo pin_tx;
    struct UartInfo uart;
};

struct UartInstance {
    struct UartHwConf hw_info;
    UART_HandleTypeDef hal_handler;
    DMA_HandleTypeDef hal_dma_rx_handler;
    DMA_HandleTypeDef hal_dma_tx_handler;
    uint8_t *rx_buf_ptr;
    uint32_t rx_buf_size;
    bool struct_init_flag;
    void (*idle_cb_ptr)(uint32_t);

    /* Position of tag that corresponds to data amount that was read from rx buffer, changes
     * when usart_read function is called
     */
    volatile uint32_t rx_buf_read_pos;
};

bool uart_deinit_ll(enum UartIDs uart_id);
bool uart_init_ll(enum UartIDs uart_id,  enum UartModes mode, uint32_t baudrate,
        bool logic_inv, uint8_t *rx_buf_ptr, uint32_t rx_buf_len, void (*idle_cb_ptr)(uint32_t));
struct UartInstance* get_uart_instance(enum UartIDs uart_id);

#ifdef __cplusplus
}
#endif

#endif /* BSP_USART_USART_LL_H_ */
