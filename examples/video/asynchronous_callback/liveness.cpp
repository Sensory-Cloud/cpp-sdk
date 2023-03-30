// Face liveness verification using SensoryCloud with OpenCV.
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
using sensory::service::VideoService;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::api::v1::video::RecognitionThreshold;

// The thickness of the face boxes to render
const auto BOX_THICKNESS = 5;
// The thickness of the font to render
const auto FONT_THICKNESS = 2;
// The scale of the font to render
const auto FONT_SCALE = 0.9;

/// @brief A bidirectional stream reactor for biometric liveness validation from
/// video stream data.
///
/// @details
/// Input data for the stream is provided by an OpenCV capture device.
///
class FaceLivenessReactor :
    public VideoService<FileSystemCredentialStore>::ValidateLivenessBidiReactor {
 private:
    /// An OpenCV matrix containing the frame data from the camera.
    cv::Mat frame;
    /// A mutual exclusion for locking access to the frame between foreground
    /// (frame capture) and background (network stream processing) threads.
    std::mutex frame_mutex;
    /// The codec to use when compressing images.
    std::string codec = ".jpg";
    /// Whether to produce verbose output in the reactor
    bool verbose = false;
    /// A flag determining whether the stream is actively running.
    std::atomic<bool> is_running;
    /// A flag determining whether the last sent contained a face.
    std::atomic<bool> did_find_face;
    /// Bounding box coordinates for the face that was detected
    std::atomic<float> xmin, ymin, xmax, ymax;
    /// A flag determining whether the last sent frame was detected as live.
    std::atomic<bool> is_live;

 public:
    /// @brief Initialize a reactor for streaming video from an OpenCV stream.
    ///
    /// @param codec_ The codec to use when compressing images.
    /// @param verbose_ True to enable verbose logging
    ///
    FaceLivenessReactor(
        const std::string& codec_ = ".jpg",
        const bool& verbose_ = false
    ) :
        VideoService<FileSystemCredentialStore>::ValidateLivenessBidiReactor(),
        codec(codec_),
        verbose(verbose_),
        is_running(true),
        did_find_face(false),
        xmin(0), ymin(0), xmax(0), ymax(0),
        is_live(false) { }

    /// @brief React to a _write done_ event.
    ///
    /// @param ok whether the write succeeded.
    ///
    void OnWriteDone(bool ok) override {
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        std::vector<unsigned char> buffer;
        {  // Lock the mutex and encode the frame into a buffer.
            std::lock_guard<std::mutex> lock(frame_mutex);
            // If the frame is empty, something went wrong, exit the loop.
            if (frame.empty()) {
                is_running = false;
                StartWritesDone();
                return;
            }
            cv::imencode(codec, frame, buffer);
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
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        did_find_face = response.didfindface();
        xmin = response.boundingbox()[0];
        ymin = response.boundingbox()[1];
        xmax = response.boundingbox()[2];
        ymax = response.boundingbox()[3];
        is_live = response.isalive();
        // Log information about the response to the terminal.
        if (verbose) {
            google::protobuf::util::JsonPrintOptions options;
            options.add_whitespace = false;
            options.always_print_primitive_fields = true;
            options.always_print_enums_as_ints = false;
            options.preserve_proto_field_names = true;
            std::string response_json;
            google::protobuf::util::MessageToJsonString(response, &response_json, options);
            std::cout << response_json << std::endl;
        }
        if (is_running)
            StartRead(&response);
        else
            OnDone({});
    }

    /// @brief Stream video from an OpenCV capture device.
    ///
    /// @param capture The OpenCV capture device.
    ///
    ::grpc::Status stream_video(cv::VideoCapture& capture) {
        // Start the call to initiate the stream in the background.
        StartCall();
        // Start capturing frames from the device.
        while (true) {
            {  // Lock the mutex and read a frame.
                std::lock_guard<std::mutex> lock(frame_mutex);
                capture >> frame;
            }
            // If the frame is empty, something went wrong, exit the capture
            // loop.
            if (frame.empty()) break;
            auto presentation_frame = frame.clone();
            if (did_find_face) {
                const auto box_color = is_live ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
                const auto label = is_live ? "Live" : "Spoof";
                // Draw the face bounding box
                cv::rectangle(presentation_frame,
                    cv::Point(xmin, ymin),
                    cv::Point(xmax, ymax),
                    box_color,
                    BOX_THICKNESS);
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
            // Show the frame in a view-finder window.
            cv::imshow("SensoryCloud Face Liveness Demo", presentation_frame);
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
        .prog("liveness")
        .description("A tool for validating face liveness using SensoryCloud.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-g", "--getmodels" })
        .action("store_true")
        .help("Whether to query for a list of available models.");
    parser.add_argument({ "-m", "--model" })
        .help("The model to use for the enrollment.");
    parser.add_argument({ "-u", "--userid" })
        .help("The name of the user ID to query the enrollments for.");
    parser.add_argument({ "-t", "--threshold" })
        .choices({"LOW", "MEDIUM", "HIGH", "HIGHEST"})
        .default_value("HIGH")
        .help("The security threshold for conducting the liveness check.");
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
    const auto GETMODELS = args.get<bool>("getmodels");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    RecognitionThreshold THRESHOLD;
    if (args.get<std::string>("threshold") == "LOW")
        THRESHOLD = RecognitionThreshold::LOW;
    else if (args.get<std::string>("threshold") == "MEDIUM")
        THRESHOLD = RecognitionThreshold::MEDIUM;
    else if (args.get<std::string>("threshold") == "HIGH")
        THRESHOLD = RecognitionThreshold::HIGH;
    else if (args.get<std::string>("threshold") == "HIGHEST")
        THRESHOLD = RecognitionThreshold::HIGHEST;
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

    // ------ Query the available video models ---------------------------------

    if (GETMODELS) {
        int error_code = 0;
        cloud.video.get_models([&error_code](VideoService<FileSystemCredentialStore>::GetModelsCallbackData* call) {
            if (!call->getStatus().ok()) {  // The call failed.
                std::cout << "Failed to get video models ("
                    << call->getStatus().error_code() << "): "
                    << call->getStatus().error_message() << std::endl;
                error_code = 1;
            } else {
                // Iterate over the models returned in the response
                for (auto& model : call->getResponse().models()) {
                    // Ignore models that aren't face biometric models.
                    if (model.modeltype() != sensory::api::common::FACE_RECOGNITION)
                        continue;
                    google::protobuf::util::JsonPrintOptions options;
                    options.add_whitespace = true;
                    options.always_print_primitive_fields = true;
                    options.always_print_enums_as_ints = false;
                    options.preserve_proto_field_names = true;
                    std::string model_json;
                    google::protobuf::util::MessageToJsonString(model, &model_json, options);
                    std::cout << model_json << std::endl;
                }
            }
        })->await();
        return error_code;
    }

    // Create an image capture object
    cv::VideoCapture capture;
    const bool IS_DEVICE_NUMERIC = !DEVICE.empty() && DEVICE.find_first_not_of("0123456789") == std::string::npos;
    if (!(IS_DEVICE_NUMERIC ? capture.open(stoi(DEVICE)) : capture.open(DEVICE))) {
        std::cout << "Capture from device " << DEVICE << " failed" << std::endl;
        return 1;
    }

    // Create the config with the recognition parameters.
    auto config = new ::sensory::api::v1::video::ValidateRecognitionConfig;
    config->set_modelname(MODEL);
    config->set_userid(USER_ID);
    config->set_threshold(THRESHOLD);
    // Initialize the stream with the cloud.
    FaceLivenessReactor reactor(CODEC, VERBOSE);
    cloud.video.validate_liveness(&reactor, config);
    // Wait for the stream to conclude. This is necessary to check the final
    // status of the call and allow any dynamically allocated data to be cleaned
    // up. If the stream is destroyed before the final `onDone` callback, odd
    // runtime errors can occur.
    status = reactor.stream_video(capture);

    if (!status.ok()) {
        std::cout << "Failed to validate liveness ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
