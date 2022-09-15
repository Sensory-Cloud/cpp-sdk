# cmake build file for SensoryCloud Applications based on portaudio.
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
    URL https://codeload.github.com/Sensory-Cloud/cpp-sdk/tar.gz/refs/tags/v0.15.2
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

add_executable(validate_event synchronous/validate_event.cpp)
target_link_libraries(validate_event PRIVATE sensorycloud)
target_link_libraries(validate_event PRIVATE portaudio)

add_executable(validate_event_async asynchronous_event/validate_event.cpp)
target_link_libraries(validate_event_async PRIVATE sensorycloud)
target_link_libraries(validate_event_async PRIVATE portaudio)

add_executable(validate_event_callback asynchronous_callback/validate_event.cpp)
target_link_libraries(validate_event_callback PRIVATE sensorycloud)
target_link_libraries(validate_event_callback PRIVATE portaudio)

# --- Transcribe ---------------------------------------------------------------

add_executable(transcribe synchronous/transcribe.cpp)
target_link_libraries(transcribe PRIVATE sensorycloud)
target_link_libraries(transcribe PRIVATE portaudio)

add_executable(transcribe_async asynchronous_event/transcribe.cpp)
target_link_libraries(transcribe_async PRIVATE sensorycloud)
target_link_libraries(transcribe_async PRIVATE portaudio)

add_executable(transcribe_callback asynchronous_callback/transcribe.cpp)
target_link_libraries(transcribe_callback PRIVATE sensorycloud)
target_link_libraries(transcribe_callback PRIVATE portaudio)

# --- Create Enrollment --------------------------------------------------------

add_executable(enroll synchronous/enroll.cpp)
target_link_libraries(enroll PRIVATE sensorycloud)
target_link_libraries(enroll PRIVATE portaudio)

add_executable(enroll_async asynchronous_event/enroll.cpp)
target_link_libraries(enroll_async PRIVATE sensorycloud)
target_link_libraries(enroll_async PRIVATE portaudio)

add_executable(enroll_callback asynchronous_callback/enroll.cpp)
target_link_libraries(enroll_callback PRIVATE sensorycloud)
target_link_libraries(enroll_callback PRIVATE portaudio)

# --- Authenticate -------------------------------------------------------------

add_executable(authenticate synchronous/authenticate.cpp)
target_link_libraries(authenticate PRIVATE sensorycloud)
target_link_libraries(authenticate PRIVATE portaudio)

add_executable(authenticate_async asynchronous_event/authenticate.cpp)
target_link_libraries(authenticate_async PRIVATE sensorycloud)
target_link_libraries(authenticate_async PRIVATE portaudio)

add_executable(authenticate_callback asynchronous_callback/authenticate.cpp)
target_link_libraries(authenticate_callback PRIVATE sensorycloud)
target_link_libraries(authenticate_callback PRIVATE portaudio)

# --- Create Enrolled Event ----------------------------------------------------

add_executable(enroll_event synchronous/enroll_event.cpp)
target_link_libraries(enroll_event PRIVATE sensorycloud)
target_link_libraries(enroll_event PRIVATE portaudio)

add_executable(enroll_event_async asynchronous_event/enroll_event.cpp)
target_link_libraries(enroll_event_async PRIVATE sensorycloud)
target_link_libraries(enroll_event_async PRIVATE portaudio)

add_executable(enroll_event_callback asynchronous_callback/enroll_event.cpp)
target_link_libraries(enroll_event_callback PRIVATE sensorycloud)
target_link_libraries(enroll_event_callback PRIVATE portaudio)

# --- Validate Enrolled Event --------------------------------------------------

add_executable(validate_enrolled_event synchronous/validate_enrolled_event.cpp)
target_link_libraries(validate_enrolled_event PRIVATE sensorycloud)
target_link_libraries(validate_enrolled_event PRIVATE portaudio)

add_executable(validate_enrolled_event_async asynchronous_event/validate_enrolled_event.cpp)
target_link_libraries(validate_enrolled_event_async PRIVATE sensorycloud)
target_link_libraries(validate_enrolled_event_async PRIVATE portaudio)

add_executable(validate_enrolled_event_callback asynchronous_callback/validate_enrolled_event.cpp)
target_link_libraries(validate_enrolled_event_callback PRIVATE sensorycloud)
target_link_libraries(validate_enrolled_event_callback PRIVATE portaudio)
