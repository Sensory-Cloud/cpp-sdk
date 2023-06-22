// An example of audio enrolled event validation based on file inputs.
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
#include <regex>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include <sndfile.h>
#include "../dep/argparse.hpp"
#include "../dep/tqdm.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::service::AudioService;
using sensory::api::v1::audio::ThresholdSensitivity;

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("validate_enrolled_event")
        .description("A tool for streaming audio files to Sensory Cloud for audio transcription.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-i", "--input" }).required(true)
        .help("The input audio file to stream to Sensory Cloud.");
    parser.add_argument({ "-m", "--model" })
        .help("The model to use for the enrollment.");
    parser.add_argument({ "-u", "--userid" })
        .help("The name of the user ID to query the enrollments for.");
    parser.add_argument({ "-e", "--enrollmentid" })
        .help("The ID of the enrollment to authenticate against.");
    parser.add_argument({ "-s", "--sensitivity" })
        .choices({"LOW", "MEDIUM", "HIGH", "HIGHEST"})
        .default_value("HIGH")
        .help("The audio sensitivity level of the model.");
    parser.add_argument({ "-g", "--group" })
        .action("store_true")
        .help("A flag determining whether the enrollment ID is for an enrollment group.");
    parser.add_argument({ "-C", "--chunksize" })
        .help("The number of audio samples per message; 0 to stream all samples in one message (default).")
        .default_value(4096);
    parser.add_argument({ "-v", "--verbose" }).action("store_true")
        .help("Produce verbose output during transcription.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto INPUT_FILE = args.get<std::string>("input");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    const auto ENROLLMENT_ID = args.get<std::string>("enrollmentid");
    ThresholdSensitivity SENSITIVITY;
    if (args.get<std::string>("sensitivity") == "LOW")
        SENSITIVITY = ThresholdSensitivity::LOW;
    else if (args.get<std::string>("sensitivity") == "MEDIUM")
        SENSITIVITY = ThresholdSensitivity::MEDIUM;
    else if (args.get<std::string>("sensitivity") == "HIGH")
        SENSITIVITY = ThresholdSensitivity::HIGH;
    else if (args.get<std::string>("sensitivity") == "HIGHEST")
        SENSITIVITY = ThresholdSensitivity::HIGHEST;
    const auto GROUP = args.get<bool>("group");
    auto CHUNK_SIZE = args.get<int>("chunksize");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create a credential store for keeping OAuth credentials in.
    sensory::token_manager::FileSystemCredentialStore keychain(".", "com.sensory.cloud.examples");

    // Create the cloud services handle.
    SensoryCloud<FileSystemCredentialStore> cloud(PATH, keychain);

    // Check the server health.
    sensory::api::common::ServerHealthResponse server_health;
    auto status = cloud.health.get_health(&server_health);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health (" << status.error_code() << "): " << status.error_message() << std::endl;
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

    // Query this user's active enrollments
    if (USER_ID != "") {
        sensory::api::v1::management::GetEnrollmentsResponse enrollment_response;
        status = cloud.management.get_enrollments(&enrollment_response, USER_ID);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get enrollments ("
                << status.error_code() << ": "
                << status.error_message() << std::endl;
            return 1;
        }
        for (auto& enrollment : enrollment_response.enrollments()) {
            if (enrollment.modeltype() != sensory::api::common::SOUND_EVENT_ENROLLABLE)
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

    // Initialize the client.
    sensory::api::v1::management::DeviceResponse response;
    status = cloud.initialize(&response);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to initialize (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }

    // Try to load the audio file.
    SNDFILE* infile = nullptr;
    SF_INFO sfinfo;
    if ((infile = sf_open(INPUT_FILE.c_str(), SFM_READ, &sfinfo)) == NULL) {
        std::cout << "Failed to open file " << INPUT_FILE << std::endl;
        return 1;
    }

    // Check that the audio is 16kHz.
    if (sfinfo.samplerate != 16000) {
        std::cout << "Attempting to load file with sample rate of "
            << sfinfo.samplerate << "Hz, but only 16000Hz audio is supported."
            << std::endl;
        return 1;
    }
    // Check that the audio is monophonic.
    if (sfinfo.channels > 1) {
        std::cout << "Attempting to load file with "
            << sfinfo.channels << " channels, but only mono audio is supported."
            << std::endl;
        return 1;
    }

    // Create an audio config that describes the format of the audio stream.
    auto audio_config = new sensory::api::v1::audio::AudioConfig;
    audio_config->set_encoding(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
    audio_config->set_sampleratehertz(sfinfo.samplerate);
    audio_config->set_audiochannelcount(sfinfo.channels);
    audio_config->set_languagecode("en");
    // Create the config with the enrolled event validation parameters.
    auto validate_enrolled_event_config = new ::sensory::api::v1::audio::ValidateEnrolledEventConfig;
    if (GROUP)
        validate_enrolled_event_config->set_enrollmentgroupid(ENROLLMENT_ID);
    else
        validate_enrolled_event_config->set_enrollmentid(ENROLLMENT_ID);
    validate_enrolled_event_config->set_sensitivity(SENSITIVITY);

    grpc::ClientContext context;
    auto stream = cloud.audio.validate_enrolled_event(&context, audio_config, validate_enrolled_event_config);

    // Create a background thread for handling the transcription responses.
    std::thread receipt_thread([&stream, &VERBOSE](){
        while (true) {
            // Read a message and break out of the loop when the read fails.
            sensory::api::v1::audio::ValidateEnrolledEventResponse response;
            if (!stream->Read(&response)) break;
            if (VERBOSE) {  // Verbose output, dump the message to the terminal
                    google::protobuf::util::JsonPrintOptions options;
                    options.add_whitespace = false;
                    options.always_print_primitive_fields = true;
                    options.always_print_enums_as_ints = false;
                    options.preserve_proto_field_names = true;
                    std::string response_json;
                    google::protobuf::util::MessageToJsonString(response, &response_json, options);
                    std::cout << response_json << std::endl;
            } else if (response.success()) {  // detected event
                std::cout << "Detected event!" << std::endl;
            }
        }
    });

    // If the chunk size is zero, disable chunking by setting the chunk size
    // to be equal to the number of samples.
    if (CHUNK_SIZE <= 0) CHUNK_SIZE = sfinfo.frames;

    auto num_chunks = sfinfo.frames / CHUNK_SIZE + (bool)(sfinfo.frames % CHUNK_SIZE);
    tqdm progress(num_chunks);
    int16_t samples[CHUNK_SIZE];
    int num_frames;
    for (int i = 0; i < num_chunks; i++) {
        auto num_frames = sf_read_short(infile, &samples[0], CHUNK_SIZE);
        sensory::api::v1::audio::ValidateEnrolledEventRequest request;
        request.set_audiocontent((uint8_t*) samples, sizeof(int16_t) * num_frames);
        if (!stream->Write(request)) break;
        progress();
    }
    stream->WritesDone();
    sf_close(infile);
    receipt_thread.join();

    // Close the stream and check the status code in case the stream broke.
    status = stream->Finish();
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "stream broke ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
