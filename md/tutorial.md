# Tutorial

The tutorial provides an overview of the SensoryCloud C++ SDK. This tutorial
will show you how to:

-   setup a C++ project and include the SensoryCloud C++ SDK in your build,
-   connect to your server and query the server's health,
-   implement secure credential storage, a token manager, and an OAuth service
    for maintaining access tokens,
-   interact with biometric video and audio services, and
-   manage enrollments and enrollment groups.

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
# CMake version 3.11 is required to use FetchContent
cmake_minimum_required(VERSION 3.11)

# C++11 with gcc >4.9 is required to compile the SensoryCloud C++ SDK.
set(CMAKE_CXX_STANDARD 11)

# Use FetchContent to compile the SDK with your project.
include(FetchContent)
FetchContent_Declare(sensorycloud
    GIT_REPOSITORY https://github.com/sensorycloud/cpp-sdk.git
    GIT_TAG        v0.1.0
)
FetchContent_MakeAvailable(sensorycloud)

...

# Define an executable and link with libsensorycloud.
add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world PRIVATE sensorycloud)
```

## Connecting to the server

To connect to your Sensory Inference server, you will need to know:

1.  the host-name or IP address that the service is running at,
2.  the specific port that the service is running at -- typically `443`, --
3.  your tenant ID that uniquely identifies your tenant in SensoryCloud, and
4.  a way of uniquely identifying devices in your application.

This information is provided to the SDK using a `sensory::Config` object:

```c++
// Create a configuration specific to your tenant and device.
sensory::Config config(
    "example.company.com",                   // the host name of the server
    443,                                     // the port number of the service
    "a376234e-5b4b-4acb-bdbc-8cac8c397ace",  // your tenant ID
    "4e07cce1-cccb-4630-a2d1-5da71e3c85a3",  // a unique device ID
    true                                     // a flag for disabling TLS
);
// Create the gRPC channel and open an http/2 connection to the server.
config.connect();
```

A single instance of `sensory::Config` should be instantiated and maintained
per instance of an application on a device. Using multiple configs for the same
service in the same instance of an application may result in undefined behavior.
You must call `connect` to generate the gRPC channel and open the HTTP/2
connection to the server.

## Checking The Server Health

It's important to check the health of your Sensory Inference server. You can do
so using the `HealthService` via the following:

```c++
// Create a health service for performing server health queries.
sensory::service::HealthService healthService(config);
// Create a response for the RPC.
sensory::api::common::ServerHealthResponse response;
// Perform the RPC and check the status for errors.
auto status = healthService.getHealth(&response);
if (!status.ok()) {  // The call failed, handle the error here.
    auto errorCode = status.error_code();
    auto errorMessage = status.error_message();
} else {  // The call succeeded, handle the response here.
    auto isHealthy = response.ishealthy();
    auto serverVersion = response.serverversion();
    auto serverID = response.id();
}
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
ones expire and also utilizes a `SecureCredentialStore` implementation to
securely store tokens. All secure services (i.e., video, management, and audio)
require a reference to a token manager.

```c++
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
if (!tokenManager.hasToken()) {  // The device is not registered.
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
        deviceName, insecureSharedSecret, credentials.id, credentials.secret);
    if (!status.ok()) {  // The call failed, handle the error here
        auto errorCode = status.error_code();
        auto errorMessage = status.error_message();
    }
}
```

## Creating an `AudioService`

`AudioService` provides methods to stream audio to SensoryCloud. It is
recommended to only have 1 instance of `AudioService` instantiated per `Config`.

```c++
// Create the audio service using the configuration and token manager.
sensory::service::AudioService<sensory::token_manager::SecureCredentialStore>
    audioService(config, tokenManager);
```

### Obtaining Audio Models

Certain audio models are available to your application depending on the models
that are configured for your instance of SensoryCloud. In order to determine
which audio models are accessible to you, you can execute the below code.

