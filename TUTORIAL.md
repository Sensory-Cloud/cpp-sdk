# Tutorial

The tutorial provides an overview of the SensoryCloud C++ SDK. This tutorial
will show you how to:

-   Setup a C++ project and include the SDK in your build,
-   Implement secure credential storage and connect to your inference server,
-   Query the server's health, and
-   Navigate the example projects.

Because this SDK is built on [gRPC][gRPC], you may benefit from referring to
the [gRPC Tutorial][gRPC Tutorial] and [gRPC Documentation][gRPC Documentation]
to familiarize yourself with some of the gRPC structures and patterns that are
used in this SDK and tutorial.

[gRPC]: https://www.grpc.io/
[gRPC Tutorial]: https://www.grpc.io/docs/languages/cpp/basics/
[gRPC Documentation]: https://grpc.github.io/grpc/cpp/annotated.html

## Project Setup

The recommended build system for projects based on this SDK is CMake. To
include the SDK in your project , the simplest solution is to add a
`FetchContent` block to your `CMakeLists.txt`. This approach will compile and
link the protobuff and gRPC dependencies locally. Below is an example of the
boilerplate CMake syntax to include the SDK in your project.

```cmake
project(your-project-name)
cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(sensorycloud
    URL https://codeload.github.com/Sensory-Cloud/cpp-sdk/tar.gz/refs/tags/v1.2.1
)
FetchContent_MakeAvailable(sensorycloud)

# ...

# Define an executable and link with libsensorycloud.
add_executable(hello_cloud hello_cloud.cpp)
target_link_libraries(hello_cloud PRIVATE sensorycloud)
```

##### Header files

The SDK definition is funneled down to a single header file for the most
common elements of the SDK. In most cases, you will only need the following
include statement to get started with the SDK.

```c++
#include <sensorycloud/sensorycloud.hpp>
```

## Credential Storage

The first piece you will need to set up for your application is a structure for
storing and persisting credentials in a way that fits your security needs. The
`MyCredentialStore` structure below provides the interface for credential
storage that should be implemented for your specific usage. This structure
behaves like a C++ map, but should persist key-value data between executions
of your application. This implementation follows C++ syntax for `map`
structures and may be trivially compatible with your existing solution for
secure key-value storage. It is worth noting that the SDK's usage of this
structure is based on templates, so there is no need to inherit from a base
class to implement this component.

```c++
struct MyCredentialStore {
    /// @brief Emplace or overwrite a key/value pair in the credential store.
    /// @param key the plain-text key of the value to store
    /// @param value the value to store
    inline void emplace(const std::string& key, const std::string& value) const;

    /// @brief Return true if the key exists in the credential store.
    /// @param key the plain-text key to check for the existence of
    inline bool contains(const std::string& key) const;

    /// @brief Look-up a secret value in the credential store.
    /// @param key the plain-text key of the value to return
    /// @returns the secret value indexed by the given key
    inline std::string at(const std::string& key) const;

    /// @brief Remove a secret key-value pair in the credential store.
    /// @param key the key of the pair to remove from the credential store
    inline void erase(const std::string& key) const;
};
```

Included with the SDK are some debugging implementations of this interface that
may be useful for your testing and/or applications.
`sensory::token_manager::FileSystemCredentialStore` provides an implementation
based on the file-system that may be useful for low-security applications,
demonstration purposes, or debugging. We also provide a
`sensory::token_manager::InMemoryCredentialStore` that may be useful for
integrating SensoryCloud with your unit tests. The in-memory store can be
instantiated without parameters, but the file-system based store must be
provided with a path to a directory to persist data to and a prefix for files
that are created in the file-system-based key-value store. For this example, we
utilize the file-system credential store:

```c++
sensory::token_manager::FileSystemCredentialStore
    credential_store(".", "com.company.cloud.debug");
```

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

## Configuring a connection

There are two ways to configure your connection to SensoryCloud in code. At
the lower level you may choose to hard-code your tenant metadata (or specify
the parameters in the method of your choosing) using the `sensory::Config` and
`sensory::RegistrationCredentials` structures. Alternatively you may choose
to use the INI file provided by our sales team to configure your connection.
Both of these connection APIs are discussed in the following sections.

### Hard-coded Config and RegistrationCredentials structures

To connect to your inference server using a hard-coded config, you will need
to know:

1.  the host-name or IP address that the service is running at,
2.  the specific port that the service is running at - typically `443`, -
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
per instance of an application on a device when using this API. Using multiple
instances of `sensory::Config` for the same remote host may result in
undefined behavior.

##### Device Registration

Devices in that interact with the inference server will need to register and
be authenticated using cross-device trust. As such, each device will need to
provide the following information when instantiating the SDK:

1.  A name for the device for identifying it among the cluster of devices,
1.  The type of device enrollment to use, e.g., `sharedSecret` if your fleet
    uses "passphrase"-based authentication or `JWT` to use JSON web tokens, and
