# Tutorial

The tutorial provides an overview of the Sensory Cloud C++ SDK. This tutorial
will show you how to:

-   setup a C++ project and include the Sensory Cloud C++ SDK in your build,
-   connect to your server and query the server's health,
-   implement secure credential storage, a token manager, and an OAuth service
    for maintaining access tokens,
-   interact with biometric video and audio services, and
-   manage enrollments and enrollment groups for your organization.

Because this SDK is built on [gRPC][gRPC], you may benefit from referring to
the [gRPC Tutorial][gRPC Tutorial] and [gRPC Documentation][gRPC Documentation]
to familiarize yourself with some of the gRPC structures and patterns that are
used in this SDK and tutorial.

[gRPC]: https://www.grpc.io/
[gRPC Tutorial]: https://www.grpc.io/docs/languages/cpp/basics/
[gRPC Documentation]: https://grpc.github.io/grpc/cpp/annotated.html

The examples in this tutorial are based on the SDK's synchronous blocking
interface. For an asynchronous interface based on an event-loop pattern using
`grpc::CompletionQueue`, please refer to
[async_event_loop.md](async_event_loop.md). For an asynchronous interface based
on callback and reactor patterns, refer to
[async_callback.md](async_callback.md).

## Project Setup

The recommended build system for projects based on this SDK is
[`cmake`](https://cmake.org/). To include the SDK in your project , the simplest
solution is to add a `FetchContent` block to your `CMakeLists.txt`. This
approach will result in the compilation and linkage of the protobuff and gRPC
dependencies locally.

```cmake
cmake_minimum_required(VERSION 3.5.1)
project(Your Project)

# Output built binaries to a `build` directory instead of the project root.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build")

# C++11 with gcc >4.9 is required to compile the Sensory Cloud C++ SDK.
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(sensorycloud
    GIT_REPOSITORY https://github.com/sensorycloud/cpp-sdk.git
    GIT_TAG        v0.1.0
)
FetchContent_MakeAvailable(sensorycloud)

...

# Define and executable and link to the sensorycloud library.
add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world PRIVATE sensorycloud)
```

<!-- TODO: Document installation with global protobuff / gRPC -->

## Connecting to the server

To connect to your Sensory Inference server, you will need to know:

1.  the host-name or IP address that the service is running at,
2.  the specific port that the service is running at -- typically `443`, --
3.  your tenant ID that uniquely identifies your tenant in Sensory Cloud, and
4.  a way of uniquely identifying devices in your application.

This information is provided to the SDK using a `sensory::Config` object:

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "io.stage.cloud.sensory.com",            // the host name of the server
    443,                                     // the port number of the service
    "a376234e-5b4b-4acb-bdbc-8cac8c397ace",  // your tenant ID
    "4e07cce1-cccb-4630-a2d1-5da71e3c85a3"   // a unique device ID
);
```

A single instance of `sensory::Config` should be instantiated and maintained
per instance of an application on a device. Using multiple configs for the same
service in the same instance of an application may result in undefined behavior.

## Checking The Server Health

It's important to check the health of your Sensory Inference server. You can do
so via the following:

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "io.stage.cloud.sensory.com",            // the host name of the server
    443,                                     // the port number of the service
    "a376234e-5b4b-4acb-bdbc-8cac8c397ace",  // your tenant ID
    "4e07cce1-cccb-4630-a2d1-5da71e3c85a3"   // a unique device ID
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

## Secure Credential Storage

`SecureCredentialStore` provides the interface for secure credential storage
that must be implemented for your specific usage. Included with the SDK are
reference implementations for certain build platforms including:

-   MacOS (through [Keychain Services][Keychain-Services]),
-   Windows (through [Credential Locker][Credential-Locker]), and
-   Linux (through [Libsecret][Libsecret]).

[Keychain-Services]: https://developer.apple.com/documentation/security/keychain_services
[Credential-Locker]: https://docs.microsoft.com/en-us/windows/uwp/security/credential-locker
[Libsecret]: https://wiki.gnome.org/Projects/Libsecret

```c++
/// @brief A keychain manager for interacting with the OS credential manager.
struct SecureCredentialStore {
    /// @brief Emplace or replace a key/value pair in the secure credential
    /// store.
    ///
    /// @param key the plain-text key of the value to store
    /// @param value the secure value to store
    /// @details
    /// Unlike most key-value store abstractions in the STL, this
    /// implementation of emplace will overwrite existing values in the
    /// key-value store.
    ///
    inline void emplace(const std::string& key, const std::string& value) const;

