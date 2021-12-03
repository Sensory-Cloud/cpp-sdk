# Copyright 2018 Sensory, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ Sensory Cloud.

cmake_minimum_required(VERSION 3.5.1)

project(sensory-cloud-cpp-sdk)

# Output built binaries to a `build` directory instead of the project root.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build")

# C++11 with gcc >4.9 is required to compile this code-base.
set(CMAKE_CXX_STANDARD 11)

if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

# --- Dependencies -----------------------------------------------------------

find_package(Threads REQUIRED)

option(GRPC_AS_SUBMODULE "Install with gRPC as a submodule" OFF)
option(GRPC_FETCHCONTENT "Install with gRPC using FetchContent" OFF)
option(CMAKE_CROSSCOMPILING "Use system protoc compiler" OFF)

if(GRPC_AS_SUBMODULE)
    # One way to build a projects that uses gRPC is to just include the
    # entire gRPC project tree via "add_subdirectory".
    # This approach is very simple to use, but the are some potential
    # disadvantages:
    # * it includes gRPC's CMakeLists.txt directly into your build script
    #   without and that can make gRPC's internal setting interfere with your
    #   own build.
    # * depending on what's installed on your system, the contents of submodules
    #   in gRPC's third_party/* might need to be available (and there might be
    #   additional prerequisites required to build them). Consider using
    #   the gRPC_*_PROVIDER options to fine-tune the expected behavior.
    #
    # A more robust approach to add dependency on gRPC is using
    # cmake's ExternalProject_Add (see cmake_externalproject/CMakeLists.txt).

    # Include the gRPC's cmake build (normally grpc source code would live
    # in a git submodule called "third_party/grpc", but this example lives in
    # the same repository as gRPC sources, so we just look a few directories up)
    add_subdirectory(../../.. ${CMAKE_CURRENT_BINARY_DIR}/grpc EXCLUDE_FROM_ALL)
    message(STATUS "Using gRPC via add_subdirectory.")

    # After using add_subdirectory, we can now use the grpc targets directly from
    # this build.
    set(_PROTOBUF_LIBPROTOBUF libprotobuf)
    set(_REFLECTION grpc++_reflection)
    if(CMAKE_CROSSCOMPILING)
        find_program(_PROTOBUF_PROTOC protoc)
    else()
        set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
    endif()
    set(_GRPC_GRPCPP grpc++)
    if(CMAKE_CROSSCOMPILING)
        find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
    else()
        set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
    endif()
elseif(GRPC_FETCHCONTENT)
    # Another way is to use CMake's FetchContent module to clone gRPC at
    # configure time. This makes gRPC's source code available to your project,
    # similar to a git submodule.
    message(STATUS "Using gRPC via add_subdirectory (FetchContent).")
    include(FetchContent)
    FetchContent_Declare(grpc
        GIT_REPOSITORY https://github.com/grpc/grpc.git
        GIT_TAG        v1.42.0
    )
    FetchContent_MakeAvailable(grpc)

    # Since FetchContent uses add_subdirectory under the hood, we can use
    # the grpc targets directly from this build.
    set(_PROTOBUF_LIBPROTOBUF libprotobuf)
    set(_REFLECTION grpc++_reflection)
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protoc>)
    set(_GRPC_GRPCPP grpc++)
    if(CMAKE_CROSSCOMPILING)
        find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
    else()
        set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
    endif()
else()
    # This branch assumes that gRPC and all its dependencies are already installed
    # on this system, so they can be located by find_package().

    # Find Protobuf installation
    # Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
    set(protobuf_MODULE_COMPATIBLE TRUE)
    find_package(Protobuf REQUIRED)
    message(STATUS "Using protobuf ${Protobuf_VERSION}")

    set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
    set(_REFLECTION gRPC::grpc++_reflection)
    if(CMAKE_CROSSCOMPILING)
        find_program(_PROTOBUF_PROTOC protoc)
    else()
        set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
    endif()

    # Find gRPC installation
    # Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
    find_package(gRPC CONFIG REQUIRED)
    message(STATUS "Using gRPC ${gRPC_VERSION}")

    set(_GRPC_GRPCPP gRPC::grpc++)
    if(CMAKE_CROSSCOMPILING)
        find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
    else()
        set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
    endif()
endif()

# message(STATUS _PROTOBUF_PROTOC=${_PROTOBUF_PROTOC})
# message(STATUS _GRPC_CPP_PLUGIN_EXECUTABLE=${_GRPC_CPP_PLUGIN_EXECUTABLE})

