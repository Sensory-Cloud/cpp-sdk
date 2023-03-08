# A docker file to build a CI image for Ubuntu 20.04.
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

# Download base image Ubuntu 20.04.
FROM ubuntu:16.04
# Set the maintainer label for this Docker image.
LABEL MAINTAINER "Christian Kauten ckauten@sensoryinc.com"
# Set the time-zone of the machine.
ENV TZ=US/Mountain
# Disable the interactive dialogue for `dpkg` (running behind apt-get).
ARG DEBIAN_FRONTEND=noninteractive

# ----------------------------------------------------------------------------
# MARK: Dependencies
# ----------------------------------------------------------------------------

# Update Ubuntu software repositories.
RUN apt-get update
RUN apt-get -y upgrade

# Install Vagrant directly from hashicorp in order to natively install `winrm`
# vagrant plugin needed to run Windows machines
RUN apt-get install jq curl -y
RUN curl -s https://checkpoint-api.hashicorp.com/v1/check/vagrant | jq -r -M '.current_version' > VAGRANT_VERSION
RUN curl -O https://releases.hashicorp.com/vagrant/$(cat VAGRANT_VERSION)/vagrant_$(cat VAGRANT_VERSION)_x86_64.deb
RUN dpkg -i vagrant_$(cat VAGRANT_VERSION)_x86_64.deb
RUN rm vagrant_$(cat VAGRANT_VERSION)_x86_64.deb
RUN rm VAGRANT_VERSION

RUN vagrant plugin install vagrant-reload
RUN vagrant plugin install winrm winrm-fs winrm-elevated
RUN vagrant plugin install vagrant-vbguest

RUN apt-get install -y qemu-kvm libvirt-daemon-system libvirt-dev
RUN vagrant plugin install vagrant-libvirt
# RUN service libvirtd start
# RUN service virtlogd start
# RUN chown root:kvm /dev/kvm

RUN mkdir /win10
RUN cd /win10
RUN vagrant init peru/windows-10-enterprise-x64-eval
# RUN VAGRANT_DEFAULT_PROVIDER=libvirt vagrant up
RUN vagrant up

# RUN apt-get install -y --fix-missing virtualbox
# RUN apt-get install -y virtualbox-dkms
# RUN dpkg-reconfigure virtualbox-dkms
# RUN dpkg-reconfigure virtualbox
# RUN apt-get install -y linux-headers-generic
# RUN VAGRANT_DEFAULT_PROVIDER=virtualbox vagrant up

# RUN apt-get install -y build-essential autoconf libtool pkg-config cmake git libssl-dev

# ----------------------------------------------------------------------------
# MARK: Protobuf and gRPC
# ----------------------------------------------------------------------------

# # Copy the common CMake file to the container, this file changes infrequently
# # to prevent the cloning of protobuf and gRPC from occurring very often.
# COPY ./cmake/common.cmake /cpp-sdk/cmake/common.cmake
# # Create a dummy CMakeLists.txt that will clone the gRPC code.
# RUN printf "cmake_minimum_required(VERSION 3.11)\nproject(clone-grpc)\ninclude(cmake/common.cmake)" > /cpp-sdk/CMakeLists.txt
# # Create a build directory for the repository and clone protobuf and gRPC.
# # Enable verbose output from `FetchContent` to monitor the cloning of gRPC,
# # which can be slow.
# RUN mkdir /cpp-sdk/build && cd /cpp-sdk/build && cmake -DGRPC_FETCHCONTENT=ON -DFETCHCONTENT_QUIET=OFF ..

# ----------------------------------------------------------------------------
# MARK: Source Files
# ----------------------------------------------------------------------------

# # Copy the files for the SDK into the image.
# COPY CMakeLists.txt /cpp-sdk/
# ADD ./cmake         /cpp-sdk/cmake
# ADD ./proto         /cpp-sdk/proto
# ADD ./include       /cpp-sdk/include
# ADD ./src           /cpp-sdk/src
# ADD ./tests         /cpp-sdk/tests
# ADD ./examples      /cpp-sdk/examples
