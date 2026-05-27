cmake_minimum_required(VERSION 3.15)

project(HopfieldANN
        DESCRIPTION "A Hopfield Artificial Neural Network implementation in C"
        LANGUAGES C
        )

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCE_FILES
    src/AppInfo.h
    src/Hopfield.c
    src/Hopfield.h
    src/HopfieldCalc.c
    src/HopfieldCalc.h
    src/HopfieldConfig.h
    src/HopfieldIO.c
    src/HopfieldIO.h
    src/main.c
)

add_executable(hopfieldann ${SOURCE_FILES})

target_include_directories(hopfieldann PRIVATE src)

set_target_properties(hopfieldann PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)

if(MSVC)
    target_compile_options(hopfieldann PRIVATE /W4)
else()
    target_compile_options(hopfieldann PRIVATE
        -Wall
        -Wextra
        -Wconversion
        -Werror=implicit-function-declaration
    )
endif()

install(TARGETS hopfieldann RUNTIME DESTINATION ".")

install(DIRECTORY "${PROJECT_SOURCE_DIR}/data" DESTINATION ".")