    /// @brief Return true if the key exists in the secure credential store.
    ///
    /// @param key the plain-text key to check for the existence of
    ///
    inline bool contains(const std::string& key) const;

    /// @brief Look-up a secret value in the secure credential store.
    ///
    /// @param key the plain-text key of the value to return
    /// @returns the secret value indexed by the given key
    ///
    inline std::string at(const std::string& key) const;

    /// @brief Remove a secret key-value pair in the secure credential store.
    ///
    /// @param key the plain-text key of the pair to remove from the keychain
    ///
    inline void erase(const std::string& key) const;
};
```

## Creating a `TokenManager`

The `TokenManger` template class handles requesting OAuth tokens when
necessary. It utilizes an `OAuthService` to request new tokens when the local
ones expire, and also utilizes a `SecureCredentialStore` implementation to
securely store tokens. All secure services require a reference to a token
manager.

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "io.stage.cloud.sensory.com",            // the host name of the server
    443,                                     // the port number of the service
    "a376234e-5b4b-4acb-bdbc-8cac8c397ace",  // your tenant ID
    "4e07cce1-cccb-4630-a2d1-5da71e3c85a3"   // a unique device ID
);

// Create the secure credential store for the token manager. You may use your
// own implementation of the `SecureCredentialStore` interface for your
// application.
sensory::token_manager::SecureCredentialStore keychain("com.product.company");
// Create the OAuth service from the configuration.
sensory::service::OAuthService oauthService(config);
// Create the token manager for handling token requests from the OAuth service
// and with a reference to the secure credential store.
sensory::token_manager::TokenManager<sensory::token_manager::SecureCredentialStore>
    tokenManager(oauthService, keychain);
```

## Registering OAuth Credentials

OAuth credentials should be registered once per unique machine. Registration is
very simple and is provided as part of the SDK. The below example shows how to
create an `OAuthService` and register a device for the first time.

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "io.stage.cloud.sensory.com",            // the host name of the server
    443,                                     // the port number of the service
    "a376234e-5b4b-4acb-bdbc-8cac8c397ace",  // your tenant ID
    "4e07cce1-cccb-4630-a2d1-5da71e3c85a3"   // a unique device ID
);

// Create the secure credential store for the token manager. You may use your
// own implementation of the `SecureCredentialStore` interface for your
// application.
sensory::token_manager::SecureCredentialStore keychain("com.product.company");
// Create the OAuth service from the configuration.
sensory::service::OAuthService oauthService(config);
// Create the token manager for handling token requests from the OAuth service
// and with a reference to the secure credential store.
sensory::token_manager::TokenManager<sensory::token_manager::SecureCredentialStore>
    tokenManager(oauthService, keychain);

