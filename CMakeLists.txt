cmake_minimum_required(VERSION 3.15)

project(HopfieldANN
        DESCRIPTION "A Hopfield Artificial Neural Network implementation in C"
        LANGUAGES C CXX
        )

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library (static) for testing
set(LIB_SOURCE_FILES
    src/HopfieldContext.c
    src/HopfieldCalc.c
    src/HopfieldIO.c
    src/HopfieldUtil.c
)

add_library(hopfield_lib STATIC ${LIB_SOURCE_FILES})
target_include_directories(hopfield_lib PUBLIC src)

if(NOT MSVC)
    target_link_libraries(hopfield_lib PRIVATE m)
endif()

if(MSVC)
    target_compile_options(hopfield_lib PRIVATE /W4)
else()
    target_compile_options(hopfield_lib PRIVATE
        -Wall
        -Wextra
        -Wconversion
        -Werror=implicit-function-declaration
    )
endif()

# Main executable
add_executable(hopfieldann src/main.c src/cli.c)
target_link_libraries(hopfieldann PRIVATE hopfield_lib)
target_include_directories(hopfieldann PRIVATE src)

set_target_properties(hopfieldann PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)

install(TARGETS hopfieldann RUNTIME DESTINATION ".")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/data" DESTINATION ".")

# Test target
enable_testing()
find_package(GTest REQUIRED)

set(TEST_SOURCE_FILES
    test/testHopfieldCalc.cpp
)

add_executable(hopfield_test ${TEST_SOURCE_FILES})
target_link_libraries(hopfield_test GTest::gtest_main hopfield_lib)
target_include_directories(hopfield_test PRIVATE src)

add_test(NAME HopfieldTest COMMAND hopfield_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

# CLI contract tests drive the binary through a bash harness (needs a Unix shell)
if(NOT WIN32)
    add_test(NAME CliContract
             COMMAND bash ${PROJECT_SOURCE_DIR}/testScripts/cli_contract_test.sh
                         $<TARGET_FILE:hopfieldann>
             WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
