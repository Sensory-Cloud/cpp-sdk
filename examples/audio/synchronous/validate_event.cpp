// An example of audio event validation using SensoryCloud with PortAudio.
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
inline int describe_pa_error(const PaError& err) {
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    return 1;
}

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("validate_event")
        .description("A tool for streaming audio files to SensoryCloud for audio event validation.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-g", "--getmodels" })
        .action("store_true")
        .help("Whether to query for a list of available models.");
    parser.add_argument({ "-m", "--model" })
        .help("The name of the event validation model to use.");
    parser.add_argument({ "-u", "--userid" })
        .help("The name of the user ID for the event validation.");
    parser.add_argument({ "-t", "--threshold" })
        .choices({"LOW", "MEDIUM", "HIGH", "HIGHEST"})
        .default_value("HIGH")
        .help("The sensitivity threshold for detecting audio events.");
    parser.add_argument({ "-tN", "--topN" })
        .help("For metric models, determines the number of ranked classes to return in inference responses.")
        .default_value(5);
    // parser.add_argument({ "-C", "--chunksize" })
    //     .help("The number of audio samples per message (default 4096).")
    //     .default_value("4096");
    // parser.add_argument({ "-S", "--samplerate" })
    //     .help("The audio sample rate of the input stream.")
    //     .choices({"9600", "11025", "12000", "16000", "22050", "24000", "32000", "44100", "48000", "88200", "96000", "192000"})
    //     .default_value("16000");
    parser.add_argument({ "-v", "--verbose" }).action("store_true")
        .help("Produce verbose output during event validation.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto GETMODELS = args.get<bool>("getmodels");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    ThresholdSensitivity THRESHOLD;
    if (args.get<std::string>("threshold") == "LOW")
        THRESHOLD = ThresholdSensitivity::LOW;
    else if (args.get<std::string>("threshold") == "MEDIUM")
        THRESHOLD = ThresholdSensitivity::MEDIUM;
    else if (args.get<std::string>("threshold") == "HIGH")
        THRESHOLD = ThresholdSensitivity::HIGH;
    else if (args.get<std::string>("threshold") == "HIGHEST")
        THRESHOLD = ThresholdSensitivity::HIGHEST;
    const auto TOPN = args.get<uint32_t>("topN");
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
        std::cout << "Failed to initialize (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }

    // ------ Query the available audio models ---------------------------------

    if (GETMODELS) {
        sensory::api::v1::audio::GetModelsResponse audioModelsResponse;
        status = cloud.audio.get_models(&audioModelsResponse);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get audio models ("
                << status.error_code() << "): "
                << status.error_message() << std::endl;
            return 1;
        }
        for (auto& model : audioModelsResponse.models()) {
            if (
                model.modeltype() != sensory::api::common::VOICE_EVENT_WAKEWORD &&
                model.modeltype() != sensory::api::common::SOUND_EVENT_FIXED
            )
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
    // Create the config with the event validation parameters.
    auto validate_event_config = new sensory::api::v1::audio::ValidateEventConfig;
    validate_event_config->set_modelname(MODEL);
    validate_event_config->set_userid(USER_ID);
    validate_event_config->set_sensitivity(THRESHOLD);
    validate_event_config->set_topn(TOPN);

    // Initialize the stream with the cloud.
    grpc::ClientContext context;
    auto stream = cloud.audio.validate_event(&context, audio_config, validate_event_config);

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
        sensory::api::v1::audio::ValidateEventRequest request;
        request.set_audiocontent(sample_block, BYTES_PER_BLOCK);
        if (!stream->Write(request)) break;

        // Read a new response from the server.
        sensory::api::v1::audio::ValidateEventResponse response;
        if (!stream->Read(&response)) break;

        // Log the result of the request to the terminal.
        if (VERBOSE) {
            google::protobuf::util::JsonPrintOptions options;
            options.add_whitespace = false;
            options.always_print_primitive_fields = true;
            options.always_print_enums_as_ints = false;
            options.preserve_proto_field_names = true;
            std::string response_json;
            google::protobuf::util::MessageToJsonString(response, &response_json, options);
            std::cout << response_json << std::endl;
        } else if (response.success()) {
            std::cout << "Detected trigger \""
                << response.resultid() << "\"" << std::endl;
        } else if (response.topnresponse().size()) {
            std::cout << "Top N results" << std::endl;
            for (const auto& thing : response.topnresponse()) {
                google::protobuf::util::JsonPrintOptions options;
                options.add_whitespace = false;
                options.always_print_primitive_fields = true;
                options.always_print_enums_as_ints = false;
                options.preserve_proto_field_names = true;
                std::string response_json;
                google::protobuf::util::MessageToJsonString(thing, &response_json, options);
                std::cout << response_json << std::endl;
            }
        }
    }

    // Close the stream and check the status code in case the stream broke.
    stream->WritesDone();
    status = stream->Finish();
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Event validation stream broke ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
    }

    // Stop the audio stream.
    err = Pa_StopStream(audioStream);
    if (err != paNoError) return describe_pa_error(err);

    // Terminate the port audio session.
    Pa_Terminate();

    return 0;
}
