# Sensory Cloud C++ SDK

This repository contains the source code for the Sensory Cloud C++ SDK.

## Requirements

### Build Tools

This project uses CMake as the primary build system. To install CMake, run the
following installation command depending on your deployment target.

#### Debian

```shell
sudo apt-get install build-essential autoconf libtool pkg-config cmake
```

#### MacOS

```shell
xcode-select --install
brew install autoconf automake libtool shtool cmake
```

### Protocol Buffer Compiler

This project relies on protocol buffers to automatically generate API interface
code. To install the protocol buffers compiler, `protoc`, run the following
command depending on your deployment target.

#### Debian

```shell
apt install -y protobuf-compiler protobuf-compiler-grpc
```

#### MacOS

```shell
brew install protobuf
```

### gRPC

This SDK uses gRPC to communicate with back-end servers. To install gRPC,
execute the following command depending on your deployment target.

#### Debian

```shell
sudo apt-get install openssl libgrpc++-dev
```

#### MacOS

```shell
brew install openssl grpc
export OPENSSL_ROOT_DIR="/usr/local/opt/openssl@3/"
export GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=/etc/ssl/cert.pem
```

<!-- sudo apt install libopencv-dev -->

## Usage

### Protocol Buffer Generation

protocol buffer and gRPC stub files can be automatically generated from the
`.proto` files in [proto](https://gitlab.com/sensory-cloud/sdk/proto) with the
command:

```shell
./cs.sh genproto
```

The generated header files and definition files will be in
[include/sensorycloud/protoc](include/sensorycloud/protoc) and
[src/sensorycloud/protoc](src/sensorycloud/protoc), respectively.

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

```
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
./cs.sh bench
```
