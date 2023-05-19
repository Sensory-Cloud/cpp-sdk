# cmake build file for SensoryCloud video service examples.
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

cmake_minimum_required(VERSION 3.14)
project(sensory-cloud-video-examples)

# Fetch the sensorycloud SDK and compile it as part of this project.
include(FetchContent)
FetchContent_Declare(sensorycloud
    URL https://codeload.github.com/Sensory-Cloud/cpp-sdk/tar.gz/refs/tags/v1.0.8
)
FetchContent_MakeAvailable(sensorycloud)

# Find the OpenCV library and compile it as part of this project. OpenCV does
# not support the FetchContent route of installation and must be installed
# system-wide.
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(dep)

# --- Authenticate -------------------------------------------------------------

add_executable(authenticate synchronous/authenticate.cpp)
target_link_libraries(authenticate PRIVATE sensorycloud)
target_link_libraries(authenticate PRIVATE ${OpenCV_LIBS})

add_executable(authenticate_async asynchronous_event/authenticate.cpp)
target_link_libraries(authenticate_async PRIVATE sensorycloud)
target_link_libraries(authenticate_async PRIVATE ${OpenCV_LIBS})

add_executable(authenticate_callback asynchronous_callback/authenticate.cpp)
target_link_libraries(authenticate_callback PRIVATE sensorycloud)
target_link_libraries(authenticate_callback PRIVATE ${OpenCV_LIBS})

# --- Enroll -------------------------------------------------------------------

add_executable(enroll synchronous/enroll.cpp)
target_link_libraries(enroll PRIVATE sensorycloud)
target_link_libraries(enroll PRIVATE ${OpenCV_LIBS})

add_executable(enroll_async asynchronous_event/enroll.cpp)
target_link_libraries(enroll_async PRIVATE sensorycloud)
target_link_libraries(enroll_async PRIVATE ${OpenCV_LIBS})

add_executable(enroll_callback asynchronous_callback/enroll.cpp)
target_link_libraries(enroll_callback PRIVATE sensorycloud)
target_link_libraries(enroll_callback PRIVATE ${OpenCV_LIBS})

# --- Liveness -----------------------------------------------------------------

add_executable(liveness synchronous/liveness.cpp)
target_link_libraries(liveness PRIVATE sensorycloud)
target_link_libraries(liveness PRIVATE ${OpenCV_LIBS})

add_executable(liveness_async asynchronous_event/liveness.cpp)
target_link_libraries(liveness_async PRIVATE sensorycloud)
target_link_libraries(liveness_async PRIVATE ${OpenCV_LIBS})

add_executable(liveness_callback asynchronous_callback/liveness.cpp)
target_link_libraries(liveness_callback PRIVATE sensorycloud)
target_link_libraries(liveness_callback PRIVATE ${OpenCV_LIBS})
