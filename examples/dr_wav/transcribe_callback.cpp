// An example of audio transcription based on PortAudio asynchronous streams.
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
#include <thread>
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/health_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>
#include <sensorycloud/services/management_service.hpp>
#include <sensorycloud/services/audio_service.hpp>
#include <sensorycloud/token_manager/secure_credential_store.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>
#include "audio_buffer.hpp"
#include "tqdm.hpp"

using sensory::token_manager::TokenManager;
using sensory::token_manager::SecureCredentialStore;
using sensory::service::HealthService;
using sensory::service::AudioService;
using sensory::service::OAuthService;

/// @brief A bidirection stream reactor for biometric enrollments from audio
/// stream data.
///
class AudioFileReactor :
    public AudioService<SecureCredentialStore>::TranscribeBidiReactor {
 private:
    /// The audio samples to transcribe to text.
    const std::vector<int16_t>& buffer;
    /// The number of channels in the input audio.
    uint32_t numChannels;
    /// The sample rate of the audio input stream.
    uint32_t sampleRate;
    /// The number of frames per block of audio.
    uint32_t framesPerBlock;
    /// The current index of the audio stream
    std::size_t index = 0;
    /// The progress bar for providing a response per frame written.
    tqdm bar;
    /// A mutex for guarding access to the `isDone` and `status` variables.
    std::mutex mutex;
    /// The current transcription from the server
    std::string transcript = "";

 public:
    /// @brief Initialize a reactor for streaming audio from a PortAudio stream.
    ///
    /// @param buffer_ The audio samples to transcribe to text
    /// @param numChannels_ The number of channels in the input stream.
    /// @param sampleRate_ The sampling rate of the audio stream.
    /// @param framesPerBlock_ The number of frames in a block of audio.
    ///
    AudioFileReactor(const std::vector<int16_t>& buffer_,
        uint32_t numChannels_ = 1,
        uint32_t sampleRate_ = 16000,
        uint32_t framesPerBlock_ = 4096
    ) :
        AudioService<SecureCredentialStore>::TranscribeBidiReactor(),
        buffer(buffer_),
        numChannels(numChannels_),
        sampleRate(sampleRate_),
        framesPerBlock(framesPerBlock_),
        bar(buffer_.size() / static_cast<float>(framesPerBlock_), "frame") { }

    /// @brief React to a _write done_ event.
    ///
    /// @param ok whether the write succeeded.
    ///
    void OnWriteDone(bool ok) override {
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;

        // If the index has exceeded the buffer size, there are no more samples
        // to write from the audio buffer.
        if (index >= buffer.size()) {
            // Signal to the stream that no more data will be written.
            StartWritesDone();
            // Lock access to the critical section for the transcript string.
            std::lock_guard<std::mutex> lock(mutex);
            // Update the transcript with the final output transcript.
            transcript = response.transcript();

            return;
        }

        // Count the number of samples to upload in this request based on the
        // index of the current sample and the number of remaining samples.
        const auto numSamples = index + framesPerBlock > buffer.size() ?
            buffer.size() - index : framesPerBlock;
        // Set the audio content for the request and start the write request.
        request.set_audiocontent(&buffer[index], numSamples * sizeof(int16_t));
        // Update the index of the current sample based on the number of samples
        // that were just pushed.
        index += numSamples;
        bar.update();
        // If the number of blocks written surpasses the maximal length, close
        // the stream.
        StartWrite(&request);
    }

    /// @brief React to a _read done_ event.
    ///
    /// @param ok whether the read succeeded.
    ///
    void OnReadDone(bool ok) override {
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        // Start the next read request.
        StartRead(&response);
    }

    /// @brief Return the current transcript.
    ///
    /// @returns the transcript upon completion of the stream.
    ///
    std::string getTranscript() {
        // Lock access to the critical section for the transcript string
        std::lock_guard<std::mutex> lock(mutex);
        return transcript;
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
        oauthService.asyncRegisterDevice(
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
    AudioService<SecureCredentialStore> audioService(config, tokenManager);

    std::string audioModel = "speech_recognition_en";
    std::string wavFile = "wav/jabberwocky16khz_mono.wav";
    std::string userID = "ckckck";
    const uint32_t SAMPLES_PER_FRAME = 4096;

    // Load the audio file and zero pad the buffer with 300ms of silence.
    AudioBuffer buffer;
    buffer.load(wavFile);
    buffer.padBack(300);
    // Check that the file is 16kHz.
    if (buffer.getSampleRate() != 16000) {
        std::cout << "Error: attempting to load WAV file with sample rate of "
            << buffer.getSampleRate() << "kHz, but only 16kHz audio is supported."
            << std::endl;
        return 1;
    }
    // Check that the file in monophonic.
    if (buffer.getChannels() > 1) {
        std::cout << "Error: attempting to load WAV file with "
            << buffer.getChannels() << " channels, but only mono audio is supported."
            << std::endl;
        return 1;
    }

    // Create the gRPC reactor to respond to streaming events.
    AudioFileReactor reactor(buffer.getSamples(),
        buffer.getChannels(),
        buffer.getSampleRate(),
        SAMPLES_PER_FRAME  // the number of frames per block
    );
    // Initialize the stream with the reactor for callbacks, given audio model,
    // the sample rate of the audio and the expected language. A user ID is also
    // necessary to transcribe audio.
    audioService.asyncTranscribeAudio(&reactor,
        audioModel,
        buffer.getSampleRate(),
        "en-US",
        userID
    );
    reactor.StartCall();
    auto status = reactor.await();

    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Transcription stream broke with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }

    std::cout << reactor.getTranscript() << std::endl;

    return 0;
}
