// An example of biometric voice enrollment using SensoryCloud with PortAudio.
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
#include <google/protobuf/util/time_util.h>
#include <iostream>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/insecure_credential_store.hpp>
#include "../dep/argparse.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::InsecureCredentialStore;

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
        .prog("enroll")
        .description("A tool for authenticating with voice biometrics using SensoryCloud.");
    parser.add_argument({ "path" })
        .help("PATH The path to an INI file containing server metadata.");
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
    // parser.add_argument({ "-C", "--chunksize" })
    //     .help("CHUNKSIZE The number of audio samples per message (default 4096).")
    //     .default_value("4096");
    // parser.add_argument({ "-S", "--samplerate" })
    //     .help("SAMPLERATE The audio sample rate of the input stream.")
    //     .choices({"9600", "11025", "12000", "16000", "22050", "24000", "32000", "44100", "48000", "88200", "96000", "192000"})
    //     .default_value("16000");
    parser.add_argument({ "-v", "--verbose" })
        .action("store_true")
        .help("VERBOSE Produce verbose output during authentication.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto GETMODELS = args.get<bool>("getmodels");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    const auto DESCRIPTION = args.get<std::string>("description");
    const auto LIVENESS = args.get<bool>("liveness");
    const auto NUM_UTTERANCES = args.get<uint32_t>("numutterances");
    const auto DURATION = args.get<float>("duration");
    const auto LANGUAGE = args.get<std::string>("language");
    const uint32_t CHUNK_SIZE = 4096;//args.get<int>("chunksize");
    const auto SAMPLE_RATE = 16000;//args.get<uint32_t>("samplerate");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create an insecure credential store for keeping OAuth credentials in.
    InsecureCredentialStore keychain(".", "com.sensory.cloud.examples");
    // Create the cloud services handle.
    SensoryCloud<InsecureCredentialStore> cloud(PATH, keychain);

    // Query the health of the remote service.
    sensory::api::common::ServerHealthResponse server_health;
    auto status = cloud.health.getHealth(&server_health);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }
    if (VERBOSE) {
        std::cout << "Server status:" << std::endl;
        std::cout << "\tisHealthy: " << server_health.ishealthy() << std::endl;
        std::cout << "\tserverVersion: " << server_health.serverversion() << std::endl;
        std::cout << "\tid: " << server_health.id() << std::endl;
    }

    if (GETMODELS) {
        sensory::api::v1::audio::GetModelsResponse audioModelsResponse;
        status = cloud.audio.getModels(&audioModelsResponse);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get audio models ("
                << status.error_code() << "): "
                << status.error_message() << std::endl;
            return 1;
        }
        for (auto& model : audioModelsResponse.models()) {
            if (model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_DEPENDENT &&
                model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_INDEPENDENT &&
                model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_WAKEWORD
            ) continue;
            std::cout << model.name() << std::endl;
        }
        return 0;
    }

    // the maximal duration of the recording in seconds
    const auto MAX_DURATION = 60;
    // The number of input channels from the microphone. This should always be
    // mono.
    const auto NUM_CHANNELS = 1;
    // The number of bytes per sample, for 16-bit audio, this is 2 bytes.
    const auto SAMPLE_SIZE = 2;
    // The number of bytes in a given chunk of samples.
    const auto BYTES_PER_BLOCK =
        CHUNK_SIZE * NUM_CHANNELS * SAMPLE_SIZE;

    // Initialize the stream for creating the enrollment.
    grpc::ClientContext context;
    auto stream = cloud.audio.createEnrollment(&context,
        sensory::service::audio::new_audio_config(
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
            SAMPLE_RATE, 1, LANGUAGE
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

    // Initialize the portaudio driver.
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

    // Open the portaudio stream with the input device.
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
    for (int i = 0; i < (MAX_DURATION * SAMPLE_RATE) / CHUNK_SIZE; ++i) {
        // Read a block of samples from the ADC.
        err = Pa_ReadStream(audioStream, sample_block, CHUNK_SIZE);
        if (err) return describe_pa_error(err);

        // Create a new request with the audio content.
        sensory::api::v1::audio::CreateEnrollmentRequest request;
        request.set_audiocontent(sample_block, BYTES_PER_BLOCK);
        if (!stream->Write(request)) break;

        // Read a new response from the server.
        sensory::api::v1::audio::CreateEnrollmentResponse response;
        if (!stream->Read(&response)) break;

        // Log the result of the request to the terminal.
        if (VERBOSE) {  // Verbose output, dump the message to the terminal
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
            std::cout << std::endl;
            std::cout << "Successfully enrolled with ID: "
                << response.enrollmentid() << std::endl;
            break;
        }
    }

    // Close the stream and check the status code in case the stream broke.
    stream->WritesDone();
    status = stream->Finish();

    // Stop the audio stream.
    err = Pa_StopStream(audioStream);
    if (err != paNoError) return describe_pa_error(err);

    // Terminate the port audio session.
    Pa_Terminate();

    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Enrollment stream broke ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}