if (!tokenManager.hasSavedCredentials()) {  // The device is not registered.
    // Generate a new `clientID` and `clientSecret` for this device and store
    // them securely in the keychain.
    auto credentials = tokenManager.generateCredentials();
    // The name of the device as a human-readable text string
    std::string deviceName("a device in your system");
    // Use a shared secret, i.e., pass-phrase to authenticate the device.
    std::string insecureSharedSecret("password");
    // Create a response for the RPC.
    sensory::api::v1::management::DeviceResponse response;
    // Perform the RPC and check the status for errors.
    auto status = oauthService.registerDevice(&response,
        deviceName,
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

## Creating an `AudioService`

`AudioService` provides methods to stream audio to Sensory Cloud. It is
recommended to only have 1 instance of `AudioService` instantiated per
`Config`. In most circumstances you will only ever have one `Config`, unless
your app communicates with multiple Sensory Cloud servers.

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "io.stage.cloud.sensory.com",            // the host name of the server
    443,                                     // the port number of the service
    "a376234e-5b4b-4acb-bdbc-8cac8c397ace",  // your tenant ID
    "4e07cce1-cccb-4630-a2d1-5da71e3c85a3"   // a unique device ID
);

// Create the secure credential store for the token manager. You may use your
// own implementation of the `SecureCredentialStore` interface for your
// application.
sensory::token_manager::SecureCredentialStore keychain("com.product.company");
// Create the OAuth service from the configuration.
sensory::service::OAuthService oauthService(config);
// Create the token manager for handling token requests from the OAuth service
// and with a reference to the secure credential store.
sensory::token_manager::TokenManager<sensory::token_manager::SecureCredentialStore>
    tokenManager(oauthService, keychain);

// Create the audio service based on the configuration and token manager.
sensory::service::AudioService<sensory::token_manager::SecureCredentialStore>
    audioService(config, tokenManager);
```

### Obtaining Audio Models

Certain audio models are available to your application depending on the models
that are configured for your instance of Sensory Cloud. In order to determine
which audio models are accessible to you, you can execute the below code.

```c++
// Create a response for the RPC.
sensory::api::v1::audio::GetModelsResponse response;
// Execute the RPC and check the status for errors.
auto status = audioService.getModels(&response);
if (!status.ok()) {  // The call failed, print a descriptive message.
    std::cout << "Failed to get audio models with\n\t" <<
        status.error_code() << ": " << status.error_message() << std::endl;
    // Handle error...
}
```

### Enrolling With Audio

```c++
TODO
```

### Authenticating With Audio

```c++
TODO
```

### Audio Events

```c++
TODO
```

### Audio Transcription

```c++
TODO
```

## Creating a `VideoService`

`VideoService` provides methods to stream images to Sensory Cloud. It is
recommended to only have 1 instance of `VideoService` instantiated per
`Config`. In most circumstances you will only ever have one `Config`, unless
your app communicates with multiple Sensory Cloud servers.

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "io.stage.cloud.sensory.com",            // the host name of the server
    443,                                     // the port number of the service
    "a376234e-5b4b-4acb-bdbc-8cac8c397ace",  // your tenant ID
    "4e07cce1-cccb-4630-a2d1-5da71e3c85a3"   // a unique device ID
);

// Create the secure credential store for the token manager. You may use your
// own implementation of the `SecureCredentialStore` interface for your
// application.
sensory::token_manager::SecureCredentialStore keychain("com.product.company");
// Create the OAuth service from the configuration.
sensory::service::OAuthService oauthService(config);
// Create the token manager for handling token requests from the OAuth service
// and with a reference to the secure credential store.
sensory::token_manager::TokenManager<sensory::token_manager::SecureCredentialStore>
    tokenManager(oauthService, keychain);

// Create the video service based on the configuration and token manager.
sensory::service::VideoService<sensory::token_manager::SecureCredentialStore>
    videoService(config, tokenManager);
```

### Obtaining Video Models

Certain video models are available to your application depending on the models
that are configured for your instance of Sensory Cloud. In order to determine
which video models are accessible to you, you can execute the below code.

```c++
// Create a response for the RPC.
sensory::api::v1::video::GetModelsResponse response;
// Execute the RPC and check the status for errors.
auto status = videoService.getModels(&response);
if (!status.ok()) {  // The call failed, print a descriptive message.
    std::cout << "Failed to get video models with\n\t" <<
        status.error_code() << ": " << status.error_message() << std::endl;
    // Handle error...
}
```

### Enrolling With Video

The synchronous API provides a blocking interface for streaming data to the
server.

```c++
// The name of the biometric model to enroll the user with.
std::string modelName("face_biometric_hektor");
// The unique ID of the user that is being enrolled.
std::string userId("60db6966-068f-4f6c-9a51-d2a3308db09b");
// A human readable description of the enrollment.
std::string enrollmentDescription("My Enrollment");
// Whether to perform a liveness check while executing the enrollment.
bool isLivenessEnabled(false);

// Create the gRPC stream for a certain enroll-able model for a particular user.
auto stream = videoService.createEnrollment(
    modelName,
    userId,
    enrollmentDescription,
    isLivenessEnabled);

// Encode your image data using JPEG compression.
std::vector<unsigned char> buffer;
encodeJPEG(yourImageData, buffer);
// Create the request from the encoded image data and write it to the stream.
// This call will block the running thread until completion.
sensory::api::v1::video::CreateEnrollmentRequest request;
request.set_imagecontent(buffer.data(), buffer.size());
stream->Write(request);
// Read a response from the server. This call will block the running thread
// until completion.
sensory::api::v1::video::CreateEnrollmentResponse response;
stream->Read(&response);
```

Instances of `CreateEnrollmentResponse` will have the following attributes:

-   `percentcomplete()` the completion percentage of the enrollment procedure.
-   `isalive()` a boolean determining whether a live face was detected in the
    frame. This attribute can only be true when creating an enrollment stream
    with liveness enabled.
-   `enrollmentid()` The ID of the enrollment _if_ it was created. This value
    is populated upon successful enrollment with the model.
-   `modelname()` The name of the model being used to perform the enrollment
-   `modelversion()` The version of the model being used to perform the
    enrollment

### Authenticating With Video

The synchronous API provides a blocking interface for streaming data to the
server.

```c++
// The ID for the enrollment for a particular user
std::string enrollmentID("60db6966-068f-4f6c-9a51-d2a3308db09b");
// a flag determining whether a liveness check should be conducted before
// authenticating against the enrollment.
bool isLivenessEnabled(false);
// The security threshold for the optional liveness check.
auto threshold = sensory::api::v1::video::RecognitionThreshold::LOW;

// Create the gRPC stream for to authenticate an enrollment for a particular
// user based on enrollment ID.
auto stream = videoService.authenticate(
    enrollmentID,
    isLivenessEnabled,
    threshold);

// Encode your image data using JPEG compression.
std::vector<unsigned char> buffer;
encodeJPEG(yourImageData, buffer);
// Create the request from the encoded image data and write it to the stream.
// This call will block the running thread until completion.
sensory::api::v1::video::AuthenticateRequest request;
request.set_imagecontent(buffer.data(), buffer.size());
stream->Write(request);
// Read a response from the server. This call will block the running thread
// until completion.
sensory::api::v1::video::AuthenticateResponse response;
stream->Read(&response);
```

Instances of `AuthenticateResponse` will have the following attributes:

-   `response.success()` a boolean determining whether the authentication was
    successful. The stream will terminate if/when this flag goes to `true`.
-   `response.score()` The score from the liveness model.
-   `response.isalive()` A boolean determining whether the last frame was
    detected as being live based on the score and selected threshold. This can
    only be `true` when liveness was enabled from the call to create the stream.

### Video Liveness

The synchronous API provides a blocking interface for streaming data to the
server.

```c++
// The particular model to use for liveness detection.
std::string videoModel("face_recognition_mathilde");
// The unique user ID of the user being validated for liveness
std::string userId("60db6966-068f-4f6c-9a51-d2a3308db09b");
// The security threshold for the optional liveness check.
auto threshold = sensory::api::v1::video::RecognitionThreshold::LOW;

// Create the gRPC stream for to authenticate an enrollment for a particular
// user based on enrollment ID.
auto stream = videoService.validateLiveness(videoModel, userID, threshold);

// Encode your image data using JPEG compression.
std::vector<unsigned char> buffer;
encodeJPEG(yourImageData, buffer);
// Create the request from the encoded image data and write it to the stream.
// This call will block the running thread until completion.
sensory::api::v1::video::ValidateRecognitionRequest request;
request.set_imagecontent(buffer.data(), buffer.size());
stream->Write(request);
// Read a response from the server. This call will block the running thread
// until completion.
sensory::api::v1::video::LivenessRecognitionResponse response;
stream->Read(&response);
```

Instances of `LivenessRecognitionResponse` will have the following attributes:

-   `response.score()` The score from the liveness model.
-   `response.isalive()` A boolean determining whether the last frame was
    detected as being live based on the score and selected threshold.

## Creating A Management Service

The `ManagementService` is used to manage typical _CRUD_ operations with
Sensory Cloud, such as deleting enrollments or creating enrollment groups.

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "io.stage.cloud.sensory.com",            // the host name of the server
    443,                                     // the port number of the service
    "a376234e-5b4b-4acb-bdbc-8cac8c397ace",  // your tenant ID
    "4e07cce1-cccb-4630-a2d1-5da71e3c85a3"   // a unique device ID
);

