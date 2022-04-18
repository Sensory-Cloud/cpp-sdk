# cmake build file for Sensory Cloud applications based on OpenCV.
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
project(sensory-cloud-opencv-applications)

# Fetch the sensorycloud SDK and compile it as part of this project.
include(FetchContent)
FetchContent_Declare(sensorycloud
    URL https://codeload.github.com/Sensory-Cloud/cpp-sdk/tar.gz/refs/tags/v0.11.4
)
FetchContent_MakeAvailable(sensorycloud)

# Find the OpenCV library and compile it as part of this project. OpenCV does
# not support the FetchContent route of installation and must be installed
# system-wide.
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(dep)

# --- Authenticate -------------------------------------------------------------

add_executable(authenticate authenticate.cpp)
target_link_libraries(authenticate PRIVATE sensorycloud)
target_link_libraries(authenticate PRIVATE ${OpenCV_LIBS})

add_executable(authenticate_async authenticate_async.cpp)
target_link_libraries(authenticate_async PRIVATE sensorycloud)
target_link_libraries(authenticate_async PRIVATE ${OpenCV_LIBS})

add_executable(authenticate_callback authenticate_callback.cpp)
target_link_libraries(authenticate_callback PRIVATE sensorycloud)
target_link_libraries(authenticate_callback PRIVATE ${OpenCV_LIBS})

# --- Create Enrollment --------------------------------------------------------

add_executable(enroll enroll.cpp)
target_link_libraries(enroll PRIVATE sensorycloud)
target_link_libraries(enroll PRIVATE ${OpenCV_LIBS})

add_executable(enroll_async enroll_async.cpp)
target_link_libraries(enroll_async PRIVATE sensorycloud)
target_link_libraries(enroll_async PRIVATE ${OpenCV_LIBS})

add_executable(enroll_callback enroll_callback.cpp)
target_link_libraries(enroll_callback PRIVATE sensorycloud)
target_link_libraries(enroll_callback PRIVATE ${OpenCV_LIBS})

# --- Validate Liveness --------------------------------------------------------

add_executable(liveness liveness.cpp)
target_link_libraries(liveness PRIVATE sensorycloud)
target_link_libraries(liveness PRIVATE ${OpenCV_LIBS})

add_executable(liveness_async liveness_async.cpp)
target_link_libraries(liveness_async PRIVATE sensorycloud)
target_link_libraries(liveness_async PRIVATE ${OpenCV_LIBS})

add_executable(liveness_callback liveness_callback.cpp)
target_link_libraries(liveness_callback PRIVATE sensorycloud)
target_link_libraries(liveness_callback PRIVATE ${OpenCV_LIBS})
