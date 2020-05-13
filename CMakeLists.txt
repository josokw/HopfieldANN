cmake_minimum_required(VERSION 2.6)
project(HopfieldANN)

# Squash deprecation warnings in Visual Studio
if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
endif (MSVC)

include_directories(src)
set( EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin )

file(
    GLOB_RECURSE HOPFIELDANN_SOURCES
    RELATIVE ${CMAKE_SOURCE_DIR}
    src/*.c
    src/*.h
)

add_executable(hopfieldann ${HOPFIELDANN_SOURCES})
# set_property(TARGET hopfieldann PROPERTY C_STANDARD 11)

install(
    TARGETS hopfieldann
    RUNTIME DESTINATION "."
)

install(
    DIRECTORY "${CMAKE_SOURCE_DIR}/data"
    DESTINATION "."
)
