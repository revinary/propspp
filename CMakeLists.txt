cmake_minimum_required(VERSION 3.21)

configure_file(.clangd.in ${CMAKE_SOURCE_DIR}/.clangd @ONLY)

project(props++
    LANGUAGES CXX
    VERSION 0.1
)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)


add_library(${PROJECT_NAME}
    property_base.cpp
    property_base.h
    bindable_value.h
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        .
)

add_subdirectory(test)
