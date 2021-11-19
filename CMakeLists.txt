cmake_minimum_required(VERSION 3.2)
project(sensory-cloud-cpp-sdk)

# Setup the C++ environment
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
add_compile_options(-std=c++11)

# Log the source directory
message(PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

# Output built binaries to a `build` directory instead of the project root
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build")

# --- Dependencies -----------------------------------------------------------

# Find the Protobuf dependency
find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
# Find the gRPC dependency
find_package(OpenSSL REQUIRED)
find_package(gRPC CONFIG REQUIRED)

include(FetchContent)

# # Install the protobuf dependency
# FetchContent_Declare(Protobuf
#     GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
#     GIT_TAG        v3.13.0
# )
# set(FETCHCONTENT_QUIET OFF)
# FetchContent_MakeAvailable(Protobuf)

# # Install the gRPC dependency
# FetchContent_Declare(gRPC
#     GIT_REPOSITORY https://github.com/grpc/grpc
#     GIT_TAG        v1.42.0
# )
# set(FETCHCONTENT_QUIET OFF)
# FetchContent_MakeAvailable(gRPC)

# --- Library ----------------------------------------------------------------

# include_directories("${PROJECT_SOURCE_DIR}/include/sensorycloud")

# Locate the header files for the sensorycloud library
file(GLOB_RECURSE INCLUDES CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/include/*.h
    ${PROJECT_SOURCE_DIR}/include/*.hpp
)
# message(INCLUDES="${INCLUDES}")

# Locate the definition files for the sensorycloud library
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/src/*.c
    ${PROJECT_SOURCE_DIR}/src/*.cc
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)
# message(SOURCES="${SOURCES}")

# Create the sensorycloud library
add_library(sensorycloud STATIC)
target_sources(sensorycloud PRIVATE ${INCLUDES} ${SOURCES})
target_include_directories(sensorycloud PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(sensorycloud PUBLIC cxx_std_11)
set_target_properties(sensorycloud PROPERTIES
    PUBLIC_HEADER include/sensorycloud/sensorycloud.hpp
)
# Link with the Protobuf library
target_link_libraries(sensorycloud PRIVATE ${Protobuf_LIBRARIES})
# Link with the OpenSSL library
target_link_libraries(sensorycloud PRIVATE OpenSSL::SSL)
target_link_libraries(sensorycloud PRIVATE OpenSSL::Crypto)
# Link with the gRPC library
target_link_libraries(sensorycloud PRIVATE gRPC::grpc++)

# add_library(sensorycloud::sensorycloud ALIAS sensorycloud)

# --- Test Cases -------------------------------------------------------------

# Install the Catch2 development dependency
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.13.7
)
FetchContent_MakeAvailable(Catch2)

# Find the source files for the test code to compile iteratively
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/tests/*.cpp)
foreach(TEST_SOURCE IN ITEMS ${TEST_SOURCES})
    # Convert the path name to an executable name to build.
    string(REPLACE "${PROJECT_SOURCE_DIR}/tests/" "test_" BINARY_NAME ${TEST_SOURCE})
    string(REPLACE "/" "_" BINARY_NAME ${BINARY_NAME})
    string(REPLACE ".cpp" "" BINARY_NAME ${BINARY_NAME})
    # Create the binary and link it with the required libraries
    add_executable(${BINARY_NAME} ${TEST_SOURCE})
    target_link_libraries(${BINARY_NAME} PRIVATE sensorycloud)
    target_link_libraries(${BINARY_NAME} PRIVATE Catch2::Catch2)
    list(APPEND BINARY_NAMES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BINARY_NAME})
endforeach()
# Create a combined target to compile all of the test codes
add_custom_target(tests ALL DEPENDS ${BINARY_NAMES})

# --- Benchmarks -------------------------------------------------------------

# TODO

# --- Examples ---------------------------------------------------------------

add_executable(hello examples/hello.cpp)
target_link_libraries(hello PRIVATE sensorycloud)
