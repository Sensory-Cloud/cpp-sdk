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

## Tutorial

### Checking The Server Health

It's important to check the health of your Sensory Inference server. You can do
so via the following:

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "io.stage.cloud.sensory.com",            // the host name of the server
    443,                                     // the port number of the service
    "cabb7700-206f-4cc7-8e79-cd7f288aa78d",  // your tenant ID
    "d895f447-91e8-486f-a783-6e3a33e4c7c5"   // a unique device ID
);

// Create a health service for performing server health queries.
sensory::service::HealthService healthService(config);
// Create a response for the RPC.
sensory::api::common::ServerHealthResponse serverHealth;
// Perform the RPC and check the status for errors.
auto status = healthService.getHealth(&serverHealth);
if (!status.ok()) {  // The call failed, print a descriptive message.
    std::cout << "Failed to get server health with\n\t" <<
        status.error_code() << ": " << status.error_message() << std::endl;
}
// Print the health of the remote service.
std::cout << "Server status:" << std::endl;
std::cout << "\tisHealthy: " << serverHealth.ishealthy() << std::endl;
std::cout << "\tserverVersion: " << serverHealth.serverversion() << std::endl;
std::cout << "\tid: " << serverHealth.id() << std::endl;
```

### Secure Credential Storage

`Keychain` provides the interface for secure credential storage that must be
implemented for your specific usage. Included with the SDK are reference
implementations for certain build platforms including:

-   MacOS (through [Keychain Services][Keychain-Services]),
-   Windows (through [Credential Locker][Credential-Locker]), and
-   Linux (through [Libsecret][Libsecret]).

[Keychain-Services]: https://developer.apple.com/documentation/security/keychain_services
[Credential-Locker]: https://docs.microsoft.com/en-us/windows/uwp/security/credential-locker
[Libsecret]: https://wiki.gnome.org/Projects/Libsecret

```c++
/// @brief A keychain manager for interacting with the OS credential manager.
struct Keychain {
    /// @brief Emplace or replace a key/value pair in the key-chain.
    ///
    /// @param key the plain-text key of the value to store
    /// @param value the secure value to store
    /// @details
    /// Unlike most key-value store abstractions in the STL, this
    /// implementation of emplace will overwrite existing values in the
    /// key-value store.
    ///
    inline void emplace(const std::string& key, const std::string& value) const;

    /// @brief Return true if the key exists in the key-chain.
    ///
    /// @param key the plain-text key to check for the existence of
    ///
    inline bool contains(const std::string& key) const;

    /// @brief Look-up a secret value in the key-chain.
    ///
    /// @param key the plain-text key of the value to return
    /// @returns the secret value indexed by the given key
    ///
    inline std::string at(const std::string& key) const;

    /// @brief Remove a secret key-value pair in the key-chain.
    ///
    /// @param key the plain-text key of the pair to remove from the keychain
    ///
    inline void erase(const std::string& key) const;
};
```

#### Creating a `TokenManager`

The `TokenManger` template class handles requesting OAuth tokens when
necessary. It utilizes an `OAuthService` to request new tokens when the local
ones expire, and also utilizes a `Keychain` implementation to securely store
tokens. All secure services require a reference to a token manager.

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "io.stage.cloud.sensory.com",            // the host name of the server
    443,                                     // the port number of the service
    "cabb7700-206f-4cc7-8e79-cd7f288aa78d",  // your tenant ID
    "d895f447-91e8-486f-a783-6e3a33e4c7c5"   // a unique device ID
);

// Create the key-chain for the token manager. You may use your own
// implementation of the `Keychain` interface for your application.
sensory::token_manager::Keychain keychain("com.product.company");
// Create the OAuth service from the configuration.
sensory::service::OAuthService oauthService(config);
// Create the token manager for handling token requests from the OAuth service
// and with a reference to the keychain.
sensory::token_manager::TokenManager<sensory::token_manager::Keychain>
    tokenManager(oauthService, keychain);
```

#### Registering OAuth Credentials

OAuth credentials should be registered once per unique machine. Registration is
very simple, and provided as part of the SDK. The below example shows how to
create an `OAuthService` and register a client for the first time.

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "io.stage.cloud.sensory.com",            // the host name of the server
    443,                                     // the port number of the service
    "cabb7700-206f-4cc7-8e79-cd7f288aa78d",  // your tenant ID
    "d895f447-91e8-486f-a783-6e3a33e4c7c5"   // a unique device ID
);

// Create the key-chain for the token manager. You may use your own
// implementation of the `Keychain` interface for your application.
sensory::token_manager::Keychain keychain("com.product.company");
// Create the OAuth service from the configuration.
sensory::service::OAuthService oauthService(config);
// Create the token manager for handling token requests from the OAuth service
// and with a reference to the keychain.
sensory::token_manager::TokenManager<sensory::token_manager::Keychain>
    tokenManager(oauthService, keychain);

