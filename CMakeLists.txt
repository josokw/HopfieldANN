cmake_minimum_required(VERSION 3.8)

project(HopfiledANN C)

set(C_MAKE_C_STANDARD 99)

include_directories(src)
set( EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin )

file(
    GLOB_RECURSE HOPFIELDANN_SOURCES
    RELATIVE ${CMAKE_SOURCE_DIR}
    src/*.c
    src/*.h
)

add_executable(hopfieldann ${HOPFIELDANN_SOURCES})

install(
    TARGETS hopfieldann
    RUNTIME DESTINATION "."
)

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/data"
    DESTINATION "."
)
