# cmake build file for C++ Sensory Cloud.
#
# Copyright 2021 Sensory, Inc.
#
# Author: Christian Kauten (ckauten@sensoryinc.com)
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

# Version 3.11 is required for the FetchContent features that are used here.
cmake_minimum_required(VERSION 3.11)
# Assign the name of the project.
project(sensory-cloud-cpp-sdk)

# Include the common files for finding/compiling Protobuf and gRPC.
include(cmake/common.cmake)
include(FetchContent)

# Setup the optional arguments for augmenting the compilation of the library.
option(SENSORY_CLOUD_BUILD_TESTS "Build test cases" OFF)
option(SENSORY_CLOUD_GENERATE_PROTO "Generate proto header and source files" OFF)
option(SENSORY_CLOUD_BUILD_SECURE_CREDENTIAL_STORE "Build SecureCredentialStore" OFF)

# --- Proto Files ------------------------------------------------------------

if (SENSORY_CLOUD_BUILD_TESTS)
    set(GRPC_OUT "generate_mock_code=true:")
else()
    set(GRPC_OUT "")
endif()

# Iterate over the proto files to compile them with protoc. The proto files are
# expected to be in the ./proto directory. Generated code (headers and source)
# will go to the ./include/sensorycloud/generated directory.
if(SENSORY_CLOUD_GENERATE_PROTO)
    file(GLOB_RECURSE PROTOS CONFIGURE_DEPENDS proto/*.proto)
    foreach(PROTO IN ITEMS ${PROTOS})
        # Create the protobuf header file
        string(REPLACE ".proto" ".pb.h" PROTO_HDR ${PROTO})
        string(REPLACE "proto/" "include/sensorycloud/generated/" PROTO_HDR ${PROTO_HDR})
        list(APPEND PROTO_HDRS ${PROTO_HDR})
        # Create the protobuf definition file
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
        # Create the command to generate the protobuf and gRPC code
        add_custom_command(
            OUTPUT "${PROTO_SRC}" "${PROTO_HDR}" "${GRPC_SRC}" "${GRPC_HDR}"
            COMMAND ${_PROTOBUF_PROTOC}
            ARGS --grpc_out "${GRPC_OUT}${PROJECT_SOURCE_DIR}/include/sensorycloud/generated"
                --cpp_out "${PROJECT_SOURCE_DIR}/include/sensorycloud/generated"
                --proto_path="${PROJECT_SOURCE_DIR}/proto"
                --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
                -I="${protobuf_SOURCE_DIR}/../src"
                "${PROTO}"
            DEPENDS "${PROTO}"
        )
    endforeach()
else()
    file(GLOB_RECURSE PROTO_HDRS CONFIGURE_DEPENDS include/*.pb.h)
    file(GLOB_RECURSE PROTO_SRCS CONFIGURE_DEPENDS include/*.pb.cc)
    file(GLOB_RECURSE GRPC_HDRS CONFIGURE_DEPENDS include/*.grpc.h)
    file(GLOB_RECURSE GRPC_SRCS CONFIGURE_DEPENDS include/*.grpc.cc)
endif()  # SENSORY_CLOUD_GENERATE_PROTO

# --- Library ----------------------------------------------------------------

# This block is taken from the protobuf CMakeLists.txt and tells the compiler
# where the header files are located. These are necessary for using google proto
# dependencies.
include_directories(
    ${ZLIB_INCLUDE_DIRECTORIES}
    ${protobuf_BINARY_DIR}
    ${protobuf_SOURCE_DIR}/../src
)

# Locate the header and source files for the sensorycloud library.
file(GLOB_RECURSE INCLUDES CONFIGURE_DEPENDS include/*.h include/*.hpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.c src/*.cc src/*.cpp)
# Create the sensorycloud library
add_library(sensorycloud
    ${INCLUDES} ${SOURCES}
    ${PROTO_HDRS} ${PROTO_SRCS}
    ${GRPC_HDRS} ${GRPC_SRCS}
)
target_include_directories(sensorycloud PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    include/sensorycloud include/sensorycloud/generated
)
target_compile_features(sensorycloud PUBLIC cxx_std_11)
set_target_properties(sensorycloud PROPERTIES
    PUBLIC_HEADER include/sensorycloud/sensorycloud.hpp
)
target_link_libraries(sensorycloud PUBLIC
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
)

# --- SecureCredentialStore Implementation -----------------------------------

if (SENSORY_CLOUD_BUILD_SECURE_CREDENTIAL_STORE)
    if (APPLE)
        message(STATUS "Using Apple Keychain implementation of SecureCredentialStore")
        # Target the Keychain library for using the MacOS keychain
        target_compile_definitions(sensorycloud PUBLIC -DKEYCHAIN_MACOS=1)
        # Target the mandatory libraries for using the Keychain in MacOS
        find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
        find_library(SECURITY_LIBRARY Security REQUIRED)
        # Target the link libraries for Keychain usage on MacOS
        target_link_libraries(sensorycloud PRIVATE ${COREFOUNDATION_LIBRARY} ${SECURITY_LIBRARY})
    elseif (UNIX)
        message(STATUS "Using Linux libsecret implementation of SecureCredentialStore")
        # Target the mandatory libraries for using libsecret on Linux
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(LIBSECRET IMPORTED_TARGET libsecret-1)
        target_link_libraries(sensorycloud PUBLIC PkgConfig::LIBSECRET)
    elseif(MSVC)
        message(STATUS "Using Windows crypt32 implementation of SecureCredentialStore")
        # Target the mandatory libraries for using Windows credential locker
        target_link_libraries(sensorycloud PRIVATE crypt32)
    endif()  # Operating System Switch
endif()  # SENSORY_CLOUD_BUILD_SECURE_CREDENTIAL_STORE

# --- Test Cases -------------------------------------------------------------

if (SENSORY_CLOUD_BUILD_TESTS)
    # Install the Catch2 development dependency
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v2.13.7
    )
    FetchContent_MakeAvailable(Catch2)

    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    # Find the source files for the test code to compile iteratively
    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/tests/*.cpp)
    foreach(TEST_SOURCE IN ITEMS ${TEST_SOURCES})
        # Convert the path name to an executable name to build.
        string(REPLACE "${PROJECT_SOURCE_DIR}/tests/" "test_" BINARY_NAME ${TEST_SOURCE})
        string(REPLACE "/" "_" BINARY_NAME ${BINARY_NAME})
        string(REPLACE ".cpp" "" BINARY_NAME ${BINARY_NAME})
        # Create the binary and link it with the required libraries
        add_executable(${BINARY_NAME} ${TEST_SOURCE})
        target_link_libraries(${BINARY_NAME} PRIVATE sensorycloud Catch2::Catch2 gmock)
        list(APPEND TEST_BINARIES ${BINARY_NAME})
    endforeach()
    # Create a combined target to compile all of the test codes
    add_custom_target(tests ALL DEPENDS ${TEST_BINARIES})
endif()  # SENSORY_CLOUD_BUILD_TESTS
