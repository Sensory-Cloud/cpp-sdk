// An example of audio transcription using SensoryCloud with PortAudio.
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

#include <portaudio.h>
#include <iostream>
#include <thread>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include "../dep/argparse.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::util::TranscriptAggregator;
using sensory::api::v1::audio::WordState;
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
        .prog("transcribe")
        .description("A tool for streaming audio files to SensoryCloud for audio transcription.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-g", "--getmodels" })
        .action("store_true")
        .help("Whether to query for a list of available models.");
    parser.add_argument({ "-m", "--model" })
        .help("The name of the transcription model to use.");
    parser.add_argument({ "-u", "--userid" })
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
    parser.add_argument({ "-L", "--language" })
        .help("The IETF BCP 47 language tag for the input audio (e.g., en-US).");
    // parser.add_argument({ "-C", "--chunksize" })
    //     .help("The number of audio samples per message (default 4096).")
    //     .default_value("4096");
    // parser.add_argument({ "-S", "--samplerate" })
    //     .help("The audio sample rate of the input stream.")
    //     .choices({"9600", "11025", "12000", "16000", "22050", "24000", "32000", "44100", "48000", "88200", "96000", "192000"})
    //     .default_value("16000");
    parser.add_argument({ "-v", "--verbose" }).action("store_true")
        .help("Produce verbose output during transcription.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto GETMODELS = args.get<bool>("getmodels");
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
    const uint32_t CHUNK_SIZE = 4096;//args.get<int>("chunksize");
    const auto SAMPLE_RATE = 16000;//args.get<uint32_t>("samplerate");
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
            std::cout << "Failed to get audio models ("
                << status.error_code() << "): "
                << status.error_message() << std::endl;
            return 1;
        }
        for (auto& model : audioModelsResponse.models()) {
            if (model.modeltype() != sensory::api::common::VOICE_TRANSCRIBE_COMMAND_AND_SEARCH)
                continue;
            std::cout << model.name() << std::endl;
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
    grpc::ClientContext context;
    auto stream = cloud.audio.transcribe(&context, audio_config, transcribe_config);

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

    // Create a thread to poll read requests in the background. Audio
    // transcription has a bursty response pattern, so a locked read-write loop
    // will not work with this service.
    std::thread receipt_thread([&stream, &VERBOSE](){
        /// An aggregator for accumulating partial updates into a transcript.
        TranscriptAggregator aggregator;
        while (true) {
            // Read a message and break out of the loop if the read fails.
            sensory::api::v1::audio::TranscribeResponse response;
            if (!stream->Read(&response)) break;
            // Set the content of the local transcript buffer.
            aggregator.process_response(response.wordlist());
            // Log the current transcription to the terminal.
            if (VERBOSE) {
                // Relative energy of the processed audio as a value between 0 and 1.
                // Can be converted to decibels in (-inf, 0] using 20 * log10(x).
                std::cout << "Audio Energy: " << response.audioenergy() << std::endl;
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
        }
    });

    // Create a buffer for the audio samples based on the number of bytes in
    // a block of samples.
    uint8_t sample_block[BYTES_PER_BLOCK];
    for (int i = 0; i < (DURATION * SAMPLE_RATE) / CHUNK_SIZE; ++i) {
        // Read a block of samples from the ADC.
        err = Pa_ReadStream(audioStream, sample_block, CHUNK_SIZE);
        if (err) return describe_pa_error(err);

        // Create a new validate event request with the audio content.
        sensory::api::v1::audio::TranscribeRequest request;
        request.set_audiocontent(sample_block, BYTES_PER_BLOCK);
        // Send the data to the server to validate the trigger.
        if (!stream->Write(request)) break;
    }

    // Stop the background receipt process and join the thread back in.
    receipt_thread.join();

    // Close the stream and check the status code in case the stream broke.
    stream->WritesDone();
    status = stream->Finish();

    // Stop the audio stream.
    err = Pa_StopStream(audioStream);
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
