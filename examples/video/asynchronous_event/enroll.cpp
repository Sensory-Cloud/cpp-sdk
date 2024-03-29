// Face enrollment using SensoryCloud with OpenCV.
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
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/health_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>
#include <sensorycloud/services/video_service.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>
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

    // Start an asynchronous RPC to fetch the names of the available models. The
    // RPC will use the grpc::CompletionQueue as an event loop.
    grpc::CompletionQueue queue;

    if (GETMODELS) {
        auto get_models_rpc = cloud.video.get_models(&queue);
        // Execute the asynchronous RPC in this thread (which will block like a
        // synchronous call).
        void* tag(nullptr);
        bool ok(false);
        queue.Next(&tag, &ok);
        int error_code = 0;
        if (ok && tag == get_models_rpc) {
            if (!get_models_rpc->getStatus().ok()) {  // the call failed, print a descriptive message
                std::cout << "Failed to get video models ("
                    << status.error_code() << "): "
                    << status.error_message() << std::endl;
                error_code = 1;
            } else {
                for (auto& model : get_models_rpc->getResponse().models()) {
                    if (model.modeltype() != sensory::api::common::FACE_BIOMETRIC)
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
        }
        delete get_models_rpc;
        return error_code;
    }

    // ------ Create a new video enrollment ------------------------------------

    // Create an image capture object
    cv::VideoCapture capture;
    const bool IS_DEVICE_NUMERIC = !DEVICE.empty() && DEVICE.find_first_not_of("0123456789") == std::string::npos;
    if (!(IS_DEVICE_NUMERIC ? capture.open(stoi(DEVICE)) : capture.open(DEVICE))) {
        std::cout << "Capture from device " << DEVICE << " failed" << std::endl;
        return 1;
    }

    // Here we use atomic variables to simplify the passage of data between
    // networking and UI contexts.
    std::atomic<bool> did_find_face(false), is_live(false), is_enrolled(false);
    std::atomic<float> xmin(0), ymin(0), xmax(0), ymax(0);
    std::atomic<float> percent_complete(0);
    // An OpenCV matrix containing the frame data from the camera.
    cv::Mat frame;
    // A mutual exclusion for locking access to the frame between foreground
    // (frame capture) and background (network stream processing) threads.
    std::mutex frame_mutex;

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
    auto stream = cloud.video.create_enrollment(&queue, config, nullptr, (void*) Events::Finish);

    // start the stream event thread in the background to handle events.
    std::thread event_thread([&](){
        void* tag(nullptr);
        bool ok(false);
        bool is_running(true);
        while (queue.Next(&tag, &ok)) {
            if (!ok) continue;
            if (tag == stream) {
                // Respond to the start of stream succeeding. All SensoryCloud
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
                if (is_enrolled) {
                    stream->getCall()->WritesDone((void*) Events::WritesDone);
                    continue;
                }
                std::vector<unsigned char> buffer;
                {  // Lock the mutex and encode the frame into a buffer.
                    std::lock_guard<std::mutex> lock(frame_mutex);
                    // If the frame is empty, something went wrong, shutdown the loop.
                    if (frame.empty()) {
                        is_running = false;
                        continue;
                    }
                    cv::imencode(CODEC, frame, buffer);
                }
                // Create the request from the encoded image data.
                sensory::api::v1::video::CreateEnrollmentRequest request;
                request.set_imagecontent(buffer.data(), buffer.size());
                stream->getCall()->Write(request, (void*) Events::Write);
            } else if (tag == (void*) Events::Read) {  // Respond to a read event.
                // Set the local instance data based on the last frame.
                did_find_face = stream->getResponse().didfindface();
                xmin = stream->getResponse().boundingbox()[0];
                ymin = stream->getResponse().boundingbox()[1];
                xmax = stream->getResponse().boundingbox()[2];
                ymax = stream->getResponse().boundingbox()[3];
                is_enrolled = !stream->getResponse().enrollmentid().empty();
                percent_complete = stream->getResponse().percentcomplete() / 100.f;
                is_live = stream->getResponse().isalive();
                // Log information about the response to the terminal.
                if (VERBOSE) {
                    google::protobuf::util::JsonPrintOptions options;
                    options.add_whitespace = false;
                    options.always_print_primitive_fields = true;
                    options.always_print_enums_as_ints = false;
                    options.preserve_proto_field_names = true;
                    std::string response_json;
                    google::protobuf::util::MessageToJsonString(stream->getResponse(), &response_json, options);
                    std::cout << response_json << std::endl;
                }
                // If the write thread has encountered an error, stop reading and exit.
                if (!is_running) break;
                // If we're finished enrolling, don't issue a new read request.
                if (!is_enrolled)
                    stream->getCall()->Read(&stream->getResponse(), (void*) Events::Read);
                else
                    std::cout << "Enrolled with ID: " << stream->getResponse().enrollmentid() << std::endl;
            } else if (tag == (void*) Events::Finish) break;
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
        cv::imshow("SensoryCloud Face Enrollment Demo", presentation_frame);
        // Listen for keyboard interrupts to terminate the capture.
        char c = (char) cv::waitKey(10);
        if (c == 27 || c == 'q' || c == 'Q') break;
    }

    // Wait for the network thread to join back in.
    event_thread.join();

    if (!stream->getStatus().ok()) {
        std::cout << "Failed to create enrollment ("
            << stream->getStatus().error_code() << "): "
            << stream->getStatus().error_message() << std::endl;
        return 1;
    }

    delete stream;

    return 0;
}
