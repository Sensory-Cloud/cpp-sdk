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
brew install cmake
```

### Protocol Buffer Compiler

This project relies on protocol buffers to automatically generate API interface
code. To install the protocol buffers compiler, `protoc`, run the following
command depending on your deployment target.

#### Debian

```shell
apt install -y protobuf-compiler
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
sudo apt-get install openssl grpc
```

#### MacOS

```shell
brew install openssl grpc
export OPENSSL_ROOT_DIR="/usr/local/opt/openssl@3/"
```

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

TODO

## Development

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
