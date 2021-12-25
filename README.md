# Sensory Cloud C++ SDK

This repository contains the source code for the Sensory Cloud C++ SDK.

## Requirements

This project uses CMake as the primary build system. To install CMake, run the
following installation command depending on your deployment target.

### Debian

```shell
sudo apt-get install build-essential autoconf libtool pkg-config cmake
```

### MacOS

```shell
xcode-select --install
brew install autoconf automake libtool shtool cmake
```

## Usage

### Library Compilation

#### Debian

TODO

<!--
```shell
sudo apt-get install openssl libgrpc++-dev
```
-->

#### MacOS

To install with a system-wide installation of gRPC and protobuff, use the
following snippet:

```shell
brew install openssl grpc protobuf
export OPENSSL_ROOT_DIR="/usr/local/opt/openssl@3/"
export GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=/etc/ssl/cert.pem
```

## Development

To generate the developing makefiles for the SDK, execute the following:

```shell
cmake -DUSE_SYSTEM_GRPC=ON -DUSE_SYSTEM_PROTO=ON -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON -DBUILD_BENCHMARKS=ON .
```

### Testing

To compile and run the unit tests, execute:

```shell
./cs.sh test
```

### Benchmarking

To compile and run the benchmarks, execute:

```shell
./cs.sh benchmark
```
