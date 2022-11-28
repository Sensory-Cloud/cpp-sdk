// An example of biometric face enrollment using SensoryCloud with OpenCV.
//
// Copyright (c) 2022 Sensory, Inc.
//
// Author: Christian Kauten (ckauten@sensoryinc.com)
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
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include "../dep/argparse.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::api::v1::video::RecognitionThreshold;

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("enroll")
        .description("A tool for enrolling with face biometrics using SensoryCloud.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-g", "--getmodels" })
        .action("store_true")
        .help("Whether to query for a list of available models.");
    parser.add_argument({ "-m", "--model" })
        .help("The model to use for the enrollment.");
    parser.add_argument({ "-u", "--userid" })
        .help("The name of the user ID to query the enrollments for.");
    parser.add_argument({ "-d", "--description" })
        .help("A text description of the enrollment.");
    parser.add_argument({ "-l", "--liveness" })
        .action("store_true")
        .help("Whether to conduct a liveness check in addition to the enrollment.");
    parser.add_argument({ "-t", "--threshold" })
        .choices({"LOW", "MEDIUM", "HIGH", "HIGHEST"})
        .default_value("HIGH")
        .help("The security threshold for conducting the liveness check.");
    parser.add_argument({ "-lN", "--num-liveness-frames" })
        .default_value(0)
        .help(
            "If liveness is enabled, this determines how many \n\t\t\t"
            "frames need to pass the liveness check before the \n\t\t\t"
            "enrollment can be successful. A value of 0 means \n\t\t\t"
            "that all frames must pass the liveness check."
        );
    parser.add_argument({ "-r", "--reference-id" })
        .help("An optional reference ID for tagging the enrollment.");
    parser.add_argument({ "-D", "--device" })
        .default_value("0")
        .help("The ID of the OpenCV device to use or a path to an image / video file.");
    parser.add_argument({ "-v", "--verbose" })
        .action("store_true")
        .help("Produce verbose output.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto GETMODELS = args.get<bool>("getmodels");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    const auto DESCRIPTION = args.get<std::string>("description");
    const auto LIVENESS = args.get<bool>("liveness");
    RecognitionThreshold THRESHOLD;
    if (args.get<std::string>("threshold") == "LOW")
        THRESHOLD = RecognitionThreshold::LOW;
    else if (args.get<std::string>("threshold") == "MEDIUM")
        THRESHOLD = RecognitionThreshold::MEDIUM;
    else if (args.get<std::string>("threshold") == "HIGH")
        THRESHOLD = RecognitionThreshold::HIGH;
    else if (args.get<std::string>("threshold") == "HIGHEST")
        THRESHOLD = RecognitionThreshold::HIGHEST;
    const auto NUM_LIVENESS_FRAMES = args.get<int>("num-liveness-frames");
    const auto REFERENCE_ID = args.get<std::string>("reference-id");
    const auto DEVICE = args.get<std::string>("device");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create a credential store for keeping OAuth credentials in.
    FileSystemCredentialStore keychain(".", "com.sensory.cloud.examples");

    // Create the cloud services handle.
    SensoryCloud<FileSystemCredentialStore> cloud(PATH, keychain);

    // Query the health of the remote service.
    sensory::api::common::ServerHealthResponse server_health;
    auto status = cloud.health.get_health(&server_health);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }
    if (VERBOSE) {
        std::cout << "Server status:" << std::endl;
        std::cout << "\tisHealthy: " << server_health.ishealthy() << std::endl;
        std::cout << "\tserverVersion: " << server_health.serverversion() << std::endl;
        std::cout << "\tid: " << server_health.id() << std::endl;
    }

    // Initialize the client.
    sensory::api::v1::management::DeviceResponse response;
    status = cloud.initialize(&response);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to initialize ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }

    // ------ Query the available video models ---------------------------------

    if (GETMODELS) {
        sensory::api::v1::video::GetModelsResponse video_models_response;
        status = cloud.video.get_models(&video_models_response);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get video models ("
                << status.error_code() << "): "
                << status.error_message() << std::endl;
            return 1;
        }
        for (auto& model : video_models_response.models()) {
            if (model.modeltype() != sensory::api::common::FACE_BIOMETRIC)
                continue;
            std::cout << model.name() << std::endl;
        }
        return 0;
    }

    // ------ Create the enrollment stream -------------------------------------

    // Create the config with the enrollment parameters.
    auto config = new ::sensory::api::v1::video::CreateEnrollmentConfig;
    config->set_modelname(MODEL);
    config->set_userid(USER_ID);
    config->set_description(DESCRIPTION);
    config->set_islivenessenabled(LIVENESS);
    config->set_livenessthreshold(THRESHOLD);
    config->set_numlivenessframesrequired(NUM_LIVENESS_FRAMES);
    config->set_referenceid(REFERENCE_ID);
    // Initialize the stream with the cloud.
    grpc::ClientContext context;
    auto stream = cloud.video.create_enrollment(&context, config);

    // Create an image capture object
    cv::VideoCapture capture;
    const bool IS_DEVICE_NUMERIC = !DEVICE.empty() && DEVICE.find_first_not_of("0123456789") == std::string::npos;
    if (!(IS_DEVICE_NUMERIC ? capture.open(stoi(DEVICE)) : capture.open(DEVICE))) {
        std::cout << "Capture from device " << DEVICE << " failed" << std::endl;
        return 1;
    }

    // A flag determining whether the last sent frame was enrolled. This flag
    // is atomic to support thread safe reads and writes.
    std::atomic<bool> is_enrolled(false);
    // The completion percentage of the enrollment request.
    std::atomic<float> percent_complete(0);
    // A flag determining whether the last sent frame was detected as live.
    std::atomic<bool> is_live(false);
    // An OpenCV matrix containing the frame data from the camera.
    cv::Mat frame;
    // A mutual exclusion for locking access to the frame between foreground
    // (frame capture) and background (network stream processing) threads.
    std::mutex frame_mutex;

    // Create a thread to perform network IO in the background.
    std::thread network_thread([&stream, &is_enrolled, &percent_complete, &is_live, &frame, &frame_mutex, &VERBOSE](){
        while (!is_enrolled) {
            std::vector<unsigned char> buffer;
            {  // Lock the mutex and encode the frame with JPEG into a buffer.
                std::lock_guard<std::mutex> lock(frame_mutex);
                // If the frame is empty, something went wrong, exit the loop.
                if (frame.empty()) break;
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
            is_enrolled = !response.enrollmentid().empty();
            percent_complete = response.percentcomplete() / 100.f;
            is_live = response.isalive();
            if (is_enrolled) {
                std::cout << "Successfully enrolled with ID: "
                    << response.enrollmentid() << std::endl;
            }
        }
    });

    // Start capturing frames from the device.
    while (!is_enrolled) {
        {  // Lock the mutex and read a frame.
            std::lock_guard<std::mutex> lock(frame_mutex);
            capture >> frame;
        }
        // If the frame is empty, something went wrong, exit the capture loop.
        if (frame.empty()) break;
        // Draw the progress bar on the frame
        auto presentation_frame = frame.clone();
        cv::rectangle(
            presentation_frame,
            cv::Point(0, 0),
            cv::Point(presentation_frame.size().width, 10),
            cv::Scalar(0, 0, 0), -1);
        cv::rectangle(
            presentation_frame,
            cv::Point(0, 0),
            cv::Point(percent_complete * presentation_frame.size().width, 10),
            cv::Scalar(0, 255, 0), -1);
        // Draw some text indicating the liveness status
        if (LIVENESS) {  // liveness is enabled
            cv::putText(presentation_frame,
                is_live ? "Live" : "Not Live",
                cv::Point(10, 40),
                cv::FONT_HERSHEY_SIMPLEX,
                1,  // font scale
                is_live ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255),
                2   // thickness
            );
        }
        // Show the frame in a viewfinder window.
        cv::imshow("SensoryCloud Face Enrollment Demo", presentation_frame);
        // Listen for keyboard interrupts to terminate the capture.
        char c = (char) cv::waitKey(10);
        if (c == 27 || c == 'q' || c == 'Q') break;
    }

    // Terminate the stream.
    stream->WritesDone();
    status = stream->Finish();
    // Wait for the network thread to join back in.
    network_thread.join();

    if (!status.ok()) {  // The stream failed, print a descriptive message.
        std::cout << "Create enrollment stream failed ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