if (!tokenManager.hasSavedCredentials()) {  // The device is not registered.
    // Generate a new `clientID` and `clientSecret` for this device and store
    // them securely in the keychain.
    const auto credentials = tokenManager.generateCredentials();
    // Use a shared secret, i.e., pass-phrase to authenticate the device.
    std::string insecureSharedSecret = "password";
    // Create a response for the RPC.
    sensory::api::v1::management::DeviceResponse rsp;
    // Perform the RPC and check the status for errors.
    auto status = oauthService.registerDevice(
        &rsp,
        userID,
        insecureSharedSecret,
        credentials.id,
        credentials.secret
    );
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to register device with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        // Handle error...
    }
}
```

#### Creating an `AudioService`

`AudioService` provides methods to stream audio to Sensory Cloud. It is
recommended to only have 1 instance of `AudioService` instantiated per
`Config`. In most circumstances you will only ever have one `Config`, unless
your app communicates with multiple Sensory Cloud servers.

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "io.stage.cloud.sensory.com",            // the host name of the server
    443,                                     // the port number of the service
    "cabb7700-206f-4cc7-8e79-cd7f288aa78d",  // your tenant ID
    "d895f447-91e8-486f-a783-6e3a33e4c7c5"   // a unique device ID
);

// Create the key-chain for the token manager. You may use your own
// implementation of the `Keychain` interface for your application.
sensory::token_manager::Keychain keychain("com.product.company");
// Create the OAuth service from the configuration.
sensory::service::OAuthService oauthService(config);
// Create the token manager for handling token requests from the OAuth service
// and with a reference to the keychain.
sensory::token_manager::TokenManager<sensory::token_manager::Keychain>
    tokenManager(oauthService, keychain);

// Create the audio service based on the configuration and token manager.
sensory::service::AudioService<sensory::token_manager::Keychain>
    audioService(config, tokenManager);
```

##### Obtaining Audio Models

Certain audio models are available to your application depending on the models
that are configured for your instance of Sensory Cloud. In order to determine
which audio models are accessible to you, you can execute the below code.

```c++
// Create a response for the RPC.
sensory::api::v1::audio::GetModelsResponse rsp;
// Execute the RPC and check the status for errors.
auto status = audioService.getModels(&rsp);
if (!status.ok()) {  // The call failed, print a descriptive message.
    std::cout << "Failed to get audio models with\n\t" <<
        status.error_code() << ": " << status.error_message() << std::endl;
    // Handle error...
}
```

##### Enrolling With Audio

```c++
TODO
```

##### Authenticating With Audio

```c++
TODO
```

##### Audio Events

```c++
TODO
```

##### Audio Transcription

```c++
TODO
```

#### Creating a `VideoService`

`VideoService` provides methods to stream images to Sensory Cloud. It is
recommended to only have 1 instance of `VideoService` instantiated per
`Config`. In most circumstances you will only ever have one `Config`, unless
your app communicates with multiple Sensory Cloud servers.

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "io.stage.cloud.sensory.com",            // the host name of the server
    443,                                     // the port number of the service
    "cabb7700-206f-4cc7-8e79-cd7f288aa78d",  // your tenant ID
    "d895f447-91e8-486f-a783-6e3a33e4c7c5"   // a unique device ID
);

// Create the key-chain for the token manager. You may use your own
// implementation of the `Keychain` interface for your application.
sensory::token_manager::Keychain keychain("com.product.company");
// Create the OAuth service from the configuration.
sensory::service::OAuthService oauthService(config);
// Create the token manager for handling token requests from the OAuth service
// and with a reference to the keychain.
sensory::token_manager::TokenManager<sensory::token_manager::Keychain>
    tokenManager(oauthService, keychain);

// Create the video service based on the configuration and token manager.
sensory::service::VideoService<sensory::token_manager::Keychain>
    videoService(config, tokenManager);
```

##### Obtaining Video Models

Certain video models are available to your application depending on the models
that are configured for your instance of Sensory Cloud. In order to determine
which video models are accessible to you, you can execute the below code.

```c++
// Create a response for the RPC.
sensory::api::v1::video::GetModelsResponse rsp;
// Execute the RPC and check the status for errors.
auto status = videoService.getModels(&rsp);
if (!status.ok()) {  // The call failed, print a descriptive message.
    std::cout << "Failed to get video models with\n\t" <<
        status.error_code() << ": " << status.error_message() << std::endl;
    // Handle error...
}
```

##### Enrolling With Video

```c++
TODO
```

##### Authenticating With Video

```c++
TODO
```

##### Video Liveness

```c++
TODO
```

#### Creating A Management Service

The `ManagementService` is used to manage typical _CRUD_ operations with
Sensory Cloud, such as deleting enrollments or creating enrollment groups.

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "io.stage.cloud.sensory.com",            // the host name of the server
    443,                                     // the port number of the service
    "cabb7700-206f-4cc7-8e79-cd7f288aa78d",  // your tenant ID
    "d895f447-91e8-486f-a783-6e3a33e4c7c5"   // a unique device ID
);

// Create the key-chain for the token manager. You may use your own
// implementation of the `Keychain` interface for your application.
sensory::token_manager::Keychain keychain("com.product.company");
// Create the OAuth service from the configuration.
sensory::service::OAuthService oauthService(config);
// Create the token manager for handling token requests from the OAuth service
// and with a reference to the keychain.
sensory::token_manager::TokenManager<sensory::token_manager::Keychain>
    tokenManager(oauthService, keychain);

// Create the management service based on the configuration and token manager.
sensory::service::ManagementService<sensory::token_manager::Keychain>
    mgmtService(config, tokenManager);
```