# Locate the proto files for the sensorycloud library
file(GLOB_RECURSE PROTOS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/proto/*.proto
)

foreach(PROTO IN ITEMS ${PROTOS})
    # Create the proto header file
    string(REPLACE ".proto" ".pb.h" PROTO_HDR ${PROTO})
    string(REPLACE "proto/" "include/sensorycloud/generated/" PROTO_HDR ${PROTO_HDR})
    list(APPEND PROTO_HDRS ${PROTO_HDR})
    # Create the proto definition file
    string(REPLACE ".proto" ".pb.cc" PROTO_SRC ${PROTO})
    string(REPLACE "proto/" "include/sensorycloud/generated/" PROTO_SRC ${PROTO_SRC})
    list(APPEND PROTO_SRCS ${PROTO_SRC})
    # Create the gRPC header file
    string(REPLACE ".proto" ".grpc.pb.h" GRPC_HDR ${PROTO})
    string(REPLACE "proto/" "include/sensorycloud/generated/" GRPC_HDR ${GRPC_HDR})
    list(APPEND GRPC_HDRS ${GRPC_HDR})
    # Create the gRPC definition file
    string(REPLACE ".proto" ".grpc.pb.cc" GRPC_SRC ${PROTO})
    string(REPLACE "proto/" "include/sensorycloud/generated/" GRPC_SRC ${GRPC_SRC})
    list(APPEND GRPC_SRCS ${GRPC_SRC})
    # Create the command to generate the protobuff code
    add_custom_command(
        OUTPUT "${PROTO_SRC}" "${PROTO_HDR}" "${GRPC_SRC}" "${GRPC_HDR}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}/include/sensorycloud/generated"
            --cpp_out "${CMAKE_CURRENT_BINARY_DIR}/include/sensorycloud/generated"
            --proto_path="${CMAKE_CURRENT_BINARY_DIR}/proto"
            --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
            -I="${protobuf_SOURCE_DIR}/../src"
            "${PROTO}"
        DEPENDS "${PROTO}" #always_rebuild
    )
endforeach()

# Dummy output which is never actually produced. Anything that depends on
# this will always be rebuilt. Stack Overflow Reference:
# https://stackoverflow.com/questions/13920072/how-to-always-run-command-when-building-regardless-of-any-dependency
add_custom_command(
    OUTPUT always_rebuild
    COMMAND cmake -E echo
)

# This block is taken from the protobuf CMakeLists.txt and tells the compiler
# where the header files are located. These are necessary for using google proto
# dependencies.
include_directories(
  ${ZLIB_INCLUDE_DIRECTORIES}
  ${protobuf_BINARY_DIR}
  ${protobuf_SOURCE_DIR}/../src)

# TODO Remove these debugging statements
message(STATUS _PROTOBUF_PROTOC=${_PROTOBUF_PROTOC})
message(STATUS protobuf_BINARY_DIR=${protobuf_BINARY_DIR})
message(STATUS protobuf_SOURCE_DIR=${protobuf_SOURCE_DIR})

# --- Library ----------------------------------------------------------------

include_directories("${CMAKE_CURRENT_BINARY_DIR}/include/sensorycloud")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include/sensorycloud/generated")

# Locate the header files for the sensorycloud library
file(GLOB_RECURSE INCLUDES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/include/*.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/*.hpp
)
# message(INCLUDES="${INCLUDES}")

# Locate the definition files for the sensorycloud library
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/src/*.c
    ${CMAKE_CURRENT_BINARY_DIR}/src/*.cc
    ${CMAKE_CURRENT_BINARY_DIR}/src/*.cpp
)
# message(SOURCES="${SOURCES}")

# Create the sensorycloud library
add_library(sensorycloud STATIC)
target_sources(sensorycloud PUBLIC ${INCLUDES} ${SOURCES}
    ${PROTO_SRCS}
    ${PROTO_HDRS}
    ${GRPC_SRCS}
    ${GRPC_HDRS}
)
target_include_directories(sensorycloud PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(sensorycloud PUBLIC cxx_std_11)
set_target_properties(sensorycloud PROPERTIES
    PUBLIC_HEADER include/sensorycloud/sensorycloud.hpp
)
target_link_libraries(sensorycloud PUBLIC
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})

if (APPLE)
    # Target the Keychain library for using the MacOS keychain
    target_compile_definitions(sensorycloud PUBLIC -DKEYCHAIN_MACOS=1)
    # Target the mandatory libraries for using the Keychain in MacOS
    find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
    find_library(SECURITY_LIBRARY Security REQUIRED)
    # Target the link libraries for Keychain usage on MacOS
    target_link_libraries(sensorycloud PRIVATE ${COREFOUNDATION_LIBRARY} ${SECURITY_LIBRARY})
elseif (UNIX)
    message(STATUS Unix)
elseif(MSVC)
    message(STATUS Windows)
endif()

# add_library(sensorycloud::sensorycloud ALIAS sensorycloud)

# --- Test Cases -------------------------------------------------------------

option(BUILD_TESTS "Build tests for ${PROJECT_NAME}" OFF)

if (BUILD_TESTS)

# Install the Catch2 development dependency
include(FetchContent)
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.13.7
)
FetchContent_MakeAvailable(Catch2)

# Find the source files for the test code to compile iteratively
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tests/*.cpp)
foreach(TEST_SOURCE IN ITEMS ${TEST_SOURCES})
    # Convert the path name to an executable name to build.
    string(REPLACE "${CMAKE_CURRENT_BINARY_DIR}/tests/" "test_" BINARY_NAME ${TEST_SOURCE})
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

endif()

# --- Benchmarks -------------------------------------------------------------

option(BUILD_BENCHMARKS "Build benchmarks for ${PROJECT_NAME}" OFF)

if (BUILD_BENCHMARKS)

# TODO

endif()

# --- Examples ---------------------------------------------------------------

option(BUILD_EXAMPLES "Build examples for ${PROJECT_NAME}" OFF)

if (BUILD_EXAMPLES)

# ----- Basic Examples -------------------------------------------------------

add_executable(hello_cloud examples/hello_cloud.cpp)
target_link_libraries(hello_cloud PRIVATE sensorycloud)

add_executable(management examples/management.cpp)
target_link_libraries(management PRIVATE sensorycloud)

add_executable(callback_management examples/callback_management.cpp)
target_link_libraries(callback_management PRIVATE sensorycloud)

# ----- Video Examples -------------------------------------------------------

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(opencv_biometric_authenticate examples/opencv/biometric_authenticate.cpp)
target_link_libraries(opencv_biometric_authenticate PRIVATE sensorycloud)
target_link_libraries(opencv_biometric_authenticate PRIVATE ${OpenCV_LIBS})

add_executable(opencv_biometric_enroll examples/opencv/biometric_enroll.cpp)
target_link_libraries(opencv_biometric_enroll PRIVATE sensorycloud)
target_link_libraries(opencv_biometric_enroll PRIVATE ${OpenCV_LIBS})

add_executable(opencv_liveness examples/opencv/liveness.cpp)
target_link_libraries(opencv_liveness PRIVATE sensorycloud)
target_link_libraries(opencv_liveness PRIVATE ${OpenCV_LIBS})




# add_executable(opencv_async_biometric_authenticate examples/opencv/biometric_async_authenticate.cpp)
# target_link_libraries(opencv_async_biometric_authenticate PRIVATE sensorycloud)
# target_link_libraries(opencv_async_biometric_authenticate PRIVATE ${OpenCV_LIBS})

add_executable(opencv_async_biometric_enroll examples/opencv/async_biometric_enroll.cpp)
target_link_libraries(opencv_async_biometric_enroll PRIVATE sensorycloud)
target_link_libraries(opencv_async_biometric_enroll PRIVATE ${OpenCV_LIBS})

# add_executable(opencv_async_liveness examples/opencv/async_liveness.cpp)
# target_link_libraries(opencv_async_liveness PRIVATE sensorycloud)
# target_link_libraries(opencv_async_liveness PRIVATE ${OpenCV_LIBS})



# add_executable(opencv_callback_biometric_authenticate examples/opencv/callback_biometric_authenticate.cpp)
# target_link_libraries(opencv_callback_biometric_authenticate PRIVATE sensorycloud)
# target_link_libraries(opencv_callback_biometric_authenticate PRIVATE ${OpenCV_LIBS})

add_executable(opencv_callback_biometric_enroll examples/opencv/callback_biometric_enroll.cpp)
target_link_libraries(opencv_callback_biometric_enroll PRIVATE sensorycloud)
target_link_libraries(opencv_callback_biometric_enroll PRIVATE ${OpenCV_LIBS})

# add_executable(opencv_callback_liveness examples/opencv/callback_liveness.cpp)
# target_link_libraries(opencv_callback_liveness PRIVATE sensorycloud)
# target_link_libraries(opencv_callback_liveness PRIVATE ${OpenCV_LIBS})

# ----- Audio Examples -------------------------------------------------------

FetchContent_Declare(portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG        v19.7.0
)
FetchContent_MakeAvailable(portaudio)

include_directories(
  ${portaudio_BINARY_DIR}
  ${portaudio_SOURCE_DIR})

add_executable(paex_record examples/portaudio/paex_record.c)
target_link_libraries(paex_record PRIVATE portaudio)

add_executable(paex_read_write_wire examples/portaudio/paex_read_write_wire.c)
target_link_libraries(paex_read_write_wire PRIVATE portaudio)

add_executable(pa_devs examples/portaudio/pa_devs.c)
target_link_libraries(pa_devs PRIVATE portaudio)

add_executable(portaudio_wakeword examples/portaudio/wake_word.cpp)
target_link_libraries(portaudio_wakeword PRIVATE sensorycloud)
target_link_libraries(portaudio_wakeword PRIVATE portaudio)

add_executable(portaudio_sound_id examples/portaudio/sound_id.cpp)
target_link_libraries(portaudio_sound_id PRIVATE sensorycloud)
target_link_libraries(portaudio_sound_id PRIVATE portaudio)

add_executable(portaudio_transcribe examples/portaudio/transcribe.cpp)
target_link_libraries(portaudio_transcribe PRIVATE sensorycloud)
target_link_libraries(portaudio_transcribe PRIVATE portaudio)

add_executable(portaudio_biometric_enroll examples/portaudio/biometric_enroll.cpp)
target_link_libraries(portaudio_biometric_enroll PRIVATE sensorycloud)
target_link_libraries(portaudio_biometric_enroll PRIVATE portaudio)

add_executable(portaudio_biometric_authenticate examples/portaudio/biometric_authenticate.cpp)
target_link_libraries(portaudio_biometric_authenticate PRIVATE sensorycloud)
target_link_libraries(portaudio_biometric_authenticate PRIVATE portaudio)

endif()
