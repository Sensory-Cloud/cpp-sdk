// An example of enrolled audio event validation using SensoryCloud with PortAudio.
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

#include <portaudio.h>
#include <iostream>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include "../dep/argparse.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::api::v1::audio::ThresholdSensitivity;

/// @brief Print a description of a PortAudio error that occurred.
///
/// @param err The error that was thrown by PortAudio.
/// @returns The OS-level error code for the message.
///
int describe_pa_error(const PaError& err) {
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    return err;
}

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("validate_enrolled_event")
        .description("A tool for validating enrolled events using SensoryCloud.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
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
    // parser.add_argument({ "-C", "--chunksize" })
    //     .help("The number of audio samples per message (default 4096).")
    //     .default_value("4096");
    // parser.add_argument({ "-S", "--samplerate" })
    //     .help("The audio sample rate of the input stream.")
    //     .choices({"9600", "11025", "12000", "16000", "22050", "24000", "32000", "44100", "48000", "88200", "96000", "192000"})
    //     .default_value("16000");
    parser.add_argument({ "-v", "--verbose" })
        .action("store_true")
        .help("Produce verbose output during authentication.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
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
    sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity THRESHOLD;
    const auto GROUP = args.get<bool>("group");
    const uint32_t CHUNK_SIZE = 4096;//args.get<int>("chunksize");
    const auto SAMPLE_RATE = 16000;//args.get<uint32_t>("samplerate");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create a credential store for keeping OAuth credentials in.
    FileSystemCredentialStore keychain(".", "com.sensory.cloud.examples");
    // Create the cloud services handle.
    SensoryCloud<FileSystemCredentialStore> cloud(PATH, keychain);

    // Query the health of the remote service.
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

    // the maximal duration of the recording in seconds
    const auto DURATION = 60;
    // The number of input channels from the microphone. This should always be
    // mono.
    const auto NUM_CHANNELS = 1;
    // The number of bytes per sample, for 16-bit audio, this is 2 bytes.
    const auto SAMPLE_SIZE = 2;
    // The number of bytes in a given chunk of samples.
    const auto BYTES_PER_BLOCK = CHUNK_SIZE * NUM_CHANNELS * SAMPLE_SIZE;

    // Create an audio config that describes the format of the audio stream.
    auto audio_config = new sensory::api::v1::audio::AudioConfig;
    audio_config->set_encoding(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
    audio_config->set_sampleratehertz(SAMPLE_RATE);
    audio_config->set_audiochannelcount(NUM_CHANNELS);
    audio_config->set_languagecode("en");
    // Create the config with the enrolled event validation parameters.
    auto validate_enrolled_event_config = new ::sensory::api::v1::audio::ValidateEnrolledEventConfig;
    if (GROUP)
        validate_enrolled_event_config->set_enrollmentgroupid(ENROLLMENT_ID);
    else
        validate_enrolled_event_config->set_enrollmentid(ENROLLMENT_ID);
    validate_enrolled_event_config->set_sensitivity(SENSITIVITY);
    // Initialize the stream with the cloud.
    grpc::ClientContext context;
    auto stream = cloud.audio.validate_enrolled_event(&context, audio_config, validate_enrolled_event_config);

    // Initialize the PortAudio driver.
    PaError err = paNoError;
    err = Pa_Initialize();
    if (err != paNoError) return describe_pa_error(err);

    // Setup the input parameters for the port audio stream.
    PaStreamParameters input_parameters;
    input_parameters.device = Pa_GetDefaultInputDevice();
    if (input_parameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        return 1;
    }
    input_parameters.channelCount = 1;
    input_parameters.sampleFormat = paInt16;  // Sensory expects 16-bit audio
    input_parameters.suggestedLatency =
        Pa_GetDeviceInfo(input_parameters.device)->defaultHighInputLatency;
    input_parameters.hostApiSpecificStreamInfo = NULL;

    // Open the PortAudio stream with the input device.
    PaStream* audioStream;
    err = Pa_OpenStream(&audioStream,
        &input_parameters,
        NULL,       // no output parameters for an input stream
        SAMPLE_RATE,
        CHUNK_SIZE,
        paClipOff,  // we won't output out-of-range samples so don't clip them
        NULL,       // using the blocking interface (no callback)
        NULL        // no data for the callback since there is none
    );
    if (err != paNoError) return describe_pa_error(err);

    // Start the audio input stream.
    err = Pa_StartStream(audioStream);
    if (err != paNoError) return describe_pa_error(err);

    // Create a buffer for the audio samples based on the number of bytes in
    // a block of samples.
    uint8_t sample_block[BYTES_PER_BLOCK];
    for (int i = 0; i < (DURATION * SAMPLE_RATE) / CHUNK_SIZE; ++i) {
        // Read a block of samples from the ADC.
        err = Pa_ReadStream(audioStream, sample_block, CHUNK_SIZE);
        if (err) return describe_pa_error(err);

        // Create a new request with the audio content.
        sensory::api::v1::audio::ValidateEnrolledEventRequest request;
        request.set_audiocontent(sample_block, BYTES_PER_BLOCK);
        if (!stream->Write(request)) break;

        // Read a new response from the server.
        sensory::api::v1::audio::ValidateEnrolledEventResponse response;
        if (!stream->Read(&response)) break;

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
        } else if (response.success()) {  // detected event
            std::cout << "Detected event!" << std::endl;
        }
    }
    std::cout << std::endl;

    // Close the stream and check the status code in case the stream broke.
    stream->WritesDone();
    status = stream->Finish();

    // Stop the audio stream.
    err = Pa_StopStream(audioStream);
    if (err != paNoError) return describe_pa_error(err);

    // Terminate the port audio session.
    Pa_Terminate();

    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Enrolled vent validation stream broke ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
