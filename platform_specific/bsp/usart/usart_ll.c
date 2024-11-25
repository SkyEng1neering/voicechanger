#include "usart_ll.h"

static struct UartInstance platform_uarts[UARTS_PLATFORM_NUM_MAX] = {0};

static bool uart_array_init_flag = false;

static struct UartHwConf uart1_config = {
        /* Rx pin config */
        {
            GPIOB,                    //GPIO port struct pointer
            &RCC->AHB4ENR,            //Clock en/dis register pointer
            RCC_AHB4ENR_GPIOBEN_Pos,  //Port enable bit position
            GPIO_PIN_15,              //GPIO pin
            GPIO_AF4_USART1           //GPIO alternate function mode
        },
        /* Tx pin config */
        {
            GPIOB,                    //GPIO port struct pointer
            &RCC->AHB4ENR,            //Clock en/dis register pointer
            RCC_AHB4ENR_GPIOBEN_Pos,  //Port enable bit position
            GPIO_PIN_14,              //GPIO pin
            GPIO_AF4_USART1           //GPIO alternate function mode
        },
        /* UART config */
        {
            USART1,                   //UART instance
            DMA1_Stream0,             //Rx DMA stream
            DMA_REQUEST_USART1_RX,    //UART Rx DMA request
            DMA1_Stream1,             //Tx DMA stream
            DMA_REQUEST_USART1_TX,    //UART Tx DMA request
            &RCC->AHB1ENR,            //DMA clock en/dis register pointer
            RCC_AHB1ENR_DMA1EN_Pos,   //DMA clock enable bit position
            &RCC->APB2ENR,            //UART clock en/dis register pointer
            RCC_APB2ENR_USART1EN_Pos, //UART clock enable bit position
            USART1_IRQn,              //UART interrupt
            5,                        //UART interrupt priority
            DMA1_Stream0_IRQn,        //Rx DMA stream interrupt
            USART_IRQ_PRIO,           //Rx DMA stream irq priority
            DMA1_Stream1_IRQn,        //Tx DMA stream interrupt
            USART_IRQ_PRIO            //Tx DMA stream irq priority
        }
};

static void reg_uart(enum UartIDs uart_id, struct UartHwConf *uart_struct_ptr) {
    platform_uarts[uart_id].hw_info = *uart_struct_ptr;
    platform_uarts[uart_id].struct_init_flag = true;
}

static void init_uarts_array() {
    reg_uart(UART_1, &uart1_config);

    uart_array_init_flag = true;
}

bool uart_deinit_ll(enum UartIDs uart_id) {
    if (uart_array_init_flag == false) {
        init_uarts_array();
    }

    struct UartInstance *uart_instance = get_uart_instance(uart_id);
    if (uart_instance == NULL) {
        return false;
    }

    /* Disable UART clocking */
    *uart_instance->hw_info.uart.uart_clk_reg_ptr &=~ (1 << uart_instance->hw_info.uart.uart_clk_en_bit_pos);

    /* Deinit uart pins */
    HAL_GPIO_DeInit(uart_instance->hw_info.pin_rx.port, uart_instance->hw_info.pin_rx.pin);
    HAL_GPIO_DeInit(uart_instance->hw_info.pin_tx.port, uart_instance->hw_info.pin_tx.pin);

    /* Deinit uart DMA */
    HAL_DMA_DeInit(&uart_instance->hal_dma_rx_handler);
    HAL_DMA_DeInit(&uart_instance->hal_dma_tx_handler);

    return true;
}

