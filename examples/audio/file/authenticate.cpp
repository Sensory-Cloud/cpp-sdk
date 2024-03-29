// An example of audio authentication based on file inputs.
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
#include <atomic>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include <sndfile.h>
#include "../dep/argparse.hpp"
#include "../dep/tqdm.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::TokenManager;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::service::HealthService;
using sensory::service::ManagementService;
using sensory::service::AudioService;
using sensory::service::OAuthService;
using sensory::api::v1::audio::ThresholdSensitivity;

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("authenticate")
        .description("A tool for streaming audio files to Sensory Cloud for audio transcription.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-i", "--input" }).required(true)
        .help("The input audio file to stream to Sensory Cloud.");
    parser.add_argument({ "-u", "--userid" })
        .help("The name of the user ID to query the enrollments for.");
    parser.add_argument({ "-e", "--enrollmentid" })
        .help("The ID of the enrollment to authenticate against.");
    parser.add_argument({ "-l", "--liveness" })
        .action("store_true")
        .help("Whether to conduct a liveness check in addition to the enrollment.");
    parser.add_argument({ "-s", "--sensitivity" })
        .choices({"LOW", "MEDIUM", "HIGH", "HIGHEST"})
        .default_value("HIGH")
        .help("The audio sensitivity level of the model.");
    parser.add_argument({ "-t", "--threshold" })
        .choices(std::vector<std::string>{"LOW", "HIGH"})
        .default_value("HIGH")
        .help("The security threshold for the authentication.");
    parser.add_argument({ "-g", "--group" })
        .action("store_true")
        .help("A flag determining whether the enrollment ID is for an enrollment group.");
    parser.add_argument({ "-T", "--token" })
        .help("A path to the binary feature vector if the server is not to store enrollments.");
    parser.add_argument({ "-L", "--language" }).required(true)
        .help("The IETF BCP 47 language tag for the input audio (e.g., en-US).");
    parser.add_argument({ "-C", "--chunksize" })
        .help("The number of audio samples per message; 0 to stream all samples in one message (default).")
        .default_value(4096);
    parser.add_argument({ "-p", "--padding" })
        .help("The number of milliseconds of padding to append to the audio buffer.")
        .default_value(300);
    parser.add_argument({ "-v", "--verbose" }).action("store_true")
        .help("Produce verbose output during transcription.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto INPUT_FILE = args.get<std::string>("input");
    const auto USER_ID = args.get<std::string>("userid");
    const auto ENROLLMENT_ID = args.get<std::string>("enrollmentid");
    const auto LIVENESS = args.get<bool>("liveness");
    ThresholdSensitivity SENSITIVITY;
    if (args.get<std::string>("sensitivity") == "LOW")
        SENSITIVITY = ThresholdSensitivity::LOW;
    else if (args.get<std::string>("sensitivity") == "MEDIUM")
        SENSITIVITY = ThresholdSensitivity::MEDIUM;
    else if (args.get<std::string>("sensitivity") == "HIGH")
        SENSITIVITY = ThresholdSensitivity::HIGH;
    else if (args.get<std::string>("sensitivity") == "HIGHEST")
        SENSITIVITY = ThresholdSensitivity::HIGHEST;
    sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity THRESHOLD;
    if (args.get<std::string>("threshold") == "LOW")
        THRESHOLD = sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW;
    else if (args.get<std::string>("threshold") == "HIGH")
        THRESHOLD = sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_HIGH;
    const auto GROUP = args.get<bool>("group");
    const auto TOKEN_FILE = args.get<std::string>("token");
    const auto LANGUAGE = args.get<std::string>("language");
    auto CHUNK_SIZE = args.get<int>("chunksize");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create a credential store for keeping OAuth credentials in.
    FileSystemCredentialStore keychain(".", "com.sensory.cloud.examples");

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

    // Initialize the client.
    sensory::api::v1::management::DeviceResponse response;
    status = cloud.initialize(&response);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to initialize (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }

    // Query this user's active enrollments
    if (USER_ID != "") {
        sensory::api::v1::management::GetEnrollmentsResponse enrollmentResponse;
        status = cloud.management.get_enrollments(&enrollmentResponse, USER_ID);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get enrollments with\n\t" <<
                status.error_code() << ": " << status.error_message() << std::endl;
            return 1;
        }
        for (auto& enrollment : enrollmentResponse.enrollments()) {
            if (enrollment.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_DEPENDENT &&
                enrollment.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_INDEPENDENT &&
                enrollment.modeltype() != sensory::api::common::VOICE_BIOMETRIC_WAKEWORD &&
                enrollment.modeltype() != sensory::api::common::SOUND_EVENT_ENROLLABLE
            ) continue;
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
    audio_config->set_languagecode(LANGUAGE);
    // Create the config with the authentication parameters.
    auto authenticate_config = new ::sensory::api::v1::audio::AuthenticateConfig;
    if (GROUP)
        authenticate_config->set_enrollmentgroupid(ENROLLMENT_ID);
    else
        authenticate_config->set_enrollmentid(ENROLLMENT_ID);
    authenticate_config->set_islivenessenabled(LIVENESS);
    authenticate_config->set_sensitivity(SENSITIVITY);
    authenticate_config->set_security(THRESHOLD);
    char* buffer = nullptr;
    if (!TOKEN_FILE.empty()) {
        std::ifstream file(TOKEN_FILE, std::ios::in | std::ios::binary);
        if (!file) {
            std::cout << "Failed to read enrollment token file at " << TOKEN_FILE << std::endl;
            return 1;
        }
        // Get the length of the file
        file.seekg(0, file.end);
        std::streampos length = file.tellg();
        file.seekg(0, file.beg);
        if (length == 0) {
            std::cout << "enrollment token file at " << TOKEN_FILE << " is empty!" << std::endl;
            return 1;
        }
        // Allocate a buffer to store the contents of the file
        buffer = (char*) malloc(length * sizeof(char));
        // Read the contents of the file into the buffer
        file.read(buffer, length);
        file.close();
        // Copy the buffer into the authenticate config
        authenticate_config->set_enrollmenttoken(buffer, length);
    }

    grpc::ClientContext context;
    auto stream = cloud.audio.authenticate(&context, audio_config, authenticate_config);

    // If the chunk size is zero, disable chunking by setting the chunk size
    // to be equal to the number of samples.
    if (CHUNK_SIZE <= 0) CHUNK_SIZE = sfinfo.frames;

    auto num_chunks = sfinfo.frames / CHUNK_SIZE + (bool)(sfinfo.frames % CHUNK_SIZE);
    tqdm progress(num_chunks);
    int16_t samples[CHUNK_SIZE];
    std::atomic<bool> is_authenticated(false);

    // Create a background thread for handling the transcription responses.
    std::thread receipt_thread([&](){
        while (true) {
            // Read a message and break out of the loop when the read fails.
            sensory::api::v1::audio::AuthenticateResponse response;
            if (!stream->Read(&response)) break;
            is_authenticated = response.success();
            // Log the result of the request to the terminal.
            if (VERBOSE) {  // Verbose output, dump the message to the terminal
                google::protobuf::util::JsonPrintOptions options;
                options.add_whitespace = false;
                options.always_print_primitive_fields = true;
                options.always_print_enums_as_ints = false;
                options.preserve_proto_field_names = true;
                std::string response_json;
                google::protobuf::util::MessageToJsonString(response, &response_json, options);
                std::cout << response_json << std::endl;
            } else {  // Friendly output, use a progress bar and display the prompt
                progress.set_postfix("authentication progress: " + std::to_string(response.percentsegmentcomplete()) + "%");
            }
        }
    });

    int num_frames;
    for (int i = 0; i < num_chunks; i++) {
        auto num_frames = sf_read_short(infile, &samples[0], CHUNK_SIZE);
        sensory::api::v1::audio::AuthenticateRequest request;
        request.set_audiocontent((uint8_t*) samples, sizeof(int16_t) * num_frames);
        if (!stream->Write(request)) break;
        if (!VERBOSE) progress();
    }
    stream->WritesDone();
    sf_close(infile);
    receipt_thread.join();
    if (!TOKEN_FILE.empty()) free(buffer);
    // Finish the progress bar according to the authentication status
    progress.set_postfix(is_authenticated ? "authentication success" : "authentication failure");
    if (!VERBOSE) progress.complete();

    // Close the stream and check the status code in case the stream broke.
    status = stream->Finish();
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "stream broke with ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
