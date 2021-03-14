# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "E:/pico-sdk")

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_tableClock C CXX ASM)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

file (GLOB U8G2_SRC "E:/u8g2/csrc/*.h" "E:/u8g2/csrc/*.c")

add_executable(pico_tableClock 
        pico_tableClock.cpp
        picture.cpp
        st7920.cpp
        MicroCtrlSys.cpp
        pico_st7920_pio_driver.cpp
        u8g2_hal_rpi_pico.cpp
        ${U8G2_SRC}
        )

include_directories("E:/u8g2/csrc")

pico_generate_pio_header(pico_tableClock ${CMAKE_CURRENT_LIST_DIR}/st7920_4bp.pio)

pico_set_program_name(pico_tableClock "pico_tableClock")
pico_set_program_version(pico_tableClock "0.1")

pico_enable_stdio_uart(pico_tableClock 1)
pico_enable_stdio_usb(pico_tableClock 0)

# Add the standard library to the build
target_link_libraries(pico_tableClock pico_stdlib hardware_rtc)

# Add any user requested libraries
target_link_libraries(pico_tableClock
        hardware_pio
        )

pico_add_extra_outputs(pico_tableClock)