```c++
// Create a response for the RPC.
sensory::api::v1::audio::GetModelsResponse response;
// Execute the RPC and check the status for errors.
auto status = audioService.getModels(&response);
if (!status.ok()) {  // The call failed, handle the error here
    auto errorCode = status.error_code();
    auto errorMessage = status.error_message();
    // ...
}
```

### Enrolling With Audio

To create an audio enrollment:

```c++
// The sample rate of the audio stream.
uint32_t sampleRate = 16000;
// The IETF BCP 47 language tag for the input audio.
std::string language = "en-US";
// The name of the biometric model to enroll the user with.
std::string model("wakeword-16kHz-alexa.ubm");
// The unique ID of the user that is being enrolled.
std::string userID("60db6966-068f-4f6c-9a51-d2a3308db09b");
// A human readable description of the enrollment.
std::string description("My Enrollment");
// Whether to perform a liveness check while executing the enrollment.
bool isLivenessEnabled(false);
// The maximum duration for a text-independent enrollment
float duration = 0.f;
// The number of utterances for a text-dependent enrollment
int numUtterances = 2;

// Create the gRPC stream for a certain enroll-able model for a particular user.
grpc::ClientContext context;
auto stream = audioService.createEnrollment(&context,
    sensory::service::audio::newAudioConfig(
        sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
        sampleRate, 1, language
    ),
    sensory::service::audio::newCreateEnrollmentConfig(
        model,
        userID,
        description,
        isLivenessEnabled,
        duration,
        numUtterances
    )
);

// Encode your audio date as 16-bit PCM
std::vector<unsigned char> buffer = getChunkOfSamples();
// Create the request from the encoded image data and write it to the stream.
// This call will block the running thread until completion.
sensory::api::v1::audio::CreateEnrollmentRequest request;
request.set_imagecontent(buffer.data(), buffer.size());
if (!stream->Write(request)) {
    // Write failed, break out of IO loop
}
// Read a response from the server. This call will block the running thread
// until completion.
sensory::api::v1::audio::CreateEnrollmentResponse response;
if (!stream->Read(&response)) {
    // Read failed, break out of IO loop
}
```

Instances of `CreateEnrollmentResponse` will have the following attributes:

-   `response.modelprompt()` The interactive prompt for the user to respond to.
    For enrollments with liveness this may be a string of digits like
    `"1 7 4 8 4 3"`. For enrolled wakewords this would be a wakeword such as
    `"computer"`.
-   `response.percentcomplete()` the completion percentage of the enrollment
    procedure as an integer between 0 and 100 (inclusive).
-   `response.percentsegmentcomplete()` the completion percentage of the
    particular segment in an enrollment procedure as an integer between 0 and
    100 (inclusive).
-   `response.audioenergy()` The level of the audio as a floating point value
    between 0 and 1 (inclusive).
-   `enrollmentid()` The ID of the enrollment if it was created. This value
    is populated upon successful enrollment with the model.
-   `modelname()` The name of the model used to perform the enrollment. This
    value is populated upon successful enrollment with the model.
-   `modelversion()` The version of the model being used to perform the
    enrollment. This value is populated upon successful enrollment with the
    model.

### Authenticating With Audio

To authenticate against an existing enrollment based on a known enrollment ID:

