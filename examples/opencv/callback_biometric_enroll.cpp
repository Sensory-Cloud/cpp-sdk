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
#include <sensorycloud/token_manager/keychain.hpp>
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
    sensory::service::HealthService healthService(config);

    // Query the health of the remote service.
    healthService.asyncGetHealth([](sensory::service::HealthService::GetHealthCallData* call) {
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
    sensory::service::OAuthService oauthService(config);
    sensory::token_manager::Keychain keychain("com.sensory.cloud");
    sensory::token_manager::TokenManager<sensory::token_manager::Keychain> tokenManager(oauthService, keychain);

    if (!tokenManager.hasSavedCredentials()) {  // the device is not registered
        // Generate a new clientID and clientSecret for this device
        const auto credentials = tokenManager.generateCredentials();

        // Query the shared pass-phrase
        std::string password = "";
        std::cout << "password: ";
        std::cin >> password;

        // Register this device with the remote host
        sensory::api::v1::management::DeviceResponse registerResponse;
        auto status = oauthService.registerDevice(&registerResponse,
            userID,
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
    sensory::service::VideoService<sensory::token_manager::Keychain>
        videoService(config, tokenManager);

    // ------ Query the available video models ---------------------------------

    std::cout << "Available video models:" << std::endl;
    videoService.asyncGetModels([](sensory::service::VideoService<sensory::token_manager::Keychain>::GetModelsCallData* call) {
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

    // Create the stream
    auto stream = videoService.createEnrollment(
        videoModel,
        userID,
        description,
        isLivenessEnabled
    );

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

    // Create a thread to poll read requests in the background. Audio
    // transcription has a bursty response pattern, so a locked read-write loop
    // will not work with this service.
    std::thread networkThread([&stream, &isEnrolled, &percentComplete, &isLive, &frame, &frameMutex](){
        while (!isEnrolled) {
            // Lock the mutual exclusion to the frame and encode it into JPEG
            std::vector<unsigned char> buffer;
            frameMutex.lock();
            cv::imencode(".jpg", frame, buffer);
            frameMutex.unlock();
            // Create the request from the encoded image data.
            sensory::api::v1::video::CreateEnrollmentRequest request;
            request.set_imagecontent(buffer.data(), buffer.size());
            stream->Write(request);
            sensory::api::v1::video::CreateEnrollmentResponse response;
            stream->Read(&response);
            // Log information about the response to the terminal.
            // std::cout << "Frame Response:     " << std::endl;
            // std::cout << "\tPercent Complete: " << response.percentcomplete() << std::endl;
            // std::cout << "\tIs Alive?:        " << response.isalive() << std::endl;
            // std::cout << "\tEnrollment ID:    " << response.enrollmentid() << std::endl;
            // std::cout << "\tModel Name:       " << response.modelname() << std::endl;
            // std::cout << "\tModel Version:    " << response.modelversion() << std::endl;
            // Set the authentication flag to the success of the response.
            isEnrolled = !response.enrollmentid().empty();
            percentComplete = response.percentcomplete() / 100.f;
            isLive = response.isalive();
        }
    });

    // Start capturing frames from the device.
    while (!isEnrolled) {
        // Lock the mutual exclusion to the frame buffer and fetch a new frame.
        frameMutex.lock();
        capture >> frame;
        frameMutex.unlock();
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

    // Terminate the stream.
    stream->WritesDone();
    auto status = stream->Finish();
    // Wait for the network thread to join back in.
    networkThread.join();

    if (!status.ok()) {  // The stream failed, print a descriptive message.
        std::cout << "Authentication stream failed with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
