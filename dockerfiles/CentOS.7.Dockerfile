# A docker file to build a CI image for CentOS 7.
#
# Author: Christian Kauten (ckauten@sensoryinc.com)
#
# Copyright (c) 2021 Sensory, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXTERNRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# ----------------------------------------------------------------------------
# MARK: Image Setup
# ----------------------------------------------------------------------------

# Download base image CentOS 7.
FROM centos:7
# Set the maintainer label for this Docker image.
LABEL MAINTAINER "Christian Kauten ckauten@sensoryinc.com"
# Set the time-zone of the machine.
ENV TZ=US/Mountain

# ----------------------------------------------------------------------------
# MARK: Dependencies
# ----------------------------------------------------------------------------

# Update CentOS software repositories.
RUN yum -y update
RUN yum -y upgrade
# Install dependencies for the SDK.
RUN yum -y group install "Development Tools"
RUN yum install -y autoconf libtool pkg-config git python3

# Install the latest version of CMake. CMake does not officially support CentOS
# 7 anymore, so CMake must be compiled from source.
RUN yum install -y wget
RUN wget https://github.com/Kitware/CMake/releases/download/v3.22.1/cmake-3.22.1.tar.gz
RUN tar -xvf cmake-3.22.1.tar.gz
RUN cd /cmake-3.22.1 && ./bootstrap -- -DCMAKE_USE_OPENSSL=OFF && make && make install

# ----------------------------------------------------------------------------
# MARK: Protobuf and gRPC
# ----------------------------------------------------------------------------

# Copy the common CMake file to the container, this file changes infrequently
# to prevent the cloning of protobuf and gRPC from occurring very often.
COPY ./cmake/common.cmake /cpp-sdk/cmake/common.cmake
# Create a dummy CMakeLists.txt that will clone the gRPC code.
RUN printf "cmake_minimum_required(VERSION 3.14)\nproject(clone-grpc)\ninclude(cmake/common.cmake)" > /cpp-sdk/CMakeLists.txt
# Create a build directory for the repository and clone protobuf and gRPC.
# Enable verbose output from `FetchContent` to monitor the cloning of gRPC,
# which can be slow.
RUN mkdir /cpp-sdk/build && cd /cpp-sdk/build && cmake -DGRPC_FETCHCONTENT=ON -DFETCHCONTENT_QUIET=OFF ..
RUN cd /cpp-sdk/build && make -j`nproc`

# ----------------------------------------------------------------------------
# MARK: Source Files
# ----------------------------------------------------------------------------

# Copy the files for the SDK into the image.
COPY CMakeLists.txt /cpp-sdk/
ADD ./cmake         /cpp-sdk/cmake
ADD ./proto         /cpp-sdk/proto
ADD ./include       /cpp-sdk/include
ADD ./src           /cpp-sdk/src
ADD ./tests         /cpp-sdk/tests
ADD ./examples      /cpp-sdk/examples
