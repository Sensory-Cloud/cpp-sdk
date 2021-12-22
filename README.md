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

<!-- ## gRPC

This SDK uses gRPC to communicate with back-end servers. To install gRPC,
execute the following command depending on your deployment target.

### Debian

```shell
sudo apt-get install openssl libgrpc++-dev
```

### MacOS

```shell
brew install openssl grpc
export OPENSSL_ROOT_DIR="/usr/local/opt/openssl@3/"
export GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=/etc/ssl/cert.pem
```
-->

## Usage

### Library Compilation

```shell
cmake -DUSE_SYSTEM_GRPC=ON -DUSE_SYSTEM_PROTO=ON .
```

```shell
cmake -DUSE_SYSTEM_GRPC=OFF -DUSE_SYSTEM_PROTO=OFF .
```

#### Debian

TODO

#### MacOS

TODO

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
