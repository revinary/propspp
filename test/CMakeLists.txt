cmake_minimum_required(VERSION 3.21)

project(props++_test
    LANGUAGES CXX
)

enable_testing()

Include(FetchContent)

FetchContent_Declare(Catch2
    GIT_REPOSITORY
        https://github.com/catchorg/Catch2.git
    GIT_TAG
        v3.0.1
)

FetchContent_MakeAvailable(Catch2)

add_executable(${PROJECT_NAME}
    property_test.cpp
)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        props++
        Catch2::Catch2WithMain
)

