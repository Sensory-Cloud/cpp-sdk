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
#include <sensorycloud/services/video_service.hpp>
#include <sensorycloud/token_manager/secure_credential_store.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

using sensory::token_manager::TokenManager;
using sensory::token_manager::SecureCredentialStore;
using sensory::service::HealthService;
using sensory::service::VideoService;
using sensory::service::OAuthService;

/// @brief A bidirection stream reactor for biometric enrollments from video
/// stream data.
///
/// @details
/// Input data for the stream is provided by an OpenCV capture device.
///
class OpenCVReactor :
    public VideoService<SecureCredentialStore>::AuthenticateBidiReactor {
 private:
    /// A flag determining whether the last sent frame was enrolled. This flag
    /// is atomic to support thread safe reads and writes.
    std::atomic<bool> isAuthenticated;
    /// A flag determining whether the last sent frame was detected as live.
    std::atomic<bool> isLive;
    /// An OpenCV matrix containing the frame data from the camera.
    cv::Mat frame;
    /// A mutual exclusion for locking access to the frame between foreground
    /// (frame capture) and background (network stream processing) threads.
    std::mutex frameMutex;

 public:
    /// @brief Initialize a reactor for streaming video from an OpenCV stream.
    OpenCVReactor() :
        VideoService<SecureCredentialStore>::AuthenticateBidiReactor(),
        isAuthenticated(false),
        isLive(false) { }

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
            cv::imencode(".jpg", frame, buffer);
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
        // Log information about the response to the terminal.
        // std::cout << "Frame Response:" << std::endl;
        // std::cout << "\tSuccess: "  << response.success() << std::endl;
        // std::cout << "\tScore: "    << response.score() << std::endl;
        // std::cout << "\tIs Alive: " << response.isalive() << std::endl;
        // Set the authentication flag to the success of the response.
        isAuthenticated = response.success();
        isLive = response.isalive();
        if (!isAuthenticated)  // Start the next read request
            StartRead(&response);
    }

    /// @brief Stream video from an OpenCV capture device.
    ///
    /// @param capture The OpenCV capture device.
    /// @param isLivenessEnabled `true` to enable the liveness check interface,
    /// `false` to disable the interface.
    ///
    ::grpc::Status streamVideo(cv::VideoCapture& capture, const bool& isLivenessEnabled) {
        // Start the call to initiate the stream in the background.
        StartCall();
        // Start capturing frames from the device.
        while (!isAuthenticated) {
            {  // Lock the mutex and read a frame.
                std::lock_guard<std::mutex> lock(frameMutex);
                capture >> frame;
            }
            // If the frame is empty, something went wrong, exit the capture
            // loop.
            if (frame.empty()) break;
            // Draw text indicating the liveness status of the last frame.
            auto presentationFrame = frame.clone();
            if (isLivenessEnabled) {  // Liveness is enabled.
                cv::putText(presentationFrame,
                    isLive ? "Live" : "Not Live",
                    cv::Point(10, 40),
                    cv::FONT_HERSHEY_SIMPLEX,
                    1,  // font scale
                    isLive ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255),
                    2   // thickness
                );
            }
            // Show the frame in a view-finder window.
            cv::imshow("Sensory Cloud Face Authentication Demo", presentationFrame);
            // Listen for keyboard interrupts to terminate the capture.
            char c = (char) cv::waitKey(10);
            if (c == 27 || c == 'q' || c == 'Q') break;
        }
        return await();
    }
};

