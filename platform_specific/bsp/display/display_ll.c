#include "display_ll.h"
#include "stm32h7xx.h"
#include "ssd1306.h"

static uint8_t               display_frame_buffer[DISPLAY_BUFFER_SIZE];
I2C_HandleTypeDef     hi2c1;


// Screen object
static SSD1306_t SSD1306;

static void display_set_on(const uint8_t on);
static void display_write_command(uint8_t byte);
static void display_write_commands(uint8_t* data, uint8_t len);
static void display_write_data(uint8_t* buffer, size_t buff_size);

void display_fill_framebuf(SSD1306_COLOR color);

bool display_i2c_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Initializes the peripherals clock
    */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
    PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        return false;
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = DISPLAY_SCL_Pin|DISPLAY_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();

    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x307075B1;//0x307075B1;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        return false;
    }
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_DISABLE) != HAL_OK) {
        return false;
    }
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
        return false;
    }
    return true;
}

void display_flush(int16_t x1, int16_t x2, int16_t y1, int16_t y2, void* color_buffer) {
    /* Divide by 8 */
    uint8_t row1 = y1 / 8;
    uint8_t row2 = y2 / 8;

//    uint8_t conf[] = {
//        OLED_CONTROL_BYTE_CMD_STREAM,
//        OLED_CMD_SET_MEMORY_ADDR_MODE,
//        0x00,
//        OLED_CMD_SET_COLUMN_RANGE,
//        (uint8_t)x1,
//        (uint8_t)x2,
//        OLED_CMD_SET_PAGE_RANGE,
//        row1,
//        row2,
//    };

//    uint8_t conf[] = {
//         0x20,
//         0x00,
//         0x21,
//         (uint8_t)x1,
//         (uint8_t)x2,
//         0x22,
//         row1,
//         row2,
//     };

    display_write_command(0x20); //OLED_CMD_SET_MEMORY_ADDR_MODE
    display_write_command(0x00);
    display_write_command(0x21); //OLED_CMD_SET_COLUMN_RANGE
    display_write_command((uint8_t)x1);
    display_write_command((uint8_t)x2);
    display_write_command(0x22); //OLED_CMD_SET_PAGE_RANGE
    display_write_command(row1);
    display_write_command(row2);
//    display_write_commands(conf, sizeof(conf));
    display_write_data(color_buffer, 128 * (1 + row2 - row1));
//
//    send_data(conf, sizeof(conf));
//    send_pixels(color_buffer, OLED_COLUMNS * (1 + row2 - row1));

    // Write data to each page of RAM. Number of pages
    // depends on the screen height:
    //
    //  * 32px   ==  4 pages
    //  * 64px   ==  8 pages
    //  * 128px  ==  16 pages
//    for(uint8_t i = 0; i < SSD1306_HEIGHT/8; i++) {
//        display_write_command(0xB0 + i); // Set the current RAM page address.
//        display_write_command(0x00 + SSD1306_X_OFFSET_LOWER);
//        display_write_command(0x10 + SSD1306_X_OFFSET_UPPER);
//        display_write_data(&display_frame_buffer[SSD1306_WIDTH*i],SSD1306_WIDTH);
//    }
}

