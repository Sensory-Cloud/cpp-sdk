// An example of audio event enrollment based on file inputs.
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
using sensory::token_manager::TokenManager;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::service::HealthService;
using sensory::service::AudioService;
using sensory::service::OAuthService;

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("enrollEvent")
        .description("A tool for streaming audio files to Sensory Cloud for audio transcription.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-i", "--input" }).required(true)
        .help("The input audio file to stream to Sensory Cloud.");
    parser.add_argument({ "-g", "--getmodels" })
        .action("store_true")
        .help("Whether to query for a list of available models.");
    parser.add_argument({ "-m", "--model" })
        .help("The model to use for the enrollment.");
    parser.add_argument({ "-u", "--userid" })
        .help("The name of the user ID to create the enrollment for.");
    parser.add_argument({ "-d", "--description" })
        .help("A text description of the enrollment.");
    parser.add_argument({ "-n", "--numutterances" })
        .default_value(0)
        .help("The number of utterances for a text independent enrollment.");
    parser.add_argument({ "-D", "--duration" })
        .default_value(0)
        .help("The duration of a text-dependent enrollment.");
    parser.add_argument({ "-r", "--reference-id" })
        .help("An optional reference ID for tagging the enrollment.");
    parser.add_argument({ "-L", "--language" })
        .help("The IETF BCP 47 language tag for the input audio (e.g., en-US).");
    parser.add_argument({ "-C", "--chunksize" })
        .help("The number of audio samples per message; 0 to stream all samples in one message (default).")
        .default_value(4096);
    parser.add_argument({ "-v", "--verbose" }).action("store_true")
        .help("Produce verbose output during transcription.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto INPUT_FILE = args.get<std::string>("input");
    const auto GETMODELS = args.get<bool>("getmodels");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    const auto DESCRIPTION = args.get<std::string>("description");
    const auto NUM_UTTERANCES = args.get<uint32_t>("numutterances");
    const auto DURATION = args.get<float>("duration");
    const auto REFERENCE_ID = args.get<std::string>("reference-id");
    const auto LANGUAGE = args.get<std::string>("language");
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
    audio_config->set_languagecode(LANGUAGE);
    // Create the config with the event enrollment parameters.
    auto create_enrollment_event_config = new ::sensory::api::v1::audio::CreateEnrollmentEventConfig;
    create_enrollment_event_config->set_modelname(MODEL);
    create_enrollment_event_config->set_userid(USER_ID);
    create_enrollment_event_config->set_description(DESCRIPTION);
    if (DURATION > 0)  // enrollment duration provided
        create_enrollment_event_config->set_enrollmentduration(DURATION);
    else if (NUM_UTTERANCES > 0)  // number of utterances provided
        create_enrollment_event_config->set_enrollmentnumutterances(NUM_UTTERANCES);
    create_enrollment_event_config->set_referenceid(REFERENCE_ID);

    grpc::ClientContext context;
    auto stream = cloud.audio.create_event_enrollment(&context, audio_config, create_enrollment_event_config);

    // Create a background thread for handling the transcription responses.
    std::thread receipt_thread([&stream, &VERBOSE](){
        while (true) {
            // Read a message and break out of the loop when the read fails.
            sensory::api::v1::audio::CreateEnrollmentResponse response;
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
            } else {  // Friendly output, use a progress bar and display the prompt
                std::vector<std::string> progress{
                    "[          ] 0%   ",
                    "[*         ] 10%  ",
                    "[**        ] 20%  ",
                    "[***       ] 30%  ",
                    "[****      ] 40%  ",
                    "[*****     ] 50%  ",
                    "[******    ] 60%  ",
                    "[*******   ] 70%  ",
                    "[********  ] 80%  ",
                    "[********* ] 90%  ",
                    "[**********] 100% "
                };
                auto prompt = response.modelprompt().length() > 0 ?
                    "Prompt: \"" + response.modelprompt() + "\"" :
                    "Text-independent model, say anything";
                std::cout << '\r'
                    << progress[int(response.percentcomplete() / 10.f)]
                    << prompt << std::flush;
            }
            // Check for enrollment success
            if (response.percentcomplete() >= 100) {
                std::cout << std::endl;
                std::cout << "Successfully enrolled with ID: "
                    << response.enrollmentid() << std::endl;
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
        sensory::api::v1::audio::CreateEnrolledEventRequest request;
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
