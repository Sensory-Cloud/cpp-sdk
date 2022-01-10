# Asynchronous Interface (Callback/Reactor)

This tutorial provides an overview of the asynchronous interface of the Sensory
Cloud C++ SDK based on callback/reactor patterns exposed by gRPC. If you need
an asynchronous interface based on an event-loop pattern, please refer to
[async_event_loop.md](async_event_loop.md). This tutorial will show you how to:

-   perform unary service requests asynchronously (i.e., health, OAuth, and
    management services) and respond to them using callbacks, and
-   set up streaming applications, i.e., audio and video services, based on
    polymorphic reactor patterns.

This tutorial assumes you are familiar with the [basics tutorial](tutorial.md).

## Checking The Server Health

It's important to check the health of your Sensory Inference server. You can do
so using the following block. Because the call is asynchronous, it will not
block the calling thread; the callback will be executed in the background upon
receipt of the response from the server.

```c++
// Create a health service for performing server health queries.
sensory::service::HealthService healthService(config);
// Start the health query asynchronously using lambda function.
healthService.getHealth([](sensory::service::HealthService::GetHealthCallData* call) {
    if (!call->getStatus().ok()) {  // The call failed, handle the error here.
        auto errorCode = call->getStatus().error_code();
        auto errorMessage = call->getStatus().error_message();
    } else {  // The call succeeded, handle the response here.
        auto isHealthy = call->getResponse().ishealthy();
        auto serverVersion = call->getResponse().serverversion();
        auto serverID = call->getResponse().id();
    }
});
```

## Registering OAuth Credentials

OAuth credentials should be registered once per unique machine. Registration is
very simple and is provided as part of the SDK. The below example shows how to
create an `OAuthService` and register a device for the first time.

```c++
if (!tokenManager.hasToken()) {  // The device is not registered.
    // Generate a new `clientID` and `clientSecret` for this device and store
    // them securely in the secure credential store.
    auto credentials = tokenManager.generateCredentials();
    // The name of the device as a human-readable text string
    std::string deviceName("a device in your system");
    // Use a shared secret, i.e., pass-phrase to authenticate the device.
    std::string insecureSharedSecret("password");
    // Register this device with the remote host
    oauthService.registerDevice(
        deviceName, insecureSharedSecret, credentials.id, credentials.secret,
        [](sensory::service::OAuthService::RegisterDeviceCallData* call) {
            if (!call->getStatus().ok()) {  // The call failed, handle the error here.
                auto errorCode = call->getStatus().error_code();
                auto errorMessage = call->getStatus().error_message();
            }
        }
    );
}
```

## Asynchronous Audio Services

The audio service contains both unary calls that utilize a callback pattern and
streaming calls that rely on reactor patterns. It's worth noting that the unary
calls can also be configured using a reactor pattern, but the callback pattern
is simpler and can often be inlined into smaller binaries depending on compiler
optimizations.

### Obtaining Audio Models

Certain audio models are available to your application depending on the models
that are configured for your instance of Sensory Cloud. In order to determine
which audio models are accessible to you, you can execute the below code.

