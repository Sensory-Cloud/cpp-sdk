# Tutorial

The tutorial provides an overview of the [SensoryCloud][sensory-cloud] C++ SDK.
This tutorial will show you how to:

-   setup a C++ project and include the [SensoryCloud][sensory-cloud] C++ SDK
    in your build,
-   implement secure credential storage and connect to your inference server,
-   query the server's health, and
-   navigate the [SensoryCloud][sensory-cloud] example projects.

Because this SDK is built on [gRPC][gRPC], you may benefit from referring to
the [gRPC Tutorial][gRPC Tutorial] and [gRPC Documentation][gRPC Documentation]
to familiarize yourself with some of the gRPC structures and patterns that are
used in this SDK and tutorial.

## Project Setup

The recommended build system for projects based on this SDK is
[`cmake`](https://cmake.org/). To include the SDK in your project , the simplest
solution is to add a `FetchContent` block to your `CMakeLists.txt`. This
approach will result in the compilation and linkage of the protobuff and gRPC
dependencies locally.

```cmake
cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(sensorycloud
    URL https://codeload.github.com/Sensory-Cloud/cpp-sdk/tar.gz/refs/tags/v1.0.4
)
FetchContent_MakeAvailable(sensorycloud)

...

# Define an executable and link with libsensorycloud.
add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world PRIVATE sensorycloud)
```

## Configuring a connection

### Endpoint Configuration

To connect to your [SensoryCloud][sensory-cloud] Inference server, you will
need to know:

1.  the host-name or IP address that the service is running at,
2.  the specific port that the service is running at -- typically `443`, --
3.  your tenant ID that uniquely identifies your tenant in SensoryCloud, and
4.  a way of uniquely identifying devices in your application.

This information can be provided to the SDK using a `sensory::Config` object:

```c++
sensory::Config config(
    "example.company.com",                   // the host name of the server
    443,                                     // the port number of the service
    "a376234e-5b4b-4acb-bdbc-8cac8c397ace",  // your tenant ID
    "4e07cce1-cccb-4630-a2d1-5da71e3c85a3",  // a unique device ID
    true                                     // a flag for enabling TLS
);
```

A single instance of `sensory::Config` should be instantiated and maintained
per instance of an application on a device. Using multiple instances of
`sensory::Config` for the same remote host may result in undefined behavior.

### Device Registration

Devices in your application will need to register and be authenticated using
cross-device trust. As such, each device will need to provide the following
information when instantiating the SDK:

1.  A name for the device for identifying it among the cluster of devices,
1.  The type of device enrollment to use, e.g., `sharedSecret` if your fleet
    uses "passphrase"-based authentication or `JWT` to use JSON web tokens, and
1.  The value for the credential which varies depending on the device
    enrollment.

```c++
sensory::RegistrationCredentials credentials(
    "Server 1",      // A human-readable name for the device.
    "sharedSecret",  // The type of enrollment (e.g., shared credential or JWT).
    "password"       // The value of the credential.
)
```

### Configuration & Registration INI Files

To simplify connection and device registration, the aforementioned information
may be serialized in an INI file in the following format:

```
[SDK-configuration]
fullyQualifiedDomainName = example.company.com:443
tenantID = a376234e-5b4b-4acb-bdbc-8cac8c397ace
credential = password
enrollmentType = sharedSecret
deviceID = 4e07cce1-cccb-4630-a2d1-5da71e3c85a3
deviceName = Server 1
isSecure = 1
```

### Credential Storage

The last thing you will need to set up for your application is a structure for
storing credentials in a way that fits your security needs. `CredentialStore`
provides the interface for credential storage that should be implemented for
your specific usage.

```c++
/// @brief A key-value interface for storing credentials, tokens, etc.
struct CredentialStore {
    /// @brief Emplace or replace a key/value pair in the credential store.
    ///
    /// @param key the plain-text key of the value to store
    /// @param value the value to store
    /// @details
    /// Unlike most key-value store abstractions in the STL, this
    /// implementation of emplace should overwrite existing values in the
    /// key-value store.
    ///
    inline void emplace(const std::string& key, const std::string& value) const;

    /// @brief Return true if the key exists in the credential store.
    ///
    /// @param key the plain-text key to check for the existence of
    ///
    inline bool contains(const std::string& key) const;

    /// @brief Look-up a secret value in the credential store.
    ///
    /// @param key the plain-text key of the value to return
    /// @returns the secret value indexed by the given key
    ///
    inline std::string at(const std::string& key) const;

    /// @brief Remove a secret key-value pair in the credential store.
    ///
    /// @param key the key of the pair to remove from the credential store
    ///
    inline void erase(const std::string& key) const;
};
```

Included with the SDK are an in-memory credential store
`sensory::token_manager::InMemoryCredentialStore`, and a file-system based
credential store `sensory::token_manager::FileSystemCredentialStore` that may
be useful for low-security applications, demonstration purposes, or debugging.

<!--
Also included with the SDK are reference implementations for certain
platform-specific secure credential persistence tools, including:

-   MacOS (through [Keychain Services][Keychain-Services]),
-   Windows (through [Credential Locker][Credential-Locker]), and
-   Linux (through [Libsecret][Libsecret]).

[Keychain-Services]: https://developer.apple.com/documentation/security/keychain_services
[Credential-Locker]: https://docs.microsoft.com/en-us/windows/uwp/security/credential-locker
[Libsecret]: https://wiki.gnome.org/Projects/Libsecret
-->

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

### SDK Instantiation

With the data and structures from the previous steps, one can construct an
instance of `sensory::SensoryCloud` that provides access to each cloud service.
The following example uses `sensory::token_manager::FileSystemCredentialStore`
as a credential store for demonstration purposes.

```c++
FileSystemCredentialStore credential_store(".", "com.company.cloud.debug");
SensoryCloud<FileSystemCredentialStore>
    cloud(config, credentials, credential_store);
```

As previously mentioned, config and registration credentials may optionally be
stored in INI files and parsed on SDK initialization using the following:

```c++
SensoryCloud<FileSystemCredentialStore> cloud("config.ini", ...);
```

## Checking The Server Health

It's important to check the health of your [SensoryCloud][sensory-cloud]
Inference server. You can do so using the `HealthService` via the following:

```c++
// Create a response for the RPC.
sensory::api::common::ServerHealthResponse server_health;
// Perform the RPC and check the status for errors.
auto status = cloud.health.getHealth(&server_health);
if (!status.ok()) {  // The call failed, handle the error here.
    auto errorCode = status.error_code();
    auto errorMessage = status.error_message();
} else {  // The call succeeded, handle the response here.
    auto isHealthy = response.ishealthy();
    auto serverVersion = response.serverversion();
    auto serverID = response.id();
}
```

## Example Projects

The remainder of the tutorial is organized in a programmatic format through
example code. Examples for each individual service may be found in the
[examples](examples) directory.

<!-- URLs -->

[sensory-cloud]: https://sensorycloud.ai/
[gRPC]: https://www.grpc.io/
[gRPC Tutorial]: https://www.grpc.io/docs/languages/cpp/basics/
[gRPC Documentation]: https://grpc.github.io/grpc/cpp/annotated.html