##### Fetching Enrollments

```c++
// The name of the user to fetch enrollments for.
std::string userID = "user";

// Create a response for the RPC.
sensory::api::v1::management::GetEnrollmentsResponse rsp;
auto status = mgmtService.getEnrollments(&rsp, userID);
if (!status.ok()) {  // The call failed, print a descriptive message.
    std::cout << "Failed to get enrollments with\n\t" <<
        status.error_code() << ": " << status.error_message() << std::endl;
    // Handle error...
}
for (auto& enrollment : rsp.enrollments()) {
    std::cout << "Description: "     << enrollment.description()  << std::endl;
    std::cout << "\tModel Name: "    << enrollment.modelname()    << std::endl;
    std::cout << "\tModel Type: "    << enrollment.modeltype()    << std::endl;
    std::cout << "\tModel Version: " << enrollment.modelversion() << std::endl;
    std::cout << "\tUser ID: "       << enrollment.userid()       << std::endl;
    std::cout << "\tDevice ID: "     << enrollment.deviceid()     << std::endl;
    std::cout << "\tCreated: "
        << google::protobuf::util::TimeUtil::ToString(enrollment.createdat())
        << std::endl;
    std::cout << "\tUpdated: "
        << google::protobuf::util::TimeUtil::ToString(enrollment.updatedat())
        << std::endl;
    std::cout << "\tID: "            << enrollment.id()    << std::endl;
}
```

##### Deleting Enrollments

```c++
// The UUID of the enrollment to delete.
std::string enrollmentID = "45ad3215-1d4c-42aa-aec4-2724e9ce1d99";

sensory::api::v1::management::EnrollmentResponse rsp;
auto status = mgmtService.deleteEnrollment(&rsp, enrollmentID);
if (!status.ok()) {  // The call failed, print a descriptive message.
    std::cout << "Failed to delete enrollment with\n\t" <<
        status.error_code() << ": " << status.error_message() << std::endl;
}
```

##### Fetching Enrollment Groups

```c++
// The name of the user to fetch enrollment groups for.
std::string userID = "user";

// Create a response for the RPC.
sensory::api::v1::management::GetEnrollmentGroupsResponse rsp;
status = mgmtService.getEnrollmentGroups(&rsp, userID);
if (!status.ok()) {  // The call failed, print a descriptive message.
    std::cout << "Failed to get enrollment groups with\n\t" <<
        status.error_code() << ": " << status.error_message() << std::endl;
    // Handle error...
}
for (auto& enrollment : rsp.enrollmentgroups()) {
    std::cout << "Description: "     << enrollment.description()  << std::endl;
    std::cout << "\tModel Name: "    << enrollment.modelname()    << std::endl;
    std::cout << "\tModel Type: "    << enrollment.modeltype()    << std::endl;
    std::cout << "\tModel Version: " << enrollment.modelversion() << std::endl;
    std::cout << "\tUser ID: "       << enrollment.userid()       << std::endl;
    std::cout << "\tCreated: "
        << google::protobuf::util::TimeUtil::ToString(enrollment.createdat())
        << std::endl;
    std::cout << "\tUpdated: "
        << google::protobuf::util::TimeUtil::ToString(enrollment.updatedat())
        << std::endl;
    std::cout << "\tID: "            << enrollment.id()    << std::endl;
}
```

##### Creating Enrollment Groups

```c++
TODO
```

##### Appending Enrollment Groups

```c++
TODO
```

##### Deleting Enrollment Groups

```c++
// The UUID of the group to delete.
std::string groupID = "13481e19-5853-47d0-ba61-6819914405bb";

sensory::api::v1::management::EnrollmentGroupResponse rsp;
auto status = mgmtService.deleteEnrollmentGroup(&rsp, groupID);
if (!status.ok()) {  // The call failed, print a descriptive message.
    std::cout << "Failed to delete enrollment group with\n\t" <<
        status.error_code() << ": " << status.error_message() << std::endl;
}
```

## Development

To generate the developing makefiles for the SDK, execute the following:

```shell
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
./cs.sh benchmark
```
