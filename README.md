# Sensory Cloud C++ SDK

This repository contains the source code for the Sensory Cloud C++ SDK.

## Requirements

### gRPC

```shell
brew install openssl grpc
```

### Protocol Buffer Compiler

This project relies on protocol buffers to automatically generate API interface
code. To install the protocol buffers compiler, `protoc`, run the following.

#### Debian

```shell
apt install -y protobuf-compiler
```

#### MacOS

```shell
brew install protobuf
```

## Usage

### Protocol Buffer Generation

gRPC and protocol buffer files can be automatically generated from the `.proto` files in [proto](proto) with the command:

```shell
./cs.sh genproto
```

The generated header files and definition files will be in [include/sensorycloud/protoc](include/sensorycloud/protoc) and [src/sensorycloud/protoc](src/sensorycloud/protoc), respectively.

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
