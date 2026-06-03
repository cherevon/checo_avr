set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

# Compilers
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)

# Utilities to create binaries (hex, eeprom)
set(CMAKE_OBJCOPY avr-objcopy CACHE FILEPATH "" FORCE)
set(CMAKE_OBJDUMP avr-objdump CACHE FILEPATH "" FORCE)
set(CMAKE_SIZE avr-size CACHE FILEPATH "" FORCE)

# Disable compiler checks
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# Search paths for includes and library avr-libc
set(CMAKE_FIND_ROOT_PATH /usr/lib/avr)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Specific executable suffix
set(CMAKE_EXECUTABLE_SUFFIX ".elf") 
set(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")