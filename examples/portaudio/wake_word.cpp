// An example of wake word audio based on PortAudio input streams.
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

#include <iostream>
#include <portaudio.h>
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/health_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>
#include <sensorycloud/services/management_service.hpp>
#include <sensorycloud/services/audio_service.hpp>
#include <sensorycloud/token_manager/keychain.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>

int main(int argc, const char** argv) {
    // Initialize the configuration to the host for given address and port
    sensory::Config config(
        "io.stage.cloud.sensory.com",
        443,
        "cabb7700-206f-4cc7-8e79-cd7f288aa78d",
        "D895F447-91E8-486F-A783-6E3A33E4C7C5"
    );
    std::cout << "Connecting to remote host: " << config.getFullyQualifiedDomainName() << std::endl;

    // Query the health of the remote service.
    sensory::service::HealthService healthService(config);
    sensory::api::common::ServerHealthResponse serverHealth;
    auto status = healthService.getHealth(&serverHealth);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "GetHealth failed with\n\t" <<
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
    sensory::token_manager::Keychain keychain("com.sensory.cloud");
    sensory::token_manager::TokenManager<sensory::token_manager::Keychain> tokenManager(oauthService, keychain);

    if (!tokenManager.hasSavedCredentials()) {  // the device is not registered
        // Generate a new clientID and clientSecret for this device
        const auto credentials = tokenManager.generateCredentials();

        // Query the shared pass-phrase
        std::string password = "";
        std::cout << "password: ";
        std::cin >> password;

        // Register this device with the remote host
        sensory::api::v1::management::DeviceResponse registerResponse;
        auto status = oauthService.registerDevice(&registerResponse,
            userID,
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

    // Query the available audio models
    std::cout << "Available audio models:" << std::endl;
    sensory::service::AudioService<sensory::token_manager::Keychain> audioService(config, tokenManager);
    sensory::api::v1::audio::GetModelsResponse audioModelsResponse;
    status = audioService.getModels(&audioModelsResponse);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "GetAudioModels failed with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    for (auto& model : audioModelsResponse.models()) {
        if (model.modeltype() != sensory::api::common::VOICE_EVENT_WAKEWORD)
            continue;
        std::cout << "\t" << model.name() << std::endl;
    }

    std::string audioModel = "";
    std::cout << "Audio model: ";
    std::cin >> audioModel;

    // Create the stream
    auto stream = audioService.validateTrigger(
        audioModel,
        16000,
        "en-US",
        userID,
        sensory::api::v1::audio::ThresholdSensitivity::LOW
    );

    PaError err = paNoError;

    err = Pa_Initialize();
    if (err != paNoError) {
        std::cout << "failed to initialize portaudio" << std::endl;
        return 1;
    }

    PaStreamParameters inputParameters;
    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        return 1;
    }
    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = paInt16;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    outputParameters.channelCount = 1;
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowInputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    PaStream* audioStream;

    // typedef struct {
    //     int    frameIndex;  /* Index into sample array. */
    //     int    maxFrameIndex;
    //     short* recordedSamples;
    // }
    // paTestData;
    // paTestData data;

    static constexpr auto DURATION = 10;
    static constexpr auto SAMPLE_RATE = 16000;
    static constexpr auto NUM_CHANNELS = 1;
    static constexpr auto FRAMES_PER_BLOCK = 4096;
    static constexpr auto SAMPLE_SIZE = 2;

    err = Pa_OpenStream(
        &audioStream,
        &inputParameters,
        NULL,//&outputParameters,  // no output parameters for input stream
        SAMPLE_RATE,
        FRAMES_PER_BLOCK,
        paClipOff,  // we won't output out of range samples so don't bother clipping them
        NULL,
        NULL
    );
    if (err != paNoError) {
        std::cout << "failed to open stream" << std::endl;
        return 1;
    };

    err = Pa_StartStream(audioStream);
    if (err != paNoError) {
        fprintf(stderr, "An error occured while using the portaudio stream\n");
        fprintf(stderr, "Error number: %d\n", err);
        fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    }

    const auto numBytes = FRAMES_PER_BLOCK * NUM_CHANNELS * SAMPLE_SIZE;
    char* sampleBlock = (char *) malloc(numBytes);

    for (int i = 0; i < (DURATION * SAMPLE_RATE) / FRAMES_PER_BLOCK; ++i) {
        err = Pa_ReadStream(audioStream, sampleBlock, FRAMES_PER_BLOCK);
        if (err) {
            fprintf( stderr, "An error occured while using the portaudio stream\n" );
            fprintf( stderr, "Error number: %d\n", err );
            fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
            break;
        }

        sensory::api::v1::audio::ValidateEventRequest request;
        request.set_audiocontent(sampleBlock, FRAMES_PER_BLOCK * SAMPLE_SIZE);
        stream->Write(request);
        sensory::api::v1::audio::ValidateEventResponse response;
        stream->Read(&response);
        std::cout << "Response" << std::endl;
        std::cout << "\tAudio Energy: " << response.audioenergy() << std::endl;
        std::cout << "\tSuccess:      " << response.success()     << std::endl;
        std::cout << "\tResult ID:    " << response.resultid()    << std::endl;
        std::cout << "\tScore:        " << response.score()       << std::endl;

        // err = Pa_WriteStream(audioStream, sampleBlock, FRAMES_PER_BLOCK);
        // if (err) {
        //     fprintf( stderr, "An error occured while using the portaudio stream\n" );
        //     fprintf( stderr, "Error number: %d\n", err );
        //     fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
        //     break;
        // }
    }
    err = Pa_StopStream(audioStream);
    // if( err != paNoError ) goto error;

    free( sampleBlock );

    Pa_Terminate();

    return 0;
}