```c++
// The sample rate of the audio stream.
uint32_t sampleRate = 16000;
// The IETF BCP 47 language tag for the input audio.
std::string language = "en-US";
// The unique ID of the user that is being enrolled.
std::string enrollmentID("60db6966-068f-4f6c-9a51-d2a3308db09b");
// Whether to perform a liveness check while executing the enrollment.
bool isLivenessEnabled(false);
// The sensitivity level of the model to input audio levels
auto sensitivity = sensory::api::v1::audio::ThresholdSensitivity::LOW;
// The security threshold of the model in terms of FAs
auto threshold = sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW
// Whether the enrollment is an enrollment group or an individual enrollment
bool isEnrollmentGroup(false);

// Create the gRPC stream for a certain enroll-able model for a particular user.
grpc::ClientContext context;
auto stream = audioService.authenticate(&context,
    sensory::service::audio::newAudioConfig(
        sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
        sampleRate, 1, language
    ),
    sensory::service::audio::newAuthenticateConfig(
        enrollmentID,
        isLivenessEnabled,
        sensitivity,
        threshold,
        isEnrollmentGroup
    )
);

// Encode your audio date as 16-bit PCM
std::vector<unsigned char> buffer = getChunkOfSamples();
// Create the request from the encoded image data and write it to the stream.
// This call will block the running thread until completion.
sensory::api::v1::audio::AuthenticateRequest request;
request.set_imagecontent(buffer.data(), buffer.size());
if (!stream->Write(request)) {
    // Write failed, break out of IO loop
}
// Read a response from the server. This call will block the running thread
// until completion.
sensory::api::v1::audio::AuthenticateResponse response;
if (!stream->Read(&response)) {
    // Read failed, break out of IO loop
}
```

Instances of `AuthenticateResponse` will have the following attributes:

-   `response.modelprompt()` The interactive prompt for the user to respond to.
    For enrollments with liveness this may be a string of digits like
    `"1 7 4 8 4 3"`. For enrolled wakewords this would be a wakeword such as
    `"computer"`.
-   `response.percentsegmentcomplete()` the completion percentage of the
    particular segment in an authentication procedure as an integer between 0
    and 100 (inclusive).
-   `response.audioenergy()` The level of the audio as a floating point value
    between 0 and 1 (inclusive).
-   `response.success()` A flag that will be true when the user successfully
    authenticates against the enrollment

### Enrolling sound events

To enroll sound events:

```c++
TODO
```

### Validating enrolled sound events

To validate enrolled sound events against existing enrollments based on an
enrollment ID:

```c++
TODO
```

### Audio Events

To detect audio trigger events, such as door knocks, coughs, glass breaks, etc.

```c++
// The sample rate of the audio stream.
uint32_t sampleRate = 16000;
// The IETF BCP 47 language tag for the input audio.
std::string language = "en-US";
// The name of the biometric model to enroll the user with.
std::string model("wakeword-16kHz-alexa.ubm");
// The unique ID of the user that is being enrolled.
std::string userID("60db6966-068f-4f6c-9a51-d2a3308db09b");
// The detection threshold for controlling the likelihood of an FA.
auto threshold = sensory::api::v1::audio::ThresholdSensitivity::LOW;

// Create the gRPC stream for a certain enroll-able model for a particular user.
grpc::ClientContext context;
auto stream = audioService.validateEvent(&context,
    sensory::service::audio::newAudioConfig(
        sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
        sampleRate, 1, language
    ),
    sensory::service::audio::newValidateEventConfig(model, userID, threshold)
);

// Encode your audio date as 16-bit PCM
std::vector<unsigned char> buffer = getChunkOfSamples();
// Create the request from the encoded image data and write it to the stream.
// This call will block the running thread until completion.
sensory::api::v1::audio::ValidateEventRequest request;
request.set_imagecontent(buffer.data(), buffer.size());
if (!stream->Write(request)) {
    // Write failed, break out of IO loop
}
// Read a response from the server. This call will block the running thread
// until completion.
sensory::api::v1::audio::ValidateEventResponse response;
if (!stream->Read(&response)) {
    // Read failed, break out of IO loop
}
```

Instances of `ValidateEventResponse` will have the following attributes:

-   `response.audioenergy()` The level of the audio as a floating point value
    between 0 and 1 (inclusive).
-   `response.success()` A flag determining whether the modeled event was
    detected.
-   `response.resultid()` A string that describes the particular event that was
    detected if `response.success()` is `true`.
-   `response.score()` The score from the model based on the last processed
    chunk of audio data.