int main(int argc, const char** argv) {
    cv::CommandLineParser parser(argc, argv, "{help h||}{@device||}");
    std::string device = parser.get<std::string>("@device");
    if (!parser.check()) {
        parser.printErrors();
        return 0;
    }

    // Initialize the configuration to the host for given address and port
    sensory::Config config(
        "io.stage.cloud.sensory.com",
        443,
        "cabb7700-206f-4cc7-8e79-cd7f288aa78d",
        "D895F447-91E8-486F-A783-6E3A33E4C7C5"
    );
    std::cout << "Connecting to remote host: " << config.getFullyQualifiedDomainName() << std::endl;

    // ------ Check server health ----------------------------------------------

    // Create the health service.
    HealthService healthService(config);

    // Query the health of the remote service.
    healthService.getHealth([](HealthService::GetHealthCallData* call) {
        if (!call->getStatus().ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get server health with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
        }
        // Report the health of the remote service
        std::cout << "Server status" << std::endl;
        std::cout << "\tIs Healthy:     " << call->getResponse().ishealthy()     << std::endl;
        std::cout << "\tServer Version: " << call->getResponse().serverversion() << std::endl;
        std::cout << "\tID:             " << call->getResponse().id()            << std::endl;
    })->await();

    // ------ Authorize the current user ---------------------------------------

    // Query the user ID
    std::string userID = "";
    std::cout << "user ID: ";
    std::cin >> userID;

    // Create an OAuth service
    OAuthService oauthService(config);
    SecureCredentialStore keychain("com.sensory.cloud");
    TokenManager<SecureCredentialStore> tokenManager(oauthService, keychain);

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
        oauthService.asyncRegisterDevice(
            name,
            password,
            credentials.id,
            credentials.secret,
            [](OAuthService::RegisterDeviceCallData* call) {
            if (!call->getStatus().ok()) {  // The call failed.
                std::cout << "Failed to register device with\n\t" <<
                    call->getStatus().error_code() << ": " <<
                    call->getStatus().error_message() << std::endl;
            }
        })->await();
    }

    // ------ Get an enrollment ID ---------------------------------------------

    // Query this user's active enrollments
    std::cout << "Active enrollments:" << std::endl;
    sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore> mgmtService(config, tokenManager);
    sensory::api::v1::management::GetEnrollmentsResponse enrollmentResponse;
    auto status = mgmtService.getEnrollments(&enrollmentResponse, userID);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    for (auto& enrollment : enrollmentResponse.enrollments()) {
        if (enrollment.modeltype() != sensory::api::common::FACE_BIOMETRIC)
            continue;
        std::cout << "\tDescription:     " << enrollment.description()  << std::endl;
        std::cout << "\t\tModel Name:    " << enrollment.modelname()    << std::endl;
        std::cout << "\t\tModel Type:    " << enrollment.modeltype()    << std::endl;
        std::cout << "\t\tModel Version: " << enrollment.modelversion() << std::endl;
        std::cout << "\t\tUser ID:       " << enrollment.userid()       << std::endl;
        std::cout << "\t\tDevice ID:     " << enrollment.deviceid()     << std::endl;
        std::cout << "\t\tCreated:       "
            << google::protobuf::util::TimeUtil::ToString(enrollment.createdat())
            << std::endl;
        std::cout << "\t\tUpdated:       "
            << google::protobuf::util::TimeUtil::ToString(enrollment.updatedat())
            << std::endl;
        std::cout << "\t\tID:            " << enrollment.id()    << std::endl;
    }

    std::string enrollmentID = "";
    std::cout << "Enrollment ID: ";
    std::cin >> enrollmentID;

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

    // ------ Create the video service -----------------------------------------

    // Create the video service based on the configuration and token manager.
    VideoService<SecureCredentialStore> videoService(config, tokenManager);

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

    // Create the stream.
    OpenCVReactor reactor;
    videoService.authenticate(&reactor, enrollmentID, isLivenessEnabled);
    // Wait for the stream to conclude. This is necessary to check the final
    // status of the call and allow any dynamically allocated data to be cleaned
    // up. If the stream is destroyed before the final `onDone` callback, odd
    // runtime errors can occur.
    status = reactor.streamVideo(capture, isLivenessEnabled);

    if (!status.ok()) {
        std::cout << "Failed to authenticate with\n\t" <<
            status.error_code() << ": " <<
            status.error_message() << std::endl;
    } else {
        std::cout << "Successfully authenticated!" << std::endl;
    }

    return 0;
}
