# SensoryCloud C++ SDK

Welcome to the SensoryCloud C++ SDK repository! This repository contains the
source code and documentation for the SensoryCloud C++ SDK, which is designed
to facilitate integration with the SensoryCloud platform for cloud inference.

## General Information

Before you start using the SensoryCloud C++ SDK, it's important to have a
SensoryCloud inference server set up. If you're new to SensoryCloud, we offer
a free trial server that allows you to test our cloud platform and determine
its suitability for your product. To learn more about deploying an inference
server with SensoryCloud, please visit the [SensoryCloud website][trial].

Once your server is up and running, you'll need the following information to
effectively interact with it using this SDK:

-   The address and port number of your inference server,
-   Your SensoryCloud tenant ID, and
-   Your configured secret key used for registering OAuth clients.

If you have any questions or need assistance with server setup or
configuration, please don't hesitate to
[contact our sales team][sales]. We
are here to help ensure a smooth and successful integration of SensoryCloud
into your product.

[trial]: https://sensorycloud.ai/free-credits/
[sales]: https://sensorycloud.ai/resources/contact-us/

## Requirements

The SensoryCloud C++ SDK uses CMake as the primary build system and supports
the gcc, clang, and MSVC compilers. To install the necessary build tools,
please follow the instructions below based on your deployment target. The
minimum requirements for installing the SDK are as follows:

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

To install the build tools on Debian-based Linux distributions, run the
following command:

```shell
apt-get install -y build-essential autoconf libtool pkg-config cmake git
```

### Fedora / RedHat / CentOS

To install the build tools on Fedora-based Linux distributions, run the
following command:

```shell
yum install -y autoconf libtool pkg-config gcc gcc-c++ cmake git
```

### MacOS

To install the build tools on macOS, run the following commands:

```shell
xcode-select --install
brew install autoconf automake libtool shtool cmake git
```

Since macOS is not strictly POSIX compliant, you need to provide gRPC with a
hint of where your SSL certificates are stored to establish a secure
connection to a server. You can typically accomplish this by exporting the
following environment variable:

```shell
export GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=/etc/ssl/cert.pem
```

<!--
### Windows

To install the build tools on Windows:

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
    URL https://codeload.github.com/Sensory-Cloud/cpp-sdk/tar.gz/refs/tags/v1.3.1
)
FetchContent_MakeAvailable(sensorycloud)
```

This code will clone and compile the libsensorycloud library locally, allowing
you to use it in your applications or other libraries. To link the library
with your program, use the following command:

```shell
target_link_libraries(your_program PRIVATE sensorycloud)
```

### Tutorial & Examples

To get started with the SensoryCloud C++ SDK, we recommend checking out the
[TUTORIAL.md](TUTORIAL.md) document, which provides a brief tutorial on how to
use the SDK. Additionally, you can find example code for each available
service and programming model in the [examples](examples) directory. These
examples serve as both a reference and a starting point for integrating
SensoryCloud with your own applications.

## Contributing

If you're interested in contributing to the SensoryCloud C++ SDK, please refer
to the [CONTRIBUTING.md](CONTRIBUTING.md) document. It contains information on
setting up the development environment, compiling the code outside of an
application context, running unit tests, and releasing new versions of the
SDK. For details on the latest features and bug fixes, please consult the
[CHANGELOG.md](CHANGELOG.md) file.

We appreciate your contributions and feedback!