### Audio Transcription

To transcribe human speech into text (based on known language):

```c++
TODO
```

## Creating a `VideoService`

`VideoService` provides methods to stream images to SensoryCloud. It is
recommended to only have 1 instance of `VideoService` instantiated per
`Config`. In most circumstances you will only ever have one `Config`, unless
your app communicates with multiple SensoryCloud servers.

```c++
// Create the video service based on the configuration and token manager.
sensory::service::VideoService<sensory::token_manager::SecureCredentialStore>
    videoService(config, tokenManager);
```

### Obtaining Video Models

Certain video models are available to your application depending on the models
that are configured for your instance of SensoryCloud. In order to determine
which video models are accessible to you, you can execute the below code.

```c++
// Create a response for the RPC.
sensory::api::v1::video::GetModelsResponse response;
// Execute the RPC and check the status for errors.
auto status = videoService.getModels(&response);
if (!status.ok()) {  // The call failed, handle the error here
    auto errorCode = status.error_code();
    auto errorMessage = status.error_message();
    // ...
}
```

### Enrolling With Video

To create a new enrollment for a user with a video stream:

```c++
// The name of the biometric model to enroll the user with.
std::string model("face_biometric_hektor");
// The unique ID of the user that is being enrolled.
std::string userID("60db6966-068f-4f6c-9a51-d2a3308db09b");
// A human readable description of the enrollment.
std::string description("My Enrollment");
// Whether to perform a liveness check while executing the enrollment.
bool isLivenessEnabled(false);
// the threshold for the liveness check
auto threshold = sensory::api::v1::video::RecognitionThreshold::LOW;

// Create the gRPC stream for a certain enroll-able model for a particular user.
auto stream = videoService.createEnrollment(
    sensory::service::video::newCreateEnrollmentConfig(
        model,
        userID,
        description,
        isLivenessEnabled,
        threshold
    )
);

// Encode your image data using JPEG compression.
std::vector<unsigned char> buffer;
encodeJPEG(yourImageData, buffer);
// Create the request from the encoded image data and write it to the stream.
// This call will block the running thread until completion.
sensory::api::v1::video::CreateEnrollmentRequest request;
request.set_imagecontent(buffer.data(), buffer.size());
if (!stream->Write(request)) {
    // Write failed, break out of IO loop
}
// Read a response from the server. This call will block the running thread
// until completion.
sensory::api::v1::video::CreateEnrollmentResponse response;
if (!stream->Read(&response)) {
    // Read failed, break out of IO loop
}
```

Instances of `CreateEnrollmentResponse` will have the following attributes:

-   `percentcomplete()` the completion percentage of the enrollment procedure
    as an integer between 0 and 100 (inclusive).
-   `isalive()` a boolean determining whether a live face was detected in the
    frame based on the score and selected security threshold. This attribute
    can only be true when the enrollment stream was started with liveness
    enabled.
-   `enrollmentid()` The ID of the enrollment if it was created. This value
    is populated upon successful enrollment with the model.
-   `modelname()` The name of the model used to perform the enrollment. This
    value is populated upon successful enrollment with the model.
-   `modelversion()` The version of the model being used to perform the
    enrollment. This value is populated upon successful enrollment with the
    model.

### Authenticating With Video

To authenticate with a video stream:

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
    sensory::service::video::newAuthenticateConfig(
        enrollmentID,
        isLivenessEnabled,
        threshold
    )
);