1.  The value for the credential which varies depending on your tenant.

```c++
sensory::RegistrationCredentials credentials(
    "Server 1",      // A human-readable name for the device.
    "sharedSecret",  // The type of enrollment ("sharedSecret" or "jwt").
    "password"       // The value of the credential.
);
```

### INI Files

To simplify connection and device registration, the aforementioned information
may be serialized in an INI file in the following format that describes your
tenant in SensoryCloud:

```
[SDK-configuration]
fullyQualifiedDomainName = example.company.com:443
tenantID = a376234e-5b4b-4acb-bdbc-8cac8c397ace
credential = password
enrollmentType = sharedSecret
isSecure = 1
```

When you sign up for SensoryCloud, you will be provided with an INI file in
the above format that you may use with our example code or for your own
purposes. Note that when using INI files, device IDs and names are provided
optionally through environment variables, i.e.,

```shell
export SENSORYCLOUD_DEVICE_ID=4e07cce1-cccb-4630-a2d1-5da71e3c85a3
export SENSORYCLOUD_DEVICE_NAME="Server 1"
```

If either of the device name or the device ID are not provided, a random one
will be generated and stored in the secure credential store. This allows for
re-use of INI files between devices with the ability to track devices in
your deployment explicitly, if needed.

## SDK Instantiation

With the data structures defined in the previous steps, one can construct an
instance of `sensory::SensoryCloud` that provides access to each cloud service.

```c++
sensory::SensoryCloud<sensory::token_manager::FileSystemCredentialStore>
    cloud(config, credentials, credential_store);
```

As previously mentioned, config and registration credentials may optionally be
stored in INI files and parsed on SDK initialization using the following where
`./config.ini` is an example path to your tenant INI file.

```c++
sensory::SensoryCloud<sensory::token_manager::FileSystemCredentialStore>
    cloud("./config.ini", credential_store);
```

_Note that our example code uses the INI construction interface._

## Checking The Server Health

Once you have a connection to the server instantiated, it's important to
check the health of your inference server. You can do so using the
`HealthService` via the following:

```c++
// Create a response for the RPC.
sensory::api::common::ServerHealthResponse response;
// Perform the RPC and check the status for errors.
auto status = cloud.health.get_health(&response);
if (!status.ok()) {  // The call failed, handle the error here.
    auto error_code = status.error_code();
    auto error_message = status.error_message();
} else {  // The call succeeded, handle the response here.
    std::cout << "Server is healthy: " << response.ishealthy() << std::endl;
}
```

## Example Projects

At this point you should be set up and ready to integrate with SensoryCloud!
The remainder of the tutorial is organized in a programmatic format through
example code. Examples for each individual service may be found in the
[examples](examples) directory.

## Full Code

For your convenience, the above passages are condensed into the following code
blocks for the two approaches for instantiating the SDK.

### Hard-coded config API

```c++
#include <sensorycloud/sensorycloud.hpp>

int main() {
    sensory::token_manager::FileSystemCredentialStore
        credential_store(".", "com.company.cloud.debug");

    sensory::Config config(
        "example.company.com",                   // the host name of the server
        443,                                     // the port number of the service
        "a376234e-5b4b-4acb-bdbc-8cac8c397ace",  // your tenant ID
        "4e07cce1-cccb-4630-a2d1-5da71e3c85a3",  // a unique device ID
        true                                     // a flag for enabling TLS
    );
    sensory::RegistrationCredentials credentials(
        "Server 1",      // A human-readable name for the device.
        "sharedSecret",  // The type of enrollment ("sharedSecret" or "jwt").
        "password"       // The value of the credential.
    );
    sensory::SensoryCloud<sensory::token_manager::FileSystemCredentialStore>
        cloud(config, credentials, credential_store);

    sensory::api::common::ServerHealthResponse response;
    auto status = cloud.health.get_health(&response);
    if (!status.ok()) {  // The call failed, handle the error here.
        auto error_code = status.error_code();
        auto error_message = status.error_message();
    } else {  // The call succeeded, handle the response here.
        std::cout << "Server is healthy: " << response.ishealthy() << std::endl;
    }
}
```

### INI file API

```c++
#include <sensorycloud/sensorycloud.hpp>

int main() {
    sensory::token_manager::FileSystemCredentialStore
        credential_store(".", "com.company.cloud.debug");
    sensory::SensoryCloud<sensory::token_manager::FileSystemCredentialStore>
        cloud("./config.ini", credential_store);

    sensory::api::common::ServerHealthResponse response;
    auto status = cloud.health.get_health(&response);
    if (!status.ok()) {  // The call failed, handle the error here.
        auto error_code = status.error_code();
        auto error_message = status.error_message();
    } else {  // The call succeeded, handle the response here.
        std::cout << "Server is healthy: " << response.ishealthy() << std::endl;
    }
}
```
