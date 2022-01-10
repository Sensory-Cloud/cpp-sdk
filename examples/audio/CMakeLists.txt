# cmake build file for Sensory Cloud Applications based on portaudio.
#
# Copyright 2021 Sensory, Inc.
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
# Assign the name of the project
project(sensory-cloud-portaudio-applications)

# Fetch the sensorycloud SDK and compile it as part of this project.
include(FetchContent)
FetchContent_Declare(sensorycloud
    GIT_REPOSITORY https://github.com/Sensory-Cloud/cpp-sdk.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(sensorycloud)

# Fetch the portaudio library and compile it as part of this project.
FetchContent_Declare(portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG        v19.7.0
)
FetchContent_MakeAvailable(portaudio)
# Setup the includes for the portaudio library
include_directories(${portaudio_BINARY_DIR} ${portaudio_SOURCE_DIR})

include_directories(dep)

# --- Events -------------------------------------------------------------------

add_executable(validate_event validate_event.cpp)
target_link_libraries(validate_event PRIVATE sensorycloud)
target_link_libraries(validate_event PRIVATE portaudio)

add_executable(validate_event_async validate_event_async.cpp)
target_link_libraries(validate_event_async PRIVATE sensorycloud)
target_link_libraries(validate_event_async PRIVATE portaudio)

add_executable(validate_event_callback validate_event_callback.cpp)
target_link_libraries(validate_event_callback PRIVATE sensorycloud)
target_link_libraries(validate_event_callback PRIVATE portaudio)

# --- Transcribe ---------------------------------------------------------------

add_executable(transcribe transcribe.cpp)
target_link_libraries(transcribe PRIVATE sensorycloud)
target_link_libraries(transcribe PRIVATE portaudio)

add_executable(transcribe_async transcribe_async.cpp)
target_link_libraries(transcribe_async PRIVATE sensorycloud)
target_link_libraries(transcribe_async PRIVATE portaudio)

add_executable(transcribe_callback transcribe_callback.cpp)
target_link_libraries(transcribe_callback PRIVATE sensorycloud)
target_link_libraries(transcribe_callback PRIVATE portaudio)

# --- Create Enrollment --------------------------------------------------------

add_executable(enroll enroll.cpp)
target_link_libraries(enroll PRIVATE sensorycloud)
target_link_libraries(enroll PRIVATE portaudio)

add_executable(enroll_async enroll_async.cpp)
target_link_libraries(enroll_async PRIVATE sensorycloud)
target_link_libraries(enroll_async PRIVATE portaudio)

add_executable(enroll_callback enroll_callback.cpp)
target_link_libraries(enroll_callback PRIVATE sensorycloud)
target_link_libraries(enroll_callback PRIVATE portaudio)

# --- Authenticate -------------------------------------------------------------

add_executable(authenticate authenticate.cpp)
target_link_libraries(authenticate PRIVATE sensorycloud)
target_link_libraries(authenticate PRIVATE portaudio)

add_executable(authenticate_async authenticate_async.cpp)
target_link_libraries(authenticate_async PRIVATE sensorycloud)
target_link_libraries(authenticate_async PRIVATE portaudio)

add_executable(authenticate_callback authenticate_callback.cpp)
target_link_libraries(authenticate_callback PRIVATE sensorycloud)
target_link_libraries(authenticate_callback PRIVATE portaudio)

# --- Create Enrolled Event ----------------------------------------------------

add_executable(enroll_event enroll_event.cpp)
target_link_libraries(enroll_event PRIVATE sensorycloud)
target_link_libraries(enroll_event PRIVATE portaudio)

add_executable(enroll_event_async enroll_event_async.cpp)
target_link_libraries(enroll_event_async PRIVATE sensorycloud)
target_link_libraries(enroll_event_async PRIVATE portaudio)

add_executable(enroll_event_callback enroll_event_callback.cpp)
target_link_libraries(enroll_event_callback PRIVATE sensorycloud)
target_link_libraries(enroll_event_callback PRIVATE portaudio)

# --- Validate Enrolled Event --------------------------------------------------

add_executable(validate_enrolled_event validate_enrolled_event.cpp)
target_link_libraries(validate_enrolled_event PRIVATE sensorycloud)
target_link_libraries(validate_enrolled_event PRIVATE portaudio)

add_executable(validate_enrolled_event_async validate_enrolled_event_async.cpp)
target_link_libraries(validate_enrolled_event_async PRIVATE sensorycloud)
target_link_libraries(validate_enrolled_event_async PRIVATE portaudio)

add_executable(validate_enrolled_event_callback validate_enrolled_event_callback.cpp)
target_link_libraries(validate_enrolled_event_callback PRIVATE sensorycloud)
target_link_libraries(validate_enrolled_event_callback PRIVATE portaudio)