// Encode your image data using JPEG compression.
std::vector<unsigned char> buffer;
encodeJPEG(yourImageData, buffer);
// Create the request from the encoded image data and write it to the stream.
// This call will block the running thread until completion.
sensory::api::v1::video::AuthenticateRequest request;
request.set_imagecontent(buffer.data(), buffer.size());
if (!stream->Write(request)) {
    // Write failed, break out of IO loop
}
// Read a response from the server. This call will block the running thread
// until completion.
sensory::api::v1::video::AuthenticateResponse response;
if (!stream->Read(&response)) {
    // Read failed, break out of IO loop
}
```

Instances of `AuthenticateResponse` will have the following attributes:

-   `response.success()` a boolean determining whether the authentication was
    successful. The stream will terminate if/when this flag goes to `true`.
    If liveness is enabled the stream will terminated with this flag goes `true`
    and the `response.isalive()` flag evaluates to `true`.
-   `response.score()` The score from the liveness model.
-   `response.isalive()` A boolean determining whether the last frame was
    detected as being live based on the score and selected threshold. This can
    only be `true` when liveness was enabled from the call to create the stream.

### Video Liveness

To validate liveness with a video stream:

```c++
// The particular model to use for liveness detection.
std::string videoModel("face_recognition_mathilde");
// The unique user ID of the user being validated for liveness
std::string userID("60db6966-068f-4f6c-9a51-d2a3308db09b");
// The security threshold for the optional liveness check.
auto threshold = sensory::api::v1::video::RecognitionThreshold::LOW;

// Create the gRPC stream for to authenticate an enrollment for a particular
// user based on enrollment ID.
auto stream = videoService.validateLiveness(
    sensory::service::video::newValidateRecognitionConfig(
        videoModel,
        userID,
        threshold
    )
);

// Encode your image data using JPEG compression.
std::vector<unsigned char> buffer;
encodeJPEG(yourImageData, buffer);
// Create the request from the encoded image data and write it to the stream.
// This call will block the running thread until completion.
sensory::api::v1::video::ValidateRecognitionRequest request;
request.set_imagecontent(buffer.data(), buffer.size());
if (!stream->Write(request)) {
    // Write failed, break out of IO loop
}
// Read a response from the server. This call will block the running thread
// until completion.
sensory::api::v1::video::LivenessRecognitionResponse response;
if (!stream->Read(&response)) {
    // Read failed, break out of IO loop
}
```

Instances of `LivenessRecognitionResponse` will have the following attributes:

-   `response.score()` The score from the liveness model.
-   `response.isalive()` A boolean determining whether the last frame was
    detected as being live based on the score and selected threshold.

## Creating A Management Service

The `ManagementService` is used to manage typical _CRUD_ operations with
SensoryCloud, such as deleting enrollments or creating enrollment groups.

```c++
// Create the management service based on the configuration and token manager.
sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>
    mgmtService(config, tokenManager);
```

### Fetching Enrollments

To fetch enrollments for a particular user:

```c++
// The name of the user to fetch enrollments for.
std::string userID = "user";

// Create a response for the RPC.
sensory::api::v1::management::GetEnrollmentsResponse response;
auto status = mgmtService.getEnrollments(&response, userID);
if (!status.ok()) {  // The call failed, handle the error here
    auto errorCode = status.error_code();
    auto errorMessage = status.error_message();
    // ...
}
for (auto& enrollment : response.enrollments()) {
    // Handle each enrollment...
}
```

Each item in the list of enrollments in the response will have the following
attributes:

-   `enrollment.description()` a string description of the enrollment.
-   `enrollment.modelname()` the name of the model used to create the
    enrollment as a string.
-   `enrollment.modeltype()` the type of the model used to create the
    enrollment as a `sensory::api::common::ModelType` enum.
-   `enrollment.modelversion()` the version of the model used to create the
    enrollment as a string.
-   `enrollment.userid()` the ID of the user that created the enrollment.
-   `enrollment.deviceid()` the ID of the device used to create the enrollment.
-   `enrollment.createdat()` a gRPC date representation of the creation date.
-   `enrollment.updatedat()` a gRPC date representation of the last update date.
-   `enrollment.id()` the UUID that uniquely identifies the enrollment.

### Deleting Enrollments

To delete an enrollment by ID:

```c++
// The UUID of the enrollment to delete.
std::string enrollmentID = "45ad3215-1d4c-42aa-aec4-2724e9ce1d99";

