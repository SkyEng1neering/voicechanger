#ifndef LOG_LIB_INC_PRINTF_CONFIG_H_
#define LOG_LIB_INC_PRINTF_CONFIG_H_


// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// numeric number including padded zeros (dynamically created on stack)
// default: 32 byte
#define PRINTF_NTOA_BUFFER_SIZE    32U

// 'ftoa' conversion buffer size, this must be big enough to hold one converted
// float number including padded zeros (dynamically created on stack)
// default: 32 byte
#define PRINTF_FTOA_BUFFER_SIZE    32U

// support for the floating point type (%f)
// default: activated
#define PRINTF_DISABLE_SUPPORT_FLOAT

// support for exponential floating point notation (%e/%g)
// default: activated
#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL

// define the default floating point precision
// default: 6 digits
#define PRINTF_DEFAULT_FLOAT_PRECISION  6U

// define the largest float suitable to print with %f
// default: 1e9
#define PRINTF_MAX_FLOAT  1e9

// support for the long long types (%llu or %p)
// default: activated
#define PRINTF_DISABLE_SUPPORT_LONG_LONG

// support for the ptrdiff_t type (%t)
// ptrdiff_t is normally defined in <stddef.h> as long or long long type
// default: activated
#define PRINTF_DISABLE_SUPPORT_PTRDIFF_T

#endif /* LOG_LIB_INC_PRINTF_CONFIG_H_ */
