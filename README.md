# Sensory Cloud C++ SDK

This repository contains the source code for the Sensory Cloud C++ SDK.

<!--
# Install libsecret-1 and setup the Keychain for the container
# RUN apt-get install -y libsecret-1-dev
# RUN export $(dbus-launch)
# RUN eval "$(printf '\n' | gnome-keyring-daemon --unlock)"
# RUN eval "$(printf '\n' | /usr/bin/gnome-keyring-daemon --start)"
-->

<!--
```shell
apt-get install -y libsecret-1-dev
export $(dbus-launch)
eval "$(printf '\n' | gnome-keyring-daemon --unlock)"
eval "$(printf '\n' | /usr/bin/gnome-keyring-daemon --start)"
```
-->

<!--
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/// @brief Return the home directory for the current user.
///
/// @returns The home directory for the user running the program
///
std::string getHomeDirectory() {
    static constexpr std::size_t MAX_PATH = 1024;
    char homedir[MAX_PATH];
#ifdef _WIN32  // Windows
    snprintf(homedir, MAX_PATH, "%s%s", getenv("HOMEDRIVE"), getenv("HOMEPATH"));
#else  // MacOS or Unix
    snprintf(homedir, MAX_PATH, "%s", getenv("HOME"));
#endif
    return std::string(strdup(homedir));
}

std::string makeSDKDirectory() {
    // Create the home directory for the SDK
    const auto SDK_DIR(getHomeDirectory() + "/.sensorycloud");
    mkdir(SDK_DIR.c_str(), 0755);
    return SDK_DIR;
}
-->

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
| [Dockerfile.Debian.11](Dockerfile.Debian.11)       | Debian    | 11         |
| [Dockerfile.Debian.10](Dockerfile.Debian.10)       | Debian    | 10         |
| [Dockerfile.Debian.9](Dockerfile.Debian.9)         | Debian    | 9          |
| [Dockerfile.RHEL.8](Dockerfile.RHEL.8)             | RHEL      | 8          |
| [Dockerfile.RHEL.7](Dockerfile.RHEL.7)             | RHEL      | 7          |
| [Dockerfile.CentOS.8](Dockerfile.CentOS.8)         | CentOS    | 8          |
| [Dockerfile.CentOS.7](Dockerfile.CentOS.7)         | CentOS    | 7          |

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

### Windows

To install the build tools for Windows:

```shell
TODO
```

## Setup

To include the Sensory Cloud C++ SDK in your project, add the following snippet
to your `CMakeLists.txt`.

```shell
include(FetchContent)
FetchContent_Declare(sensorycloud
    GIT_REPOSITORY https://github.com/Sensory-Cloud/cpp-sdk.git
    GIT_TAG        v0.6.0
)
FetchContent_MakeAvailable(sensorycloud)
```

This will clone and compile `libsensorycloud` locally to be used in
applications or other libraries using, i.e.,

```shell
target_link_libraries(your_program PRIVATE sensorycloud)
```

## Linked Libraries

The Sensory Cloud SDK is built on top of protobuf and gRPC and must be linked
against both libraries. It is recommended (by the gRPC library authors) to
compile these libraries locally using `FetchContent` or git sub-modules, but
they can also be installed system-wide.

### Local Installation (`FetchContent`)

To automatically fetch and install protobuf and gRPC for a local installation,
execute the following:

```shell
cmake -DGRPC_AS_SUBMODULE=OFF -DGRPC_FETCHCONTENT=ON <source directory>
```

### Local Installation (`add_subdirectory`)

To compile against local clone of gRPC, place a copy of the gRPC source tree in
the `third_party/grpc` directory and execute the following:

```shell
cmake -DGRPC_AS_SUBMODULE=ON -DGRPC_FETCHCONTENT=OFF <source directory>
```

### Global Installation (MacOS)

The homebrew package manager for MacOS maintains version matched binaries of
protobuf and gRPC that can be used for a system-wide installation of these two
libraries. To install with a system-wide installation of protobuf and gRPC on
MacOS, use the following snippet:

```shell
brew install openssl protobuf grpc
cmake -DGRPC_AS_SUBMODULE=OFF -DGRPC_FETCHCONTENT=OFF <source directory>
export OPENSSL_ROOT_DIR="/usr/local/opt/openssl@3/"
make
export GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=/etc/ssl/cert.pem
```

Note that because MacOS does not follow the POSIX standard, some additional
environment variables are used to provide information about the OpenSSL library
and public certificates on disk used for Transport Layer Security (TLS).

## Compilation

```shell
mkdir build
cd build
cmake ..
make
```

### Test Cases

To generate the makefiles for building test cases, execute:

```shell
cmake -DSENSORY_CLOUD_BUILD_TESTS=ON <source directory>
```

### SecureCredentialStore

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

### Tutorials and Examples

Please refer to the [md/tutorial.md](md/tutorial.md) for a tutorial of the
blocking synchronous interface of the SDK. Tutorials for the asynchronous
interface can be found in the
[md/async_event_loop.md](md/async_event_loop.md) and
[md/async_callback.md](md/async_callback.md) for event-loop and reactor
patterns, respectively. Examples code can be found in the [examples](examples/)
directory.