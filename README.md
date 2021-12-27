# Sensory Cloud C++ SDK

This repository contains the source code for the Sensory Cloud C++ SDK.

## Requirements

This project uses CMake as the primary build system and gcc as the primary
compiler. To install the build tools, run the following installation command
depending on your deployment target. The minimum requirements for installation
of the SDK are:

| Dependency | Version   |
|:-----------|:----------|
| `gcc`      | >= 4.9    |
| `cmake`    | >= 3.11   |

For older versions of Linux that do not ship with the latest CMake, it may be
necessary to install a more recent version. Refer to the Dockerfile for your
deployment target for version-specific scripting to install the latest CMake.
If you intend to deploy to a container, these images may serve as a starting
point for integrating the Sensory Cloud SDK into your project.

| Dockerfile                                         | Distro    | Version    |
|:---------------------------------------------------|:----------|:-----------|
| [Dockerfile.Ubuntu.21.10](Dockerfile.Ubuntu.21.10) | Ubuntu    | 21.10      |
| [Dockerfile.Ubuntu.20.04](Dockerfile.Ubuntu.20.04) | Ubuntu    | 20.04      |
| [Dockerfile.Ubuntu.18.04](Dockerfile.Ubuntu.18.04) | Ubuntu    | 18.04      |
| [Dockerfile.Ubuntu.16.04](Dockerfile.Ubuntu.16.04) | Ubuntu    | 16.04      |
| [Dockerfile.Debian.9](Dockerfile.Debian.9)         | Debian    | 9          |
| [Dockerfile.Debian.8](Dockerfile.Debian.8)         | Debian    | 8          |
| [Dockerfile.Debian.7](Dockerfile.Debian.7)         | Debian    | 7          |
| [Dockerfile.RHEL.8](Dockerfile.RHEL.8)             | RHEL      | 8          |
| [Dockerfile.RHEL.7](Dockerfile.RHEL.7)             | RHEL      | 7          |
| [Dockerfile.CentOS.8](Dockerfile.CentOS.8)         | CentOS    | 8          |
| [Dockerfile.CentOS.7](Dockerfile.CentOS.7)         | CentOS    | 7          |

### Debian / Ubuntu

To install the build tools for Debian-based Linux:

```shell
apt-get install -y build-essential autoconf libtool pkg-config cmake git libssl-dev
```

### RedHat / CentOS

To install the build tools for RedHat-based Linux:

```shell
yum install -y autoconf libtool pkg-config gcc gcc-c++ git openssl-devel
```

### MacOS

To install the build tools for MacOS:

```shell
xcode-select --install
brew install autoconf automake libtool shtool cmake
```

### Windows

To install the build tools for Windows:

```shell
TODO
```

## Setup & Libraries

The Sensory Cloud SDK is built on protobuf and gRPC and requires an active
installation of both libraries as such. It is recommended to compile these
libraries locally using `FetchContent` or git sub-modules, but they can also be
installed system-wide.

### Local Installation (`FetchContent`)

<!-- sudo apt-get install openssl libgrpc++-dev -->

To automatically fetch and install protobuf and gRPC for a local installation,
execute the following:

```shell
cmake -DGRPC_AS_SUBMODULE=OFF -DGRPC_FETCHCONTENT=ON .
make
```

### Local Installation (`add_subdirectory`)

To compile against local clone of gRPC, place a copy of the gRPC source tree in
the [third_party/grpc](third_party/grpc) directory and execute the following:

```shell
cmake -DGRPC_AS_SUBMODULE=ON -DGRPC_FETCHCONTENT=OFF .
make
```

### Global Installation (MacOS)

The homebrew package manager for MacOS maintains version matched binaries of
protobuf and gRPC that can be used for a system-wide installation of these two
libraries. To install with a system-wide installation of gRPC and protobuff on
MacOS, use the following snippet:

```shell
brew install openssl protobuf grpc
cmake -DGRPC_AS_SUBMODULE=OFF -DGRPC_FETCHCONTENT=OFF .
export OPENSSL_ROOT_DIR="/usr/local/opt/openssl@3/"
make
export GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=/etc/ssl/cert.pem
```

Note that because MacOS does not follow the POSIX standard, some additional
environment variables are used to provide information about the OpenSSL library
and public certificates on disk used for Transport Layer Security (TLS).

## Compilation

By default, the `CMakeLists.txt` only compiles the dependencies. The
Sensory Cloud SDK library, test cases, benchmarks, and example applications
are only compiled when explicitly directed. The following commands can be used
to update the local `CMakeCache.txt` with options to compile test cases,
benchmarks, and example applications. When the makefiles have been configured,
`make` can be used to compile the source code.

### Library

To generate only the `libsensorycloud` library, execute:

```shell
cmake -DBUILD_LIBRARY=ON .
```

### Test Cases

To generate the makefiles for building test cases, execute:

```shell
cmake -DBUILD_TESTS=ON .
```

### Benchmarks

To generate the makefiles for building benchmarks, execute:

```shell
cmake -DBUILD_BENCHMARKS=ON .
```

### Examples

To generate the makefiles for building example applications, execute:

```shell
cmake -DBUILD_EXAMPLES=ON .
```

<!-- ### Testing

To compile and run the unit tests, execute:

```shell
./cs.sh test
``` -->

<!-- ### Benchmarking

To compile and run the benchmarks, execute:

```shell
./cs.sh benchmark
``` -->
