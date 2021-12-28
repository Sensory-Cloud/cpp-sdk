// An example of audio authentication based on PortAudio asynchronous input streams.
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
#include <google/protobuf/util/time_util.h>
#include <iostream>
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/health_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>
#include <sensorycloud/services/management_service.hpp>
#include <sensorycloud/services/audio_service.hpp>
#include <sensorycloud/token_manager/secure_credential_store.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>

using sensory::token_manager::TokenManager;
using sensory::token_manager::SecureCredentialStore;
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
    public AudioService<SecureCredentialStore>::AuthenticateBidiReactor {
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

    std::atomic<bool> authenticated;

 public:
    /// @brief Initialize a reactor for streaming audio from a PortAudio stream.
    ///
    /// @param capture_ The PortAudio capture device.
    /// @param numChannels_ The number of channels in the input stream.
    /// @param sampleSize_ The number of bytes in an individual frame.
    /// @param sampleRate_ The sampling rate of the audio stream.
    /// @param framesPerBlock_ The number of frames in a block of audio.
    ///
    PortAudioReactor(PaStream* capture_,
        uint32_t numChannels_ = 1,
        uint32_t sampleSize_ = 2,
        uint32_t sampleRate_ = 16000,
        uint32_t framesPerBlock_ = 4096
    ) :
        AudioService<SecureCredentialStore>::AuthenticateBidiReactor(),
        capture(capture_),
        numChannels(numChannels_),
        sampleSize(sampleSize_),
        sampleRate(sampleRate_),
        framesPerBlock(framesPerBlock_),
        sampleBlock(static_cast<uint8_t*>(malloc(framesPerBlock_ * numChannels_ * sampleSize_))),
        authenticated(false) {
        if (capture == nullptr)
            throw std::runtime_error("capture must point to an allocated stream.");
    }

    /// @brief React to a _write done_ event.
    ///
    /// @param ok whether the write succeeded.
    ///
    void OnWriteDone(bool ok) override {
        // If the status is not OK, then an error occurred during the stream.
        if (!ok || authenticated) return;
        // Read a block of samples from the ADC.
        auto err = Pa_ReadStream(capture, sampleBlock.get(), framesPerBlock);
        if (err) {
            describe_pa_error(err);
            return;
        }
        // Set the audio content for the request and start the write request
        request.set_audiocontent(sampleBlock.get(), framesPerBlock * sampleSize);
        // If the user has been authenticated, close the stream.
        if (authenticated)
            StartWritesDone();
        else  // Send the data to the server to authenticate the user.
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
        std::cout << "Response" << std::endl;
        std::cout << "\tPercent Segment Complete: " << response.percentsegmentcomplete() << std::endl;
        std::cout << "\tAudio Energy:             " << response.audioenergy()            << std::endl;
        std::cout << "\tSuccess:                  " << response.success()                << std::endl;
        std::cout << "\tModel Prompt:             " << response.modelprompt()            << std::endl;

        if (response.success())  // Authentication succeeded, stop reading.
            authenticated = true;
        else  // Start the next read request
            StartRead(&response);
    }
};

int main(int argc, const char** argv) {
    // Initialize the configuration to the host for given address and port
    sensory::Config config(
        "io.stage.cloud.sensory.com",
        443,
        "cabb7700-206f-4cc7-8e79-cd7f288aa78d",
        "D895F447-91E8-486F-A783-6E3A33E4C7C5"
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
    std::string userID = "";
    std::cout << "user ID: ";
    std::cin >> userID;

    // Create an OAuth service
    OAuthService oauthService(config);
    sensory::token_manager::SecureCredentialStore keychain("com.sensory.cloud");
    sensory::token_manager::TokenManager<sensory::token_manager::SecureCredentialStore>
        tokenManager(oauthService, keychain);

    if (!tokenManager.hasSavedCredentials()) {  // the device is not registered
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

    // ------ Fetch the metadata about the enrollment --------------------------

    sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>
        mgmtService(config, tokenManager);

    // Query this user's active enrollments
    std::cout << "Active enrollments:" << std::endl;
    sensory::api::v1::management::GetEnrollmentsResponse enrollmentResponse;
    auto status = mgmtService.getEnrollments(&enrollmentResponse, userID);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get enrollments with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    for (auto& enrollment : enrollmentResponse.enrollments()) {
        if (enrollment.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_DEPENDENT &&
            enrollment.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_INDEPENDENT &&
            enrollment.modeltype() != sensory::api::common::VOICE_BIOMETRIC_WAKEWORD &&
            enrollment.modeltype() != sensory::api::common::SOUND_EVENT_ENROLLABLE
        ) continue;
        std::cout << "\tDescription:     " << enrollment.description()  << std::endl;
        std::cout << "\t\tModel Name:    " << enrollment.modelname()    << std::endl;
        std::cout << "\t\tModel Type:    " << enrollment.modeltype()    << std::endl;
        std::cout << "\t\tModel Version: " << enrollment.modelversion() << std::endl;
        std::cout << "\t\tUser ID:       " << enrollment.userid()       << std::endl;
        std::cout << "\t\tDevice ID:     " << enrollment.deviceid()     << std::endl;
        std::cout << "\t\tCreated:       "
            << google::protobuf::util::TimeUtil::ToString(enrollment.createdat())
            << std::endl;
        std::cout << "\t\tUpdated:       "
            << google::protobuf::util::TimeUtil::ToString(enrollment.updatedat())
            << std::endl;
        std::cout << "\t\tID:            " << enrollment.id()    << std::endl;
    }

    std::string enrollmentID = "";
    std::cout << "Enrollment ID: ";
    std::cin >> enrollmentID;

    // Determine whether to conduct a liveness check.
    std::string liveness;
    bool isLivenessEnabled(false);
    while (true) {
        std::cout << "Liveness Check [yes|y, no|n]: ";
        std::cin >> liveness;
        if (liveness == "yes" || liveness == "y") {
            isLivenessEnabled = true;
            break;
        } else if (liveness == "no" || liveness == "n") {
            isLivenessEnabled = false;
            break;
        } else {
            continue;
        }
    }

    // ------ Create the audio service -----------------------------------------

    // Create the audio service based on the configuration and token manager.
    AudioService<SecureCredentialStore> audioService(config, tokenManager);

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
        FRAMES_PER_BLOCK
    );
    // Initialize the stream with the reactor for callbacks, given audio model,
    // the sample rate of the audio and the expected language. A user ID is also
    // necessary to detect audio events.
    audioService.authenticate(&reactor,
        enrollmentID,
        SAMPLE_RATE,
        "en-US",
        isLivenessEnabled,
        sensory::api::v1::audio::ThresholdSensitivity::LOW,
        sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW
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
