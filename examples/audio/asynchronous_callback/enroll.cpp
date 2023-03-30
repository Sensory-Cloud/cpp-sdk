// An example of biometric voice enrollment using SensoryCloud with PortAudio.
//
// Copyright (c) 2023 Sensory, Inc.
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
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include "../dep/argparse.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::service::AudioService;

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

/// @brief A bi-directional stream reactor for audio signal enrollment.
///
class PortAudioReactor :
    public AudioService<FileSystemCredentialStore>::CreateEnrollmentBidiReactor {
 private:
    /// The capture device that input audio is streaming in from.
    PaStream* capture;
    /// The number of channels in the input audio.
    uint32_t num_channels;
    /// The number of bytes per audio sample (i.e., 2 for 16-bit audio)
    uint32_t sample_size;
    /// The sample rate of the audio input stream.
    uint32_t sample_rate;
    /// The number of frames per block of audio.
    uint32_t frames_per_block;
    /// The maximum duration of the stream in seconds.
    float duration;
    /// The buffer for the block of samples from the port audio input device.
    std::unique_ptr<uint8_t> sample_block;
    /// A boolean determining whether the enrollment succeeded.
    std::atomic<bool> is_enrolled;
    /// Whether to produce verbose output from the reactor
    bool verbose = false;

 public:
    /// @brief Initialize a reactor for streaming audio from a PortAudio stream.
    ///
    /// @param capture_ The PortAudio capture device.
    /// @param num_channels_ The number of channels in the input stream.
    /// @param sample_size_ The number of bytes in an individual frame.
    /// @param sample_rate_ The sampling rate of the audio stream.
    /// @param frames_per_block_ The number of frames in a block of audio.
    /// @param duration_ the maximum duration for the audio capture
    ///
    PortAudioReactor(PaStream* capture_,
        uint32_t num_channels_ = 1,
        uint32_t sample_size_ = 2,
        uint32_t sample_rate_ = 16000,
        uint32_t frames_per_block_ = 4096,
        float duration_ = 60,
        const bool& verbose_ = false
    ) :
        AudioService<FileSystemCredentialStore>::CreateEnrollmentBidiReactor(),
        capture(capture_),
        num_channels(num_channels_),
        sample_size(sample_size_),
        sample_rate(sample_rate_),
        frames_per_block(frames_per_block_),
        duration(duration_),
        verbose(verbose_),
        sample_block(static_cast<uint8_t*>(malloc(frames_per_block_ * num_channels_ * sample_size_))),
        is_enrolled(false) {
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
        // If enrollment succeeded, send the writes done signal
        if (is_enrolled) {
            StartWritesDone();
            return;
        }
        // Read a block of samples from the ADC.
        auto err = Pa_ReadStream(capture, sample_block.get(), frames_per_block);
        if (err) {
            describe_pa_error(err);
            return;
        }
        // Set the audio content for the request and start the write request
        request.set_audiocontent(sample_block.get(), num_channels * frames_per_block * sample_size);
        StartWrite(&request);
    }

    /// @brief React to a _read done_ event.
    ///
    /// @param ok whether the read succeeded.
    ///
    void OnReadDone(bool ok) override {
        // If the status is not OK, then an error occurred during the stream.
        if (!ok) return;
        if (verbose) {  // Verbose output, dump the message to the terminal
            std::cout << "Response" << std::endl;
            std::cout << "\tPercent Complete:         " << response.percentcomplete()        << std::endl;
            std::cout << "\tPercent Segment Complete: " << response.percentsegmentcomplete() << std::endl;
            std::cout << "\tAudio Energy:             " << response.audioenergy()            << std::endl;
            std::cout << "\tEnrollment ID:            " << response.enrollmentid()           << std::endl;
            std::cout << "\tModel Name:               " << response.modelname()              << std::endl;
            std::cout << "\tModel Version:            " << response.modelversion()           << std::endl;
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
                << progress[int(response.percentcomplete() / 10.f)]
                << prompt << std::flush;
        }
        // Check for enrollment success
        if (response.percentcomplete() >= 100) {
            is_enrolled = true;
            std::cout << std::endl;
            std::cout << "Successfully enrolled with ID: "
                << response.enrollmentid() << std::endl;
        } else  // Start the next read request
            StartRead(&response);
    }
};

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("enroll")
        .description("A tool for authenticating with voice biometrics using SensoryCloud.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-g", "--getmodels" })
        .action("store_true")
        .help("Whether to query for a list of available models.");
    parser.add_argument({ "-m", "--model" })
        .help("The model to use for the enrollment.");
    parser.add_argument({ "-u", "--userid" })
        .help("The name of the user ID to create the enrollment for.");
    parser.add_argument({ "-d", "--description" })
        .help("A text description of the enrollment.");
    parser.add_argument({ "-l", "--liveness" })
        .action("store_true")
        .help("Whether to conduct a liveness check in addition to the enrollment.");
    parser.add_argument({ "-n", "--numutterances" })
        .default_value(0)
        .help("The number of utterances for a text independent enrollment.");
    parser.add_argument({ "-D", "--duration" })
        .default_value(0)
        .help("The duration of a text-dependent enrollment.");
    parser.add_argument({ "-r", "--reference-id" })
        .help("An optional reference ID for tagging the enrollment.");
    parser.add_argument({ "-L", "--language" })
        .help("The IETF BCP 47 language tag for the input audio (e.g., en-US).");
    // parser.add_argument({ "-C", "--chunksize" })
    //     .help("The number of audio samples per message (default 4096).")
    //     .default_value("4096");
    // parser.add_argument({ "-S", "--samplerate" })
    //     .help("The audio sample rate of the input stream.")
    //     .choices({"9600", "11025", "12000", "16000", "22050", "24000", "32000", "44100", "48000", "88200", "96000", "192000"})
    //     .default_value("16000");
    parser.add_argument({ "-v", "--verbose" })
        .action("store_true")
        .help("Produce verbose output during authentication.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto PATH = args.get<std::string>("path");
    const auto GETMODELS = args.get<bool>("getmodels");
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    const auto DESCRIPTION = args.get<std::string>("description");
    const auto LIVENESS = args.get<bool>("liveness");
    const auto NUM_UTTERANCES = args.get<uint32_t>("numutterances");
    const auto DURATION = args.get<float>("duration");
    const auto REFERENCE_ID = args.get<std::string>("reference-id");
    const auto LANGUAGE = args.get<std::string>("language");
    const uint32_t CHUNK_SIZE = 4096;//args.get<int>("chunksize");
    const auto SAMPLE_RATE = 16000;//args.get<uint32_t>("samplerate");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create a credential store for keeping OAuth credentials in.
    FileSystemCredentialStore keychain(".", "com.sensory.cloud.examples");
    // Create the cloud services handle.
    SensoryCloud<FileSystemCredentialStore> cloud(PATH, keychain);

    // Query the health of the remote service.
    sensory::api::common::ServerHealthResponse server_health;
    auto status = cloud.health.get_health(&server_health);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }
    if (VERBOSE) {
        std::cout << "Server status:" << std::endl;
        std::cout << "\tisHealthy: " << server_health.ishealthy() << std::endl;
        std::cout << "\tserverVersion: " << server_health.serverversion() << std::endl;
        std::cout << "\tid: " << server_health.id() << std::endl;
    }

    // Initialize the client.
    sensory::api::v1::management::DeviceResponse response;
    status = cloud.initialize(&response);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to initialize (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }

    // ------ Query the available audio models ---------------------------------

    if (GETMODELS) {
        int error_code = 0;
        cloud.audio.get_models([&error_code](AudioService<FileSystemCredentialStore>::GetModelsCallbackData* call) {
            if (!call->getStatus().ok()) {  // The call failed.
                std::cout << "Failed to get audio models with\n\t" <<
                    call->getStatus().error_code() << ": " <<
                    call->getStatus().error_message() << std::endl;
                error_code = 1;
            } else {
                for (auto& model : call->getResponse().models()) {
                    // Ignore models that aren't face biometric models.
                    if (model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_DEPENDENT &&
                        model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_TEXT_INDEPENDENT &&
                        model.modeltype() != sensory::api::common::VOICE_BIOMETRIC_WAKEWORD
                    ) continue;
                    std::cout << model.name() << std::endl;
                }
            }
        })->await();
        return error_code;
    }

    // the maximal duration of the recording in seconds
    static constexpr auto MAX_DURATION = 60;
    // The number of input channels from the microphone. This should always be
    // mono.
    static constexpr auto NUM_CHANNELS = 1;
    // The number of bytes per sample, for 16-bit audio, this is 2 bytes.
    static constexpr auto SAMPLE_SIZE = 2;
    // The number of bytes in a given chunk of samples.
    static constexpr auto BYTES_PER_BLOCK = CHUNK_SIZE * NUM_CHANNELS * SAMPLE_SIZE;

    // Initialize the PortAudio driver.
    PaError err = paNoError;
    err = Pa_Initialize();
    if (err != paNoError) return describe_pa_error(err);

    // Setup the input parameters for the port audio stream.
    PaStreamParameters input_parameters;
    input_parameters.device = Pa_GetDefaultInputDevice();
    if (input_parameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        return 1;
    }
    input_parameters.channelCount = 1;
    input_parameters.sampleFormat = paInt16;  // Sensory expects 16-bit audio
    input_parameters.suggestedLatency =
        Pa_GetDeviceInfo(input_parameters.device)->defaultHighInputLatency;
    input_parameters.hostApiSpecificStreamInfo = NULL;

    // Open the PortAudio stream with the input device.
    PaStream* capture;
    err = Pa_OpenStream(&capture,
        &input_parameters,
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

    // Create an audio config that describes the format of the audio stream.
    auto audio_config = new sensory::api::v1::audio::AudioConfig;
    audio_config->set_encoding(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
    audio_config->set_sampleratehertz(SAMPLE_RATE);
    audio_config->set_audiochannelcount(NUM_CHANNELS);
    audio_config->set_languagecode(LANGUAGE);
    // Create the config with the enrollment parameters.
    auto create_enrollment_config = new sensory::api::v1::audio::CreateEnrollmentConfig;
    create_enrollment_config->set_modelname(MODEL);
    create_enrollment_config->set_userid(USER_ID);
    create_enrollment_config->set_description(DESCRIPTION);
    create_enrollment_config->set_islivenessenabled(LIVENESS);
    if (DURATION > 0)
        create_enrollment_config->set_enrollmentduration(DURATION);
    if (NUM_UTTERANCES > 0)
        create_enrollment_config->set_enrollmentnumutterances(NUM_UTTERANCES);
    create_enrollment_config->set_referenceid(REFERENCE_ID);
    // Initialize the stream with the cloud.
    PortAudioReactor reactor(capture,
        NUM_CHANNELS,
        SAMPLE_SIZE,
        SAMPLE_RATE,
        CHUNK_SIZE,
        MAX_DURATION,
        VERBOSE
    );
    cloud.audio.create_enrollment(&reactor, audio_config, create_enrollment_config);
    reactor.StartCall();
    status = reactor.await();

    // Stop the audio stream.
    err = Pa_StopStream(capture);
    if (err != paNoError) return describe_pa_error(err);

    // Terminate the port audio session.
    Pa_Terminate();

    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Wake-word stream broke ("
            << status.error_code() << "): "
            << status.error_message() << std::endl;
        return 1;
    }

    return 0;
}
