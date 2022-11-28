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
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include "dep/audio_buffer.hpp"
#include "dep/argparse.hpp"
#include "dep/tqdm.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::service::AudioService;
using sensory::util::TranscriptAggregator;
using sensory::api::v1::audio::WordState;
using sensory::api::v1::audio::ThresholdSensitivity;

/// @brief A bi-directional stream reactor for audio signal transcription.
///
class AudioBufferReactor :
    public AudioService<FileSystemCredentialStore>::TranscribeBidiReactor {
 private:
    /// The audio samples.
    const std::vector<int16_t>& buffer;
    /// The number of channels in the audio buffer.
    uint32_t num_channels;
    /// The sample rate of the audio buffer.
    uint32_t sample_rate;
    /// The number of samples per block of audio sent to the server.
    uint32_t frames_per_block;
    /// The current index of the audio stream.
    std::size_t index = 0;
    /// An aggregator for accumulating partial updates into a transcript.
    TranscriptAggregator aggregator;
    /// Whether to produce verbose output.
    bool verbose = false;
    /// The progress bar for providing a response per frame written.
    tqdm bar;

    // A contained scope for the data associated with receiving the FINAL
    // signal from the server.
    struct {
     private:
        /// A flag determining whether the FINAL signal has been received.
        bool did_receive = false;
        /// A mutual exclusion for locking access to the critical section.
        std::mutex mutex;
        /// A condition variable for signalling to awaiting processes.
        std::condition_variable condition;

     public:
        /// Wait for a signal from the condition variable.
        inline void wait() {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [this] { return did_receive; });
        }

        /// Notify awaiting processes that the condition variable has changed.
        inline void notify_one() {
            std::lock_guard<std::mutex> lock(mutex);
            did_receive = true;
            condition.notify_one();
        }
    } final;

 public:
    /// @brief Initialize the reactor.
    ///
    /// @param buffer_ The audio buffer to transcribe to text
    /// @param num_channels_ The number of channels in the audio buffer.
    /// @param sample_rate_ The sample rate of the audio buffer.
    /// @param frames_per_block_ The number of frames per block of audio.
    /// @param verbose_ Whether to produce verbose output from the reactor.
    ///
    AudioBufferReactor(const std::vector<int16_t>& buffer_,
        uint32_t num_channels_ = 1,
        uint32_t sample_rate_ = 16000,
        uint32_t frames_per_block_ = 4096,
        const bool& verbose_ = false
    ) :
        AudioService<FileSystemCredentialStore>::TranscribeBidiReactor(),
        buffer(buffer_),
        num_channels(num_channels_),
        sample_rate(sample_rate_),
        frames_per_block(num_channels_ * frames_per_block_),
        verbose(verbose_),
        bar(buffer_.size() / static_cast<float>(frames_per_block_), "frame") { }

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
        const auto num_samples = index + frames_per_block > buffer.size() ?
            buffer.size() - index : frames_per_block;
        // Set the audio content for the request.
        request.set_audiocontent(&buffer[index], num_samples * sizeof(int16_t));
        // Update the index of the current sample based on the number of samples
        // that are being sent up in this request.
        index += num_samples;
        if (frames_per_block < buffer.size()) bar.update();

        // If the index has exceeded the buffer size, there are no more samples
        // to write from the audio buffer. Add the FINAL post-processing action
        // to this last message to indicate that no more data will be sent up.
        if (index >= buffer.size()) {
            auto action = new ::sensory::api::v1::audio::AudioRequestPostProcessingAction;
            action->set_action(::sensory::api::v1::audio::FINAL);
            request.set_allocated_postprocessingaction(action);
            std::cout << "Audio uploaded, awaiting FINAL response..." << std::endl;
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
        }
        // Set the content of the local transcript buffer.
        aggregator.process_response(response.wordlist());
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
    inline std::string get_transcript() {
        return aggregator.get_transcript();
    }
};

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("transcribe")
        .description("A tool for streaming audio files to SensoryCloud for audio transcription.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-i", "--input" }).required(true)
        .help("The input audio file to stream to SensoryCloud.");
    parser.add_argument({ "-o", "--output" })
        .help("The output file to write the transcription to.");
    parser.add_argument({ "-m", "--model" }).required(true)
        .help("The name of the transcription model to use.");
    parser.add_argument({ "-u", "--userid" }).required(true)
        .help("The name of the user ID for the transcription.");
    parser.add_argument({ "-cp", "--capitalization-punctuation"}).action("store_true")
        .help("Enable capitalization and punctuation.");
    parser.add_argument({ "-S", "--single-utterance"}).action("store_true")
        .help("Enable single utterance mode.");
    parser.add_argument({ "-Vs", "--vad-sensitivity"})
        .help("How sensitive the voice activity detector should be when single utterance mode is enabled.")
        .default_value("LOW");
    parser.add_argument({ "-Vd", "--vad-duration"})
        .help("The number of seconds of silence to detect before automatically ending the stream when single utterance mode is enabled.")
        .default_value(1.f);
    parser.add_argument({ "-CV", "--custom-vocabulary"})
        .help("An optional set of custom vocab words as a list of comma de-limited strings, e.g.,\n\t\t\t-CV \"<WORD 1>,<SOUNDS LIKE 1>,<SOUNDS LIKE 2>\" \"<WORD 2>,<SOUNDS LIKE 3>\"")
        .nargs("+");
    parser.add_argument({ "-CVs", "--custom-vocabulary-sensitivity"})
        .help("How aggressive the word replacement should be when using a custom vocabulary.")
        .default_value("MEDIUM");
    parser.add_argument({ "-CVid", "--custom-vocabulary-id"})
        .help("An optional ID of a server-side custom vocabulary list to use.");
    parser.add_argument({ "-l", "--language" }).required(true)
        .help("The IETF BCP 47 language tag for the input audio (e.g., en-US).");
    parser.add_argument({ "-C", "--chunksize" })
        .help("The number of audio samples per message; 0 to stream all samples in one message (default 4096).")
        .default_value(4096);
    parser.add_argument({ "-p", "--padding" })
        .help("The number of milliseconds of padding to append to the audio buffer (default 600).")
        .default_value(600);
    parser.add_argument({ "-v", "--verbose" }).action("store_true")
        .help("Produce verbose output during transcription.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto INPUT_FILE = args.get<std::string>("input");
    const auto OUTPUT_FILE = args.get<std::string>("output");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    const auto CAPITALIZATION_PUNCTUATION = args.get<bool>("capitalization-punctuation");
    const auto SINGLE_UTTERANCE = args.get<bool>("single-utterance");
    ThresholdSensitivity VAD_SENSITIVITY;
    if (args.get<std::string>("vad-sensitivity") == "LOW")
        VAD_SENSITIVITY = ThresholdSensitivity::LOW;
    else if (args.get<std::string>("vad-sensitivity") == "MEDIUM")
        VAD_SENSITIVITY = ThresholdSensitivity::MEDIUM;
    else if (args.get<std::string>("vad-sensitivity") == "HIGH")
        VAD_SENSITIVITY = ThresholdSensitivity::HIGH;
    else if (args.get<std::string>("vad-sensitivity") == "HIGHEST")
        VAD_SENSITIVITY = ThresholdSensitivity::HIGHEST;
    const auto VAD_DURATION = args.get<float>("vad-duration");
    const auto CUSTOM_VOCAB = args.get<std::vector<std::string>>("--custom-vocabulary");
    ThresholdSensitivity CUSTOM_VOCAB_SENSITIVITY;
    if (args.get<std::string>("custom-vocabulary-sensitivity") == "LOW")
        CUSTOM_VOCAB_SENSITIVITY = ThresholdSensitivity::LOW;
    else if (args.get<std::string>("custom-vocabulary-sensitivity") == "MEDIUM")
        CUSTOM_VOCAB_SENSITIVITY = ThresholdSensitivity::MEDIUM;
    else if (args.get<std::string>("custom-vocabulary-sensitivity") == "HIGH")
        CUSTOM_VOCAB_SENSITIVITY = ThresholdSensitivity::HIGH;
    else if (args.get<std::string>("custom-vocabulary-sensitivity") == "HIGHEST")
        CUSTOM_VOCAB_SENSITIVITY = ThresholdSensitivity::HIGHEST;
    const auto CUSTOM_VOCAB_ID = args.get<std::string>("custom-vocabulary-id");
    const auto LANGUAGE = args.get<std::string>("language");
    const auto CHUNK_SIZE = args.get<int>("chunksize");
    const auto PADDING = args.get<float>("padding");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create a credential store for keeping OAuth credentials in.
    FileSystemCredentialStore keychain(".", "com.sensory.cloud.examples");

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

    // Load the audio file.
    AudioBuffer buffer;
    buffer.load(INPUT_FILE);
    // Check that the file is 16kHz.
    if (buffer.get_sample_rate() != 16000) {
        std::cout << "Attempting to load WAV file with sample rate of "
            << buffer.get_sample_rate() << "Hz, but only 16000Hz audio is supported."
            << std::endl;
        return 1;
    }
    // Check that the file in monophonic.
    if (buffer.get_channels() > 1) {
        std::cout << "Attempting to load WAV file with "
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
    // Create the config with the transcription parameters.
    auto transcribe_config = new sensory::api::v1::audio::TranscribeConfig;
    transcribe_config->set_modelname(MODEL);
    transcribe_config->set_userid(USER_ID);
    transcribe_config->set_enablepunctuationcapitalization(CAPITALIZATION_PUNCTUATION);
    transcribe_config->set_dosingleutterance(SINGLE_UTTERANCE);
    transcribe_config->set_vadsensitivity(VAD_SENSITIVITY);
    transcribe_config->set_vadduration(VAD_DURATION);
    if (not CUSTOM_VOCAB.empty()) {  // Custom vocab requires at least 1 word
        auto custom_vocab = new sensory::api::v1::audio::CustomVocabularyWords;
        for (const auto& alternative_pronunciation : CUSTOM_VOCAB)
            custom_vocab->add_words(alternative_pronunciation);
        transcribe_config->set_allocated_customwordlist(custom_vocab);
    }
    transcribe_config->set_customvocabrewardthreshold(CUSTOM_VOCAB_SENSITIVITY);
    transcribe_config->set_customvocabularyid(CUSTOM_VOCAB_ID);

    // Initialize the stream with the cloud.
    AudioBufferReactor reactor(buffer.get_samples(),
        buffer.get_channels(),
        buffer.get_sample_rate(),
        CHUNK_SIZE > 0 ? CHUNK_SIZE : buffer.get_num_samples(),
        VERBOSE
    );
    cloud.audio.transcribe(&reactor, audio_config, transcribe_config);
    reactor.StartCall();

    // Wait for the call to terminate and check the final status.
    status = reactor.await();
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Transcription stream broke (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }

    if (OUTPUT_FILE.empty()) {  // No output file, write to standard output.
        std::cout << reactor.get_transcript() << std::endl;
    } else {  // Write the results to the given filename.
        std::ofstream output_file(OUTPUT_FILE, std::ofstream::out);
        output_file << reactor.get_transcript() << std::endl;
        output_file.close();
    }

    return 0;
}
