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

# Find SSL, which must be installed externally (it has no CMake script support)
find_package(OpenSSL REQUIRED)
message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")

include(FetchContent)

# Find/Fetch the ProtoBuf dependency
option(USE_SYSTEM_PROTO "Use system-installed ProtoBuf" OFF)
if(USE_SYSTEM_PROTO)  # Use the system-wide ProtoBuf installation
    message(STATUS "Using system-installed ProtoBuf")
    find_package(Protobuf REQUIRED)
    include_directories(${Protobuf_INCLUDE_DIRS})
else()  # Install a local copy of ProtoBuf
    message(STATUS "Using local ProtoBuf")
    FetchContent_Declare(Protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG        v3.13.0
    )
    set(FETCHCONTENT_QUIET OFF)
    FetchContent_MakeAvailable(Protobuf)
endif()

# Find/Fetch the gRPC dependency
option(USE_SYSTEM_GRPC "Use system-installed gRPC" OFF)
if(USE_SYSTEM_GRPC)  # Use the system-wide gRPC installation
    message(STATUS "Using system-installed gRPC")
    find_package(gRPC CONFIG REQUIRED)
    # Create an alias to match how grpc++ will be namespaced when installed
    # locally using FetchContent.
    add_library(grpc++ ALIAS gRPC::grpc++)
else()  # Install a local copy of gRPC
    message(STATUS "Using local gRPC")
    FetchContent_Declare(gRPC
        GIT_REPOSITORY https://github.com/grpc/grpc
        GIT_TAG        v1.42.0
    )
    set(FETCHCONTENT_QUIET OFF)
    FetchContent_MakeAvailable(gRPC)
endif()

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
target_link_libraries(sensorycloud PUBLIC OpenSSL::SSL)
target_link_libraries(sensorycloud PUBLIC OpenSSL::Crypto)
# Link with the gRPC library
target_link_libraries(sensorycloud PRIVATE grpc++)

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
    list(APPEND TEST_BINARIES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BINARY_NAME})
endforeach()
# Create a combined target to compile all of the test codes
add_custom_target(tests ALL DEPENDS ${TEST_BINARIES})

# --- Benchmarks -------------------------------------------------------------

# TODO

# --- Examples ---------------------------------------------------------------

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(hello_cloud examples/hello_cloud.cpp)
target_link_libraries(hello_cloud PRIVATE sensorycloud)

add_executable(opencv_face_auth examples/opencv/face_auth.cpp)
target_link_libraries(opencv_face_auth PRIVATE sensorycloud)
target_link_libraries(opencv_face_auth PRIVATE ${OpenCV_LIBS})

# # Find the source files for the test code to compile iteratively
# file(GLOB_RECURSE EXAMPLE_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/examples/*.cpp)
# foreach(EXAMPLE_SOURCE IN ITEMS ${EXAMPLE_SOURCES})
#     # Convert the path name to an executable name to build.
#     string(REPLACE "${PROJECT_SOURCE_DIR}/examples/" "example_" BINARY_NAME ${EXAMPLE_SOURCE})
#     string(REPLACE "/" "_" BINARY_NAME ${BINARY_NAME})
#     string(REPLACE ".cpp" "" BINARY_NAME ${BINARY_NAME})
#     # Create the binary and link it with the required libraries
#     add_executable(${BINARY_NAME} ${EXAMPLE_SOURCE})
#     target_link_libraries(${BINARY_NAME} PRIVATE sensorycloud)
#     list(APPEND EXAMPLE_BINARIES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BINARY_NAME})
# endforeach()
# # Create a combined target to compile all of the test codes
# add_custom_target(examples ALL DEPENDS ${EXAMPLE_BINARIES})
