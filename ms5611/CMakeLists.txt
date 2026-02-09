cmake_minimum_required(VERSION 3.13)

include(./pico_sdk_import.cmake)

project(ms5611)

pico_sdk_init()

add_executable(ms5611 main.c)

target_link_libraries(ms5611
            pico_stdlib
            hardware_i2c
)

pico_enable_stdio_usb(ms5611 1)
pico_enable_stdio_uart(ms5611 0)

pico_add_extra_outputs(ms5611)