```c++
audioService.getModels([](sensory::service::AudioService<SecureCredentialStore>::GetModelsCallData* call) {
    if (!call->getStatus().ok()) {  // The call failed, handle the error here.
        auto errorCode = call->getStatus().error_code();
        auto errorMessage = call->getStatus().error_message();
    } else {  // The call succeed, handle the response here.
        // Iterate over the models returned in the response.
        for (auto& model : call->getResponse().models()) {
            // The model type uniquely identifies the type of the model. For
            // more information about available models, refer to the enum
            // `sensory::api::common::ModelType`.
            auto modelType = model.modeltype();
            // You will need to know the model name to initiate audio services
            // with the model.
            auto modelName = model.name();
        }
    }
});
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

Unary calls, such as asynchronous model fetching, can be implemented using
either a callback or reactor pattern. To stream video asynchronously to the
server, you will need to follow a reactor pattern and define an object-oriented
handler to respond to read and write events.

### Obtaining Video Models

Certain video models are available to your application depending on the models
that are configured for your instance of Sensory Cloud. In order to determine
which video models are accessible to you, you can execute the below cod based
on a callback pattern.

```c++
videoService.getModels([](sensory::service::VideoService<SecureCredentialStore>::GetModelsCallData* call) {
    if (!call->getStatus().ok()) {  // The call failed, handle the error here.
        auto errorCode = call->getStatus().error_code();
        auto errorMessage = call->getStatus().error_message();
    } else {  // The call succeed, handle the response here.
        // Iterate over the models returned in the response.
        for (auto& model : call->getResponse().models()) {
            // The model type uniquely identifies the type of the model. For
            // more information about available models, refer to the enum
            // `sensory::api::common::ModelType`.
            auto modelType = model.modeltype();
            // You will need to know the model name to initiate audio services
            // with the model.
            auto modelName = model.name();
        }
    }
});
```

### Enrolling With Video

The code block below provides a stub of a reactor for enrollment creation
streams. Please see
[examples/opencv/biometric_enroll_callback.cpp](examples/opencv/biometric_enroll_callback.cpp)
for an demonstration of enrollment creation based on OpenCV video streams.

```c++
/// @brief A bi-directional stream reactor for biometric enrollments from video
/// stream data.
///
class CreateEnrollmentReactor :
    public VideoService<SecureCredentialStore>::CreateEnrollmentBidiReactor {
 private:
    /// A flag determining whether the last sent frame was enrolled. This flag
    /// is atomic to support thread safe reads and writes.
    std::atomic<bool> isEnrolled;
    /// An tensor containing the frame data from the camera.
    Image frame;
    /// A mutual exclusion for locking access to the frame between foreground
    /// (frame capture) and background (network stream processing) threads.
    std::mutex frameMutex;

 public:
    /// @brief Initialize a reactor for streaming video from an OpenCV stream.
    CreateEnrollmentReactor() :
        VideoService<SecureCredentialStore>::CreateEnrollmentBidiReactor(),
        isEnrolled(false) { }

    /// @brief React to a _write done_ event.
    ///
    /// @param ok whether the write succeeded.
    ///
    void OnWriteDone(bool ok) override {
        if (isEnrolled) {  // Successfully enrolled! Close the stream.
            StartWritesDone();
            return;
        }
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        std::vector<unsigned char> buffer;
        {  // Lock the mutex and encode the frame with JPEG into a buffer.
            std::lock_guard<std::mutex> lock(frameMutex);
            // TODO: fill a buffer with JPEG encoded image data. A quality
            // factor of 95 is recommended for compression.
            jpeg_encode(buffer, frame, 95);
        }
        // Create the request from the encoded image data.
        request.set_imagecontent(buffer.data(), buffer.size());
        /// Start the next write request with the current frame.
        StartWrite(&request);
    }

    /// @brief React to a _read done_ event.
    ///
    /// @param ok whether the read succeeded.
    ///
    void OnReadDone(bool ok) override {
        // If the enrollment is complete, there is no more data to read.
        if (isEnrolled) return;
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;

        // The completion percentage is an integer in [0, 100] indicating the
        // progress of the enrollment.
        const auto percentComplete = response.percentcomplete();
        // If liveness is enabled, the `is alive` flag determines whether the
        // last frame was scored as a live image.
        const auto isAlive = response.isalive();
        // If/when the enrollment succeeds, the enrollment ID will be returned.
        const auto enrollmentID = response.enrollmentid();
        // If/when the enrollment succeeds, the model name and version
        // are returned. The can be useful when saving enrollment IDs to
        // a persistent store.
        const auto modelName = response.modelname();
        const auto modelVersion = response.modelversion();

        // If the enrollment ID is not empty, then the enrollment succeeded.
        isEnrolled = !response.enrollmentid().empty();
        if (!isEnrolled)  // Still enrolling, start the next read request.
            StartRead(&response);
    }

    /// @brief Stream video to create an enrollment.
    ///
    /// @param capture The image capture stream.
    ///
    ::grpc::Status streamVideo(VideoCapture& capture) {
        // Start the call to initiate the stream in the background.
        StartCall();
        // Start capturing frames from the device.
        while (!isEnrolled) {
            {  // Lock the mutex and read a frame.
                std::lock_guard<std::mutex> lock(frameMutex);
                capture >> frame;
            }
        }
        // Wait for the stream to conclude. This is necessary to check the
        // final status of the call and allow any dynamically allocated data
        // to be cleaned up. If the stream is destroyed before the final
        // `onDone` callback, odd runtime errors can occur.
        return await();
    }
};

