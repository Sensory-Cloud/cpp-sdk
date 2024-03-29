// An example of audio transcription based on audio file inputs.
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
#include <mutex>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include <sndfile.h>
#include "../dep/argparse.hpp"
#include "../dep/tqdm.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::util::TranscriptAggregator;
using sensory::api::v1::audio::WordState;
using sensory::api::v1::audio::ThresholdSensitivity;

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
    parser.add_argument({ "-i", "--input" })
        .help("The input audio file to stream to SensoryCloud.");
    parser.add_argument({ "-o", "--output" })
        .help("The output file to write the transcription to.");
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
    parser.add_argument({ "-Wm", "--wake-word-model"})
        .help("A wake-word model to use for event-triggered transcription.");
    parser.add_argument({ "-Ws", "--wake-word-sensitivity"})
        .help("The sensitivity level for detecting wake-words.")
        .choices({"LOW", "MEDIUM", "HIGH", "HIGHEST"})
        .default_value("LOW");
    parser.add_argument({ "-C", "--chunksize" })
        .help("The number of audio samples per message; 0 to stream all samples in one message (default 4096).")
        .default_value(4096);
    parser.add_argument({ "-off", "--offline"}).action("store_true")
        .help("Process data offline instead of in a real-time stream.");
    parser.add_argument({ "-v", "--verbose" }).action("store_true")
        .help("Produce verbose output during transcription.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto GETMODELS = args.get<bool>("getmodels");
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
    const auto WAKE_WORD_MODEL = args.get<std::string>("wake-word-model");
    ThresholdSensitivity WAKE_WORD_SENSITIVITY;
    if (args.get<std::string>("wake-word-sensitivity") == "LOW")
        WAKE_WORD_SENSITIVITY = ThresholdSensitivity::LOW;
    else if (args.get<std::string>("wake-word-sensitivity") == "MEDIUM")
        WAKE_WORD_SENSITIVITY = ThresholdSensitivity::MEDIUM;
    else if (args.get<std::string>("wake-word-sensitivity") == "HIGH")
        WAKE_WORD_SENSITIVITY = ThresholdSensitivity::HIGH;
    else if (args.get<std::string>("wake-word-sensitivity") == "HIGHEST")
        WAKE_WORD_SENSITIVITY = ThresholdSensitivity::HIGHEST;
    auto CHUNK_SIZE = args.get<int>("chunksize");
    const auto VERBOSE = args.get<bool>("verbose");
    const auto OFFLINE = args.get<bool>("offline");

    // Create a credential store for keeping OAuth credentials in.
    FileSystemCredentialStore keychain(".", "com.sensory.cloud.examples");

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

    // Query the available models
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
            if (model.modeltype() != sensory::api::common::VOICE_TRANSCRIBE_GRAMMAR)
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
    audio_config->set_languagecode("en");
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
    if (!WAKE_WORD_MODEL.empty()) {
        auto wake_word_config = new sensory::api::v1::audio::TranscribeEventConfig;
        wake_word_config->set_modelname(WAKE_WORD_MODEL);
        wake_word_config->set_sensitivity(WAKE_WORD_SENSITIVITY);
        transcribe_config->set_allocated_wakewordconfig(wake_word_config);
    }
    transcribe_config->set_doofflinemode(OFFLINE);

    grpc::ClientContext context;
    auto stream = cloud.audio.transcribe(&context, audio_config, transcribe_config);

    // Create a background thread for handling the transcription responses.
    std::thread receipt_thread([&stream, &OUTPUT_FILE, &VERBOSE](){
        /// An aggregator accumulates the partial updates into a transcript.
        TranscriptAggregator aggregator;
        while (true) {
            // Read a message and break out of the loop when the read fails.
            sensory::api::v1::audio::TranscribeResponse response;
            if (!stream->Read(&response)) break;
            aggregator.process_response(response.wordlist());
            if (!VERBOSE) continue;
            google::protobuf::util::JsonPrintOptions options;
            options.add_whitespace = false;
            options.always_print_primitive_fields = true;
            options.always_print_enums_as_ints = false;
            options.preserve_proto_field_names = true;
            std::string response_json;
            google::protobuf::util::MessageToJsonString(response, &response_json, options);
            std::cout << response_json << std::endl;
        }
        if (OUTPUT_FILE.empty()) {  // No output file, write to standard output.
            std::cout << aggregator.get_transcript() << std::endl;
        } else {  // Write the results to the given filename.
            std::ofstream output_file(OUTPUT_FILE, std::ofstream::out);
            output_file << aggregator.get_transcript() << std::endl;
            output_file.close();
        }
    });

    // If the chunk size is zero, disable chunking by setting the chunk size
    // to be equal to the number of samples.
    if (CHUNK_SIZE <= 0) CHUNK_SIZE = sfinfo.frames;

    // Pre-calculate the number of chunks to process for determining done-ness.
    auto num_chunks = sfinfo.frames / CHUNK_SIZE + (bool)(sfinfo.frames % CHUNK_SIZE);
    tqdm progress(num_chunks);
    int16_t samples[CHUNK_SIZE];
    for (int i = 0; i < num_chunks; i++) {
        auto num_frames = sf_read_short(infile, &samples[0], CHUNK_SIZE);
        sensory::api::v1::audio::TranscribeRequest request;
        request.set_audiocontent((uint8_t*) samples, sizeof(int16_t) * num_frames);
        // Detect the last chunk and write the post-processing action
        if (i == num_chunks - 1) {
            auto action = new ::sensory::api::v1::audio::AudioRequestPostProcessingAction;
            action->set_action(::sensory::api::v1::audio::FINAL);
            request.set_allocated_postprocessingaction(action);
            std::cout << "Audio uploaded, awaiting FINAL response..." << std::endl;
        }
        // Send the data to the server for transcription.
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
