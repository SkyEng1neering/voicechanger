cmake_minimum_required(VERSION 3.22)

# specify cross-compilers and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# project settings
project(voicechanger C CXX ASM)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

add_compile_options(-mcpu=cortex-m7)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)
add_compile_options(--specs=nano.specs)
add_compile_options(-fstack-usage)
add_compile_options(-mfloat-abi=hard)
add_compile_options(-mthumb)
#add_compile_options(-Wall -Wconversion)

# uncomment to mitigate c++17 absolute addresses warnings
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

# Enable assembler files preprocessing
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
#    add_compile_options(-Os)
endif ()

add_definitions(
        -DDEBUG
        -DUSE_HAL_DRIVER
        -DSTM32H750xx
        -DLOG_USE_CUSTOM_CONF
        -DPRINTF_INCLUDE_CONFIG_H
        -DCMSIS_device_header=<stm32h7xx.h>
)

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32H750VBTX_FLASH.ld)
message(STATUS "Use linker script: " ${LINKER_SCRIPT})
add_link_options(-mcpu=cortex-m7 -mthumb)
add_link_options(-T ${LINKER_SCRIPT})
add_link_options(--specs=nosys.specs)
add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-static --specs=nano.specs -mfloat-abi=hard -mfpu=fpv5-d16)

#add_link_options(
#        -mcpu=cortex-m7        # Указываем процессорное ядро
#        -mthumb                # Используем инструкции Thumb
#        -T ${LINKER_SCRIPT}    # Указываем скрипт линковщика
#        --specs=nosys.specs    # Для отключения стандартных системных вызовов
#        --specs=nano.specs     # Используем уменьшенную стандартную библиотеку
#        -Wl,-gc-sections       # Удаляем неиспользуемые секции
#        -Wl,--print-memory-usage
#        -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map # Генерация карты памяти
#        -static                # Ссылка на статические библиотеки
#        -mfloat-abi=hard       # Указываем аппаратную поддержку FPU
#        -mfpu=fpv5-d16         # FPU с поддержкой 16 регистров
#        -u _printf_float       # Обеспечиваем поддержку чисел с плавающей точкой в printf
#)

include_directories(
        app
        libs/CMSIS/Device/ST/STM32H7xx/Include
        libs/CMSIS/Include
        libs/FreeRTOS/Source/CMSIS_RTOS
        libs/FreeRTOS/Source/include
        libs/FreeRTOS/Source/portable/GCC/ARM_CM4F
        libs/STM32H7xx_HAL_Driver/Inc
        libs/STM32H7xx_HAL_Driver/Inc/Legacy
        libs/ulogger/inc
        libs/fatfs_lib
        platform_independent/tasks/audio_tasks
        platform_independent/libs_definitions/fatfs
        platform_independent/libs_definitions/log
        platform_independent/custom/pcm_file
        platform_independent/custom/us_comm
        platform_specific/bsp/buttons
        platform_specific/bsp/current_meas
        platform_specific/bsp/dac
        platform_specific/bsp/debug_gpio
        platform_specific/bsp/display
        platform_specific/bsp/leds
        platform_specific/bsp/mics
        platform_specific/bsp/power
        platform_specific/bsp/sd_card
        platform_specific/bsp/timebase_us
        platform_specific/bsp/usart
        platform_specific/system
)

file(GLOB_RECURSE SOURCES "app/*.*" "libs/*.*" "platform_independent/*.*" "platform_specific/*.*")

list(FILTER SOURCES EXCLUDE REGEX ".*/display_tasks/.*")

add_executable(${PROJECT_NAME}.elf ${SOURCES})

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
        COMMAND cp ${BIN_FILE} ${CMAKE_BINARY_DIR}/bin/app.bin
)