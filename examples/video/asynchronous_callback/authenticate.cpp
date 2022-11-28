// An example of biometric face authentication using SensoryCloud with OpenCV.
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
using sensory::service::VideoService;

/// @brief A bidirection stream reactor for biometric enrollments from video
/// stream data.
///
/// @details
/// Input data for the stream is provided by an OpenCV capture device.
///
class OpenCVReactor :
    public VideoService<FileSystemCredentialStore>::AuthenticateBidiReactor {
 private:
    /// A flag determining whether the last sent frame was enrolled. This flag
    /// is atomic to support thread safe reads and writes.
    std::atomic<bool> is_authenticated;
    // The score from the liveness model.
    std::atomic<float> score;
    /// A flag determining whether the last sent frame was detected as live.
    std::atomic<bool> is_live;
    /// An OpenCV matrix containing the frame data from the camera.
    cv::Mat frame;
    /// A mutual exclusion for locking access to the frame between foreground
    /// (frame capture) and background (network stream processing) threads.
    std::mutex frame_mutex;
    /// Whether liveness is enabled for the reactor
    bool is_livenessEnabled = false;
    /// Whether to produce verbose output from the reactor
    bool verbose = false;
    /// A flag determining whether the stream is actively running.
    std::atomic<bool> is_running;

 public:
    /// @brief Initialize a reactor for streaming video from an OpenCV stream.
    ///
    /// @param is_livenessEnabled_ `true` to enable the liveness check interface,
    /// `false` to disable the interface.
    ///
    OpenCVReactor(
        const bool& is_livenessEnabled_ = false,
        const bool& verbose_ = false
    ) :
        VideoService<FileSystemCredentialStore>::AuthenticateBidiReactor(),
        is_authenticated(false),
        score(100),
        is_live(false),
        is_livenessEnabled(is_livenessEnabled_),
        verbose(verbose_),
        is_running(true) { }

    /// @brief Return true if the user successfully authenticated
    inline bool get_is_authenticated() const { return is_authenticated; }

    /// @brief React to a _write done_ event.
    ///
    /// @param ok whether the write succeeded.
    ///
    void OnWriteDone(bool ok) override {
        if (is_authenticated) {  // Successfully authenticated! Close the stream.
            StartWritesDone();
            return;
        }
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        std::vector<unsigned char> buffer;
        {  // Lock the mutex and encode the frame with JPEG into a buffer.
            std::lock_guard<std::mutex> lock(frame_mutex);
            if (frame.empty()) {
                is_running = false;
                StartWritesDone();
                return;
            }
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
        if (is_authenticated) return;
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        // Log information about the response to the terminal.
        if (verbose) {
            std::cout << "Frame Response:" << std::endl;
            std::cout << "\tSuccess: "  << response.success() << std::endl;
            std::cout << "\tScore: "    << response.score() << std::endl;
            std::cout << "\tIs Alive: " << response.isalive() << std::endl;
        }
        // Set the authentication flag to the success of the response.
        is_authenticated = response.success();
        if (is_livenessEnabled)
            is_authenticated = is_authenticated && response.isalive();
        score = response.score();
        is_live = response.isalive();
        if (!is_running) {
            OnDone({});
            return;
        }
        if (!is_authenticated)  // Start the next read request
            StartRead(&response);
    }

    /// @brief Stream video from an OpenCV capture device.
    ///
    /// @param capture The OpenCV capture device.
    ///
    ::grpc::Status stream_video(cv::VideoCapture& capture) {
        // Start the call to initiate the stream in the background.
        StartCall();
        // Start capturing frames from the device.
        while (!is_authenticated) {
            {  // Lock the mutex and read a frame.
                std::lock_guard<std::mutex> lock(frame_mutex);
                capture >> frame;
            }
            // If the frame is empty, something went wrong, exit the capture
            // loop.
            if (frame.empty()) break;
            // Draw text indicating the liveness status of the last frame.
            auto presentation_frame = frame.clone();
            if (is_livenessEnabled) {  // Liveness is enabled.
                cv::putText(presentation_frame,
                    is_live ? "Live" : "Not Live",
                    cv::Point(10, 40),
                    cv::FONT_HERSHEY_SIMPLEX,
                    1,  // font scale
                    is_live ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255),
                    2   // thickness
                );
            }
            // Show the frame in a view-finder window.
            cv::imshow("SensoryCloud Face Authentication Demo", presentation_frame);
            // Listen for keyboard interrupts to terminate the capture.
            char c = (char) cv::waitKey(10);
            if (c == 27 || c == 'q' || c == 'Q') break;
        }
        return await();
    }
};

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
    const auto VERBOSE = args.get<bool>("verbose");

    // Create a credential store for keeping OAuth credentials in.
    FileSystemCredentialStore keychain(".", "com.sensory.cloud.examples");

    // Create the cloud services handle.
    SensoryCloud<FileSystemCredentialStore> cloud(PATH, keychain);

    // ------ Check server health ----------------------------------------------

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

    // ------ Get an enrollment ID ---------------------------------------------

    if (USER_ID != "") {
        sensory::api::v1::management::GetEnrollmentsResponse enrollment_response;
        auto status = cloud.management.get_enrollments(&enrollment_response, USER_ID);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get enrollments ("
                << status.error_code() << "): "
                << status.error_message() << std::endl;
            return 1;
        }
        for (auto& enrollment : enrollment_response.enrollments()) {
            if (enrollment.modeltype() != sensory::api::common::FACE_BIOMETRIC)
                continue;
            std::cout << "Description:     " << enrollment.description()  << std::endl;
            std::cout << "\tModel Name:    " << enrollment.modelname()    << std::endl;
            std::cout << "\tModel Type:    " << enrollment.modeltype()    << std::endl;
            std::cout << "\tModel Version: " << enrollment.modelversion() << std::endl;
            std::cout << "\tUser ID:       " << enrollment.userid()       << std::endl;
            std::cout << "\tDevice ID:     " << enrollment.deviceid()     << std::endl;
            std::cout << "\tCreated:       "
                << google::protobuf::util::TimeUtil::ToString(enrollment.createdat())
                << std::endl;
            std::cout << "\tUpdated:       "
                << google::protobuf::util::TimeUtil::ToString(enrollment.updatedat())
                << std::endl;
            std::cout << "\tID:            " << enrollment.id()           << std::endl;
            std::cout << "\tReference ID:  " << enrollment.referenceid()  << std::endl;
        }
    }

    // Create an image capture object
    cv::VideoCapture capture;
    const bool IS_DEVICE_NUMERIC = !DEVICE.empty() && DEVICE.find_first_not_of("0123456789") == std::string::npos;
    if (!(IS_DEVICE_NUMERIC ? capture.open(stoi(DEVICE)) : capture.open(DEVICE))) {
        std::cout << "Capture from device " << DEVICE << " failed" << std::endl;
        return 1;
    }

    // Create the config with the authentication parameters.
    auto config = new ::sensory::api::v1::video::AuthenticateConfig;
    if (GROUP)
        config->set_enrollmentgroupid(ENROLLMENT_ID);
    else
        config->set_enrollmentid(ENROLLMENT_ID);
    config->set_islivenessenabled(LIVENESS);
    config->set_livenessthreshold(THRESHOLD);
    // Initialize the stream with the cloud.
    OpenCVReactor reactor(LIVENESS, VERBOSE);
    cloud.video.authenticate(&reactor, config);
    // Wait for the stream to conclude. This is necessary to check the final
    // status of the call and allow any dynamically allocated data to be cleaned
    // up. If the stream is destroyed before the final `onDone` callback, odd
    // runtime errors can occur.
    status = reactor.stream_video(capture);

    if (!status.ok()) {
        std::cout << "Failed to authenticate ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    } else if (reactor.get_is_authenticated()) {
        std::cout << "Successfully authenticated!" << std::endl;
    } else {
        std::cout << "Failed to authenticate!" << std::endl;
    }
    return 0;
}
