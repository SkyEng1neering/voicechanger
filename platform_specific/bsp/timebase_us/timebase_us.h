#ifndef PLATFORM_SPECIFIC_BSP_TIMEBASE_US_TIMEBASE_US_H_
#define PLATFORM_SPECIFIC_BSP_TIMEBASE_US_TIMEBASE_US_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"

bool timebase_us_init();
uint64_t get_tick_us_overflow_cnt();
uint16_t get_tick_us_lo16();
uint64_t get_tick_us();
uint32_t get_tick_ms();
void set_tick_us(uint64_t tick);
void timebase_us_callback();
void tb_delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_SPECIFIC_BSP_TIMEBASE_US_TIMEBASE_US_H_ */