// Create the secure credential store for the token manager. You may use your
// own implementation of the `SecureCredentialStore` interface for your
// application.
sensory::token_manager::SecureCredentialStore keychain("com.product.company");
// Create the OAuth service from the configuration.
sensory::service::OAuthService oauthService(config);
// Create the token manager for handling token requests from the OAuth service
// and with a reference to the secure credential store.
sensory::token_manager::TokenManager<sensory::token_manager::SecureCredentialStore>
    tokenManager(oauthService, keychain);

// Create the management service based on the configuration and token manager.
sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>
    mgmtService(config, tokenManager);
```

### Fetching Enrollments

```c++
// The name of the user to fetch enrollments for.
std::string userID = "user";

// Create a response for the RPC.
sensory::api::v1::management::GetEnrollmentsResponse response;
auto status = mgmtService.getEnrollments(&response, userID);
if (!status.ok()) {  // The call failed, print a descriptive message.
    std::cout << "Failed to get enrollments with\n\t" <<
        status.error_code() << ": " << status.error_message() << std::endl;
    // Handle error...
}
for (auto& enrollment : response.enrollments()) {
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

### Deleting Enrollments

```c++
// The UUID of the enrollment to delete.
std::string enrollmentID = "45ad3215-1d4c-42aa-aec4-2724e9ce1d99";

sensory::api::v1::management::EnrollmentResponse response;
auto status = mgmtService.deleteEnrollment(&response, enrollmentID);
if (!status.ok()) {  // The call failed, print a descriptive message.
    std::cout << "Failed to delete enrollment with\n\t" <<
        status.error_code() << ": " << status.error_message() << std::endl;
}
```

### Fetching Enrollment Groups

```c++
// The name of the user to fetch enrollment groups for.
std::string userID = "user";

// Create a response for the RPC.
sensory::api::v1::management::GetEnrollmentGroupsResponse response;
status = mgmtService.getEnrollmentGroups(&response, userID);
if (!status.ok()) {  // The call failed, print a descriptive message.
    std::cout << "Failed to get enrollment groups with\n\t" <<
        status.error_code() << ": " << status.error_message() << std::endl;
    // Handle error...
}
for (auto& enrollment : response.enrollmentgroups()) {
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

### Creating Enrollment Groups

```c++
TODO
```

### Appending Enrollment Groups

```c++
TODO
```

### Deleting Enrollment Groups

```c++
// The UUID of the group to delete.
std::string groupID = "13481e19-5853-47d0-ba61-6819914405bb";

sensory::api::v1::management::EnrollmentGroupResponse response;
auto status = mgmtService.deleteEnrollmentGroup(&response, groupID);
if (!status.ok()) {  // The call failed, print a descriptive message.
    std::cout << "Failed to delete enrollment group with\n\t" <<
        status.error_code() << ": " << status.error_message() << std::endl;
}
```
