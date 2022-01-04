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
#include <sensorycloud/services/video_service.hpp>
#include <sensorycloud/token_manager/insecure_credential_store.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

using sensory::token_manager::TokenManager;
using sensory::token_manager::InsecureCredentialStore;
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
    public VideoService<InsecureCredentialStore>::CreateEnrollmentBidiReactor {
 private:
    /// A flag determining whether the last sent frame was enrolled. This flag
    /// is atomic to support thread safe reads and writes.
    std::atomic<bool> isEnrolled;
    /// The completion percentage of the enrollment request.
    std::atomic<float> percentComplete;
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
        VideoService<InsecureCredentialStore>::CreateEnrollmentBidiReactor(),
        isEnrolled(false),
        percentComplete(0),
        isLive(false) { }

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
        if (isEnrolled) return;
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        // Log information about the response to the terminal.
        // std::cout << "Frame Response:     " << std::endl;
        // std::cout << "\tPercent Complete: " << response.percentcomplete() << std::endl;
        // std::cout << "\tIs Alive?:        " << response.isalive() << std::endl;
        // std::cout << "\tEnrollment ID:    " << response.enrollmentid() << std::endl;
        // std::cout << "\tModel Name:       " << response.modelname() << std::endl;
        // std::cout << "\tModel Version:    " << response.modelversion() << std::endl;
        // If the enrollment ID is not empty, then the enrollment succeeded.
        isEnrolled = !response.enrollmentid().empty();
        // Set the completion percentage of the enrollment.
        percentComplete = response.percentcomplete() / 100.f;
        // Set the liveness status of the last frame.
        isLive = response.isalive();
        if (!isEnrolled)  // Start the next read request.
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
        while (!isEnrolled) {
            {  // Lock the mutex and read a frame.
                std::lock_guard<std::mutex> lock(frameMutex);
                capture >> frame;
            }
            // If the frame is empty, something went wrong, exit the capture
            // loop.
            if (frame.empty()) break;
            // Draw the progress bar on the frame. Do this on a copy of the
            // so that the presentation frame is not sent to the server for
            // enrollment.
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
            // Draw text indicating the liveness status of the last frame.
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
            cv::imshow("Sensory Cloud Face Enrollment Demo", presentationFrame);
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
    TokenManager<InsecureCredentialStore> tokenManager(oauthService, keychain);

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
        oauthService.registerDevice(
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

    // ------ Create the video service -----------------------------------------

    // Create the video service based on the configuration and token manager.
    VideoService<InsecureCredentialStore>
        videoService(config, tokenManager);

    // ------ Query the available video models ---------------------------------

    std::cout << "Available video models:" << std::endl;
    videoService.getModels([](VideoService<InsecureCredentialStore>::GetModelsCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to get video models with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
        }
        // Iterate over the models returned in the response
        for (auto& model : call->getResponse().models()) {
            // Ignore models that aren't face biometric models.
            if (model.modeltype() != sensory::api::common::FACE_BIOMETRIC)
                continue;
            std::cout << "\t" << model.name() << std::endl;
        }
    })->await();

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
    videoService.createEnrollment(&reactor,
        videoModel,
        userID,
        description,
        isLivenessEnabled
    );
    // Wait for the stream to conclude. This is necessary to check the final
    // status of the call and allow any dynamically allocated data to be cleaned
    // up. If the stream is destroyed before the final `onDone` callback, odd
    // runtime errors can occur.
    auto status = reactor.streamVideo(capture, isLivenessEnabled);

    if (!status.ok()) {
        std::cout << "Failed to enroll with\n\t" <<
            status.error_code() << ": " <<
            status.error_message() << std::endl;
    } else {
        std::cout << "Successful enrollment! Your enrollment ID is:" << std::endl;
        std::cout << reactor.response.enrollmentid() << std::endl;
    }

    return 0;
}