// The name of the biometric model to enroll the user with.
std::string modelName("face_biometric_hektor");
// The unique ID of the user that is being enrolled.
std::string userID("60db6966-068f-4f6c-9a51-d2a3308db09b");
// A human readable description of the enrollment.
std::string enrollmentDescription("My Enrollment");
// Whether to perform a liveness check while executing the enrollment.
bool isLivenessEnabled(false);

// Create the stream with the reactor defined above.
CreateEnrollmentReactor reactor;
videoService.createEnrollment(&reactor,
    modelName,
    userID,
    enrollmentDescription,
    isLivenessEnabled
);
// Stream images to create and enrollment with a video capture device. This
// will block until the enrollment completes or an error occurs.
auto status = reactor.streamVideo(capture);

if (!status.ok()) {
    std::cout << "Failed to enroll with\n\t" <<
        status.error_code() << ": " <<
        status.error_message() << std::endl;
} else {
    std::cout << "Successful enrollment! Your enrollment ID is:" << std::endl;
    std::cout << reactor.response.enrollmentid() << std::endl;
}
```

### Authenticating With Video

The code block below provides a stub of a reactor for authentication streams.
Please see
[examples/opencv/biometric_authenticate_callback.cpp](examples/opencv/biometric_authenticate_callback.cpp)
for an demonstration of authentication based on OpenCV video streams.

```c++
/// @brief A bi-directional stream reactor for biometric enrollments from
/// video stream data.
///
class VideoAuthenticationReactor :
    public VideoService<SecureCredentialStore>::AuthorizeBidiReactor {
 private:
    /// A flag determining whether the last sent frame was enrolled. This flag
    /// is atomic to support thread safe reads and writes.
    std::atomic<bool> isAuthenticated;
    /// A tensor containing the frame data from the camera.
    Image frame;
    /// A mutual exclusion for locking access to the frame between foreground
    /// (frame capture) and background (network stream processing) threads.
    std::mutex frameMutex;

 public:
    /// @brief Initialize a reactor for streaming video from an OpenCV stream.
    VideoAuthenticationReactor() :
        VideoService<SecureCredentialStore>::AuthorizeBidiReactor(),
        isAuthenticated(false) { }

    /// @brief React to a _write done_ event.
    ///
    /// @param ok whether the write succeeded.
    ///
    void OnWriteDone(bool ok) override {
        if (isAuthenticated) {  // Successfully authenticated! Close the stream.
            StartWritesDone();
            return;
        }
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        std::vector<unsigned char> buffer;
        {  // Lock the mutex and encode the frame with JPEG into a buffer.
            std::lock_guard<std::mutex> lock(frameMutex);
            // TODO: fill a buffer with JPEG encoded image data. A quality
            // factor of 95 is recommended for compression.
            jpeg_encode(buffer, frame, 95);
        }
        // Create the request from the encoded image data.
        request.set_imagecontent(buffer.data(), buffer.size());
        /// Start the next write request with the current frame.
        StartWrite(&request);
    }

    /// @brief React to a _read done_ event.
    ///
    /// @param ok whether the read succeeded.
    ///
    void OnReadDone(bool ok) override {
        // If the enrollment is complete, there is no more data to read.
        if (isAuthenticated) return;
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;

        // The score is the probability of spoof P[Spoof | Image] if the
        // liveness check is enabled.
        auto score = response.score();
        // A boolean representing if P[Spoof | Image] was lower than the
        // specified security threshold.
        auto isAlive = response.isalive();

        // Set the authentication flag to the success of the response.
        isAuthenticated = response.success();
        if (!isAuthenticated)  // Start the next read request
            StartRead(&response);
    }

    /// @brief Stream video from a capture device.
    ///
    /// @param capture The capture device.
    ///
    ::grpc::Status streamVideo(VideoCapture& capture) {
        // Start the call to initiate the stream in the background.
        StartCall();
        // Start capturing frames from the device.
        while (!isAuthenticated) {
            {  // Lock the mutex and read a frame.
                std::lock_guard<std::mutex> lock(frameMutex);
                capture >> frame;
            }
        }
        // Wait for the stream to conclude. This is necessary to check the
        // final status of the call and allow any dynamically allocated data
        // to be cleaned up. If the stream is destroyed before the final
        // `onDone` callback, odd runtime errors can occur.
        return await();
    }
};

