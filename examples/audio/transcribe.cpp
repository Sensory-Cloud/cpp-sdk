// An example of audio transcription based on PortAudio blocking input streams.
//
// Author: Christian Kauten (ckauten@sensoryinc.com)
//
// Copyright (c) 2021 Sensory, Inc.
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
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/health_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>
#include <sensorycloud/services/management_service.hpp>
#include <sensorycloud/services/audio_service.hpp>
#include <sensorycloud/token_manager/insecure_credential_store.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>
#include "dep/argparse.hpp"

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
        .description("A tool for streaming audio files to Sensory Cloud for audio transcription.");
    parser.add_argument({ "-H", "--host" }).required(true)
        .help("HOST The hostname of a Sensory Cloud inference server.");
    parser.add_argument({ "-P", "--port" }).required(true)
        .help("PORT The port number that the Sensory Cloud inference server is running at.");
    parser.add_argument({ "-T", "--tenant" }).required(true)
        .help("TENANT The ID of your tenant on a Sensory Cloud inference server.");
    parser.add_argument({ "-I", "--insecure" }).action("store_true")
        .help("INSECURE Disable TLS.");
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
    const auto HOSTNAME = args.get<std::string>("host");
    const auto PORT = args.get<uint16_t>("port");
    const auto TENANT = args.get<std::string>("tenant");
    const auto IS_SECURE = !args.get<bool>("insecure");
    const auto GETMODELS = args.get<bool>("getmodels");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    const auto LANGUAGE = args.get<std::string>("language");
    const uint32_t CHUNK_SIZE = 4096;//args.get<int>("chunksize");
    const auto SAMPLE_RATE = 16000;//args.get<uint32_t>("samplerate");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create an insecure credential store for keeping OAuth credentials in.
    sensory::token_manager::InsecureCredentialStore keychain(".", "com.sensory.cloud.examples");
    if (!keychain.contains("deviceID"))
        keychain.emplace("deviceID", sensory::token_manager::uuid_v4());
    const auto DEVICE_ID(keychain.at("deviceID"));

    // Initialize the configuration for the service.
    sensory::Config config(HOSTNAME, PORT, TENANT, DEVICE_ID, IS_SECURE);

    // Query the health of the remote service.
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

    // Create an OAuth service
    sensory::service::OAuthService oauthService(config);
    sensory::token_manager::TokenManager<sensory::token_manager::InsecureCredentialStore>
        tokenManager(oauthService, keychain);

    if (!tokenManager.hasToken()) {  // the device is not registered
        // Generate a new clientID and clientSecret for this device
        const auto credentials = tokenManager.generateCredentials();

        std::cout << "Registering device with server..." << std::endl;

        // Query the friendly device name
        std::string name = "";
        std::cout << "Device Name: ";
        std::cin >> name;

        // Query the shared pass-phrase
        std::string password = "";
        std::cout << "password: ";
        std::cin >> password;

        // Register this device with the remote host
        sensory::api::v1::management::DeviceResponse registerResponse;
        auto status = oauthService.registerDevice(&registerResponse,
            name, password, credentials.id, credentials.secret);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to register device with\n\t" <<
                status.error_code() << ": " << status.error_message() << std::endl;
            return 1;
        }
    }

    // ------ Create the audio service -----------------------------------------

    sensory::service::AudioService<sensory::token_manager::InsecureCredentialStore>
        audioService(config, tokenManager);

    // ------ Query the available audio models ---------------------------------

    if (GETMODELS) {
        sensory::api::v1::audio::GetModelsResponse audioModelsResponse;
        status = audioService.getModels(&audioModelsResponse);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get audio models with\n\t" <<
                status.error_code() << ": " << status.error_message() << std::endl;
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
    const auto BYTES_PER_BLOCK =
        CHUNK_SIZE * NUM_CHANNELS * SAMPLE_SIZE;

    // Create the network stream
    auto stream = audioService.transcribe(
        sensory::service::newAudioConfig(
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
            SAMPLE_RATE, NUM_CHANNELS, LANGUAGE
        ),
        sensory::service::newTranscribeConfig(MODEL, USER_ID)
    );

    // Initialize the portaudio driver.
    PaError err = paNoError;
    err = Pa_Initialize();
    if (err != paNoError) return describe_pa_error(err);

    // Setup the input parameters for the port audio stream.
    PaStreamParameters inputParameters;
    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        return 1;
    }
    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = paInt16;  // Sensory expects 16-bit audio
    inputParameters.suggestedLatency =
        Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    // Open the portaudio stream with the input device.
    PaStream* audioStream;
    err = Pa_OpenStream(&audioStream,
        &inputParameters,
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
        while (true) {
            // Read a message and break out of the loop if the read fails.
            sensory::api::v1::audio::TranscribeResponse response;
            if (!stream->Read(&response)) break;
            // Log the current transcription to the terminal.
            if (VERBOSE) {
                std::cout << "Response" << std::endl;
                std::cout << "\tAudio Energy: " << response.audioenergy()     << std::endl;
                std::cout << "\tTranscript:   " << response.transcript()      << std::endl;
                std::cout << "\tIs Partial:   " << response.ispartialresult() << std::endl;
            } else {
                #if defined(_WIN32) || defined(_WIN64)  // Windows
                    std::system("clr");
                #else
                    std::system("clear");
                #endif
                std::cout << response.transcript() << std::endl;
            }
        }
    });

    // Create a buffer for the audio samples based on the number of bytes in
    // a block of samples.
    uint8_t sampleBlock[BYTES_PER_BLOCK];
    for (int i = 0; i < (DURATION * SAMPLE_RATE) / CHUNK_SIZE; ++i) {
        // Read a block of samples from the ADC.
        err = Pa_ReadStream(audioStream, sampleBlock, CHUNK_SIZE);
        if (err) return describe_pa_error(err);

        // Create a new validate event request with the audio content.
        sensory::api::v1::audio::TranscribeRequest request;
        request.set_audiocontent(sampleBlock, BYTES_PER_BLOCK);
        // Send the data to the server to validate the trigger.
        if (!stream->Write(request)) break;
    }

    // Stop the background receipt process and join the thread back in.
    receipt_thread.join();

    // Close the stream and check the status code in case the stream broke.
    stream->WritesDone();
    status = stream->Finish();
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Transcription stream broke with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
    }

    // Stop the audio stream.
    err = Pa_StopStream(audioStream);
    if (err != paNoError) return describe_pa_error(err);

    // Terminate the port audio session.
    Pa_Terminate();

    return 0;
}
