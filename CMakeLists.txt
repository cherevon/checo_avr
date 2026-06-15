cmake_minimum_required(VERSION 3.27)

project("checo_avr_repo" LANGUAGES CXX)

# Add libraries listed in this repository
add_subdirectory("lib/checo_avr")

# Add applications listed in this repository
add_subdirectory("app/seven_segment_driver")
