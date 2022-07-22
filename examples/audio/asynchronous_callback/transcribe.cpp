// An example of audio transcription using SensoryCloud with PortAudio.
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

#include <portaudio.h>
#include <iostream>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/insecure_credential_store.hpp>
#include "../dep/argparse.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::InsecureCredentialStore;
using sensory::service::AudioService;
using sensory::service::audio::TranscriptAggregator;
using sensory::api::v1::audio::WordState;

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

/// @brief A bidirection stream reactor for biometric enrollments from audio
/// stream data.
///
/// @details
/// Input data for the stream is provided by a PortAudio capture device.
///
class PortAudioReactor :
    public AudioService<InsecureCredentialStore>::TranscribeBidiReactor {
 private:
    /// The capture device that input audio is streaming in from.
    PaStream* capture;
    /// The number of channels in the input audio.
    uint32_t num_channels;
    /// The number of bytes per audio sample (i.e., 2 for 16-bit audio)
    uint32_t sample_size;
    /// The sample rate of the audio input stream.
    uint32_t sample_rate;
    /// The number of frames per block of audio.
    uint32_t frames_per_block;
    /// The maximum duration of the stream in seconds.
    float duration;
    /// An aggregator for accumulating partial updates into a transcript.
    TranscriptAggregator aggregator;
    /// Whether to produce verbose output from the reactor
    bool verbose = false;
    /// The buffer for the block of samples from the port audio input device.
    std::unique_ptr<uint8_t> sample_block;
    /// The number of blocks that have been written to the server.
    std::atomic<uint32_t> blocks_written;

 public:
    /// @brief Initialize a reactor for streaming audio from a PortAudio stream.
    ///
    /// @param capture_ The PortAudio capture device.
    /// @param num_channels_ The number of channels in the input stream.
    /// @param sample_size_ The number of bytes in an individual frame.
    /// @param sample_rate_ The sampling rate of the audio stream.
    /// @param frames_per_block_ The number of frames in a block of audio.
    /// @param duration_ the maximum duration for the audio capture
    ///
    PortAudioReactor(PaStream* capture_,
        uint32_t num_channels_ = 1,
        uint32_t sample_size_ = 2,
        uint32_t sample_rate_ = 16000,
        uint32_t frames_per_block_ = 4096,
        float duration_ = 60,
        bool verbose_ = false
    ) :
        AudioService<InsecureCredentialStore>::TranscribeBidiReactor(),
        capture(capture_),
        num_channels(num_channels_),
        sample_size(sample_size_),
        sample_rate(sample_rate_),
        frames_per_block(frames_per_block_),
        duration(duration_),
        verbose(verbose_),
        sample_block(static_cast<uint8_t*>(malloc(frames_per_block_ * num_channels_ * sample_size_))),
        blocks_written(0) {
        if (capture == nullptr)
            throw std::runtime_error("capture must point to an allocated stream.");
    }

    /// @brief React to a _write done_ event.
    ///
    /// @param ok whether the write succeeded.
    ///
    void OnWriteDone(bool ok) override {
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        // If max duration samples have elapsed, send the writes done signal.
        if (blocks_written++ > (duration * sample_rate) / frames_per_block) {
            StartWritesDone();
            return;
        }
        // Read a block of samples from the ADC.
        auto err = Pa_ReadStream(capture, sample_block.get(), frames_per_block);
        if (err) {
            describe_pa_error(err);
            return;
        }
        // Set the audio content for the request and start the write request
        request.set_audiocontent(sample_block.get(), num_channels * frames_per_block * sample_size);
        StartWrite(&request);
    }

    /// @brief React to a _read done_ event.
    ///
    /// @param ok whether the read succeeded.
    ///
    void OnReadDone(bool ok) override {
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        // Set the content of the local transcript buffer.
        aggregator.process_response(response.wordlist());
        // Log the current transcription to the terminal.
        if (verbose) {
            // Relative energy of the processed audio as a value between 0 and 1.
            // Can be converted to decibels in (-inf, 0] using 20 * log10(x).
            std::cout << "Audio Energy: " << response.audioenergy() << std::endl;
            // The text of the current transcript as a sliding window on the last
            // ~7 seconds of processed audio.
            std::cout << "Sliding Transcript: " << response.transcript() << std::endl;
            // The word list contains the directives to the TranscriptAggregator
            // for accumulating the sliding window transcript over time.
            for (const auto& word : response.wordlist().words()) {
                std::string state = "";
                switch (word.wordstate()) {
                    case WordState::WORDSTATE_PENDING: state = "PENDING"; break;
                    case WordState::WORDSTATE_FINAL: state = "FINAL"; break;
                    default: break;
                }
                std::cout << "word=" << word.word() << ", "
                    << "state=" << state << ", "
                    << "index=" << word.wordindex() << ", "
                    << "confidence=" << word.confidence() << ", "
                    << "begin_time=" << word.begintimems() << ", "
                    << "end_time=" << word.endtimems() << std::endl;
            }
            // The post-processing actions convey pipeline specific
            // functionality to/from the server. In this case the "FINAL" action
            // is sent to indicate when the server has finished transcribing.
            if (response.has_postprocessingaction()) {
                const auto& action = response.postprocessingaction();
                std::cout << "Post-processing "
                    << "actionid=" << action.actionid() << ", "
                    << "action=" << action.action() << std::endl;
            }
            std::cout << "Aggregated Transcript: " << aggregator.get_transcript() << std::endl;
            std::cout << std::endl;
        } else {
            #if defined(_WIN32) || defined(_WIN64)  // Windows
                std::system("clr");
            #else
                std::system("clear");
            #endif
            std::cout << aggregator.get_transcript() << std::endl;
        }
        // Start the next read request
        StartRead(&response);
    }
};

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("transcribe")
        .description("A tool for streaming audio files to SensoryCloud for audio transcription.");
    parser.add_argument({ "path" })
        .help("PATH The path to an INI file containing server metadata.");
    parser.add_argument({ "-g", "--getmodels" })
        .action("store_true")
        .help("GETMODELS Whether to query for a list of available models.");
    parser.add_argument({ "-m", "--model" })
        .help("MODEL The name of the transcription model to use.");
    parser.add_argument({ "-u", "--userid" })
        .help("USERID The name of the user ID for the transcription.");
    parser.add_argument({ "-L", "--language" })
        .help("LANGUAGE The IETF BCP 47 language tag for the input audio (e.g., en-US).");
    // parser.add_argument({ "-C", "--chunksize" })
    //     .help("CHUNKSIZE The number of audio samples per message (default 4096).")
    //     .default_value("4096");
    // parser.add_argument({ "-S", "--samplerate" })
    //     .help("SAMPLERATE The audio sample rate of the input stream.")
    //     .choices({"9600", "11025", "12000", "16000", "22050", "24000", "32000", "44100", "48000", "88200", "96000", "192000"})
    //     .default_value("16000");
    parser.add_argument({ "-v", "--verbose" }).action("store_true")
        .help("VERBOSE Produce verbose output during transcription.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto GETMODELS = args.get<bool>("getmodels");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    const auto LANGUAGE = args.get<std::string>("language");
    const uint32_t CHUNK_SIZE = 4096;//args.get<int>("chunksize");
    const auto SAMPLE_RATE = 16000;//args.get<uint32_t>("samplerate");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create an insecure credential store for keeping OAuth credentials in.
    InsecureCredentialStore keychain(".", "com.sensory.cloud.examples");
    // Create the cloud services handle.
    SensoryCloud<InsecureCredentialStore> cloud(PATH, keychain);

    // Query the health of the remote service.
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

    // ------ Query the available audio models ---------------------------------

    if (GETMODELS) {
        int error_code = 0;
        cloud.audio.getModels([&error_code](AudioService<InsecureCredentialStore>::GetModelsCallData* call) {
            if (!call->getStatus().ok()) {  // The call failed.
                std::cout << "Failed to get audio models ("
                    << call->getStatus().error_code() << "): "
                    << call->getStatus().error_message() << std::endl;
                error_code = 1;
            } else {
                // Iterate over the models returned in the response
                for (auto& model : call->getResponse().models()) {
                    // Ignore models that aren't face biometric models.
                    if (model.modeltype() != sensory::api::common::VOICE_TRANSCRIBE_COMMAND_AND_SEARCH)
                        continue;
                    std::cout << model.name() << std::endl;
                }
            }
        })->await();
        return error_code;
    }

    // the maximal duration of the recording in seconds
    const auto DURATION = 60;
    // The number of input channels from the microphone. This should always be
    // mono.
    const auto NUM_CHANNELS = 1;
    // The number of bytes per sample, for 16-bit audio, this is 2 bytes.
    const auto SAMPLE_SIZE = 2;
    // The number of bytes in a given chunk of samples.
    const auto BYTES_PER_BLOCK =
        CHUNK_SIZE * NUM_CHANNELS * SAMPLE_SIZE;

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
    PaStream* capture;
    err = Pa_OpenStream(&capture,
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
    err = Pa_StartStream(capture);
    if (err != paNoError) return describe_pa_error(err);

    // Create the gRPC reactor to respond to streaming events.
    PortAudioReactor reactor(capture,
        NUM_CHANNELS,
        SAMPLE_SIZE,
        SAMPLE_RATE,
        CHUNK_SIZE,
        DURATION,
        VERBOSE
    );
    // Initialize the stream with the reactor for callbacks, given audio model,
    // the sample rate of the audio and the expected language. A user ID is also
    // necessary to transcribe audio.
    cloud.audio.transcribe(&reactor,
        sensory::service::audio::new_audio_config(
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
            SAMPLE_RATE, NUM_CHANNELS, LANGUAGE
        ),
        sensory::service::audio::new_transcribe_config(MODEL, USER_ID)
    );

    reactor.StartCall();
    status = reactor.await();

    // Stop the audio stream.
    err = Pa_StopStream(capture);
    if (err != paNoError) return describe_pa_error(err);

    // Terminate the port audio session.
    Pa_Terminate();

    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Transcription stream broke ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
