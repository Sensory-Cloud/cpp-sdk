// An example of a voice assistant application based on Sensory Cloud services.
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

/// @brief A bidirection stream reactor for validating triggers from audio
/// stream data.
///
/// @details
/// Input data for the stream is provided by a PortAudio capture device.
///
class ValidateEventReactor :
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
    /// The buffer for the block of samples from the port audio input device.
    std::unique_ptr<uint8_t> sampleBlock;
    /// A flag determining whether the trigger was detected.
    std::atomic<bool> didTrigger;

 public:
    /// @brief Initialize a reactor for streaming audio from a PortAudio stream.
    ///
    /// @param capture_ The PortAudio capture device.
    /// @param numChannels_ The number of channels in the input stream.
    /// @param sampleSize_ The number of bytes in an individual frame.
    /// @param sampleRate_ The sampling rate of the audio stream.
    /// @param framesPerBlock_ The number of frames in a block of audio.
    ///
    ValidateEventReactor(PaStream* capture_,
        uint32_t numChannels_ = 1,
        uint32_t sampleSize_ = 2,
        uint32_t sampleRate_ = 16000,
        uint32_t framesPerBlock_ = 4096
    ) :
        AudioService<InsecureCredentialStore>::ValidateEventBidiReactor(),
        capture(capture_),
        numChannels(numChannels_),
        sampleSize(sampleSize_),
        sampleRate(sampleRate_),
        framesPerBlock(framesPerBlock_),
        sampleBlock(static_cast<uint8_t*>(malloc(framesPerBlock_ * numChannels_ * sampleSize_))),
        didTrigger(false) {
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
        if (!didTrigger)  // Send the data to the server to validate the audio event.
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
        // std::cout << "Response" << std::endl;
        // std::cout << "\tAudio Energy: " << response.audioenergy() << std::endl;
        // std::cout << "\tSuccess:      " << response.success()     << std::endl;
        // std::cout << "\tResult ID:    " << response.resultid()    << std::endl;
        // std::cout << "\tScore:        " << response.score()       << std::endl;
        if (response.success()) {  // Flag the trigger and stop reading messages.
            didTrigger = true;
            StartWritesDone();
        } else {  // Start the next read request.
            StartRead(&response);
        }
    }

    /// @brief Return a boolean determining whether the wake-word triggered.
    ///
    /// @returns `true` if the wake-word was detected in the audio stream,
    /// `false` otherwise.
    ///
    inline bool getDidTrigger() const { return didTrigger; }
};

/// @brief A bidirection stream reactor for transcribing text from audio
/// stream data.
///
/// @details
/// Input data for the stream is provided by a PortAudio capture device.
///
class AudioTranscriptionReactor :
    public AudioService<InsecureCredentialStore>::TranscribeBidiReactor {
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
    /// The buffer for the block of samples from the port audio input device.
    std::unique_ptr<uint8_t> sampleBlock;
    /// The number of blocks that have been written to the server.
    std::atomic<uint32_t> blocks_written;
    /// A boolean determining whether the audio transcription has finished.
    std::atomic<bool> isFinishedTranscribing;

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
    AudioTranscriptionReactor(PaStream* capture_,
        uint32_t numChannels_ = 1,
        uint32_t sampleSize_ = 2,
        uint32_t sampleRate_ = 16000,
        uint32_t framesPerBlock_ = 4096,
        float duration_ = 60
    ) :
        AudioService<InsecureCredentialStore>::TranscribeBidiReactor(),
        capture(capture_),
        numChannels(numChannels_),
        sampleSize(sampleSize_),
        sampleRate(sampleRate_),
        framesPerBlock(framesPerBlock_),
        duration(duration_),
        sampleBlock(static_cast<uint8_t*>(malloc(framesPerBlock_ * numChannels_ * sampleSize_))),
        blocks_written(0),
        isFinishedTranscribing(false) {
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
        if (++blocks_written > (duration * sampleRate) / framesPerBlock || isFinishedTranscribing)
            StartWritesDone();
        else  // Send the data to the server to transcribe the audio.
            StartWrite(&request);
    }

    /// @brief React to a _read done_ event.
    ///
    /// @param ok whether the read succeeded.
    ///
    void OnReadDone(bool ok) override {
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        // Log the current transcription to the terminal.
        // std::cout << "Response" << std::endl;
        // std::cout << "\tAudio Energy: " << response.audioenergy()     << std::endl;
        // std::cout << "\tTranscript:   " << response.transcript()      << std::endl;
        // std::cout << "\tIs Partial:   " << response.ispartialresult() << std::endl;
        if (!response.ispartialresult()) {  // Log the fully transcribed result.
            std::cout << response.transcript() << std::endl;
            isFinishedTranscribing = true;
        } else {  // Start the next read request
            StartRead(&response);
        }
    }
};

