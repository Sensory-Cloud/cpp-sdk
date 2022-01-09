// An example of wake word triggers based on PortAudio asynchronous input streams.
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

/// @brief A bidirection stream reactor for biometric enrollments from audio
/// stream data.
///
/// @details
/// Input data for the stream is provided by a PortAudio capture device.
///
class PortAudioReactor :
    public AudioService<InsecureCredentialStore>::ValidateEventBidiReactor {
 private:
    /// The capture device that input audio is streaming in from.
    PaStream* capture;
    /// The number of channels in the input audio.
    uint32_t numChannels;
    /// The number of bytes per audio sample (i.e., 2 for 16-bit audio)
    uint32_t sampleSize;
    /// The sample rate of the audio input stream.
    uint32_t sampleRate;
    /// The number of frames per block of audio.
    uint32_t framesPerBlock;
    /// The maximum duration of the stream in seconds.
    float duration;
    /// Whether to produce verbose output from the reactor.
    bool verbose = false;
    /// The buffer for the block of samples from the port audio input device.
    std::unique_ptr<uint8_t> sampleBlock;
    /// The number of blocks that have been written to the server.
    std::atomic<uint32_t> blocks_written;

 public:
    /// @brief Initialize a reactor for streaming audio from a PortAudio stream.
    ///
    /// @param capture_ The PortAudio capture device.
    /// @param numChannels_ The number of channels in the input stream.
    /// @param sampleSize_ The number of bytes in an individual frame.
    /// @param sampleRate_ The sampling rate of the audio stream.
    /// @param framesPerBlock_ The number of frames in a block of audio.
    /// @param duration_ the maximum duration for the audio capture
    ///
    PortAudioReactor(PaStream* capture_,
        uint32_t numChannels_ = 1,
        uint32_t sampleSize_ = 2,
        uint32_t sampleRate_ = 16000,
        uint32_t framesPerBlock_ = 4096,
        float duration_ = 60,
        const bool& verbose_ = false
    ) :
        AudioService<InsecureCredentialStore>::ValidateEventBidiReactor(),
        capture(capture_),
        numChannels(numChannels_),
        sampleSize(sampleSize_),
        sampleRate(sampleRate_),
        framesPerBlock(framesPerBlock_),
        duration(duration_),
        verbose(verbose_),
        sampleBlock(static_cast<uint8_t*>(malloc(framesPerBlock_ * numChannels_ * sampleSize_))),
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
        // Read a block of samples from the ADC.
        auto err = Pa_ReadStream(capture, sampleBlock.get(), framesPerBlock);
        if (err) {
            describe_pa_error(err);
            return;
        }
        // Set the audio content for the request and start the write request
        request.set_audiocontent(sampleBlock.get(), numChannels * framesPerBlock * sampleSize);
        // If the number of blocks written surpasses the maximal length, close
        // the stream.
        if (++blocks_written > (duration * sampleRate) / framesPerBlock)
            StartWritesDone();
        else  // Send the data to the server to validate the audio event.
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
        .description("A tool for streaming audio files to Sensory Cloud for audio event validation.");
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
        .help("MODEL The name of the event validation model to use.");
    parser.add_argument({ "-u", "--userid" })
        .help("USERID The name of the user ID for the event validation.");
    parser.add_argument({ "-t", "--threshold" })
        .choices({"LOW", "MEDIUM", "HIGH", "HIGHEST"})
        .default_value("HIGH")
        .help("THRESHOLD The sensitivity threshold for detecting audio events.");
    parser.add_argument({ "-L", "--language" })
        .help("LANGUAGE The IETF BCP 47 language tag for the input audio (e.g., en-US).");
    // parser.add_argument({ "-C", "--chunksize" })
    //     .help("CHUNKSIZE The number of audio samples per message; 0 to stream all samples in one message (default).")
    //     .default_value(4096);
    parser.add_argument({ "-v", "--verbose" }).action("store_true")
        .help("VERBOSE Produce verbose output during event validation.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto HOSTNAME = args.get<std::string>("host");
    const auto PORT = args.get<uint16_t>("port");
    const auto TENANT = args.get<std::string>("tenant");
    const auto IS_SECURE = !args.get<bool>("insecure");
    const auto GETMODELS = args.get<bool>("getmodels");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    sensory::api::v1::audio::ThresholdSensitivity THRESHOLD;
    if (args.get<std::string>("threshold") == "LOW")
        THRESHOLD = sensory::api::v1::audio::ThresholdSensitivity::LOW;
    else if (args.get<std::string>("threshold") == "MEDIUM")
        THRESHOLD = sensory::api::v1::audio::ThresholdSensitivity::MEDIUM;
    else if (args.get<std::string>("threshold") == "HIGH")
        THRESHOLD = sensory::api::v1::audio::ThresholdSensitivity::HIGH;
    else if (args.get<std::string>("threshold") == "HIGHEST")
        THRESHOLD = sensory::api::v1::audio::ThresholdSensitivity::HIGHEST;
    const auto LANGUAGE = args.get<std::string>("language");
    // const auto CHUNK_SIZE = args.get<int>("chunksize");
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
    OAuthService oauthService(config);
    sensory::token_manager::TokenManager<sensory::token_manager::InsecureCredentialStore>
        tokenManager(oauthService, keychain);

    if (!tokenManager.hasToken()) {  // the device is not registered
        // Generate a new clientID and clientSecret for this device
        const auto credentials = tokenManager.generateCredentials();

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
            name,
            password,
            credentials.id,
            credentials.secret,
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
                    if (model.modeltype() != sensory::api::common::VOICE_EVENT_WAKEWORD)
                        continue;
                    std::cout << model.name() << std::endl;
                }
            }
        })->await();
        return errCode;
    }

    // the maximal duration of the recording in seconds
    static constexpr auto DURATION = 60;
    // the sample rate of the input audio stream. This should match the sample
    // rate of the selected model
    static constexpr auto SAMPLE_RATE = 16000;
    // The number of input channels from the microphone. This should always be
    // mono.
    static constexpr auto NUM_CHANNELS = 1;
    // The size of the audio sample blocks, i.e., the number of samples to read
    // from the ADC per step and send to Sensory, Cloud.
    static constexpr auto FRAMES_PER_BLOCK = 4096;
    // The number of bytes per sample, for 16-bit audio, this is 2 bytes.
    static constexpr auto SAMPLE_SIZE = 2;
    // The number of bytes in a given chunk of samples.
    static constexpr auto BYTES_PER_BLOCK =
        FRAMES_PER_BLOCK * NUM_CHANNELS * SAMPLE_SIZE;

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
        FRAMES_PER_BLOCK,
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
        FRAMES_PER_BLOCK,
        DURATION,
        VERBOSE
    );
    // Initialize the stream with the reactor for callbacks, given audio model,
    // the sample rate of the audio and the expected language. A user ID is also
    // necessary to detect audio events.
    audioService.validateEvent(&reactor,
        sensory::service::newAudioConfig(
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
            SAMPLE_RATE, 1, LANGUAGE
        ),
        sensory::service::newValidateEventConfig(MODEL, USER_ID, THRESHOLD)
    );
    reactor.StartCall();
    status = reactor.await();

    // Stop the audio stream.
    err = Pa_StopStream(capture);
    if (err != paNoError) return describe_pa_error(err);

    // Terminate the port audio session.
    Pa_Terminate();

    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Wake-word stream broke with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
