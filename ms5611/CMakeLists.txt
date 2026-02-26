# defining minimum cmake version
cmake_minimum_required(VERSION 3.13)

# including a cmake file to import pico sdk
include(./pico_sdk_import.cmake)

# names the project
project(ms5611)

# initializes the pico sdk
pico_sdk_init()

# add the files which will compile
add_executable(ms5611 main.c)

# including the libraries
# in this case we have only the pico sdk libraries
target_link_libraries(ms5611
            pico_stdlib
            hardware_i2c
)

# enables usb serial bus to serial communication and disables uart 
# but not we use to comminucate with enviroments instead we disable the uart which we use for debuging
pico_enable_stdio_usb(ms5611 1)
pico_enable_stdio_uart(ms5611 0)

# this function will give us the compiled files
pico_add_extra_outputs(ms5611)
