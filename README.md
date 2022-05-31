# Sensory Cloud C++ SDK

This repository contains the source code for the [Sensory Cloud][sensory-cloud]
C++ SDK.

## General Information

Before getting started, you must spin up a [Sensory Cloud][sensory-cloud]
inference server or have [Sensory][sensory-cloud] spin one up for you. You must
also have the following pieces of information:

-   Your inference server URL (and port number)
-   Your Sensory Tenant ID (UUID)

[sensory-cloud]: https://sensorycloud.ai/

## Requirements

This project uses [CMake][cmake] as the primary build system and [gcc][gcc] as
the primary compiler. To install the build tools, run the following
installation command depending on your deployment target. The minimum
requirements for installation of the SDK are:

| Dependency       | Version   |
|:-----------------|:----------|
| [`gcc`][gcc]     | >= 4.9    |
| [`cmake`][cmake] | >= 3.11   |

[cmake]: https://cmake.org/
[gcc]: https://gcc.gnu.org/

### Debian / Ubuntu

To install the build tools for Debian-based Linux:

```shell
apt-get install -y build-essential autoconf libtool pkg-config cmake git
```

### RedHat / CentOS

To install the build tools for RedHat-based Linux:

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
environment variable

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

To include the Sensory Cloud C++ SDK in your project, add the following snippet
to your `CMakeLists.txt`.

```shell
include(FetchContent)
FetchContent_Declare(sensorycloud
    URL https://codeload.github.com/Sensory-Cloud/cpp-sdk/tar.gz/refs/tags/v0.11.4
)
FetchContent_MakeAvailable(sensorycloud)
```

This will clone and compile `libsensorycloud` locally to be used in
applications or other libraries using:

```shell
target_link_libraries(your_program PRIVATE sensorycloud)
```

### Examples

Example code for each available service and programming model are available in
the [examples](examples) directory. These act both as a point of reference and
as a starting point for the integration of Sensory Cloud with your application.

Please refer to [md/tutorial.md](md/tutorial.md) for a tutorial of the
blocking synchronous interface of the SDK. Tutorials for the asynchronous
interface can be found in the
[md/async_event_loop.md](md/async_event_loop.md) and
[md/async_callback.md](md/async_callback.md) for event-loop and reactor
patterns, respectively.

<!--
### SecureCredentialStore _(Experimental feature)_

To optionally compile an operating system specific `SecureCredentialStore`
using system libraries, execute:

```shell
cmake -DSENSORY_CLOUD_BUILD_SECURE_CREDENTIAL_STORE=ON <source directory>
```

The table below provides information about the implementations of
`SecureCredentialStore` that are provided. Please refer to
[md/tutorial.md](md/tutorial.md) for more information about the
`SecureCredentialStore` object.

| Operating System  | Secure Secret Library                  |
|:------------------|:---------------------------------------|
| Linux             | [Libsecret][Libsecret]                 |
| Mac OS            | [Keychain Services][Keychain-Services] |
| Windows           | [Credential Locker][Credential-Locker] |

[Keychain-Services]: https://developer.apple.com/documentation/security/keychain_services
[Credential-Locker]: https://docs.microsoft.com/en-us/windows/uwp/security/credential-locker
[Libsecret]: https://wiki.gnome.org/Projects/Libsecret
-->

## Development

The [md/development.md](md/development.md) document describes how to setup the
development environment for the SDK, compile the code outside of the context of
an application, run unit tests, and release new versions of the SDK.