// The ID for the enrollment for a particular user
std::string enrollmentID("60db6966-068f-4f6c-9a51-d2a3308db09b");
// a flag determining whether a liveness check should be conducted before
// authenticating against the enrollment.
bool isLivenessEnabled(false);
// The security threshold for the optional liveness check.
auto threshold = sensory::api::v1::video::RecognitionThreshold::LOW;

// Create the stream.
VideoAuthenticationReactor reactor;
videoService.authenticate(&reactor, enrollmentID, isLivenessEnabled, threshold);
// Wait for the stream to conclude. This is necessary to check the final
// status of the call and allow any dynamically allocated data to be cleaned
// up. If the stream is destroyed before the final `onDone` callback, odd
// runtime errors can occur.
status = reactor.streamVideo(capture);

if (!status.ok()) {
    std::cout << "Failed to authenticate with\n\t" <<
        status.error_code() << ": " <<
        status.error_message() << std::endl;
} else {
    std::cout << "Successfully authenticated!" << std::endl;
}
```

### Video Liveness

The code block below provides a stub of a reactor for liveness validation
streams. Please see
[examples/opencv/liveness_callback.cpp](examples/opencv/liveness_callback.cpp)
for an demonstration of liveness validation based on OpenCV video streams.

```c++
/// @brief A bi-directional stream reactor for biometric liveness validation
/// from video stream data.
///
class VideoLivenessReactor :
    public VideoService<SecureCredentialStore>::ValidateLivenessBidiReactor {
 private:
    /// A flag determining whether the last sent frame was detected as live.
    std::atomic<bool> isLive;
    /// A code for adjusting the face when the face box is misaligned.
    std::atomic<FaceAlignment> alignmentCode;
    /// A tensor containing the frame data from the camera.
    Image frame;
    /// A mutual exclusion for locking access to the frame between foreground
    /// (frame capture) and background (network stream processing) threads.
    std::mutex frameMutex;

 public:
    /// @brief Initialize a reactor for streaming video from an OpenCV stream.
    VideoLivenessReactor() :
        VideoService<SecureCredentialStore>::ValidateLivenessBidiReactor(),
        isLive(false),
        alignmentCode(FaceAlignment::Valid) { }

    /// @brief React to a _write done_ event.
    ///
    /// @param ok whether the write succeeded.
    ///
    void OnWriteDone(bool ok) override {
        // if (?) { TODO
        //     StartWritesDone();
        //     return;
        // }
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        std::vector<unsigned char> buffer;
        {  // Lock the mutex and encode the frame with JPEG into a buffer.
            std::lock_guard<std::mutex> lock(frameMutex);
            // TODO: fill a buffer with JPEG encoded image data. A quality
            // factor of 95 is recommended for compression.
            jpeg_encode(buffer, frame, 95);
        }
        // Create the request from the encoded image data.
        request.set_imagecontent(buffer.data(), buffer.size());
        /// Start the next write request with the current frame.
        StartWrite(&request);
    }

    /// @brief React to a _read done_ event.
    ///
    /// @param ok whether the read succeeded.
    ///
    void OnReadDone(bool ok) override {
        // if (?) return; // TODO
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        // Log information about the response to the terminal.
        // std::cout << "Frame Response:" << std::endl;
        // std::cout << "\tScore: "    << response.score() << std::endl;
        // std::cout << "\tIs Alive: " << response.isalive() << std::endl;
        // Set the liveness status of the last frame.
        isLive = response.isalive();
        alignmentCode = response.score() < 100 ?
            FaceAlignment::Valid : static_cast<FaceAlignment>(response.score());
        StartRead(&response);
    }

    /// @brief Stream video from a capture device.
    ///
    /// @param capture The capture device.
    ///
    ::grpc::Status streamVideo(cv::VideoCapture& capture) {
        // Start the call to initiate the stream in the background.
        StartCall();
        // Start capturing frames from the device.
        while (true) {
            {  // Lock the mutex and read a frame.
                std::lock_guard<std::mutex> lock(frameMutex);
                capture >> frame;
            }
            // Decode the error message to display on the view finder.
            std::string message = "";
            switch (static_cast<FaceAlignment>(alignmentCode)) {
            case FaceAlignment::Valid:        // No pre-processor issue.
                message = "Spoof!";
                break;
            case FaceAlignment::Unknown:      // Unknown pre-processor issue.
                message = "Unknown Face Error";
                break;
            case FaceAlignment::NoFace:       // No face detected in the frame.
                message = "No Face Detected";
                break;
            case FaceAlignment::SmallFace:    // Face in the frame is too small.
                message = "Face Too Small";
                break;
            case FaceAlignment::BadFQ:        // Image quality is too low.
                message = "Face Too Low Quality";
                break;
            case FaceAlignment::NotCentered:  // Face not centered in the frame.
                message = "Face Not Centered";
                break;
            case FaceAlignment::NotVertical:  // Face not upright in the frame.
                message = "Face Not Vertical";
                break;
            }
            // If the frame is live, no error occurred, so overwrite the message
            // with an indicator that the frame is live.
            if (isLive) message = "Live!";
        }
        return await();
    }
};

