# cmake build file for C++ SensoryCloud.
#
# Copyright (c) 2023 Sensory, Inc.
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

# Version 3.14 is required for the FetchContent features that are used here.
cmake_minimum_required(VERSION 3.14)
project(sensory-cloud-cpp-sdk VERSION 1.2.3)

# Include the common files for finding/compiling Protobuf and gRPC.
include(cmake/common.cmake)
include(GNUInstallDirs)

# Setup the optional arguments for augmenting the compilation of the library.
option(SENSORY_CLOUD_BUILD_TESTS "Build test cases" OFF)
option(SENSORY_CLOUD_GENERATE_PROTO "Generate proto header and source files" OFF)
option(SENSORY_CLOUD_BUILD_SECURE_CREDENTIAL_STORE "Build SecureCredentialStore" OFF)
option(SENSORY_CLOUD_BUILD_EXAMPLES "Build all example applications" OFF)
option(SENSORY_CLOUD_BUILD_EXAMPLES_AUDIO "Build audio example applications" OFF)
option(SENSORY_CLOUD_BUILD_EXAMPLES_VIDEO "Build video example applications" OFF)
option(SENSORY_CLOUD_BUILD_EXAMPLES_ASSISTANT "Build assistant example applications" OFF)
option(SENSORY_CLOUD_BUILD_EXAMPLES_MANAGEMENT "Build management and admin applications" OFF)

# --- Proto Files ------------------------------------------------------------

