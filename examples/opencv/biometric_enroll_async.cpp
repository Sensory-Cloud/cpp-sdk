// An example of face services based on OpenCV camera streams.
//
// Author: Christian Kauten (ckauten@sensoryinc.com)
//
// Copyright (c) 2021 Sensory, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXTERNRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <iostream>
#include <thread>
#include <mutex>
#include <google/protobuf/util/time_util.h>
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/health_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>
#include <sensorycloud/services/management_service.hpp>
#include <sensorycloud/services/audio_service.hpp>
#include <sensorycloud/services/video_service.hpp>
#include <sensorycloud/token_manager/insecure_credential_store.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

int main(int argc, const char** argv) {
    cv::CommandLineParser parser(argc, argv, "{help h||}{@device||}");
    // if (parser.has("help")) {
    //     help(argv);
    //     return 0;
    // }
    // double scale = parser.get<double>("scale");
    // if (scale < 1) scale = 1;
    // bool tryflip = parser.has("try-flip");
    std::string device = parser.get<std::string>("@device");
    if (!parser.check()) {
        parser.printErrors();
        return 0;
    }

    // Create an insecure credential store for keeping OAuth credentials in.
    sensory::token_manager::InsecureCredentialStore keychain(".", "com.sensory.cloud.examples");
    if (!keychain.contains("deviceID"))
        keychain.emplace("deviceID", sensory::token_manager::uuid_v4());
    const auto DEVICE_ID(keychain.at("deviceID"));

    // Initialize the configuration to the host for given address and port
    sensory::Config config(
        "io.stage.cloud.sensory.com",
        443,
        "cabb7700-206f-4cc7-8e79-cd7f288aa78d",
        DEVICE_ID
    );
    std::cout << "Connecting to remote host: " << config.getFullyQualifiedDomainName() << std::endl;

    // ------ Check server health ----------------------------------------------

    // Query the health of the remote service.
    sensory::service::HealthService healthService(config);
    sensory::api::common::ServerHealthResponse serverHealth;
    auto status = healthService.getHealth(&serverHealth);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    // Report the health of the remote service
    std::cout << "Server status:" << std::endl;
    std::cout << "\tisHealthy: " << serverHealth.ishealthy() << std::endl;
    std::cout << "\tserverVersion: " << serverHealth.serverversion() << std::endl;
    std::cout << "\tid: " << serverHealth.id() << std::endl;

    // ------ Authorize the current user ---------------------------------------

    // Query the user ID
    std::string userID = "";
    std::cout << "user ID: ";
    std::cin >> userID;

    // Create an OAuth service
    sensory::service::OAuthService oauthService(config);
    sensory::token_manager::TokenManager<sensory::token_manager::InsecureCredentialStore> tokenManager(oauthService, keychain);

    if (!tokenManager.hasSavedCredentials()) {  // the device is not registered
        // Generate a new clientID and clientSecret for this device
        const auto credentials = tokenManager.generateCredentials();

        // Query the friendly device name
        std::string name = "";
        std::cout << "Device Name: ";
        std::cin >> name;

        // Query the shared pass-phrase
        std::string password = "";
        std::cout << "password: ";
        std::cin >> password;

        // Register this device with the remote host
        sensory::api::v1::management::DeviceResponse registerResponse;
        auto status = oauthService.registerDevice(&registerResponse,
            name,
            password,
            credentials.id,
            credentials.secret
        );
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to register device with\n\t" <<
                status.error_code() << ": " << status.error_message() << std::endl;
            return 1;
        }
    }

    // ------ Create the video service -----------------------------------------

    // Create the video service based on the configuration and token manager.
    sensory::service::VideoService<sensory::token_manager::InsecureCredentialStore>
        videoService(config, tokenManager);

    // ------ Query the available video models ---------------------------------

    // Start an asynchronous RPC to fetch the names of the available models. The
    // RPC will use the grpc::CompletionQueue as an event loop.
    grpc::CompletionQueue queue;
    auto getModelsRPC = videoService.getModels(&queue);

    // Execute the asynchronous RPC in this thread (which will block like a
    // synchronous call).
    void* tag(nullptr);
    bool ok(false);
    queue.Next(&tag, &ok);
    if (ok && tag == getModelsRPC) {
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get video models with\n\t" <<
                status.error_code() << ": " << status.error_message() << std::endl;
            return 1;
        }
        for (auto& model : getModelsRPC->getResponse().models()) {
            if (model.modeltype() != sensory::api::common::FACE_BIOMETRIC)
                continue;
            std::cout << "\t" << model.name() << std::endl;
        }
        delete getModelsRPC;
    }

    std::string videoModel = "";
    std::cout << "Video model: ";
    std::cin >> videoModel;

    // Determine whether to conduct a liveness check.
    std::string liveness;
    bool isLivenessEnabled(false);
    while (true) {
        std::cout << "Liveness Check [yes|y, no|n]: ";
        std::cin >> liveness;
        if (liveness == "yes" || liveness == "y") {
            isLivenessEnabled = true;
            break;
        } else if (liveness == "no" || liveness == "n") {
            isLivenessEnabled = false;
            break;
        } else {
            continue;
        }
    }

    // Get the description of the model.
    std::string description;
    std::cout << "Description: ";
    std::cin.ignore();
    std::getline(std::cin, description);

    // ------ Create a new video enrollment ------------------------------------

    // Create an image capture object
    cv::VideoCapture capture;
    // Look for a device ID from the command line
    if (device.empty() || (isdigit(device[0]) && device.size() == 1)) {
        // Default to device 0 if the device was not provided. Use ASCII
        // subtraction to convert the digit to an integer.
        int camera = device.empty() ? 0 : device[0] - '0';
        if (!capture.open(camera)) {
            std::cout << "Capture from camera #" << camera << " didn't work" << std::endl;
            return 1;
        }
    } else {  // Device ID was not a valid integer value.
        std::cout << "Device ID \"" << device << "\" is not a valid integer!" << std::endl;
    }

    // A flag determining whether the last sent frame was enrolled. This flag
    // is atomic to support thread safe reads and writes.
    std::atomic<bool> isEnrolled(false);
    // The completion percentage of the enrollment request.
    std::atomic<float> percentComplete(0);
    // A flag determining whether the last sent frame was detected as live.
    std::atomic<bool> isLive(false);
    // An OpenCV matrix containing the frame data from the camera.
    cv::Mat frame;
    // A mutual exclusion for locking access to the frame between foreground
    // (frame capture) and background (network stream processing) threads.
    std::mutex frameMutex;

    // Create the enrollment stream.
    auto stream = videoService.createEnrollment(&queue,
        sensory::service::newCreateEnrollmentConfig(
            videoModel,
            userID,
            description,
            isLivenessEnabled,
            sensory::api::v1::video::RecognitionThreshold::LOW
        )
    );

    /// Tagged events in the CompletionQueue handler.
    enum class Events {
        /// The `Write` event for sending data up to the server.
        Write = 1,
        /// The `Read` event for receiving messages from the server.
        Read = 2,
        /// The `WritesDone` event indicating that no more data will be sent up.
        WritesDone = 3,
        /// The `Finish` event indicating that the stream has terminated.
        Finish = 4
    };

    // start the stream event thread in the background to handle events.
    std::thread eventThread([&stream, &queue, &isEnrolled, &percentComplete, &isLive, &frame, &frameMutex](){
        void* tag(nullptr);
        bool ok(false);
        while (queue.Next(&tag, &ok)) {
            if (!ok) break;
            if (tag == stream) {
                // Respond to the start of stream succeeding. All Sensory Cloud
                // AV streams require a configuration message to be sent to the
                // server that provides information about the stream. This
                // information is generated by the SDK when the stream is
                // created, but cannot be sent until the stream is initialized.
                // By calling `Write` with the request attached to the call, we
                // send this first configuration message to the server. The
                // request object in the call can then be re-used for image data
                // in other tag branches. Tag writes and reads uniquely such
                // that they can be handled by different branches of this event
                // loop.
                stream->getCall()->Write(stream->getRequest(), (void*) Events::Write);
                stream->getCall()->Read(&stream->getResponse(), (void*) Events::Read);
            } else if (tag == (void*) Events::Write) {  // Respond to a write event.
                // If we successfully enrolled, there is no more data to send
                // to the server, notify gRPC that there will be no more writes
                // to half-close the stream. We'll give this a unique tag so
                // we can respond to the completion of the `WritesDone` in
                // another branch to fully terminate the stream.
                if (isEnrolled) {
                    stream->getCall()->WritesDone((void*) Events::WritesDone);
                    continue;
                }
                // To send image data to the server, it must be JPEG compressed.
                std::vector<unsigned char> buffer;
                {  // Lock the mutex and encode the frame with JPEG into a buffer.
                    std::lock_guard<std::mutex> lock(frameMutex);
                    cv::imencode(".jpg", frame, buffer);
                }
                // Create the request from the encoded image data.
                sensory::api::v1::video::CreateEnrollmentRequest request;
                request.set_imagecontent(buffer.data(), buffer.size());
                stream->getCall()->Write(request, (void*) Events::Write);
            } else if (tag == (void*) Events::Read) {  // Respond to a read event.
                // Set the local instance data based on the last frame. The is
                // enrolled flag will only go high once an enrollment has been
                // successfully created. This will coincide with the completion
                // percentage reaching 100%. The isLive flag will only go high
                // if liveness is enabled and the last frame was detected as a
                // live human.
                isEnrolled = !stream->getResponse().enrollmentid().empty();
                percentComplete = stream->getResponse().percentcomplete() / 100.f;
                isLive = stream->getResponse().isalive();
                // If we're finished enrolling, don't issue a new read request.
                if (!isEnrolled)
                    stream->getCall()->Read(&stream->getResponse(), (void*) Events::Read);
            } else if (tag == (void*) Events::WritesDone) {  // Respond to `WritesDone`
                // Finish the stream and terminate.
                stream->getCall()->Finish(&stream->getStatus(), (void*) Events::Finish);
            } else if (tag == (void*) Events::Finish) {  // Respond to `Finish`
                // Check the final status of the stream and delete the call
                // handle now that the stream has terminated.
                if (!stream->getStatus().ok()) {
                    std::cout << "Authentication stream failed with\n\t"
                        << stream->getStatus().error_code() << ": "
                        << stream->getStatus().error_message() << std::endl;
                }
                delete stream;
                // By this point, we can guarantee that no write/read events
                // will be coming to the completion queue. Break out of the
                // event loop to terminate the background processing thread.
                break;
            }
        }
    });

    // Start capturing frames from the device.
    while (!isEnrolled) {
        {  // Lock the mutex and read a frame.
            std::lock_guard<std::mutex> lock(frameMutex);
            capture >> frame;
        }
        // If the frame is empty, something went wrong, exit the capture loop.
        if (frame.empty()) break;
        // Draw the progress bar on the frame
        auto presentationFrame = frame.clone();
        cv::rectangle(
            presentationFrame,
            cv::Point(0, 0),
            cv::Point(presentationFrame.size().width, 10),
            cv::Scalar(0, 0, 0), -1);
        cv::rectangle(
            presentationFrame,
            cv::Point(0, 0),
            cv::Point(percentComplete * presentationFrame.size().width, 10),
            cv::Scalar(0, 255, 0), -1);
        // Draw some text indicating the liveness status
        if (isLivenessEnabled) {  // liveness is enabled
            cv::putText(presentationFrame,
                isLive ? "Live" : "Not Live",
                cv::Point(10, 40),
                cv::FONT_HERSHEY_SIMPLEX,
                1,  // font scale
                isLive ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255),
                2   // thickness
            );
        }
        // Show the frame in a viewfinder window.
        cv::imshow("Sensory Cloud Face Enrollment Demo", presentationFrame);
        // Listen for keyboard interrupts to terminate the capture.
        char c = (char) cv::waitKey(10);
        if (c == 27 || c == 'q' || c == 'Q') break;
    }

    // Wait for the network thread to join back in.
    eventThread.join();

    if (!stream->getStatus().ok()) {
        std::cout << "Failed to create enrollment with\n\t" <<
            stream->getStatus().error_code() << ": " <<
            stream->getStatus().error_message() << std::endl;
    } else {
        std::cout << "Successfully created enrollment!" << std::endl;
    }

    return 0;
}
