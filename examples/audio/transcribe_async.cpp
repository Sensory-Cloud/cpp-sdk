// An example of audio transcription based on PortAudio asynchronous streams.
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
#include <thread>
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/health_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>
#include <sensorycloud/services/management_service.hpp>
#include <sensorycloud/services/audio_service.hpp>
#include <sensorycloud/token_manager/insecure_credential_store.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>
#include "dep/argparse.hpp"

using sensory::token_manager::TokenManager;
using sensory::token_manager::InsecureCredentialStore;
using sensory::service::HealthService;
using sensory::service::AudioService;
using sensory::service::OAuthService;

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
    config.connect();

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
    OAuthService oauthService(config);
    sensory::token_manager::TokenManager<sensory::token_manager::InsecureCredentialStore>
        tokenManager(oauthService, keychain);

    if (!tokenManager.hasToken()) {  // the device is not registered
        // Generate a new clientID and clientSecret for this device
        const auto credentials = tokenManager.hasSavedCredentials() ?
            tokenManager.getSavedCredentials() : tokenManager.generateCredentials();

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
        oauthService.registerDevice(
            name, password, credentials.id, credentials.secret,
            [](OAuthService::RegisterDeviceCallData* call) {
            if (!call->getStatus().ok()) {  // The call failed.
                std::cout << "Failed to register device with\n\t" <<
                    call->getStatus().error_code() << ": " <<
                    call->getStatus().error_message() << std::endl;
            }
        })->await();
    }

    // ------ Create the audio service -----------------------------------------

    // Create the audio service based on the configuration and token manager.
    AudioService<InsecureCredentialStore> audioService(config, tokenManager);

    // ------ Query the available audio models ---------------------------------

    if (GETMODELS) {
        int errCode = 0;
        audioService.getModels([&errCode](AudioService<InsecureCredentialStore>::GetModelsCallData* call) {
            if (!call->getStatus().ok()) {  // The call failed.
                std::cout << "Failed to get audio models with\n\t" <<
                    call->getStatus().error_code() << ": " <<
                    call->getStatus().error_message() << std::endl;
                errCode = 1;
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
        return errCode;
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

    /// Tagged events in the CompletionQueue handler.
    enum class Events {
        /// The `Write` event for sending data up to the server.
        Write = 1,
        /// The `Read` event for receiving messages from the server.
        Read = 2,
        /// The `WritesDone` event indicating that no more data will be sent up.
        WritesDone = 3,
        /// The `Finish` event indicating that the stream has terminated.
        Finish = 4
    };

    // Start an asynchronous RPC to fetch the names of the available models. The
    // RPC will use the grpc::CompletionQueue as an event loop.
    grpc::CompletionQueue queue;
    auto stream = audioService.transcribe(&queue,
        sensory::service::audio::newAudioConfig(
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
            SAMPLE_RATE, NUM_CHANNELS, LANGUAGE
        ),
        sensory::service::audio::newTranscribeConfig(MODEL, USER_ID),
        nullptr,
        (void*) Events::Finish
    );

    // start the stream event thread in the background to handle events.
    std::thread audioThread([&stream, &queue, &VERBOSE](){
        // The number of audio blocks written for detecting expiration of the
        // stream.
        uint32_t blocks_written = 0;
        // The sample block of audio.
        std::unique_ptr<uint8_t> sampleBlock(static_cast<uint8_t*>(malloc(BYTES_PER_BLOCK)));

        // Initialize the PortAudio driver.
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

        // Open the PortAudio stream with the input device.
        PaStream* capture;
        err = Pa_OpenStream(&capture,
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
        err = Pa_StartStream(capture);
        if (err != paNoError) return describe_pa_error(err);

        void* tag(nullptr);
        bool ok(false);
        while (queue.Next(&tag, &ok)) {
            if (!ok) continue;
            if (tag == stream) {
                // Respond to the start of stream succeeding. All Sensory Cloud
                // AV streams require a configuration message to be sent to the
                // server that provides information about the stream. This
                // information is generated by the SDK when the stream is
                // created, but cannot be sent until the stream is initialized.
                // By calling `Write` with the request attached to the call, we
                // send this first configuration message to the server. The
                // request object in the call can then be re-used for image data
                // in other tag branches. Tag writes and reads uniquely such
                // that they can be handled by different branches of this event
                // loop.
                stream->getCall()->Write(stream->getRequest(), (void*) Events::Write);
                stream->getCall()->Read(&stream->getResponse(), (void*) Events::Read);
            } else if (tag == (void*) Events::Write) {  // Respond to a write event.
                // If the time has expired, close the stream.
                if (blocks_written++ > (DURATION * SAMPLE_RATE) / CHUNK_SIZE) {
                    stream->getCall()->WritesDone((void*) Events::WritesDone);
                    continue;
                }
                // Read a block of samples from the ADC.
                auto err = Pa_ReadStream(capture, sampleBlock.get(), CHUNK_SIZE);
                if (err) {
                    describe_pa_error(err);
                    break;
                }
                // Set the audio content for the request and start the write request
                stream->getRequest().set_audiocontent(sampleBlock.get(), BYTES_PER_BLOCK);
                stream->getCall()->Write(stream->getRequest(), (void*) Events::Write);
            } else if (tag == (void*) Events::Read) {  // Respond to a read event.
                if (VERBOSE) {
                    std::cout << "Response" << std::endl;
                    std::cout << "\tAudio Energy: " << stream->getResponse().audioenergy()     << std::endl;
                    std::cout << "\tTranscript:   " << stream->getResponse().transcript()      << std::endl;
                    std::cout << "\tIs Partial:   " << stream->getResponse().ispartialresult() << std::endl;
                } else {
                    #if defined(_WIN32) || defined(_WIN64)  // Windows
                        std::system("clr");
                    #else
                        std::system("clear");
                    #endif
                    std::cout << stream->getResponse().transcript() << std::endl;
                }
                stream->getCall()->Read(&stream->getResponse(), (void*) Events::Read);
            } else if (tag == (void*) Events::Finish) break;
        }

        // Stop the audio stream.
        err = Pa_StopStream(capture);
        if (err != paNoError) return describe_pa_error(err);

        // Terminate the port audio session.
        Pa_Terminate();

        return 0;
    });

    // Wait for the audio thread to join back in.
    audioThread.join();

    if (!stream->getStatus().ok()) {
        std::cout << "Wake word stream broke with\n\t" <<
            stream->getStatus().error_code() << ": " <<
            stream->getStatus().error_message() << std::endl;
    }

    delete stream;

    return 0;
}
