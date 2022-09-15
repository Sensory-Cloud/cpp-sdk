// An example of audio enrollment based on file inputs.
//
// Copyright (c) 2021 Sensory, Inc.
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
#include <sensorycloud/token_manager/insecure_credential_store.hpp>
#include "dep/audio_buffer.hpp"
#include "dep/argparse.hpp"
#include "dep/tqdm.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::TokenManager;
using sensory::token_manager::InsecureCredentialStore;
using sensory::service::HealthService;
using sensory::service::AudioService;
using sensory::service::OAuthService;

/// @brief A bidirection stream reactor for biometric enrollments from audio
/// stream data.
///
class AudioFileReactor :
    public AudioService<InsecureCredentialStore>::CreateEnrollmentBidiReactor {
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
    /// The current index of the audio stream.
    std::size_t index = 0;
    /// A boolean determining whether the enrollment succeeded.
    std::atomic<bool> is_enrolled;
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
        AudioService<InsecureCredentialStore>::CreateEnrollmentBidiReactor(),
        buffer(buffer_),
        num_channels(num_channels_),
        sample_rate(sample_rate_),
        frames_per_block(num_channels_ * frames_per_block_),
        verbose(verbose_),
        bar(buffer_.size() / static_cast<float>(frames_per_block_), "frame"),
        is_enrolled(false) { }

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
        if (verbose) {  // Verbose output, dump the message to the terminal
            std::cout << "Response" << std::endl;
            std::cout << "\tPercent Complete:         " << response.percentcomplete()        << std::endl;
            std::cout << "\tPercent Segment Complete: " << response.percentsegmentcomplete() << std::endl;
            std::cout << "\tAudio Energy:             " << response.audioenergy()            << std::endl;
            std::cout << "\tEnrollment ID:            " << response.enrollmentid()           << std::endl;
            std::cout << "\tModel Name:               " << response.modelname()              << std::endl;
            std::cout << "\tModel Version:            " << response.modelversion()           << std::endl;
            std::cout << "\tModel Prompt:             " << response.modelprompt()            << std::endl;
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
            is_enrolled = true;
            std::cout << std::endl;
            std::cout << "Successfully enrolled with ID: "
                << response.enrollmentid() << std::endl;
        } else  // Start the next read request
            StartRead(&response);
    }
};

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("enroll")
        .description("A tool for streaming audio files to Sensory Cloud for audio transcription.");
    parser.add_argument({ "path" })
        .help("PATH The path to an INI file containing server metadata.");
    parser.add_argument({ "-i", "--input" }).required(true)
        .help("INPUT The input audio file to stream to Sensory Cloud.");
    parser.add_argument({ "-g", "--getmodels" })
        .action("store_true")
        .help("GETMODELS Whether to query for a list of available models.");
    parser.add_argument({ "-m", "--model" })
        .help("MODEL The model to use for the enrollment.");
    parser.add_argument({ "-u", "--userid" })
        .help("USERID The name of the user ID to create the enrollment for.");
    parser.add_argument({ "-d", "--description" })
        .help("DESCRIPTION A text description of the enrollment.");
    parser.add_argument({ "-l", "--liveness" })
        .action("store_true")
        .help("LIVENESS Whether to conduct a liveness check in addition to the enrollment.");
    parser.add_argument({ "-n", "--numutterances" })
        .default_value(0)
        .help("NUMUTTERANCES The number of utterances for a text independent enrollment.");
    parser.add_argument({ "-D", "--duration" })
        .default_value(0)
        .help("DURATION The duration of a text-dependent enrollment.");
    parser.add_argument({ "-L", "--language" })
        .help("LANGUAGE The IETF BCP 47 language tag for the input audio (e.g., en-US).");
    parser.add_argument({ "-C", "--chunksize" })
        .help("CHUNKSIZE The number of audio samples per message; 0 to stream all samples in one message (default).")
        .default_value(0);
    parser.add_argument({ "-p", "--padding" })
        .help("PADDING The number of milliseconds of padding to append to the audio buffer.")
        .default_value(300);
    parser.add_argument({ "-v", "--verbose" }).action("store_true")
        .help("VERBOSE Produce verbose output during transcription.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto INPUT_FILE = args.get<std::string>("input");
    const auto GETMODELS = args.get<bool>("getmodels");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    const auto DESCRIPTION = args.get<std::string>("description");
    const auto LIVENESS = args.get<bool>("liveness");
    const auto NUM_UTTERANCES = args.get<uint32_t>("numutterances");
    const auto DURATION = args.get<float>("duration");
    const auto LANGUAGE = args.get<std::string>("language");
    const auto CHUNK_SIZE = args.get<int>("chunksize");
    const auto VERBOSE = args.get<bool>("verbose");
    const auto PADDING = args.get<float>("padding");

    // Create an insecure credential store for keeping OAuth credentials in.
    sensory::token_manager::InsecureCredentialStore keychain(".", "com.sensory.cloud.examples");

    // Create the cloud services handle.
    SensoryCloud<InsecureCredentialStore> cloud(PATH, keychain);

    // Check the server health.
    sensory::api::common::ServerHealthResponse server_healthResponse;
    auto status = cloud.health.getHealth(&server_healthResponse);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }
    if (VERBOSE) {
        std::cout << "Server status" << std::endl;
        std::cout << "\tIs Healthy:     " << server_healthResponse.ishealthy()     << std::endl;
        std::cout << "\tServer Version: " << server_healthResponse.serverversion() << std::endl;
        std::cout << "\tID:             " << server_healthResponse.id()            << std::endl;
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
        int error_code = 0;
        cloud.audio.getModels([&error_code](AudioService<InsecureCredentialStore>::GetModelsCallData* call) {
            if (!call->getStatus().ok()) {  // The call failed.
                std::cout << "Failed to get audio models with\n\t" <<
                    call->getStatus().error_code() << ": " <<
                    call->getStatus().error_message() << std::endl;
                error_code = 1;
            } else {
                for (auto& model : call->getResponse().models()) {
                    // Ignore models that aren't face biometric models.
                    if (model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_DEPENDENT &&
                        model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_INDEPENDENT &&
                        model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_WAKEWORD
                    ) continue;
                    std::cout << model.name() << std::endl;
                }
            }
        })->await();
        return error_code;
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

    // Create the gRPC reactor to respond to streaming events.
    AudioFileReactor reactor(buffer.get_samples(),
        buffer.get_channels(),
        buffer.get_sample_rate(),
        CHUNK_SIZE > 0 ? CHUNK_SIZE : buffer.get_num_samples(),
        VERBOSE
    );
    // Initialize the stream with the reactor for handling callbacks.
    cloud.audio.createEnrollment(&reactor,
        sensory::service::audio::new_audio_config(
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
            buffer.get_sample_rate(), 1, LANGUAGE
        ),
        sensory::service::audio::new_create_enrollment_config(
            MODEL,
            USER_ID,
            DESCRIPTION,
            LIVENESS,
            DURATION,
            NUM_UTTERANCES
        )
    );
    reactor.StartCall();
    // Wait for the call to terminate and check the final status.
    status = reactor.await();
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Stream broke (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