int main(int argc, const char** argv) {
    // Create an insecure credential store for keeping OAuth credentials in.
    sensory::token_manager::InsecureCredentialStore keychain(".", "com.sensory.cloud.examples");
    if (!keychain.contains("deviceID"))
        keychain.emplace("deviceID", sensory::token_manager::uuid_v4());
    const auto DEVICE_ID(keychain.at("deviceID"));

    // Initialize the configuration to the host for given address and port
    sensory::Config config(
        "io.stage.cloud.sensory.com",
        443,
        "cabb7700-206f-4cc7-8e79-cd7f288aa78d",
        DEVICE_ID
    );

    // Query the health of the remote service.
    HealthService healthService(config);
    // Query the health of the remote service.
    healthService.getHealth([](HealthService::GetHealthCallData* call) {
        if (!call->getStatus().ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get server health with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
        }
        // Report the health of the remote service
        std::cout << "Server status" << std::endl;
        std::cout << "\tIs Healthy:     " << call->getResponse().ishealthy()     << std::endl;
        std::cout << "\tServer Version: " << call->getResponse().serverversion() << std::endl;
        std::cout << "\tID:             " << call->getResponse().id()            << std::endl;
    })->await();

    // Query the user ID
    std::string userID = "ckckck";
    // std::cout << "user ID: ";
    // std::cin >> userID;

    // Create an OAuth service
    OAuthService oauthService(config);
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

    // std::cout << "Available audio models:" << std::endl;
    // audioService.getModels([](AudioService<InsecureCredentialStore>::GetModelsCallData* call) {
    //     if (!call->getStatus().ok()) {  // The call failed.
    //         std::cout << "Failed to get audio models with\n\t" <<
    //             call->getStatus().error_code() << ": " <<
    //             call->getStatus().error_message() << std::endl;
    //     }
    //     // Iterate over the models returned in the response
    //     for (auto& model : call->getResponse().models()) {
    //         // Ignore models that aren't face biometric models.
    //         if (model.modeltype() != sensory::api::common::VOICE_EVENT_WAKEWORD)
    //             continue;
    //         std::cout << "\t" << model.name() << std::endl;
    //     }
    // })->await();

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

    while (true) {
        // Create the gRPC reactor to respond to streaming events.
        ValidateEventReactor validateEventReactor(capture,
            NUM_CHANNELS,
            SAMPLE_SIZE,
            SAMPLE_RATE,
            FRAMES_PER_BLOCK
        );
        // Initialize the stream with the reactor for callbacks, given audio
        // model, the sample rate of the audio and the expected language. A
        // user ID is also necessary to detect audio events.
        audioService.validateEvent(&validateEventReactor,
            sensory::service::audio::newAudioConfig(
                sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
                SAMPLE_RATE, 1, "en-US"
            ),
            sensory::service::audio::newValidateEventConfig(
                "wakeword-16kHz-alexa.trg",
                userID,
                sensory::api::v1::audio::ThresholdSensitivity::HIGHEST
            )
        );
        validateEventReactor.StartCall();
        auto status = validateEventReactor.await();

        if (!status.ok()) {  // The call failed, print a descriptive message.
            std::cout << "Wake-word stream broke with\n\t" <<
                status.error_code() << ": " << status.error_message() << std::endl;
            return 1;
        }

        if (!validateEventReactor.getDidTrigger()) continue;

        std::cout << "yes?" << std::endl;

        // Create the gRPC reactor to respond to streaming events.
        AudioTranscriptionReactor reactor(capture,
            NUM_CHANNELS,
            SAMPLE_SIZE,
            SAMPLE_RATE,
            FRAMES_PER_BLOCK,
            DURATION
        );
        // Initialize the stream with the reactor for callbacks, given audio model,
        // the sample rate of the audio and the expected language. A user ID is also
        // necessary to transcribe audio.
        audioService.transcribe(&reactor,
            sensory::service::audio::newAudioConfig(
                sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
                SAMPLE_RATE, 1, "en-US"
            ),
            sensory::service::audio::newTranscribeConfig("speech_recognition_en", userID)
        );
        reactor.StartCall();
        status = reactor.await();

        if (!status.ok()) {  // The call failed, print a descriptive message.
            std::cout << "Transcription stream broke with\n\t" <<
                status.error_code() << ": " << status.error_message() << std::endl;
            return 1;
        }
    }

    // Stop the audio stream.
    err = Pa_StopStream(capture);
    if (err != paNoError) return describe_pa_error(err);

    // Terminate the port audio session.
    Pa_Terminate();



    return 0;
}
