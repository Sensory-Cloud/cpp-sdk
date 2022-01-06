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

# --- Wake Word ----------------------------------------------------------------

add_executable(wakeword wake_word.cpp)
target_link_libraries(wakeword PRIVATE sensorycloud)
target_link_libraries(wakeword PRIVATE portaudio)

add_executable(wakeword_async wake_word_async.cpp)
target_link_libraries(wakeword_async PRIVATE sensorycloud)
target_link_libraries(wakeword_async PRIVATE portaudio)

add_executable(wakeword_callback wake_word_callback.cpp)
target_link_libraries(wakeword_callback PRIVATE sensorycloud)
target_link_libraries(wakeword_callback PRIVATE portaudio)

# --- Sound ID -----------------------------------------------------------------

add_executable(sound_id sound_id.cpp)
target_link_libraries(sound_id PRIVATE sensorycloud)
target_link_libraries(sound_id PRIVATE portaudio)

add_executable(sound_id_async sound_id_async.cpp)
target_link_libraries(sound_id_async PRIVATE sensorycloud)
target_link_libraries(sound_id_async PRIVATE portaudio)

add_executable(sound_id_callback sound_id_callback.cpp)
target_link_libraries(sound_id_callback PRIVATE sensorycloud)
target_link_libraries(sound_id_callback PRIVATE portaudio)

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

add_executable(biometric_enroll biometric_enroll.cpp)
target_link_libraries(biometric_enroll PRIVATE sensorycloud)
target_link_libraries(biometric_enroll PRIVATE portaudio)

add_executable(biometric_enroll_async biometric_enroll_async.cpp)
target_link_libraries(biometric_enroll_async PRIVATE sensorycloud)
target_link_libraries(biometric_enroll_async PRIVATE portaudio)

add_executable(biometric_enroll_callback biometric_enroll_callback.cpp)
target_link_libraries(biometric_enroll_callback PRIVATE sensorycloud)
target_link_libraries(biometric_enroll_callback PRIVATE portaudio)

# --- Authenticate -------------------------------------------------------------

add_executable(biometric_authenticate biometric_authenticate.cpp)
target_link_libraries(biometric_authenticate PRIVATE sensorycloud)
target_link_libraries(biometric_authenticate PRIVATE portaudio)

add_executable(biometric_authenticate_async biometric_authenticate_async.cpp)
target_link_libraries(biometric_authenticate_async PRIVATE sensorycloud)
target_link_libraries(biometric_authenticate_async PRIVATE portaudio)

add_executable(biometric_authenticate_callback biometric_authenticate_callback.cpp)
target_link_libraries(biometric_authenticate_callback PRIVATE sensorycloud)
target_link_libraries(biometric_authenticate_callback PRIVATE portaudio)
