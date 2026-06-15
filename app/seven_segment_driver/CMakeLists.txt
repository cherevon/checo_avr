cmake_minimum_required(VERSION 3.23)

project("seven_segment_driver" LANGUAGES CXX)

# Microcontroller model and its frequency
set(MCU "attiny13a")
set(F_CPU "9600000UL")

# Compiler configuration
add_compile_options(
    -mmcu=${MCU}
    -DF_CPU=${F_CPU}
    -Os 
    -Wall 
    -Wextra
    -ffunction-sections 
    -fdata-sections
    -fno-exceptions
    -fno-rtti
)

# Linker configuration
add_link_options(
    -mmcu=${MCU}
    -Wl,--gc-sections
    -Wl,--relax
)

# Project target
add_executable(${PROJECT_NAME}
    "src/main.cpp"
)

# Target compile settings
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE checo::avr)

# Generate .hex-file
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex -R .eeprom ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
)

# Print size of the firmware
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
)