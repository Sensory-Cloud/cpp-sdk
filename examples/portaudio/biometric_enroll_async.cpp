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
    std::string userID = "";
    std::cout << "user ID: ";
    std::cin >> userID;

    // Create an OAuth service
    OAuthService oauthService(config);
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

    std::cout << "Available audio models:" << std::endl;
    audioService.getModels([](AudioService<InsecureCredentialStore>::GetModelsCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to get audio models with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
        }
        // Iterate over the models returned in the response
        for (auto& model : call->getResponse().models()) {
            // Ignore models that aren't face biometric models.
            if (model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_DEPENDENT &&
                model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_INDEPENDENT &&
                model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_WAKEWORD
            ) continue;
            std::cout << "\t" << model.name() << std::endl;
        }
    })->await();

    std::string audioModel = "";
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

    // the maximal duration of the recording in seconds
    static constexpr auto DURATION = 60;
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

    // Start an asynchronous RPC to fetch the names of the available models. The
    // RPC will use the grpc::CompletionQueue as an event loop.
    grpc::CompletionQueue queue;
    auto stream = audioService.createEnrollment(&queue,
        sensory::service::newAudioConfig(
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
            sampleRate, 1, "en-US"
        ),
        sensory::service::newCreateEnrollmentConfig(
            audioModel,
            userID,
            description,
            isLivenessEnabled,
            0, 0
        )
    );

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

    // start the stream event thread in the background to handle events.
    std::thread audioThread([&stream, &queue, &sampleRate](){
        // The sample block of audio.
        std::unique_ptr<uint8_t> sampleBlock(static_cast<uint8_t*>(malloc(BYTES_PER_BLOCK)));
        // A flag determining whether the user has been enrolled.
        bool isEnrolled(false);

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
            sampleRate,
            FRAMES_PER_BLOCK,
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
            if (!ok) break;
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
                // Read a block of samples from the ADC.
                auto err = Pa_ReadStream(capture, sampleBlock.get(), FRAMES_PER_BLOCK);
                if (err) {
                    describe_pa_error(err);
                    break;
                }
                // Set the audio content for the request and start the write request
                stream->getRequest().set_audiocontent(sampleBlock.get(), FRAMES_PER_BLOCK * SAMPLE_SIZE);
                // If the user has been authenticated, close the stream.
                if (isEnrolled)
                    stream->getCall()->WritesDone((void*) Events::WritesDone);
                else  // Send the data to the server to authenticate the user.
                    stream->getCall()->Write(stream->getRequest(), (void*) Events::Write);
            } else if (tag == (void*) Events::Read) {  // Respond to a read event.
                // std::cout << "Response" << std::endl;
                // std::cout << "\tPercent Complete:         " << response.percentcomplete()        << std::endl;
                // std::cout << "\tPercent Segment Complete: " << response.percentsegmentcomplete() << std::endl;
                // std::cout << "\tAudio Energy:             " << response.audioenergy()            << std::endl;
                // std::cout << "\tEnrollment ID:            " << response.enrollmentid()           << std::endl;
                // std::cout << "\tModel Name:               " << response.modelname()              << std::endl;
                // std::cout << "\tModel Version:            " << response.modelversion()           << std::endl;
                // std::cout << "\tModel Prompt:             " << response.modelprompt()            << std::endl;
                if (stream->getResponse().percentcomplete() < 100)  // Start the next read request
                    stream->getCall()->Read(&stream->getResponse(), (void*) Events::Read);
                else  // Enrollment succeeded, stop reading.
                    isEnrolled = true;
            } else if (tag == (void*) Events::WritesDone) {  // Respond to `WritesDone`
                // Finish the stream and terminate.
                stream->getCall()->Finish(&stream->getStatus(), (void*) Events::Finish);
            } else if (tag == (void*) Events::Finish) {  // Respond to `Finish`
                // Check the final status of the stream and delete the call
                // handle now that the stream has terminated.
                if (!stream->getStatus().ok()) {
                    std::cout << "Authentication stream failed with\n\t"
                        << stream->getStatus().error_code() << ": "
                        << stream->getStatus().error_message() << std::endl;
                }
                // By this point, we can guarantee that no write/read events
                // will be coming to the completion queue. Break out of the
                // event loop to terminate the background processing thread.
                break;
            }
        }

        if (isEnrolled) {
            std::cout << "Successfully enrolled!" << std::endl;
        } else {
            std::cout << "Enrollment failed!" << std::endl;
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
        std::cout << "Failed to enroll with\n\t" <<
            stream->getStatus().error_code() << ": " <<
            stream->getStatus().error_message() << std::endl;
    }

    delete stream;

    return 0;
}
