cmake_minimum_required(VERSION 3.27)

project("checo_avr")

# Project target
add_library(${PROJECT_NAME} INTERFACE)
add_library(checo::avr ALIAS ${PROJECT_NAME})

# Project includes
target_include_directories(${PROJECT_NAME} INTERFACE "include")
