cmake_minimum_required(VERSION 3.15)

project(HopfieldANN 
        DESCRIPTION "A Hopfield Artificial Neural Network implementation in C"      
        LANGUAGES C
        )

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/W4)  # MSVC equivalent of -Wall -Wextra
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wconversion
        -Werror=implicit-function-declaration
    )
endif()

file(
    GLOB_RECURSE HOPFIELDANN_SOURCES
    RELATIVE ${CMAKE_SOURCE_DIR}
    src/*.c
    src/*.h
)

add_executable(hopfieldann ${HOPFIELDANN_SOURCES})

target_include_directories(hopfieldann PRIVATE src)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

install(
    TARGETS hopfieldann
    RUNTIME DESTINATION "."
)

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/data"
    DESTINATION "."
)
