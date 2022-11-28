// An example of audio event validation based on file inputs.
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
#include <regex>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include "dep/audio_buffer.hpp"
#include "dep/argparse.hpp"
#include "dep/tqdm.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::TokenManager;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::service::AudioService;
using sensory::api::v1::audio::ThresholdSensitivity;

/// @brief A bi-directional stream reactor for audio signal event validation.
///
class AudioFileReactor :
    public AudioService<FileSystemCredentialStore>::ValidateEventBidiReactor {
 private:
    /// The audio samples to send to the cloud.
    const std::vector<int16_t>& buffer;
    /// The number of channels in the input audio.
    uint32_t num_channels;
    /// The sample rate of the audio input stream.
    uint32_t sample_rate;
    /// The number of frames per block of audio.
    uint32_t frames_per_block;
    /// Whether to produce verbose output from the server.
    bool verbose = false;
    /// A mutex for guarding access to the `isDone` and `status` variables.
    std::mutex mutex;
    /// The current index of the audio stream.
    std::size_t index = 0;
    /// The progress bar for providing a response per frame written.
    tqdm bar;

 public:
    /// @brief Initialize a reactor for streaming audio from a PortAudio stream.
    ///
    /// @param buffer_ The audio samples to send to the cloud
    /// @param num_channels_ The number of channels in the input stream.
    /// @param sample_rate_ The sampling rate of the audio stream.
    /// @param frames_per_block_ The number of frames in a block of audio.
    /// @param verbose_ Whether to produce verbose output from the reactor.
    ///
    AudioFileReactor(const std::vector<int16_t>& buffer_,
        uint32_t num_channels_ = 1,
        uint32_t sample_rate_ = 16000,
        uint32_t frames_per_block_ = 4096,
        const bool& verbose_ = false
    ) :
        AudioService<FileSystemCredentialStore>::ValidateEventBidiReactor(),
        buffer(buffer_),
        num_channels(num_channels_),
        sample_rate(sample_rate_),
        frames_per_block(num_channels_ * frames_per_block_),
        verbose(verbose_),
        bar(buffer_.size() / static_cast<float>(frames_per_block_), "frame") { }

    /// @brief React to a _write done_ event.
    ///
    /// @param ok whether the write succeeded.
    ///
    void OnWriteDone(bool ok) override {
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;

        // If the index has exceeded the buffer size, there are no more samples
        // to write from the audio buffer.
        if (index >= buffer.size()) {
            // Signal to the stream that no more data will be written.
            StartWritesDone();
            return;
        }

        // Count the number of samples to upload in this request based on the
        // index of the current sample and the number of remaining samples.
        const auto numSamples = index + frames_per_block > buffer.size() ?
            buffer.size() - index : frames_per_block;
        // Set the audio content for the request and start the write request.
        request.set_audiocontent(&buffer[index], numSamples * sizeof(int16_t));
        // Update the index of the current sample based on the number of samples
        // that were just pushed.
        index += numSamples;
        if (frames_per_block < buffer.size())  // Update progress bar if chunking
            bar.update();
        // If the number of blocks written surpasses the maximal length, close
        // the stream.
        StartWrite(&request);
    }

    /// @brief React to a _read done_ event.
    ///
    /// @param ok whether the read succeeded.
    ///
    void OnReadDone(bool ok) override {
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        // Log the current audio event status to the terminal.
        if (verbose) {
            std::cout << "Response" << std::endl;
            std::cout << "\tAudio Energy: " << response.audioenergy() << std::endl;
            std::cout << "\tSuccess:      " << response.success()     << std::endl;
            std::cout << "\tResult ID:    " << response.resultid()    << std::endl;
            std::cout << "\tScore:        " << response.score()       << std::endl;
        } else if (response.success()) {
            std::cout << "Detected trigger \""
                << response.resultid() << "\"" << std::endl;
        }
        // Start the next read request
        StartRead(&response);
    }
};

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("validate_event")
        .description("A tool for streaming audio files to Sensory Cloud for audio transcription.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-i", "--input" }).required(true)
        .help("The input audio file to stream to Sensory Cloud.");
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
    parser.add_argument({ "-L", "--language" })
        .help("The IETF BCP 47 language tag for the input audio (e.g., en-US).");
    parser.add_argument({ "-C", "--chunksize" })
        .help("The number of audio samples per message; 0 to stream all samples in one message (default).")
        .default_value(0);
    parser.add_argument({ "-p", "--padding" })
        .help("The number of milliseconds of padding to append to the audio buffer.")
        .default_value(300);
    parser.add_argument({ "-v", "--verbose" }).action("store_true")
        .help("Produce verbose output during transcription.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto INPUT_FILE = args.get<std::string>("input");
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
    const auto LANGUAGE = args.get<std::string>("language");
    const auto CHUNK_SIZE = args.get<int>("chunksize");
    const auto VERBOSE = args.get<bool>("verbose");
    const auto PADDING = args.get<float>("padding");

    // Create a credential store for keeping OAuth credentials in.
    sensory::token_manager::FileSystemCredentialStore keychain(".", "com.sensory.cloud.examples");

    // Create the cloud services handle.
    SensoryCloud<FileSystemCredentialStore> cloud(PATH, keychain);

    // Check the server health.
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

    // ------ Create the audio service -----------------------------------------

    // Load the audio file.
    AudioBuffer buffer;
    buffer.load(INPUT_FILE);
    // Check that the file is 16kHz.
    if (buffer.get_sample_rate() != 16000) {
        std::cout << "Error: attempting to load WAV file with sample rate of "
            << buffer.get_sample_rate() << "Hz, but only 16000Hz audio is supported."
            << std::endl;
        return 1;
    }
    // Check that the file in monophonic.
    if (buffer.get_channels() > 1) {
        std::cout << "Error: attempting to load WAV file with "
            << buffer.get_channels() << " channels, but only mono audio is supported."
            << std::endl;
        return 1;
    }
    // Pad the file with silence.
    buffer.pad_back(PADDING);

    // Create an audio config that describes the format of the audio stream.
    auto audio_config = new sensory::api::v1::audio::AudioConfig;
    audio_config->set_encoding(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
    audio_config->set_sampleratehertz(buffer.get_sample_rate());
    audio_config->set_audiochannelcount(buffer.get_channels());
    audio_config->set_languagecode(LANGUAGE);
    // Create the config with the event validation parameters.
    auto validate_event_config = new sensory::api::v1::audio::ValidateEventConfig;
    validate_event_config->set_modelname(MODEL);
    validate_event_config->set_userid(USER_ID);
    validate_event_config->set_sensitivity(THRESHOLD);

    // Initialize the stream with the cloud.
    AudioFileReactor reactor(buffer.get_samples(),
        buffer.get_channels(),
        buffer.get_sample_rate(),
        CHUNK_SIZE > 0 ? CHUNK_SIZE : buffer.get_num_samples(),
        VERBOSE
    );
    cloud.audio.validate_event(&reactor, audio_config, validate_event_config);
    reactor.StartCall();

    // Wait for the call to terminate and check the final status.
    status = reactor.await();
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Stream broke (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
