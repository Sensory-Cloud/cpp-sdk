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
#include <sensorycloud/token_manager/insecure_credential_store.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include "dep/argparse.hpp"

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("enroll")
        .description("A tool for authenticating with face biometrics using Sensory Cloud.");
    parser.add_argument({ "-H", "--host" })
        .required(true)
        .help("HOST The hostname of a Sensory Cloud inference server.");
    parser.add_argument({ "-P", "--port" })
        .required(true)
        .help("PORT The port number that the Sensory Cloud inference server is running at.");
    parser.add_argument({ "-T", "--tenant" })
        .required(true)
        .help("TENANT The ID of your tenant on a Sensory Cloud inference server.");
    parser.add_argument({ "-I", "--insecure" })
        .action("store_true")
        .help("INSECURE Disable TLS.");
    parser.add_argument({ "-g", "--getmodels" })
        .action("store_true")
        .help("GETMODELS Whether to query for a list of available models.");
    parser.add_argument({ "-m", "--model" })
        .help("MODEL The model to use for the enrollment.");
    parser.add_argument({ "-u", "--userid" })
        .help("USERID The name of the user ID to query the enrollments for.");
    parser.add_argument({ "-d", "--description" })
        .help("DESCRIPTION A text description of the enrollment.");
    parser.add_argument({ "-l", "--liveness" })
        .action("store_true")
        .help("LIVENESS Whether to conduct a liveness check in addition to the enrollment.");
    parser.add_argument({ "-t", "--threshold" })
        .choices({"LOW", "MEDIUM", "HIGH", "HIGHEST"})
        .default_value("HIGH")
        .help("THRESHOLD The security threshold for conducting the liveness check.");
    parser.add_argument({ "-D", "--device" })
        .default_value(0)
        .help("DEVICE The ID of the OpenCV device to use.");
    parser.add_argument({ "-v", "--verbose" })
        .action("store_true")
        .help("VERBOSE Produce verbose output during authentication.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto HOSTNAME = args.get<std::string>("host");
    const auto PORT = args.get<uint16_t>("port");
    const auto TENANT = args.get<std::string>("tenant");
    const auto IS_SECURE = !args.get<bool>("insecure");
    const auto GETMODELS = args.get<bool>("getmodels");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    const auto DESCRIPTION = args.get<std::string>("description");
    const auto LIVENESS = args.get<bool>("liveness");
    sensory::api::v1::video::RecognitionThreshold THRESHOLD;
    if (args.get<std::string>("threshold") == "LOW")
        THRESHOLD = sensory::api::v1::video::RecognitionThreshold::LOW;
    else if (args.get<std::string>("threshold") == "MEDIUM")
        THRESHOLD = sensory::api::v1::video::RecognitionThreshold::MEDIUM;
    else if (args.get<std::string>("threshold") == "HIGH")
        THRESHOLD = sensory::api::v1::video::RecognitionThreshold::HIGH;
    else if (args.get<std::string>("threshold") == "HIGHEST")
        THRESHOLD = sensory::api::v1::video::RecognitionThreshold::HIGHEST;
    const auto DEVICE = args.get<int>("device");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create an insecure credential store for keeping OAuth credentials in.
    sensory::token_manager::InsecureCredentialStore keychain(".", "com.sensory.cloud.examples");
    if (!keychain.contains("deviceID"))
        keychain.emplace("deviceID", sensory::token_manager::uuid_v4());
    const auto DEVICE_ID(keychain.at("deviceID"));

    // Initialize the configuration to the host for given address and port
    sensory::Config config(HOSTNAME, PORT, TENANT, DEVICE_ID, IS_SECURE);

    // Query the health of the remote service.
    sensory::service::HealthService healthService(config);
    sensory::api::common::ServerHealthResponse serverHealth;
    auto status = healthService.getHealth(&serverHealth);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    } else if (VERBOSE) {
        // Report the health of the remote service
        std::cout << "Server status:" << std::endl;
        std::cout << "\tisHealthy: " << serverHealth.ishealthy() << std::endl;
        std::cout << "\tserverVersion: " << serverHealth.serverversion() << std::endl;
        std::cout << "\tid: " << serverHealth.id() << std::endl;
    }

    // ------ Enroll the current user ----------------------------------------

    // Create an OAuth service
    sensory::service::OAuthService oauthService(config);
    sensory::token_manager::TokenManager<sensory::token_manager::InsecureCredentialStore> tokenManager(oauthService, keychain);

    if (!tokenManager.hasToken()) {  // the device is not registered
        // Generate a new clientID and clientSecret for this device
        const auto credentials = tokenManager.generateCredentials();

        std::cout << "Registering device with server..." << std::endl;

        // Query the friendly device name
        std::string name = "";
        std::cout << "Device Name: ";
        std::cin >> name;

        // Query the shared pass-phrase
        std::string password = "";
        std::cout << "Password: ";
        std::cin >> password;

        // Register this device with the remote host
        sensory::api::v1::management::DeviceResponse registerResponse;
        auto status = oauthService.registerDevice(&registerResponse,
            name, password, credentials.id, credentials.secret);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to register device with\n\t" <<
                status.error_code() << ": " << status.error_message() << std::endl;
            return 1;
        }
    }

    // Query the available video models
    sensory::service::VideoService<sensory::token_manager::InsecureCredentialStore> videoService(config, tokenManager);

    if (GETMODELS) {
        sensory::api::v1::video::GetModelsResponse videoModelsResponse;
        status = videoService.getModels(&videoModelsResponse);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get video models with\n\t" <<
                status.error_code() << ": " << status.error_message() << std::endl;
            return 1;
        }
        for (auto& model : videoModelsResponse.models()) {
            if (model.modeltype() != sensory::api::common::FACE_BIOMETRIC)
                continue;
            std::cout << model.name() << std::endl;
        }
        return 0;
    }

    // Create the stream
    grpc::ClientContext context;
    auto stream = videoService.createEnrollment(&context,
        sensory::service::video::newCreateEnrollmentConfig(
            MODEL, USER_ID, DESCRIPTION, LIVENESS, THRESHOLD
        )
    );

    // Create an image capture object
    cv::VideoCapture capture;
    if (!capture.open(DEVICE)) {
        std::cout << "Capture from camera #" << DEVICE << " failed" << std::endl;
        return 1;
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
    std::thread networkThread([&stream, &isEnrolled, &percentComplete, &isLive, &frame, &frameMutex, &VERBOSE](){
        while (!isEnrolled) {
            std::vector<unsigned char> buffer;
            {  // Lock the mutex and encode the frame with JPEG into a buffer.
                std::lock_guard<std::mutex> lock(frameMutex);
                cv::imencode(".jpg", frame, buffer);
            }
            // Create a new request with the video content.
            sensory::api::v1::video::CreateEnrollmentRequest request;
            request.set_imagecontent(buffer.data(), buffer.size());
            if (!stream->Write(request)) break;
            // Read a new response from the server.
            sensory::api::v1::video::CreateEnrollmentResponse response;
            if (!stream->Read(&response)) break;
            // Log information about the response to the terminal.
            if (VERBOSE) {
                std::cout << "Frame Response:     " << std::endl;
                std::cout << "\tPercent Complete: " << response.percentcomplete() << std::endl;
                std::cout << "\tIs Alive?:        " << response.isalive() << std::endl;
                std::cout << "\tEnrollment ID:    " << response.enrollmentid() << std::endl;
                std::cout << "\tModel Name:       " << response.modelname() << std::endl;
                std::cout << "\tModel Version:    " << response.modelversion() << std::endl;
            }
            // Set the authentication flag to the success of the response.
            isEnrolled = !response.enrollmentid().empty();
            percentComplete = response.percentcomplete() / 100.f;
            isLive = response.isalive();
            if (isEnrolled) {
                std::cout << "Successfully enrolled with ID: "
                    << response.enrollmentid() << std::endl;
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
        if (LIVENESS) {  // liveness is enabled
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
    status = stream->Finish();
    // Wait for the network thread to join back in.
    networkThread.join();

    if (!status.ok()) {  // The stream failed, print a descriptive message.
        std::cout << "Create enrollment stream failed with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
