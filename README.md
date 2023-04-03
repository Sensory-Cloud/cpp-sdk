# SensoryCloud C++ SDK

This repository contains the source code for the [SensoryCloud][sensory-cloud]
C++ SDK.

## General Information

Before getting started you must spin up a [SensoryCloud][sensory-cloud]
inference server or have [SensoryCloud][sensory-cloud] spin one up for you. You
must also have the following pieces of information:

-   your inference server address and port number,
-   your SensoryCloud tenant ID, and
-   your configured secret key used for registering OAuth clients.

[sensory-cloud]: https://sensorycloud.ai/

## Requirements

This project uses [CMake][cmake] as the primary build system and supports
the [gcc][gcc], [clang][clang], and [MSVC][MSVC] compilers. To install the
build tools, run the following installation command depending on your
deployment target. The minimum requirements for installation of the SDK are:

| Dependency       | Version |
|:-----------------|:--------|
| [`cmake`][cmake] | >= 3.14 |
| [`gcc`][gcc]     | >= 5.1  |
| [`clang`][clang] | >= 4    |
| [`MSVC`][MSVC]   | >= 2015 |

[cmake]: https://cmake.org/
[gcc]: https://gcc.gnu.org/
[clang]: https://clang.llvm.org/get_started.html
[MSVC]: https://visualstudio.microsoft.com/vs/features/cplusplus/

### Debian / Ubuntu

To install the build tools for Debian-based Linux:

```shell
apt-get install -y build-essential autoconf libtool pkg-config cmake git
```

### Fedora / RedHat / CentOS

To install the build tools for Fedora-based Linux:

```shell
yum install -y autoconf libtool pkg-config gcc gcc-c++ cmake git
```

### MacOS

To install the build tools for MacOS:

```shell
xcode-select --install
brew install autoconf automake libtool shtool cmake git
```

Because MacOS is not strictly POSIX compliant, you will need to provide gRPC
with a hint of where your SSL certificates are stored to properly connect to a
secure server. This can typically be accomplished by exporting the following
environment variable:

```shell
export GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=/etc/ssl/cert.pem
```

<!--
### Windows

To install the build tools for Windows:

```shell
TODO
```
-->

## Integration

To include the SensoryCloud C++ SDK in your project, add the following snippet
to your `CMakeLists.txt`.

```shell
include(FetchContent)
FetchContent_Declare(sensorycloud
    URL https://codeload.github.com/Sensory-Cloud/cpp-sdk/tar.gz/refs/tags/v1.0.3
)
FetchContent_MakeAvailable(sensorycloud)
```

This will clone and compile `libsensorycloud` locally to be used in
applications or other libraries using:

```shell
target_link_libraries(your_program PRIVATE sensorycloud)
```

### Tutorial & Examples

Please refer to [TUTORIAL.md](TUTORIAL.md) for a brief tutorial of the
SDK. Example code for each available service and programming model are
available in the [examples](examples) directory. These act both as a point of
reference and as a starting point for the integration of SensoryCloud with your
application.

## Contributing

The [CONTRIBUTING.md](CONTRIBUTING.md) document describes how to setup the
development environment for the SDK, compile the code outside of the context of
an application, run unit tests, and release new versions of the SDK. Please
refer to the [CHANGELOG.md](CHANGELOG.md) for the latest features and bug fixes.
