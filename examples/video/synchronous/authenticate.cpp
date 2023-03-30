// Face authentication using SensoryCloud with OpenCV.
//
// Copyright (c) 2023 Sensory, Inc.
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
#include <google/protobuf/util/json_util.h>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include "../dep/argparse.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::api::v1::video::RecognitionThreshold;

// The thickness of the face boxes to render
const auto BOX_THICKNESS = 5;
// The thickness of the font to render
const auto FONT_THICKNESS = 2;
// The scale of the font to render
const auto FONT_SCALE = 0.9;

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("authenticate")
        .description("A tool for authenticating with face biometrics using SensoryCloud.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-u", "--userid" })
        .help("The name of the user ID to query the enrollments for.");
    parser.add_argument({ "-e", "--enrollmentid" })
        .help("The ID of the enrollment to authenticate against.");
    parser.add_argument({ "-l", "--liveness" })
        .action("store_true")
        .help("Whether to conduct a liveness check in addition to the enrollment.");
    parser.add_argument({ "-t", "--threshold" })
        .choices({"LOW", "MEDIUM", "HIGH", "HIGHEST"})
        .default_value("HIGH")
        .help("The security threshold for conducting the liveness check.");
    parser.add_argument({ "-g", "--group" })
        .action("store_true")
        .help("A flag determining whether the enrollment ID is for an enrollment group.");
    parser.add_argument({ "-D", "--device" })
        .default_value("0")
        .help("The ID of the OpenCV device to use or a path to an image / video file.");
    parser.add_argument({ "-C", "--codec" })
        .default_value("jpg")
        .help("The codec to use when compressing image data.");
    parser.add_argument({ "-v", "--verbose" })
        .action("store_true")
        .help("Produce verbose output.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto USER_ID = args.get<std::string>("userid");
    const auto ENROLLMENT_ID = args.get<std::string>("enrollmentid");
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
    const auto GROUP = args.get<bool>("group");
    const auto DEVICE = args.get<std::string>("device");
    const auto CODEC = "." + args.get<std::string>("codec");
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
        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true;
        options.always_print_primitive_fields = true;
        options.always_print_enums_as_ints = false;
        options.preserve_proto_field_names = true;
        std::string server_health_json;
        google::protobuf::util::MessageToJsonString(server_health, &server_health_json, options);
        std::cout << server_health_json << std::endl;
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

    // ------ Query the available enrollments ----------------------------------

    if (USER_ID != "") {
        sensory::api::v1::management::GetEnrollmentsResponse enrollment_response;
        status = cloud.management.get_enrollments(&enrollment_response, USER_ID);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get enrollments ("
                << status.error_code() << "): "
                << status.error_message() << std::endl;
            return 1;
        }
        for (auto& enrollment : enrollment_response.enrollments()) {
            if (enrollment.modeltype() != sensory::api::common::FACE_BIOMETRIC)
                continue;
            google::protobuf::util::JsonPrintOptions options;
            options.add_whitespace = true;
            options.always_print_primitive_fields = true;
            options.always_print_enums_as_ints = false;
            options.preserve_proto_field_names = true;
            std::string enrollment_json;
            google::protobuf::util::MessageToJsonString(enrollment, &enrollment_json, options);
            std::cout << enrollment_json << std::endl;
        }
        return 0;
    }

    // ------ Create the authentication stream ---------------------------------

    // Create the config with the authentication parameters.
    auto config = new ::sensory::api::v1::video::AuthenticateConfig;
    if (GROUP)
        config->set_enrollmentgroupid(ENROLLMENT_ID);
    else
        config->set_enrollmentid(ENROLLMENT_ID);
    config->set_islivenessenabled(LIVENESS);
    config->set_livenessthreshold(THRESHOLD);
    // Initialize the stream with the cloud.
    grpc::ClientContext context;
    auto stream = cloud.video.authenticate(&context, config);

    // Create an image capture object
    cv::VideoCapture capture;
    const bool IS_DEVICE_NUMERIC = !DEVICE.empty() && DEVICE.find_first_not_of("0123456789") == std::string::npos;
    if (!(IS_DEVICE_NUMERIC ? capture.open(stoi(DEVICE)) : capture.open(DEVICE))) {
        std::cout << "Capture from device " << DEVICE << " failed" << std::endl;
        return 1;
    }

    // Here we use atomic variables to simplify the passage of data between
    // networking and UI contexts.
    std::atomic<bool> did_find_face(false), is_live(false), is_authenticated(false);
    std::atomic<float> xmin(0), ymin(0), xmax(0), ymax(0);
    std::atomic<float> score(0);
    // An OpenCV matrix containing the frame data from the camera.
    cv::Mat frame;
    // A mutual exclusion for locking access to the frame between foreground
    // (frame capture) and background (network stream processing) threads.
    std::mutex frame_mutex;

    // Create a thread to run network IO in the background.
    std::thread network_thread([&](){
        while (!is_authenticated) {
            std::vector<unsigned char> buffer;
            {  // Lock the mutex and encode the frame into a buffer.
                std::lock_guard<std::mutex> lock(frame_mutex);
                // If the frame is empty, something went wrong, exit the loop.
                if (frame.empty()) break;
                cv::imencode(CODEC, frame, buffer);
            }
            // Create a new request with the video content.
            sensory::api::v1::video::AuthenticateRequest request;
            request.set_imagecontent(buffer.data(), buffer.size());
            if (!stream->Write(request)) break;
            // Read a new response from the server.
            sensory::api::v1::video::AuthenticateResponse response;
            if (!stream->Read(&response)) break;
            did_find_face = response.didfindface();
            xmin = response.boundingbox()[0];
            ymin = response.boundingbox()[1];
            xmax = response.boundingbox()[2];
            ymax = response.boundingbox()[3];
            is_authenticated = response.success();
            score = response.score();
            is_live = response.isalive();
            if (is_authenticated)
                stream->WritesDone();
            // Print the response structure in a JSON format.
            if (VERBOSE) {
                google::protobuf::util::JsonPrintOptions options;
                options.add_whitespace = false;
                options.always_print_primitive_fields = true;
                options.always_print_enums_as_ints = false;
                options.preserve_proto_field_names = true;
                std::string response_json;
                google::protobuf::util::MessageToJsonString(response, &response_json, options);
                std::cout << response_json << std::endl;
            }
        }
    });

    // Start capturing frames from the device.
    while (!is_authenticated) {
        {  // Lock the mutex and read a frame.
            std::lock_guard<std::mutex> lock(frame_mutex);
            capture >> frame;
        }
        // If the frame is empty, something went wrong, exit the capture loop.
        if (frame.empty()) break;
        auto presentation_frame = frame.clone();
        if (did_find_face) {  // Draw the bounding box on the frame.
            const auto box_color = (!LIVENESS || is_live) ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
            // Draw the face bounding box
            cv::rectangle(presentation_frame,
                cv::Point(xmin, ymin),
                cv::Point(xmax, ymax),
                box_color,
                BOX_THICKNESS);
            if (LIVENESS) {  // Render the liveness decision
                const auto label = is_live ? "Live" : "Spoof";
                // Determine the size of the label
                const auto text_size = cv::getTextSize(label,
                    cv::FONT_HERSHEY_SIMPLEX,
                    FONT_SCALE,
                    FONT_THICKNESS,
                    nullptr  // baseline is not used
                );
                // Create a solid background to render the label on top of
                cv::rectangle(presentation_frame,
                    cv::Point(xmin + BOX_THICKNESS - 1, ymin + BOX_THICKNESS - 1),
                    cv::Point(xmin + text_size.width + BOX_THICKNESS + FONT_THICKNESS + 1, ymin + text_size.height + BOX_THICKNESS + FONT_THICKNESS + 5),
                    box_color, cv::FILLED);
                // Render the text label for the frame
                cv::putText(presentation_frame, label,
                    cv::Point(xmin + BOX_THICKNESS, ymin + text_size.height + BOX_THICKNESS),
                    cv::FONT_HERSHEY_SIMPLEX,
                    FONT_SCALE,
                    cv::Scalar(255, 255, 255),
                    FONT_THICKNESS
                );
            }
        }
        // Show the frame in a viewfinder window.
        cv::imshow("SensoryCloud Face Authentication Demo", presentation_frame);
        // Listen for keyboard interrupts to terminate the capture.
        char c = (char) cv::waitKey(10);
        if (c == 27 || c == 'q' || c == 'Q') break;
    }

    // Wait for the network thread to join back in.
    network_thread.join();
    status = stream->Finish();

    if (!status.ok()) {  // The stream failed, print a descriptive message.
        std::cout << "Authentication stream failed ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    } else if (is_authenticated) {
        std::cout << "Successfully authenticated!" << std::endl;
    } else {
        std::cout << "Failed to authenticate!" << std::endl;
    }

    return 0;
}
