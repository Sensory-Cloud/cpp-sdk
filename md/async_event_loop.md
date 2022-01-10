# Asynchronous Interface (Event-Loop)

This tutorial provides an overview of the asynchronous interface of the Sensory
Cloud SDK based on an event-loop pattern using
[`grpc::CompletionQueue`][CompletionQueue]. It is recommended to use callback
/reactor patterns described by the [async_callback.md](async_callback.md)
tutorial when possible; however, there may be times where explicit maintenance
of the event-loop is necessary. This tutorial will show you how to:

-   setup the [`grpc::CompletionQueue`][CompletionQueue] to act as the
    event-loop for registering asynchronous events to run in the background,
-   perform unary service requests asynchronously (i.e., health, OAuth, and
    management services) and respond to them using the completion queue, and
-   set up streaming applications, i.e., audio and video services,
    asynchronously and respond to them using the queue.

This tutorial assumes you are familiar with the
[synchronous blocking tutorial](tutorial.md).

[CompletionQueue]: https://grpc.github.io/grpc/cpp/classgrpc_1_1_completion_queue.html

## Checking The Server Health

It's important to check the health of your Sensory Inference server. You can do
so via the following:

```c++
// Create a health service for performing server health queries.
sensory::service::HealthService healthService(config);
// Create a completion queue for handling callbacks
::grpc::CompletionQueue queue;
// Create the get health query asynchronously using the queue.
auto call = healthService.getHealth(&queue);
void* tag(nullptr);
bool ok(false);
// Process the next asynchronous event in the queue.
queue.Next(&tag, &ok);
if (ok && tag == call) {
    if (!call->getStatus().ok()) {  // The call failed, handle the error here.
        auto errorCode = call->getStatus().error_code();
        auto errorMessage = call->getStatus().error_message();
    } else {  // The call succeeded, handle the response here.
        auto isHealthy = call->getResponse().ishealthy();
        auto serverVersion = call->getResponse().serverversion();
        auto serverID = call->getResponse().id();
    }
}
// Delete the stream when done using it.
delete call;
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

    // Create a completion queue for handling callbacks
    ::grpc::CompletionQueue queue;
    // Create the get device registration query asynchronously using the queue.
    auto call = oauthService.registerDevice(&queue,
        deviceName, insecureSharedSecret, credentials.id, credentials.secret);
    void* tag(nullptr);
    bool ok(false);
    // Process the next asynchronous event in the queue.
    queue.Next(&tag, &ok);
    if (ok && tag == call) {
        if (!call->getStatus().ok()) {  // The call failed, handle the error here
            auto errorCode = call->getStatus().error_code();
            auto errorMessage = call->getStatus().error_message();
        }
    }
    // Delete the stream when done using it.
    delete call;
}
```

## Asynchronous Audio Services

### Obtaining Audio Models

Certain audio models are available to your application depending on the models
that are configured for your instance of Sensory Cloud. In order to determine
which audio models are accessible to you, you can execute the below code.

```c++
TODO
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

## Asynchronous Video Services

### Obtaining Video Models

Certain video models are available to your application depending on the models
that are configured for your instance of Sensory Cloud. In order to determine
which video models are accessible to you, you can execute the below code.

```c++
TODO
```

### Enrolling With Video

The synchronous API provides a blocking interface for streaming data to the
server.

```c++
// The name of the biometric model to enroll the user with.
std::string modelName("face_biometric_hektor");
// The unique ID of the user that is being enrolled.
std::string userID("60db6966-068f-4f6c-9a51-d2a3308db09b");
// A human readable description of the enrollment.
std::string enrollmentDescription("My Enrollment");
// Whether to perform a liveness check while executing the enrollment.
bool isLivenessEnabled(false);

TODO
```

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

TODO
```

### Video Liveness

The synchronous API provides a blocking interface for streaming data to the
server.

```c++
// The particular model to use for liveness detection.
std::string videoModel("face_recognition_mathilde");
// The unique user ID of the user being validated for liveness
std::string userID("60db6966-068f-4f6c-9a51-d2a3308db09b");
// The security threshold for the optional liveness check.
auto threshold = sensory::api::v1::video::RecognitionThreshold::LOW;

TODO
```

## Asynchronous Management Service

### Fetching Enrollments

```c++
// The name of the user to fetch enrollments for.
std::string userID = "user";

TODO
```

### Deleting Enrollments

```c++
// The UUID of the enrollment to delete.
std::string enrollmentID = "45ad3215-1d4c-42aa-aec4-2724e9ce1d99";

TODO
```

### Fetching Enrollment Groups

```c++
// The name of the user to fetch enrollment groups for.
std::string userID = "user";

TODO
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

TODO
```