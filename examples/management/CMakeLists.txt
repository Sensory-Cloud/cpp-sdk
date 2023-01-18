# cmake build file for SensoryCloud management service examples.
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

cmake_minimum_required(VERSION 3.11)
project(sensory-cloud-management-examples)

# Fetch the sensorycloud SDK and compile it as part of this project.
include(FetchContent)
FetchContent_Declare(sensorycloud
    URL https://codeload.github.com/Sensory-Cloud/cpp-sdk/tar.gz/refs/tags/v0.17.5
)
FetchContent_MakeAvailable(sensorycloud)

include_directories(dep)

add_executable(management management.cpp)
target_link_libraries(management PRIVATE sensorycloud)

add_executable(management_callback management_callback.cpp)
target_link_libraries(management_callback PRIVATE sensorycloud)

add_executable(management_async management_async.cpp)
target_link_libraries(management_async PRIVATE sensorycloud)

add_executable(apitool apitool.cpp)
target_link_libraries(apitool PRIVATE sensorycloud)
