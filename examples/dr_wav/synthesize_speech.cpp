// An example of text-to-speech (TTS) to a WAV file using SensoryCloud.
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
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include "../dep/argparse.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::FileSystemCredentialStore;

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("synthesize_speech")
        .description("A tool for synthesizing speech from phrases using SensoryCloud.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-o", "--output" })
        .help("The output path to write the audio samples to.")
        .default_value("speech.wav");
    parser.add_argument({ "-g", "--getmodels" })
        .action("store_true")
        .help("Whether to query for a list of available models.");
    parser.add_argument({ "-l", "--language" })
        .help("The IETF BCP 47 language tag for the input audio (e.g., en-US).");
    parser.add_argument({ "-V", "--voice" })
        .help("The name of the voice to use.");
    parser.add_argument({ "-p", "--phrase" })
        .help("The phrase to synthesize into speech.");
    parser.add_argument({ "-v", "--verbose" }).action("store_true")
        .help("Produce verbose output during synthesis.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto OUTPUT = args.get<std::string>("output");
    const auto GETMODELS = args.get<bool>("getmodels");
    const auto LANGUAGE = args.get<std::string>("language");
    const auto VOICE = args.get<std::string>("voice");
    const auto PHRASE = args.get<std::string>("phrase");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create a credential store for keeping OAuth credentials in.
    FileSystemCredentialStore keychain(".", "com.sensory.cloud.examples");
    // Create the cloud services handle.
    SensoryCloud<FileSystemCredentialStore> cloud(PATH, keychain);

    // Query the health of the remote service.
    sensory::api::common::ServerHealthResponse server_health_response;
    auto status = cloud.health.get_health(&server_health_response);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }
    if (VERBOSE) {
        std::cout << "Server status" << std::endl;
        std::cout << "\tIs Healthy:     " << server_health_response.ishealthy()     << std::endl;
        std::cout << "\tServer Version: " << server_health_response.serverversion() << std::endl;
        std::cout << "\tID:             " << server_health_response.id()            << std::endl;
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
            std::cout << "Failed to get synthesis models ("
                << status.error_code() << "): "
                << status.error_message() << std::endl;
            return 1;
        }
        for (auto& model : audioModelsResponse.models()) {
            if (model.modeltype() != sensory::api::common::VOICE_SYNTHESIS)
                continue;
            std::cout << model.name() << std::endl;
        }
        return 0;
    }

    // Create an audio config that describes the format of the audio stream.
    auto audio_config = new sensory::api::v1::audio::AudioConfig;
    audio_config->set_encoding(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
    audio_config->set_sampleratehertz(22050);
    audio_config->set_audiochannelcount(1);
    audio_config->set_languagecode(LANGUAGE);

    // Initialize the stream with the cloud.
    grpc::ClientContext context;
    auto stream = cloud.audio.synthesize_speech(&context, audio_config, VOICE, PHRASE);

    // Open a binary file-stream to write the audio contents to.
    std::ofstream file(OUTPUT, std::ios::out | std::ios::binary);
    while (true) {
        sensory::api::v1::audio::SynthesizeSpeechResponse response;
        // Break out on read failure. `Read` will fail if an error occurs or
        // when we are done reading bytes from the server.
        if (!stream->Read(&response)) break;
        file << response.audiocontent();
    }
    file.close();  // We're done writing to the WAV file.

    // Close the stream and check the status code in case the stream broke.
    status = stream->Finish();
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Speech synthesis stream broke ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