// The particular model to use for liveness detection.
std::string videoModel("face_recognition_mathilde");
// The unique user ID of the user being validated for liveness
std::string userID("60db6966-068f-4f6c-9a51-d2a3308db09b");
// The security threshold for the liveness check.
auto threshold = sensory::api::v1::video::RecognitionThreshold::LOW;

// Create the stream.
VideoLivenessReactor reactor;
videoService.validateLiveness(&reactor, videoModel, userID, threshold);
// Wait for the stream to conclude. This is necessary to check the final
// status of the call and allow any dynamically allocated data to be cleaned
// up. If the stream is destroyed before the final `onDone` callback, odd
// runtime errors can occur.
auto status = reactor.streamVideo(capture);

if (!status.ok()) {
    std::cout << "Failed to validate liveness with\n\t" <<
        status.error_code() << ": " <<
        status.error_message() << std::endl;
}
```

## Asynchronous Management Service

This section describes how to asynchronously query, update, and delete
enrollments and enrollment groups using the management service. Please see
[examples](management_callback.cpp) for a demonstration of the asynchronous
callback interface for the management service.

### Fetching Enrollments

```c++
// The name of the user to fetch enrollments for.
std::string userID = "user";

mgmtService.getEnrollments(userID,
    [](sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>::GetEnrollmentsCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed, handle the error here.
            auto errorCode = call->getStatus().error_code();
            auto errorMessage = call->getStatus().error_message();
        } else if (call->getResponse().enrollments().size() == 0) {
            // The user has no active enrollments.
        } else {
            for (auto& enrollment : call->getResponse().enrollments()) {
                // The description is a human-readable text that describes this
                // enrollment.
                auto description = enrollment.description();
                // The name the of the model used for the enrollment.
                auto modelName = enrollment.modelname();
                // The model type uniquely identifies the type of the model.
                // For more information about available models, refer to the
                // enum `sensory::api::common::ModelType`.
                auto modelType = enrollment.modeltype();
                // The version of the model the enrollment was generated using.
                auto modelVersion = enrollment.modelversion();
                // The ID of the user that created the enrollment.
                auto userID = enrollment.userid();
                // The ID of the device that the user created the enrollment on.
                auto deviceID = enrollment.deviceid();
                // The date that the enrollment was created. See
                // `google::protobuf::util::TimeUtil` for structures and
                // functions to interact with the protobuf date.
                auto creationDate = enrollment.createdat();
                // The date that the enrollment was last updated. See
                // `google::protobuf::util::TimeUtil` for structures and
                // functions to interact with the protobuf date.
                auto updateDate = enrollment.updatedat();
                // The unique ID of this enrollment.
                auto id = enrollment.id();
            }
        }
    }
);
```

### Deleting Enrollments

```c++
// The UUID of the enrollment to delete.
std::string enrollmentID = "45ad3215-1d4c-42aa-aec4-2724e9ce1d99";