void display_init(void) {

    // Init OLED
    display_set_on(0); //display off

    display_write_command(0x20); //Set Memory Addressing Mode
    display_write_command(0x00); // 00b,Horizontal Addressing Mode; 01b,Vertical Addressing Mode;
                                // 10b,Page Addressing Mode (RESET); 11b,Invalid

    display_write_command(0xB0); //Set Page Start Address for Page Addressing Mode,0-7

#ifdef SSD1306_MIRROR_VERT
    display_write_command(0xC0); // Mirror vertically
#else
    display_write_command(0xC8); //Set COM Output Scan Direction
#endif

    display_write_command(0x00); //---set low column address
    display_write_command(0x10); //---set high column address

    display_write_command(0x40); //--set start line address - CHECK

    display_set_contrast(0xFF);

#ifdef SSD1306_MIRROR_HORIZ
    display_write_command(0xA0); // Mirror horizontally
#else
    display_write_command(0xA1); //--set segment re-map 0 to 127 - CHECK
#endif

#ifdef SSD1306_INVERSE_COLOR
    display_write_command(0xA7); //--set inverse color
#else
    display_write_command(0xA6); //--set normal color
#endif

// Set multiplex ratio.
#if (SSD1306_HEIGHT == 128)
    // Found in the Luma Python lib for SH1106.
    display_write_command(0xFF);
#else
    display_write_command(0xA8); //--set multiplex ratio(1 to 64) - CHECK
#endif

#if (SSD1306_HEIGHT == 32)
    display_write_command(0x1F); //
#elif (SSD1306_HEIGHT == 64)
    display_write_command(0x3F); //
#elif (SSD1306_HEIGHT == 128)
    display_write_command(0x3F); // Seems to work for 128px high displays too.
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    display_write_command(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content

    display_write_command(0xD3); //-set display offset - CHECK
    display_write_command(0x00); //-not offset

    display_write_command(0xD5); //--set display clock divide ratio/oscillator frequency
    display_write_command(0xF0); //--set divide ratio

    display_write_command(0xD9); //--set pre-charge period
    display_write_command(0x22); //

    display_write_command(0xDA); //--set com pins hardware configuration - CHECK
#if (SSD1306_HEIGHT == 32)
    display_write_command(0x02);
#elif (SSD1306_HEIGHT == 64)
    display_write_command(0x12);
#elif (SSD1306_HEIGHT == 128)
    display_write_command(0x12);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    display_write_command(0xDB); //--set vcomh
    display_write_command(0x20); //0x20,0.77xVcc

    display_write_command(0x8D); //--set DC-DC enable
    display_write_command(0x14); //
    display_set_on(1); //--turn on SSD1306 panel

    // Clear screen
    display_fill_framebuf(Black);

    // Flush buffer to screen
    display_update();

    // Set default values for screen object
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;

    SSD1306.Initialized = 1;
}

void display_sleep_in(void) {
//    uint8_t conf[] = {
//        OLED_CONTROL_BYTE_CMD_STREAM,
//        OLED_CMD_DISPLAY_OFF
//    };
//
//    send_data(conf, sizeof(conf));
}

void display_sleep_out(void) {
//    uint8_t conf[] = {
//        OLED_CONTROL_BYTE_CMD_STREAM,
//        OLED_CMD_DISPLAY_ON
//    };
//
//    send_data(conf, sizeof(conf));
}

void display_set_contrast(const uint8_t value) {
    const uint8_t kSetContrastControlRegister = 0x81;
    display_write_command(kSetContrastControlRegister);
    display_write_command(value);
}

void display_fill_framebuf(SSD1306_COLOR color) {
    uint32_t i;

    for(i = 0; i < sizeof(display_frame_buffer); i++) {
        display_frame_buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

void display_update(void) {
    // Write data to each page of RAM. Number of pages
    // depends on the screen height:
    //
    //  * 32px   ==  4 pages
    //  * 64px   ==  8 pages
    //  * 128px  ==  16 pages
    for(uint8_t i = 0; i < SSD1306_HEIGHT/8; i++) {
        display_write_command(0xB0 + i); // Set the current RAM page address.
        display_write_command(0x00 + SSD1306_X_OFFSET_LOWER);
        display_write_command(0x10 + SSD1306_X_OFFSET_UPPER);
        display_write_data(&display_frame_buffer[SSD1306_WIDTH*i],SSD1306_WIDTH);
    }
}

static void display_set_on(const uint8_t on) {
    uint8_t value;
    if (on) {
        value = 0xAF;   // Display on
        SSD1306.DisplayOn = 1;
    } else {
        value = 0xAE;   // Display off
        SSD1306.DisplayOn = 0;
    }
    display_write_command(value);
}

static void display_write_commands(uint8_t* data, uint8_t len) {
    HAL_I2C_Mem_Write(&hi2c1, DISPLAY_I2C_ADDR, 0x00, 1, data, len, HAL_MAX_DELAY);
}

// Send a byte to the command register
static void display_write_command(uint8_t byte) {
    HAL_I2C_Mem_Write(&hi2c1, DISPLAY_I2C_ADDR, 0x00, 1, &byte, 1, HAL_MAX_DELAY);
}

// Send data
static void display_write_data(uint8_t* buffer, size_t buff_size) {
    HAL_I2C_Mem_Write(&hi2c1, DISPLAY_I2C_ADDR, 0x40, 1, buffer, buff_size, HAL_MAX_DELAY);
}