sensory::api::v1::management::EnrollmentResponse response;
auto status = mgmtService.deleteEnrollment(&response, enrollmentID);
if (!status.ok()) {  // The call failed, handle the error here
    auto errorCode = status.error_code();
    auto errorMessage = status.error_message();
    // ...
}
```

### Fetching Enrollment Groups

```c++
// The name of the user to fetch enrollment groups for.
std::string userID = "user";

// Create a response for the RPC.
sensory::api::v1::management::GetEnrollmentGroupsResponse response;
status = mgmtService.getEnrollmentGroups(&response, userID);
if (!status.ok()) {  // The call failed, handle the error here
    auto errorCode = status.error_code();
    auto errorMessage = status.error_message();
    // ...
}
for (auto& enrollment : response.enrollmentgroups()) {
    // Handle each enrollment group...
}
```

Each item in the list of enrollment groups in the response will have the
following attributes:

-   `enrollment.description()` a string description of the enrollment group.
-   `enrollment.modelname()` the name of the model used to create the
    enrollment group as a string.
-   `enrollment.modeltype()` the type of the model used to create the
    enrollment group as a `sensory::api::common::ModelType` enum.
-   `enrollment.modelversion()` the version of the model used to create the
    enrollment group as a string.
-   `enrollment.userid()` the ID of the user that created the enrollment group.
-   `enrollment.createdat()` a gRPC date representation of the creation date of
    the enrollment group.
-   `enrollment.updatedat()` a gRPC date representation of the last update date
    of the enrollment group.
-   `enrollment.id()` the UUID that uniquely identifies the enrollment group.

### Creating Enrollment Groups

To create a new enrollment group:

```c++
// The ID of the user that is creating the enrollment group.
std::string userID = "userID";
// The ID to use when creating the group, leave blank to automatically generate.
std::string groupID = "";
// The name of the enrollment group.
std::string name = "Group Name";
// A test description of the enrollment group.
std::string description = "A description of the enrollment group";
// The model to use for the enrollment group.
std::string model = "face_biometric_hektor";

sensory::api::v1::management::EnrollmentGroupResponse response;
auto status = mgmtService.createEnrollmentGroup(
    &response, userID, groupID, name, description, model);
if (!status.ok()) {  // The call failed, handle the error here
    auto errorCode = status.error_code();
    auto errorMessage = status.error_message();
    // ...
}
```

### Appending Enrollment Groups

To append a list of enrollments to an enrollment group:

```c++
// The UUID of the group to append enrollments to
std::string groupID = "f861f226-b608-47ad-95f4-02bf48ec351b";
// A vector of unique enrollments to append to the group. These enrollments
// should be based on the same model as the enrollment group.
std::vector<std::string> enrollments{
    "c437729b-3ce4-4af7-8f49-9da671cc89b2",
    "eb1cfc5e-ff19-48aa-a702-55e19989fccb",
    "076ec27d-a0ca-4f59-8a0e-42c5d4ccf03a"
};

sensory::api::v1::management::EnrollmentGroupResponse response;
auto status = mgmtService.appendEnrollmentGroup(&response, groupID, enrollments);
if (!status.ok()) {  // The call failed, handle the error here
    auto errorCode = status.error_code();
    auto errorMessage = status.error_message();
    // ...
}
```

### Deleting Enrollment Groups

To delete an enrollment group by ID:

```c++
// The UUID of the group to delete.
std::string groupID = "13481e19-5853-47d0-ba61-6819914405bb";

sensory::api::v1::management::EnrollmentGroupResponse response;
auto status = mgmtService.deleteEnrollmentGroup(&response, groupID);
if (!status.ok()) {  // The call failed, handle the error here
    auto errorCode = status.error_code();
    auto errorMessage = status.error_message();
    // ...
}
```
