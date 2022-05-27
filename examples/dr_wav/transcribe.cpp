// An example of audio transcription based on audio file inputs.
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
#include <mutex>
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/health_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>
#include <sensorycloud/services/management_service.hpp>
#include <sensorycloud/services/audio_service.hpp>
#include <sensorycloud/token_manager/insecure_credential_store.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>
#include "dep/audio_buffer.hpp"
#include "dep/argparse.hpp"
#include "dep/tqdm.hpp"

using sensory::token_manager::TokenManager;
using sensory::token_manager::InsecureCredentialStore;
using sensory::service::HealthService;
using sensory::service::AudioService;
using sensory::service::OAuthService;

/// @brief A bi-directional reactor for transcribing an audio buffer to text.
///
class AudioFileReactor :
    public AudioService<InsecureCredentialStore>::TranscribeBidiReactor {
 private:
    /// The audio samples.
    const std::vector<int16_t>& buffer;
    /// The number of channels in the audio buffer.
    uint32_t numChannels;
    /// The sample rate of the audio buffer.
    uint32_t sampleRate;
    /// The number of samples per block of audio sent to the server.
    uint32_t framesPerBlock;
    /// The current index of the audio stream.
    std::size_t index = 0;
    /// Whether to produce verbose output.
    bool verbose = false;
    /// The progress bar for providing a response per frame written.
    tqdm bar;

    /// A contained scope for the transcript data from the server.
    struct {
     private:
        /// A mutex for guarding access to the transcript.
        std::mutex mutex;
        /// The current transcription from the server
        std::string text = "";

     public:
        /// Set the transcript to a new value.
        ///
        /// @param text_ The text to set the transcript to.
        ///
        inline void set(const std::string& text_ = "") {
            std::lock_guard<std::mutex> lock(mutex);
            text = text_;
        }

        /// @brief Return the current transcript.
        inline std::string get() {
            std::lock_guard<std::mutex> lock(mutex);
            return text;
        }
    } transcript;

    // A contained scope for the data associated with receiving the FINAL
    // signal from the server.
    struct {
     private:
        /// A flag determining whether the FINAL signal has been received.
        bool didReceive = false;
        /// A mutual exclusion for locking access to the critical section.
        std::mutex mutex;
        /// A condition variable for signalling to awaiting processes.
        std::condition_variable condition;

     public:
        /// Wait for a signal from the condition variable.
        inline void wait() {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [this] { return didReceive; });
        }

        /// Notify awaiting processes that the condition variable has changed.
        inline void notify_one() {
            std::lock_guard<std::mutex> lock(mutex);
            didReceive = true;
            condition.notify_one();
        }
    } final;

 public:
    /// @brief Initialize the reactor.
    ///
    /// @param buffer_ The audio buffer to transcribe to text
    /// @param numChannels_ The number of channels in the audio buffer.
    /// @param sampleRate_ The sample rate of the audio buffer.
    /// @param framesPerBlock_ The number of frames per block of audio.
    /// @param verbose_ Whether to produce verbose output from the reactor.
    ///
    AudioFileReactor(const std::vector<int16_t>& buffer_,
        uint32_t numChannels_ = 1,
        uint32_t sampleRate_ = 16000,
        uint32_t framesPerBlock_ = 4096,
        const bool& verbose_ = false
    ) :
        AudioService<InsecureCredentialStore>::TranscribeBidiReactor(),
        buffer(buffer_),
        numChannels(numChannels_),
        sampleRate(sampleRate_),
        framesPerBlock(numChannels_ * framesPerBlock_),
        verbose(verbose_),
        bar(buffer_.size() / static_cast<float>(framesPerBlock_), "frame") { }

    /// @brief React to a "write done" event.
    ///
    /// @param ok whether the write succeeded.
    ///
    void OnWriteDone(bool ok) override {
        // If the status is not OK, an error occurred, exit gracefully.
        if (!ok) return;

        // If the index has exceeded the buffer size, there are no more samples
        // to write from the audio buffer.
        if (index >= buffer.size()) {
            // Wait for the FINAL signal from the server.
            final.wait();
            // Now that the FINAL signal has been received, we can shut down
            // the stream.
            StartWritesDone();
            return;
        }

        // Count the number of samples to upload in this request based on the
        // index of the current sample and the number of remaining samples.
        const auto numSamples = index + framesPerBlock > buffer.size() ?
            buffer.size() - index : framesPerBlock;
        // Set the audio content for the request.
        request.set_audiocontent(&buffer[index], numSamples * sizeof(int16_t));
        // Update the index of the current sample based on the number of samples
        // that are being sent up in this request.
        index += numSamples;
        if (framesPerBlock < buffer.size()) bar.update();

        // If the index has exceeded the buffer size, there are no more samples
        // to write from the audio buffer. Add the FINAL post-processing action
        // to this last message to indicate that no more data will be sent up.
        if (index >= buffer.size()) {
            auto action = new ::sensory::api::v1::audio::AudioRequestPostProcessingAction;
            action->set_action(::sensory::api::v1::audio::FINAL);
            request.set_allocated_postprocessingaction(action);
        }

        // If the number of blocks written surpasses the maximal length, close
        // the stream.
        StartWrite(&request);
    }

    /// @brief React to a "read done" event.
    ///
    /// @param ok whether the read succeeded.
    ///
    void OnReadDone(bool ok) override {
        // If the status is not OK, an error occurred, exit gracefully.
        if (!ok) return;
        if (verbose) {
            std::cout << "\tAudio Energy: " << response.audioenergy()     << std::endl;
            std::cout << "\tTranscript:   " << response.transcript()      << std::endl;
            std::cout << "\tIs Partial:   " << response.ispartialresult() << std::endl;
            if (response.has_postprocessingaction()) {
                const auto& action = response.postprocessingaction();
                std::cout << "\tPost Processing" << std::endl;
                std::cout << "\t\t Action ID: " << action.actionid() << std::endl;
                std::cout << "\t\t Action: " << action.action() << std::endl;
            }
            std::cout << std::endl;
        }
        // Set the content of the local transcript buffer.
        transcript.set(response.transcript());
        // Look for a post-processing action to determine the end of the stream.
        if (response.has_postprocessingaction()) {
            const auto& action = response.postprocessingaction();
            // If the action is the FINAL action, the server has finished
            // processing the audio and has no more messages to send.
            if (action.action() == ::sensory::api::v1::audio::FINAL) {
                // Notify the waiting write loop that the FINAL signal has been
                // received from the server.
                final.notify_one();
                // Grace-fully shut down the read loop by not starting another
                // read request.
                return;
            }
        }
        // Start the next read request.
        StartRead(&response);
    }

    /// @brief Return the current transcript.
    inline std::string getTranscript() { return transcript.get(); }
};

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("transcribe")
        .description("A tool for streaming audio files to Sensory Cloud for audio transcription.");
    parser.add_argument({ "-H", "--host" }).required(true)
        .help("HOST The hostname of a Sensory Cloud inference server.");
    parser.add_argument({ "-P", "--port" }).required(true)
        .help("PORT The port number that the Sensory Cloud inference server is running at.");
    parser.add_argument({ "-T", "--tenant" }).required(true)
        .help("TENANT The ID of your tenant on a Sensory Cloud inference server.");
    parser.add_argument({ "-I", "--insecure" }).action("store_true")
        .help("INSECURE Disable TLS.");
    parser.add_argument({ "-i", "--input" }).required(true)
        .help("INPUT The input audio file to stream to Sensory Cloud.");
    parser.add_argument({ "-o", "--output" })
        .help("OUTPUT The output file to write the transcription to.");
    parser.add_argument({ "-m", "--model" }).required(true)
        .help("MODEL The name of the transcription model to use.");
    parser.add_argument({ "-u", "--userid" }).required(true)
        .help("USERID The name of the user ID for the transcription.");
    parser.add_argument({ "-l", "--language" }).required(true)
        .help("LANGUAGE The IETF BCP 47 language tag for the input audio (e.g., en-US).");
    parser.add_argument({ "-C", "--chunksize" })
        .help("CHUNKSIZE The number of audio samples per message; 0 to stream all samples in one message (default 4096).")
        .default_value(4096);
    parser.add_argument({ "-p", "--padding" })
        .help("PADDING The number of milliseconds of padding to append to the audio buffer.")
        .default_value(0);
    parser.add_argument({ "-v", "--verbose" }).action("store_true")
        .help("VERBOSE Produce verbose output during transcription.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto INPUT_FILE = args.get<std::string>("input");
    const auto OUTPUT_FILE = args.get<std::string>("output");
    const auto HOSTNAME = args.get<std::string>("host");
    const auto PORT = args.get<uint16_t>("port");
    const auto TENANT = args.get<std::string>("tenant");
    const auto IS_SECURE = !args.get<bool>("insecure");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    const auto LANGUAGE = args.get<std::string>("language");
    const auto CHUNK_SIZE = args.get<int>("chunksize");
    const auto VERBOSE = args.get<bool>("verbose");
    const auto PADDING = args.get<float>("padding");

    // Create an insecure credential store for keeping OAuth credentials in.
    sensory::token_manager::InsecureCredentialStore keychain(".", "com.sensory.cloud.examples");
    if (!keychain.contains("deviceID"))
        keychain.emplace("deviceID", sensory::token_manager::uuid_v4());
    const auto DEVICE_ID(keychain.at("deviceID"));

    // Initialize the configuration for the Sensory Cloud service.
    sensory::Config config(HOSTNAME, PORT, TENANT, DEVICE_ID, IS_SECURE);
    config.connect();

    // Query the health of the remote service to ensure the server is live.
    sensory::service::HealthService healthService(config);
    sensory::api::common::ServerHealthResponse serverHealthResponse;
    auto status = healthService.getHealth(&serverHealthResponse);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    } else if (VERBOSE) {
        std::cout << "Server status" << std::endl;
        std::cout << "\tIs Healthy:     " << serverHealthResponse.ishealthy()     << std::endl;
        std::cout << "\tServer Version: " << serverHealthResponse.serverversion() << std::endl;
        std::cout << "\tID:             " << serverHealthResponse.id()            << std::endl;
    }

    // Create an OAuth service and a token manager for device registration.
    OAuthService oauthService(config);
    sensory::token_manager::TokenManager<sensory::token_manager::InsecureCredentialStore>
        tokenManager(oauthService, keychain);

    // Attempt to login and register the device if needed.
    status = tokenManager.registerDevice([]() -> std::tuple<std::string, std::string> {
        std::cout << "Registering device with server..." << std::endl;
        // Query the device name from the standard input.
        std::string name = "";
        std::cout << "Device name: ";
        std::cin >> name;
        // Query the credential for the user from the standard input.
        std::string credential = "";
        std::cout << "Credential: ";
        std::cin >> credential;
        // Return the device name and credential as a tuple.
        return {name, credential};
    });
    // Check the status code from the attempted registration.
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to register device with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }

    // ------ Create the audio service -----------------------------------------

    // Create the audio service based on the configuration and token manager.
    AudioService<InsecureCredentialStore> audioService(config, tokenManager);

    // Load the audio file.
    AudioBuffer buffer;
    buffer.load(INPUT_FILE);
    // Check that the file is 16kHz.
    if (buffer.getSampleRate() != 16000) {
        std::cout << "Error: attempting to load WAV file with sample rate of "
            << buffer.getSampleRate() << "Hz, but only 16000Hz audio is supported."
            << std::endl;
        return 1;
    }
    // Check that the file in monophonic.
    if (buffer.getChannels() > 1) {
        std::cout << "Error: attempting to load WAV file with "
            << buffer.getChannels() << " channels, but only mono audio is supported."
            << std::endl;
        return 1;
    }
    // Pad the file with silence.
    buffer.padBack(PADDING);

    // Create the gRPC reactor to respond to streaming events.
    AudioFileReactor reactor(buffer.getSamples(),
        buffer.getChannels(),
        buffer.getSampleRate(),
        CHUNK_SIZE > 0 ? CHUNK_SIZE : buffer.getNumSamples(),
        VERBOSE
    );
    // Initialize the stream and start the RPC.
    audioService.transcribe(&reactor,
        sensory::service::audio::newAudioConfig(
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
            buffer.getSampleRate(), 1, LANGUAGE
        ),
        sensory::service::audio::newTranscribeConfig(MODEL, USER_ID)
    );
    reactor.StartCall();
    // Wait for the call to terminate and check the final status.
    status = reactor.await();
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Transcription stream broke with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }

    if (OUTPUT_FILE.empty()) {  // No output file, write to standard output.
        std::cout << reactor.getTranscript() << std::endl;
    } else {  // Write the results to the given filename.
        std::ofstream output_file(OUTPUT_FILE, std::ofstream::out);
        output_file << reactor.getTranscript() << std::endl;
        output_file.close();
    }

    return 0;
}
