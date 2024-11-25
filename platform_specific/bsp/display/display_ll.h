#ifndef BSP_DISPLAY_DISPLAY_LL_H_
#define BSP_DISPLAY_DISPLAY_LL_H_

#include <stdbool.h>
#include <stdint.h>

#define DISPLAY_I2C_ADDR                    (0x3C << 1)
#define DISPLAY_SCL_Pin                     GPIO_PIN_6
#define DISPLAY_SCL_GPIO_Port               GPIOB
#define DISPLAY_SDA_Pin                     GPIO_PIN_7
#define DISPLAY_SDA_GPIO_Port               GPIOB

#define DISPLAY_HEIGHT                      64
#define DISPLAY_WIDTH                       128
#define DISPLAY_BUFFER_SIZE                 (DISPLAY_HEIGHT * DISPLAY_WIDTH / 8)

bool display_i2c_init();
void display_flush(int16_t x1, int16_t x2, int16_t y1, int16_t y2, void* color_buffer);
void display_init(void);

void display_sleep_in(void);
void display_sleep_out(void);

void display_set_contrast(const uint8_t value);
void display_update(void);

#endif /* BSP_DISPLAY_DISPLAY_LL_H_ */
