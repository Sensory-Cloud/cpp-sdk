# cmake build file for Sensory Cloud audio applications based on file inputs.
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
project(sensory-cloud-audio-file-transcription)

# Fetch the sensorycloud SDK and compile it as part of this project.
include(FetchContent)
FetchContent_Declare(sensorycloud
    URL https://codeload.github.com/Sensory-Cloud/cpp-sdk/tar.gz/refs/tags/v0.12.4
)
FetchContent_MakeAvailable(sensorycloud)

# Add the application to the build chain.
include_directories(dep)
add_executable(transcribe transcribe.cpp)
target_link_libraries(transcribe PRIVATE sensorycloud)
