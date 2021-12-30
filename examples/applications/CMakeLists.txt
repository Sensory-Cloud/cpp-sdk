# cmake build file for Sensory Cloud Voice Assistant.
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
project(sensory-cloud-voice-assistant)

# Fetch the sensorycloud SDK and compile it as part of this project.
include(FetchContent)
FetchContent_Declare(sensorycloud
    GIT_REPOSITORY git@gitlab.com:sensory-cloud/sdk/cpp-sdk.git
    GIT_TAG        develop
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

# Add the application to the build chain.
add_executable(voice_assistant voice_assistant.cpp)
target_link_libraries(voice_assistant PRIVATE sensorycloud portaudio)