# If testing is enabled, specify to the --grpc_out flag of protoc that the mock
# code should be generated.
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
            OUTPUT "${PROTO_SRC}" "${PROTO_HDR}" "${GRPC_SRC}" "${GRPC_HDR}" "${GRPC_MOCKS}"
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
    file(GLOB_RECURSE GRPC_HDRS CONFIGURE_DEPENDS include/*.grpc.pb.h)
    file(GLOB_RECURSE GRPC_SRCS CONFIGURE_DEPENDS include/*.grpc.pb.cc)
endif()  # SENSORY_CLOUD_GENERATE_PROTO

# --- Library ----------------------------------------------------------------

file(GLOB_RECURSE INCLUDES CONFIGURE_DEPENDS include/*.h include/*.hpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.c src/*.cc src/*.cpp)
add_library(sensorycloud SHARED
    ${INCLUDES} ${SOURCES}
    ${PROTO_HDRS} ${PROTO_SRCS}
    ${GRPC_HDRS} ${GRPC_SRCS}
)
target_include_directories(sensorycloud PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    include/sensorycloud include/sensorycloud/generated
    ${protobuf_BINARY_DIR}
    ${protobuf_SOURCE_DIR}/../src
    ${grpc_SOURCE_DIR}/third_party/boringssl-with-bazel/src/include
)
target_compile_features(sensorycloud PUBLIC cxx_std_11)
set_target_properties(sensorycloud PROPERTIES PUBLIC_HEADER include/sensorycloud/sensorycloud.hpp)
set_target_properties(sensorycloud PROPERTIES VERSION ${PROJECT_VERSION})
target_link_libraries(sensorycloud PUBLIC ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

# --- SecureCredentialStore Implementation -----------------------------------

if (SENSORY_CLOUD_BUILD_SECURE_CREDENTIAL_STORE)
    add_compile_definitions(BUILD_SECURE_CREDENTIAL_STORE)
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

# --- Installation -----------------------------------------------------------

# Setup the configuration files for the installation of the library.
include(CMakePackageConfigHelpers)
configure_file(cmake/sensorycloudConfig.cmake.in
  sensorycloudConfig.cmake @ONLY)
write_basic_package_version_file(${PROJECT_BINARY_DIR}/sensorycloudConfigVersion.cmake
  VERSION ${CMAKE_CPP_VERSION}
  COMPATIBILITY AnyNewerVersion)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/sensorycloudConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/sensorycloudConfig.cmake"
    INSTALL_DESTINATION lib/cmake/mylib
)

# Setup the installation targets, exports, and files.
install(TARGETS sensorycloud
    EXPORT sensorycloudTargets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    PUBLIC_HEADER DESTINATION include/sensorycloud COMPONENT Development
    BUNDLE DESTINATION bin COMPONENT Runtime
)
# install(EXPORT sensorycloudTargets DESTINATION lib/cmake/sensorycloud)
install(FILES
    ${PROJECT_BINARY_DIR}/sensorycloudConfig.cmake
    ${PROJECT_BINARY_DIR}/sensorycloudConfigVersion.cmake
  DESTINATION lib/cmake/sensorycloud
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/sensorycloud DESTINATION include)

# --- Test Cases -------------------------------------------------------------

if (SENSORY_CLOUD_BUILD_TESTS)
    include(FetchContent)
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

# --- Examples -----------------------------------------------------------------

if (SENSORY_CLOUD_BUILD_EXAMPLES OR SENSORY_CLOUD_BUILD_EXAMPLES_AUDIO)
    include(FetchContent)
    # Fetch the portaudio library and compile it as part of this project.
    FetchContent_Declare(portaudio
        GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
        GIT_TAG        v19.7.0
    )
    FetchContent_MakeAvailable(portaudio)
    include_directories(${portaudio_BINARY_DIR} ${portaudio_SOURCE_DIR})

    # Fetch the sndfile library and compile it as part of this project.
    FetchContent_Declare(sndfile
        GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
        GIT_TAG        1.1.0
    )
    FetchContent_MakeAvailable(sndfile)

    include_directories(examples/audio/dep)

    # --- Events ---------------------------------------------------------------

    add_executable(examples_audio_validate_event examples/audio/synchronous/validate_event.cpp)
    target_link_libraries(examples_audio_validate_event PRIVATE sensorycloud)
    target_link_libraries(examples_audio_validate_event PRIVATE portaudio)

    add_executable(examples_audio_validate_event_async examples/audio/asynchronous_event/validate_event.cpp)
    target_link_libraries(examples_audio_validate_event_async PRIVATE sensorycloud)
    target_link_libraries(examples_audio_validate_event_async PRIVATE portaudio)

    add_executable(examples_audio_validate_event_callback examples/audio/asynchronous_callback/validate_event.cpp)
    target_link_libraries(examples_audio_validate_event_callback PRIVATE sensorycloud)
    target_link_libraries(examples_audio_validate_event_callback PRIVATE portaudio)

    add_executable(examples_audio_validate_event_file examples/audio/file/validate_event.cpp)
    target_link_libraries(examples_audio_validate_event_file PRIVATE sensorycloud)
    target_link_libraries(examples_audio_validate_event_file PRIVATE sndfile)

    # --- Transcribe -----------------------------------------------------------

    add_executable(examples_audio_transcribe examples/audio/synchronous/transcribe.cpp)
    target_link_libraries(examples_audio_transcribe PRIVATE sensorycloud)
    target_link_libraries(examples_audio_transcribe PRIVATE portaudio)

    add_executable(examples_audio_transcribe_async examples/audio/asynchronous_event/transcribe.cpp)
    target_link_libraries(examples_audio_transcribe_async PRIVATE sensorycloud)
    target_link_libraries(examples_audio_transcribe_async PRIVATE portaudio)

    add_executable(examples_audio_transcribe_callback examples/audio/asynchronous_callback/transcribe.cpp)
    target_link_libraries(examples_audio_transcribe_callback PRIVATE sensorycloud)
    target_link_libraries(examples_audio_transcribe_callback PRIVATE portaudio)

    add_executable(examples_audio_transcribe_file examples/audio/file/transcribe.cpp)
    target_link_libraries(examples_audio_transcribe_file PRIVATE sensorycloud)
    target_link_libraries(examples_audio_transcribe_file PRIVATE sndfile)

    # --- Create Enrollment ----------------------------------------------------

    add_executable(examples_audio_enroll examples/audio/synchronous/enroll.cpp)
    target_link_libraries(examples_audio_enroll PRIVATE sensorycloud)
    target_link_libraries(examples_audio_enroll PRIVATE portaudio)

    add_executable(examples_audio_enroll_async examples/audio/asynchronous_event/enroll.cpp)
    target_link_libraries(examples_audio_enroll_async PRIVATE sensorycloud)
    target_link_libraries(examples_audio_enroll_async PRIVATE portaudio)

    add_executable(examples_audio_enroll_callback examples/audio/asynchronous_callback/enroll.cpp)
    target_link_libraries(examples_audio_enroll_callback PRIVATE sensorycloud)
    target_link_libraries(examples_audio_enroll_callback PRIVATE portaudio)

    add_executable(examples_audio_enroll_file examples/audio/file/enroll.cpp)
    target_link_libraries(examples_audio_enroll_file PRIVATE sensorycloud)
    target_link_libraries(examples_audio_enroll_file PRIVATE sndfile)

    # --- Authenticate ---------------------------------------------------------

    add_executable(examples_audio_authenticate examples/audio/synchronous/authenticate.cpp)
    target_link_libraries(examples_audio_authenticate PRIVATE sensorycloud)
    target_link_libraries(examples_audio_authenticate PRIVATE portaudio)

    add_executable(examples_audio_authenticate_async examples/audio/asynchronous_event/authenticate.cpp)
    target_link_libraries(examples_audio_authenticate_async PRIVATE sensorycloud)
    target_link_libraries(examples_audio_authenticate_async PRIVATE portaudio)

    add_executable(examples_audio_authenticate_callback examples/audio/asynchronous_callback/authenticate.cpp)
    target_link_libraries(examples_audio_authenticate_callback PRIVATE sensorycloud)
    target_link_libraries(examples_audio_authenticate_callback PRIVATE portaudio)

    add_executable(examples_audio_authenticate_file examples/audio/file/authenticate.cpp)
    target_link_libraries(examples_audio_authenticate_file PRIVATE sensorycloud)
    target_link_libraries(examples_audio_authenticate_file PRIVATE sndfile)

    # --- Create Enrolled Event ------------------------------------------------

    add_executable(examples_audio_enroll_event examples/audio/synchronous/enroll_event.cpp)
    target_link_libraries(examples_audio_enroll_event PRIVATE sensorycloud)
    target_link_libraries(examples_audio_enroll_event PRIVATE portaudio)

    add_executable(examples_audio_enroll_event_async examples/audio/asynchronous_event/enroll_event.cpp)
    target_link_libraries(examples_audio_enroll_event_async PRIVATE sensorycloud)
    target_link_libraries(examples_audio_enroll_event_async PRIVATE portaudio)

    add_executable(examples_audio_enroll_event_callback examples/audio/asynchronous_callback/enroll_event.cpp)
    target_link_libraries(examples_audio_enroll_event_callback PRIVATE sensorycloud)
    target_link_libraries(examples_audio_enroll_event_callback PRIVATE portaudio)

    add_executable(examples_audio_enroll_event_file examples/audio/file/enroll_event.cpp)
    target_link_libraries(examples_audio_enroll_event_file PRIVATE sensorycloud)
    target_link_libraries(examples_audio_enroll_event_file PRIVATE sndfile)

    # --- Validate Enrolled Event ----------------------------------------------

    add_executable(examples_audio_validate_enrolled_event examples/audio/synchronous/validate_enrolled_event.cpp)
    target_link_libraries(examples_audio_validate_enrolled_event PRIVATE sensorycloud)
    target_link_libraries(examples_audio_validate_enrolled_event PRIVATE portaudio)

    add_executable(examples_audio_validate_enrolled_event_async examples/audio/asynchronous_event/validate_enrolled_event.cpp)
    target_link_libraries(examples_audio_validate_enrolled_event_async PRIVATE sensorycloud)
    target_link_libraries(examples_audio_validate_enrolled_event_async PRIVATE portaudio)

    add_executable(examples_audio_validate_enrolled_event_callback examples/audio/asynchronous_callback/validate_enrolled_event.cpp)
    target_link_libraries(examples_audio_validate_enrolled_event_callback PRIVATE sensorycloud)
    target_link_libraries(examples_audio_validate_enrolled_event_callback PRIVATE portaudio)

    add_executable(examples_audio_validate_enrolled_event_file examples/audio/file/validate_enrolled_event.cpp)
    target_link_libraries(examples_audio_validate_enrolled_event_file PRIVATE sensorycloud)
    target_link_libraries(examples_audio_validate_enrolled_event_file PRIVATE sndfile)

    # --- Synthesize speech ----------------------------------------------------

    add_executable(examples_audio_synthesize_speech examples/audio/file/synthesize_speech.cpp)
    target_link_libraries(examples_audio_synthesize_speech PRIVATE sensorycloud)

endif()

if (SENSORY_CLOUD_BUILD_EXAMPLES OR SENSORY_CLOUD_BUILD_EXAMPLES_VIDEO)
    # Find the OpenCV library and compile it as part of the project. OpenCV
    # does not support the FetchContent route of installation and must be
    # installed system-wide.
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})

    include_directories(examples/video/dep)

    # --- Authenticate ---------------------------------------------------------

    add_executable(examples_video_authenticate examples/video/synchronous/authenticate.cpp)
    target_link_libraries(examples_video_authenticate PRIVATE sensorycloud)
    target_link_libraries(examples_video_authenticate PRIVATE ${OpenCV_LIBS})

    add_executable(examples_video_authenticate_async examples/video/asynchronous_event/authenticate.cpp)
    target_link_libraries(examples_video_authenticate_async PRIVATE sensorycloud)
    target_link_libraries(examples_video_authenticate_async PRIVATE ${OpenCV_LIBS})

    add_executable(examples_video_authenticate_callback examples/video/asynchronous_callback/authenticate.cpp)
    target_link_libraries(examples_video_authenticate_callback PRIVATE sensorycloud)
    target_link_libraries(examples_video_authenticate_callback PRIVATE ${OpenCV_LIBS})

    # --- Create Enrollment ----------------------------------------------------

    add_executable(examples_video_enroll examples/video/synchronous/enroll.cpp)
    target_link_libraries(examples_video_enroll PRIVATE sensorycloud)
    target_link_libraries(examples_video_enroll PRIVATE ${OpenCV_LIBS})

    add_executable(examples_video_enroll_async examples/video/asynchronous_event/enroll.cpp)
    target_link_libraries(examples_video_enroll_async PRIVATE sensorycloud)
    target_link_libraries(examples_video_enroll_async PRIVATE ${OpenCV_LIBS})

    add_executable(examples_video_enroll_callback examples/video/asynchronous_callback/enroll.cpp)
    target_link_libraries(examples_video_enroll_callback PRIVATE sensorycloud)
    target_link_libraries(examples_video_enroll_callback PRIVATE ${OpenCV_LIBS})

    # --- Validate Liveness ----------------------------------------------------

    add_executable(examples_video_liveness examples/video/synchronous/liveness.cpp)
    target_link_libraries(examples_video_liveness PRIVATE sensorycloud)
    target_link_libraries(examples_video_liveness PRIVATE ${OpenCV_LIBS})

    add_executable(examples_video_liveness_async examples/video/asynchronous_event/liveness.cpp)
    target_link_libraries(examples_video_liveness_async PRIVATE sensorycloud)
    target_link_libraries(examples_video_liveness_async PRIVATE ${OpenCV_LIBS})

    add_executable(examples_video_liveness_callback examples/video/asynchronous_callback/liveness.cpp)
    target_link_libraries(examples_video_liveness_callback PRIVATE sensorycloud)
    target_link_libraries(examples_video_liveness_callback PRIVATE ${OpenCV_LIBS})
endif()

if (SENSORY_CLOUD_BUILD_EXAMPLES OR SENSORY_CLOUD_BUILD_EXAMPLES_MANAGEMENT)
    include_directories(examples/management/dep)

    add_executable(examples_management examples/management/management.cpp)
    target_link_libraries(examples_management PRIVATE sensorycloud)

    add_executable(examples_management_callback examples/management/management_callback.cpp)
    target_link_libraries(examples_management_callback PRIVATE sensorycloud)

    add_executable(examples_management_async examples/management/management_async.cpp)
    target_link_libraries(examples_management_async PRIVATE sensorycloud)
endif()

if (SENSORY_CLOUD_BUILD_EXAMPLES OR SENSORY_CLOUD_BUILD_EXAMPLES_ASSISTANT)
    include_directories(examples/assistant/dep)

    add_executable(examples_assistant examples/assistant/assistant.cpp)
    target_link_libraries(examples_assistant PRIVATE sensorycloud)
endif()
