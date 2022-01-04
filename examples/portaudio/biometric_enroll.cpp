// An example of Sound ID triggers based on PortAudio blocking input streams.
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
#include <sensorycloud/token_manager/insecure_credential_store.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>

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
    sensory::service::HealthService healthService(config);
    sensory::api::common::ServerHealthResponse serverHealth;
    auto status = healthService.getHealth(&serverHealth);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    // Report the health of the remote service
    std::cout << "Server status:" << std::endl;
    std::cout << "\tisHealthy: " << serverHealth.ishealthy() << std::endl;
    std::cout << "\tserverVersion: " << serverHealth.serverversion() << std::endl;
    std::cout << "\tid: " << serverHealth.id() << std::endl;

    // Query the user ID
    std::string userID = "";
    std::cout << "user ID: ";
    std::cin >> userID;

    // Create an OAuth service
    sensory::service::OAuthService oauthService(config);
    sensory::token_manager::TokenManager<sensory::token_manager::InsecureCredentialStore>
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
        sensory::api::v1::management::DeviceResponse registerResponse;
        status = oauthService.registerDevice(&registerResponse,
            name,
            password,
            credentials.id,
            credentials.secret
        );
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to register device with\n\t" <<
                status.error_code() << ": " << status.error_message() << std::endl;
            return 1;
        }
    }

    sensory::service::ManagementService<sensory::token_manager::InsecureCredentialStore>
        mgmtService(config, tokenManager);
    sensory::service::AudioService<sensory::token_manager::InsecureCredentialStore>
        audioService(config, tokenManager);

    // Query the available audio models
    std::cout << "Available audio models:" << std::endl;
    sensory::api::v1::audio::GetModelsResponse audioModelsResponse;
    status = audioService.getModels(&audioModelsResponse);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get audio models with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    for (auto& model : audioModelsResponse.models()) {
        if (model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_DEPENDENT &&
            model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_INDEPENDENT &&
            model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_WAKEWORD
        ) continue;
        std::cout << "\t" << model.name() << std::endl;
    }

    // Get the name of the audio model.
    std::string audioModel;
    std::cout << "Audio model: ";
    std::cin >> audioModel;

    // Determine the sample rate of the model.
    int32_t sampleRate(0);
    if (audioModel.find("8kHz") != std::string::npos)
        sampleRate = 8000;
    else if (audioModel.find("16kHz") != std::string::npos)
        sampleRate = 16000;

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

    // Get the description of the model.
    std::string description;
    std::cout << "Description: ";
    std::cin.ignore();
    std::getline(std::cin, description);

    // Determine whether to conduct a voice liveness check.
    sensory::service::AudioService<sensory::token_manager::InsecureCredentialStore>::CreateEnrollmentStream stream;
    stream = audioService.createEnrollment(
        audioModel,
        sampleRate,
        "en-US",
        userID,
        description,
        isLivenessEnabled
    );

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

    // Initialize the portaudio driver.
    PaError err = paNoError;
    err = Pa_Initialize();
    if (err != paNoError) goto paerror;

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
        FRAMES_PER_BLOCK,
        paClipOff,  // we won't output out-of-range samples so don't clip them
        NULL,       // using the blocking interface (no callback)
        NULL        // no data for the callback since there is none
    );
    if (err != paNoError) goto paerror;

    // Start the audio input stream.
    err = Pa_StartStream(audioStream);
    if (err != paNoError) goto paerror;

    // Create a buffer for the audio samples based on the number of bytes in
    // a block of samples.
    uint8_t sampleBlock[BYTES_PER_BLOCK];
    for (int i = 0; i < (DURATION * SAMPLE_RATE) / FRAMES_PER_BLOCK; ++i) {
        // Read a block of samples from the ADC.
        err = Pa_ReadStream(audioStream, sampleBlock, FRAMES_PER_BLOCK);
        if (err) goto paerror;

        // Create a new validate event request with the audio content.
        sensory::api::v1::audio::CreateEnrollmentRequest request;
        request.set_audiocontent(sampleBlock, FRAMES_PER_BLOCK * SAMPLE_SIZE);
        // Send the data to the server to validate the trigger.
        stream->Write(request);
        sensory::api::v1::audio::CreateEnrollmentResponse response;
        stream->Read(&response);
        // Log the result of the request to the terminal.
        std::cout << "Response" << std::endl;
        std::cout << "\tPercent Complete:         " << response.percentcomplete()        << std::endl;
        std::cout << "\tPercent Segment Complete: " << response.percentsegmentcomplete() << std::endl;
        std::cout << "\tAudio Energy:             " << response.audioenergy()            << std::endl;
        std::cout << "\tEnrollment ID:            " << response.enrollmentid()           << std::endl;
        std::cout << "\tModel Name:               " << response.modelname()              << std::endl;
        std::cout << "\tModel Version:            " << response.modelversion()           << std::endl;
        std::cout << "\tModel Prompt:             " << response.modelprompt()            << std::endl;

        if (response.percentcomplete() >= 100) break;
    }

    // Stop the audio stream.
    err = Pa_StopStream(audioStream);
    if (err != paNoError) goto paerror;

    // Terminate the port audio session.
    Pa_Terminate();

    return 0;

paerror:
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    return 1;
}