bool uart_init_ll(enum UartIDs uart_id, enum UartModes mode, uint32_t baudrate,
        bool logic_inv, uint8_t *rx_buf_ptr, uint32_t rx_buf_len, void (*idle_cb_ptr)(uint32_t)) {
    if (uart_array_init_flag == false) {
        init_uarts_array();
    }

    struct UartInstance *uart_instance = get_uart_instance(uart_id);
    if (uart_instance == NULL) {
        return false;
    }

    /* Register RX buffer */
    uart_instance->rx_buf_ptr = rx_buf_ptr;
    uart_instance->rx_buf_size = rx_buf_len;

    /* Register idle line callback */
    uart_instance->idle_cb_ptr = idle_cb_ptr;

    /* Enable GPIO, DMA and UART clocking */
    *uart_instance->hw_info.pin_rx.rcc_reg_ptr |= (1 << uart_instance->hw_info.pin_rx.port_en_bit_pos);
    *uart_instance->hw_info.pin_tx.rcc_reg_ptr |= (1 << uart_instance->hw_info.pin_tx.port_en_bit_pos);
    *uart_instance->hw_info.uart.uart_clk_reg_ptr |= (1 << uart_instance->hw_info.uart.uart_clk_en_bit_pos);
    *uart_instance->hw_info.uart.dma_clk_reg_ptr |= (1 << uart_instance->hw_info.uart.dma_clk_en_bit_pos);

    /* Configure GPIO in UART mode */
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    GPIO_InitStruct.Pin = uart_instance->hw_info.pin_rx.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = uart_instance->hw_info.pin_rx.alternate_function;
    HAL_GPIO_Init(uart_instance->hw_info.pin_rx.port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = uart_instance->hw_info.pin_tx.pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = uart_instance->hw_info.pin_tx.alternate_function;
    HAL_GPIO_Init(uart_instance->hw_info.pin_tx.port, &GPIO_InitStruct);

    /* Configure UART Rx DMA */
    uart_instance->hal_dma_rx_handler.Instance = uart_instance->hw_info.uart.dma_rx_instance;
    uart_instance->hal_dma_rx_handler.Init.Request = uart_instance->hw_info.uart.dma_rx_request;
    uart_instance->hal_dma_rx_handler.Init.Direction = DMA_PERIPH_TO_MEMORY;
    uart_instance->hal_dma_rx_handler.Init.PeriphInc = DMA_PINC_DISABLE;
    uart_instance->hal_dma_rx_handler.Init.MemInc = DMA_MINC_ENABLE;
    uart_instance->hal_dma_rx_handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    uart_instance->hal_dma_rx_handler.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    uart_instance->hal_dma_rx_handler.Init.Mode = DMA_CIRCULAR;
    uart_instance->hal_dma_rx_handler.Init.Priority = DMA_PRIORITY_MEDIUM;
    uart_instance->hal_dma_rx_handler.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    /* Initialize DMA */
    if (HAL_DMA_Init(&uart_instance->hal_dma_rx_handler) != HAL_OK) {
        return false;
    }
    __HAL_LINKDMA(&uart_instance->hal_handler, hdmarx, uart_instance->hal_dma_rx_handler);

    /* Configure UART Tx DMA */
    uart_instance->hal_dma_tx_handler.Instance = uart_instance->hw_info.uart.dma_tx_instance;
    uart_instance->hal_dma_tx_handler.Init.Request = uart_instance->hw_info.uart.dma_tx_request;
    uart_instance->hal_dma_tx_handler.Init.Direction = DMA_MEMORY_TO_PERIPH;
    uart_instance->hal_dma_tx_handler.Init.PeriphInc = DMA_PINC_DISABLE;
    uart_instance->hal_dma_tx_handler.Init.MemInc = DMA_MINC_ENABLE;
    uart_instance->hal_dma_tx_handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    uart_instance->hal_dma_tx_handler.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    uart_instance->hal_dma_tx_handler.Init.Mode = DMA_NORMAL;
    uart_instance->hal_dma_tx_handler.Init.Priority = DMA_PRIORITY_MEDIUM;
    uart_instance->hal_dma_tx_handler.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    /* Initialize DMA */
    if (HAL_DMA_Init(&uart_instance->hal_dma_tx_handler) != HAL_OK) {
        return false;
    }
    __HAL_LINKDMA(&uart_instance->hal_handler, hdmatx, uart_instance->hal_dma_tx_handler);

    /* Enable interrupts */
//	HAL_NVIC_SetPriority(uart_instance->hw_info.uart.dma_rx_irq_id, uart_instance->hw_info.uart.dma_rx_irq_prio, 0);
//	HAL_NVIC_EnableIRQ(uart_instance->hw_info.uart.dma_rx_irq_id);
    HAL_NVIC_SetPriority(uart_instance->hw_info.uart.dma_tx_irq_id, uart_instance->hw_info.uart.dma_tx_irq_prio, 0);
    HAL_NVIC_EnableIRQ(uart_instance->hw_info.uart.dma_tx_irq_id);
    HAL_NVIC_SetPriority(uart_instance->hw_info.uart.irq_id, uart_instance->hw_info.uart.irq_prio, 0);
    HAL_NVIC_EnableIRQ(uart_instance->hw_info.uart.irq_id);

    /* Configure UART */
    uart_instance->hal_handler.Instance = uart_instance->hw_info.uart.uart_instance;
    uart_instance->hal_handler.Init.BaudRate = baudrate;

    switch (mode) {
        case UART_8N1:
            uart_instance->hal_handler.Init.WordLength = UART_WORDLENGTH_8B;
            uart_instance->hal_handler.Init.StopBits = UART_STOPBITS_1;
            uart_instance->hal_handler.Init.Parity = UART_PARITY_NONE;
            break;
        case UART_8N2:
            uart_instance->hal_handler.Init.WordLength = UART_WORDLENGTH_8B;
            uart_instance->hal_handler.Init.StopBits = UART_STOPBITS_2;
            uart_instance->hal_handler.Init.Parity = UART_PARITY_NONE;
            break;
        case UART_8E1:
            uart_instance->hal_handler.Init.WordLength = UART_WORDLENGTH_8B;
            uart_instance->hal_handler.Init.StopBits = UART_STOPBITS_1;
            uart_instance->hal_handler.Init.Parity = UART_PARITY_EVEN;
            break;
        case UART_8E2:
            uart_instance->hal_handler.Init.WordLength = UART_WORDLENGTH_8B;
            uart_instance->hal_handler.Init.StopBits = UART_STOPBITS_2;
            uart_instance->hal_handler.Init.Parity = UART_PARITY_EVEN;
            break;
        case UART_8O1:
            uart_instance->hal_handler.Init.WordLength = UART_WORDLENGTH_8B;
            uart_instance->hal_handler.Init.StopBits = UART_STOPBITS_1;
            uart_instance->hal_handler.Init.Parity = UART_PARITY_ODD;
            break;
        case UART_8O2:
            uart_instance->hal_handler.Init.WordLength = UART_WORDLENGTH_8B;
            uart_instance->hal_handler.Init.StopBits = UART_STOPBITS_2;
            uart_instance->hal_handler.Init.Parity = UART_PARITY_ODD;
            break;
        default:
            uart_instance->hal_handler.Init.WordLength = UART_WORDLENGTH_8B;
            uart_instance->hal_handler.Init.StopBits = UART_STOPBITS_1;
            uart_instance->hal_handler.Init.Parity = UART_PARITY_NONE;
            break;
    }

    uart_instance->hal_handler.Init.Mode = UART_MODE_TX_RX;
    uart_instance->hal_handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart_instance->hal_handler.Init.OverSampling = UART_OVERSAMPLING_16;
    uart_instance->hal_handler.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    uart_instance->hal_handler.Init.ClockPrescaler = UART_PRESCALER_DIV1;

    if (logic_inv == true) {
        uart_instance->hal_handler.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_TXINVERT_INIT | UART_ADVFEATURE_RXINVERT_INIT;
        uart_instance->hal_handler.AdvancedInit.TxPinLevelInvert = UART_ADVFEATURE_TXINV_ENABLE;
        uart_instance->hal_handler.AdvancedInit.RxPinLevelInvert = UART_ADVFEATURE_RXINV_ENABLE;
    } else {
        uart_instance->hal_handler.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    }

    if (HAL_UART_Init(&uart_instance->hal_handler) != HAL_OK) {
        return false;
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&uart_instance->hal_handler, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK) {
        return false;
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&uart_instance->hal_handler, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK) {
        return false;
    }
    if (HAL_UARTEx_DisableFifoMode(&uart_instance->hal_handler) != HAL_OK) {
        return false;
    }

    /* Enable idle line detection interrupt */
    if (uart_instance->idle_cb_ptr != NULL) {
        SET_BIT(uart_instance->hal_handler.Instance->CR1, USART_CR1_IDLEIE);
        SET_BIT(uart_instance->hal_handler.Instance->ICR, USART_ICR_IDLECF);
    }

    /* Start data receiving via DMA */
    if (HAL_UART_Receive_DMA(&uart_instance->hal_handler, uart_instance->rx_buf_ptr, (uint16_t)uart_instance->rx_buf_size) != HAL_OK) {
        return false;
    }

    return true;
}

struct UartInstance* get_uart_instance(enum UartIDs uart_id) {
    /* Check if given uart index is out of range, or uninitialized */
    if ((uart_id >= UARTS_PLATFORM_NUM_MAX)	|| (platform_uarts[uart_id].struct_init_flag != true)) {
        return NULL;
    }
    return &platform_uarts[uart_id];
}
