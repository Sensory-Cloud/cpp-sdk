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

    // Query the available video models
    std::cout << "Available video models:" << std::endl;
    sensory::service::VideoService<sensory::token_manager::Keychain> videoService(config, tokenManager);
    sensory::api::v1::video::GetModelsResponse videoModelsResponse;
    status = videoService.getModels(&videoModelsResponse);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get video models with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    for (auto& model : videoModelsResponse.models()) {
        if (model.modeltype() != sensory::api::common::FACE_RECOGNITION)
            continue;
        std::cout << "\t" << model.name() << std::endl;
    }

    std::string videoModel = "";
    std::cout << "Video model: ";
    std::cin >> videoModel;

    // Create the stream
    auto stream = videoService.validateLiveness(videoModel, userID,
        sensory::api::v1::video::RecognitionThreshold::LOW);

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

    enum class FaceAlignment : int {
        Valid = 0,
        Unknown = 100,
        NoFace = 101,
        SmallFace = 102,
        BadFQ = 103,
        NotCentered = 104,
        NotVertical = 105
    };

    // A flag determining whether the last sent frame contained a live face.
    // This flag is atomic to support thread safe reads and writes.
    std::atomic<bool> isLive(false);
    std::atomic<FaceAlignment> alignmentCode(FaceAlignment::Valid);
    // An OpenCV matrix containing the frame data from the camera.
    cv::Mat frame;
    // A mutual exclusion for locking access to the frame between foreground
    // (frame capture) and background (network stream processing) threads.
    std::mutex frameMutex;

    // Create a thread to poll read requests in the background. Audio
    // transcription has a bursty response pattern, so a locked read-write loop
    // will not work with this service.
    std::thread networkThread([&stream, &isLive, &alignmentCode, &frame, &frameMutex](){
        while (true) {
            // Lock the mutual exclusion to the frame and encode it into JPEG
            std::vector<unsigned char> buffer;
            frameMutex.lock();
            cv::imencode(".jpg", frame, buffer);
            frameMutex.unlock();
            // Create the request from the encoded image data.
            sensory::api::v1::video::ValidateRecognitionRequest request;
            request.set_imagecontent(buffer.data(), buffer.size());
            if (!stream->Write(request)) break;
            sensory::api::v1::video::LivenessRecognitionResponse response;
            if (!stream->Read(&response)) break;
            // Log information about the response to the terminal.
            // std::cout << "Frame Response:" << std::endl;
            // std::cout << "\tScore: "    << response.score() << std::endl;
            // std::cout << "\tIs Alive: " << response.isalive() << std::endl;
            // Set the authentication flag to the success of the response.
            isLive = response.isalive();
            alignmentCode = response.score() < 100 ?
                FaceAlignment::Valid : static_cast<FaceAlignment>(response.score());
        }
    });

    // Start capturing frames from the device.
    while (true) {
        // Lock the mutual exclusion to the frame buffer and fetch a new frame.
        frameMutex.lock();
        capture >> frame;
        frameMutex.unlock();
        // If the frame is empty, something went wrong, exit the capture loop.
        if (frame.empty()) break;
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
        auto presentationFrame = frame.clone();
        cv::putText(presentationFrame,
            message,
            cv::Point(10, 40),
            cv::FONT_HERSHEY_SIMPLEX,
            1,  // font scale
            isLive ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255),
            2   // thickness
        );
        // Show the frame in a viewfinder window.
        cv::imshow("Sensory Cloud Face Liveness Demo", presentationFrame);
        // Listen for keyboard interrupts to terminate the capture.
        char c = (char) cv::waitKey(10);
        if (c == 27 || c == 'q' || c == 'Q') break;
    }

    // Terminate the stream.
    stream->WritesDone();
    status = stream->Finish();
    // Wait for the network thread to join back in.
    networkThread.join();

    if (!status.ok()) {  // The stream failed, print a descriptive message.
        std::cout << "Authentication stream failed with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
