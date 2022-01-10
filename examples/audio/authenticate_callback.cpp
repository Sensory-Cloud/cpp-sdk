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
    public AudioService<InsecureCredentialStore>::AuthenticateBidiReactor {
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
    /// Whether to produce verbose output from the reactor.
    bool verbose = false;
    /// The buffer for the block of samples from the port audio input device.
    std::unique_ptr<uint8_t> sampleBlock;
    /// Whether the user successfully authenticated
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
        uint32_t framesPerBlock_ = 4096,
        const bool& verbose_ = false
    ) :
        AudioService<InsecureCredentialStore>::AuthenticateBidiReactor(),
        capture(capture_),
        numChannels(numChannels_),
        sampleSize(sampleSize_),
        sampleRate(sampleRate_),
        framesPerBlock(framesPerBlock_),
        verbose(verbose_),
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
        if (!ok) return;
        // If authentication succeeded, send the writes done signal
        if (authenticated) {
            StartWritesDone();
            return;
        }
        // Read a block of samples from the ADC.
        auto err = Pa_ReadStream(capture, sampleBlock.get(), framesPerBlock);
        if (err) {
            describe_pa_error(err);
            return;
        }
        // Set the audio content for the request and start the write request
        request.set_audiocontent(sampleBlock.get(), numChannels * framesPerBlock * sampleSize);
        StartWrite(&request);
    }

    /// @brief React to a _read done_ event.
    ///
    /// @param ok whether the read succeeded.
    ///
    void OnReadDone(bool ok) override {
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        // Log the result of the request to the terminal.
        if (verbose) {  // Verbose output, dump the message to the terminal
            std::cout << "Response" << std::endl;
            std::cout << "\tPercent Segment Complete: " << response.percentsegmentcomplete() << std::endl;
            std::cout << "\tAudio Energy:             " << response.audioenergy()            << std::endl;
            std::cout << "\tSuccess:                  " << response.success()                << std::endl;
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
                << progress[int(response.percentsegmentcomplete() / 10.f)]
                << prompt << std::flush;
        }
        // Check for successful authentication
        if (response.success()) {  // Authentication succeeded, stop reading.
            std::cout << std::endl << "Successfully authenticated!";
            authenticated = true;
        } else  // Start the next read request
            StartRead(&response);
    }
};

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("authenticate")
        .description("A tool for authenticating with voice biometrics using Sensory Cloud.");
    parser.add_argument({ "-H", "--host" })
        .required(true)
        .help("HOST The hostname of a Sensory Cloud inference server.");
    parser.add_argument({ "-P", "--port" })
        .required(true)
        .help("PORT The port number that the Sensory Cloud inference server is running at.");
    parser.add_argument({ "-T", "--tenant" })
        .required(true)
        .help("TENANT The ID of your tenant on a Sensory Cloud inference server.");
    parser.add_argument({ "-I", "--insecure" })
        .action("store_true")
        .help("INSECURE Disable TLS.");
    parser.add_argument({ "-m", "--model" })
        .help("MODEL The model to use for the enrollment.");
    parser.add_argument({ "-u", "--userid" })
        .help("USERID The name of the user ID to query the enrollments for.");
    parser.add_argument({ "-e", "--enrollmentid" })
        .help("ENROLLMENTID The ID of the enrollment to authenticate against.");
    parser.add_argument({ "-l", "--liveness" })
        .action("store_true")
        .help("LIVENESS Whether to conduct a liveness check in addition to the enrollment.");
    parser.add_argument({ "-s", "--sensitivity" })
        .choices({"LOW", "MEDIUM", "HIGH", "HIGHEST"})
        .default_value("HIGH")
        .help("SENSITIVITY The audio sensitivity level of the model.");
    parser.add_argument({ "-t", "--threshold" })
        .choices(std::vector<std::string>{"LOW", "HIGH"})
        .default_value("HIGH")
        .help("THRESHOLD The security threshold for the authentication.");
    parser.add_argument({ "-g", "--group" })
        .action("store_true")
        .help("GROUP A flag determining whether the enrollment ID is for an enrollment group.");
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
    const auto HOSTNAME = args.get<std::string>("host");
    const auto PORT = args.get<uint16_t>("port");
    const auto TENANT = args.get<std::string>("tenant");
    const auto IS_SECURE = !args.get<bool>("insecure");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    const auto ENROLLMENT_ID = args.get<std::string>("enrollmentid");
    const auto LIVENESS = args.get<bool>("liveness");
    sensory::api::v1::audio::ThresholdSensitivity SENSITIVITY;
    if (args.get<std::string>("threshold") == "LOW")
        SENSITIVITY = sensory::api::v1::audio::ThresholdSensitivity::LOW;
    else if (args.get<std::string>("threshold") == "MEDIUM")
        SENSITIVITY = sensory::api::v1::audio::ThresholdSensitivity::MEDIUM;
    else if (args.get<std::string>("threshold") == "HIGH")
        SENSITIVITY = sensory::api::v1::audio::ThresholdSensitivity::HIGH;
    else if (args.get<std::string>("threshold") == "HIGHEST")
        SENSITIVITY = sensory::api::v1::audio::ThresholdSensitivity::HIGHEST;
    sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity THRESHOLD;
    if (args.get<std::string>("threshold") == "LOW")
        THRESHOLD = sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW;
    else if (args.get<std::string>("threshold") == "HIGH")
        THRESHOLD = sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_HIGH;
    const auto GROUP = args.get<bool>("group");
    const auto LANGUAGE = args.get<std::string>("language");
    const uint32_t CHUNK_SIZE = 4096;//args.get<int>("chunksize");
    const auto SAMPLE_RATE = 16000;//args.get<uint32_t>("samplerate");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create an insecure credential store for keeping OAuth credentials in.
    sensory::token_manager::InsecureCredentialStore keychain(".", "com.sensory.cloud.examples");
    if (!keychain.contains("deviceID"))
        keychain.emplace("deviceID", sensory::token_manager::uuid_v4());
    const auto DEVICE_ID(keychain.at("deviceID"));

    // Initialize the configuration to the host for given address and port
    sensory::Config config(HOSTNAME, PORT, TENANT, DEVICE_ID, IS_SECURE);
    config.connect();

    // Query the health of the remote service.
    sensory::service::HealthService healthService(config);
    sensory::api::common::ServerHealthResponse serverHealth;
    auto status = healthService.getHealth(&serverHealth);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    } else if (VERBOSE) {
        // Report the health of the remote service
        std::cout << "Server status:" << std::endl;
        std::cout << "\tisHealthy: " << serverHealth.ishealthy() << std::endl;
        std::cout << "\tserverVersion: " << serverHealth.serverversion() << std::endl;
        std::cout << "\tid: " << serverHealth.id() << std::endl;
    }

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

    // ------ Fetch the metadata about the enrollment --------------------------

    // Query this user's active enrollments
    if (USER_ID != "") {
        sensory::service::ManagementService<sensory::token_manager::InsecureCredentialStore>
            mgmtService(config, tokenManager);
        sensory::api::v1::management::GetEnrollmentsResponse enrollmentResponse;
        status = mgmtService.getEnrollments(&enrollmentResponse, USER_ID);
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
            std::cout << "Description:     " << enrollment.description()  << std::endl;
            std::cout << "\tModel Name:    " << enrollment.modelname()    << std::endl;
            std::cout << "\tModel Type:    " << enrollment.modeltype()    << std::endl;
            std::cout << "\tModel Version: " << enrollment.modelversion() << std::endl;
            std::cout << "\tUser ID:       " << enrollment.userid()       << std::endl;
            std::cout << "\tDevice ID:     " << enrollment.deviceid()     << std::endl;
            std::cout << "\tCreated:       "
                << google::protobuf::util::TimeUtil::ToString(enrollment.createdat())
                << std::endl;
            std::cout << "\tUpdated:       "
                << google::protobuf::util::TimeUtil::ToString(enrollment.updatedat())
                << std::endl;
            std::cout << "\tID:            " << enrollment.id()    << std::endl;
        }
        return 0;
    }

    // ------ Create the audio service -----------------------------------------

    // Create the audio service based on the configuration and token manager.
    AudioService<InsecureCredentialStore> audioService(config, tokenManager);

    // The number of input channels from the microphone. This should always be
    // mono.
    const auto NUM_CHANNELS = 1;
    // The number of bytes per sample, for 16-bit audio, this is 2 bytes.
    const auto SAMPLE_SIZE = 2;
    // The number of bytes in a given chunk of samples.
    const auto BYTES_PER_BLOCK =
        CHUNK_SIZE * NUM_CHANNELS * SAMPLE_SIZE;

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

    // Create the gRPC reactor to respond to streaming events.
    PortAudioReactor reactor(capture,
        NUM_CHANNELS,
        SAMPLE_SIZE,
        SAMPLE_RATE,
        CHUNK_SIZE,
        VERBOSE
    );
    // Initialize the stream with the reactor for callbacks, given audio model,
    // the sample rate of the audio and the expected language. A user ID is also
    // necessary to detect audio events.
    audioService.authenticate(&reactor,
        sensory::service::audio::newAudioConfig(
            sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16,
            SAMPLE_RATE, 1, LANGUAGE
        ),
        sensory::service::audio::newAuthenticateConfig(
            ENROLLMENT_ID, LIVENESS, SENSITIVITY, THRESHOLD, GROUP
        )
    );
    reactor.StartCall();
    status = reactor.await();
    std::cout << std::endl;

    // Stop the audio stream.
    err = Pa_StopStream(capture);
    if (err != paNoError) return describe_pa_error(err);

    // Terminate the port audio session.
    Pa_Terminate();

    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Authentication stream broke with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
