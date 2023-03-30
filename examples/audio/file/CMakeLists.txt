# cmake build file for SensoryCloud audio applications based on file inputs.
#
# Copyright (c) 2023 Sensory, Inc.
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

cmake_minimum_required(VERSION 3.11)
project(sensory-cloud-audio-service-file-examples)

include(FetchContent)

FetchContent_Declare(sensorycloud
    URL https://codeload.github.com/Sensory-Cloud/cpp-sdk/tar.gz/refs/tags/v0.17.4
)
FetchContent_MakeAvailable(sensorycloud)

FetchContent_Declare(sndfile
    GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
    GIT_TAG        1.1.0
)
FetchContent_MakeAvailable(sndfile)

include_directories(dep)

add_executable(transcribe transcribe.cpp)
target_link_libraries(transcribe PRIVATE sensorycloud)
target_link_libraries(transcribe PRIVATE sndfile)

add_executable(synthesize_speech synthesize_speech.cpp)
target_link_libraries(synthesize_speech PRIVATE sensorycloud)

add_executable(authenticate authenticate.cpp)
target_link_libraries(authenticate PRIVATE sensorycloud)
target_link_libraries(authenticate PRIVATE sndfile)

add_executable(enroll enroll.cpp)
target_link_libraries(enroll PRIVATE sensorycloud)
target_link_libraries(enroll PRIVATE sndfile)

add_executable(enroll_event enroll_event.cpp)
target_link_libraries(enroll_event PRIVATE sensorycloud)
target_link_libraries(enroll_event PRIVATE sndfile)

add_executable(validate_enrolled_event validate_enrolled_event.cpp)
target_link_libraries(validate_enrolled_event PRIVATE sensorycloud)
target_link_libraries(validate_enrolled_event PRIVATE sndfile)

add_executable(validate_event validate_event.cpp)
target_link_libraries(validate_event PRIVATE sensorycloud)
target_link_libraries(validate_event PRIVATE sndfile)