mgmtService.deleteEnrollment(enrollmentID,
    [](sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>::DeleteEnrollmentCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed, handle the error here.
            auto errorCode = call->getStatus().error_code();
            auto errorMessage = call->getStatus().error_message();
        }
    }
);
```

### Fetching Enrollment Groups

```c++
// The name of the user to fetch enrollment groups for.
std::string userID = "user";

mgmtService.getEnrollmentGroups(userID,
    [](sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>::GetEnrollmentGroupsCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed, handle the error here.
            auto errorCode = call->getStatus().error_code();
            auto errorMessage = call->getStatus().error_message();
        } else if (call->getResponse().enrollmentgroups().size() == 0) {
            // The user belongs to no enrollment groups.
        } else {
            for (auto& group : call->getResponse().enrollmentgroups()) {
                // The description is a human-readable text that describes this
                // enrollment group.
                auto description = group.description();
                // The name the of the model used for the enrollment group.
                auto modelName = group.modelname();
                // The model type uniquely identifies the type of the model.
                // For more information about available models, refer to the
                // enum `sensory::api::common::ModelType`.
                auto modelType = group.modeltype();
                // The version of the model the enrollment group utilizes.
                auto modelVersion = group.modelversion();
                // The ID of the user that owns the enrollment group.
                auto userID = group.userid();
                // The date that the enrollment group was created. See
                // `google::protobuf::util::TimeUtil` for structures and
                // functions to interact with the protobuf date.
                auto creationDate = group.createdat();
                // The date that the enrollment group was last updated. See
                // `google::protobuf::util::TimeUtil` for structures and
                // functions to interact with the protobuf date.
                auto updateDate = group.updatedat();
                // The unique ID of this enrollment group.
                auto id = group.id();
            }
        }
    }
);
```

### Creating Enrollment Groups

```c++
// The ID of the user that owns this enrollment group.
std::string userID = "a61de369-aeb3-42f2-8605-932373d40294";
// The name of the group.
std::string groupName = "group-222";
// A human-readable description of the enrollment group.
std::string description = "A demo face biometric group";
// The name of the model used for the enrollment group.
std::string modelName = "face_biometric_hektor";

mgmtService.createEnrollmentGroup(userID, "", groupName, description, modelName,
    [](sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>::CreateEnrollmentGroupCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed, handle the error here.
            auto errorCode = call->getStatus().error_code();
            auto errorMessage = call->getStatus().error_message();
        }
    }
);
```

### Appending Enrollment Groups

```c++
// The UUID of the group to append an enrollment to.
std::string groupID = "13481e19-5853-47d0-ba61-6819914405bb";
// A vector of enrollment IDs to append to the group.
std::vector<std::string> enrollments{
    "edb1f8be-ba7c-438e-a08b-97b2e4ed5fce",
    "e5d8b2c6-6738-4693-bae5-4a8633b71df4",
    "59908f63-eb5e-4626-bb58-041ebb6da593"
};

mgmtService.appendEnrollmentGroup(groupID, enrollments,
    [](sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>::AppendEnrollmentGroupCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed, handle the error here.
            auto errorCode = call->getStatus().error_code();
            auto errorMessage = call->getStatus().error_message();
        }
    }
);
```

### Deleting Enrollment Groups

```c++
// The UUID of the group to delete.
std::string groupID = "13481e19-5853-47d0-ba61-6819914405bb";

mgmtService.deleteEnrollmentGroup(groupID,
    [](sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>::DeleteEnrollmentGroupCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed, handle the error here.
            auto errorCode = call->getStatus().error_code();
            auto errorMessage = call->getStatus().error_message();
        }
    }
);
```
