// An example of biometric voice authentication using SensoryCloud with PortAudio.
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
#include <google/protobuf/util/time_util.h>
#include <iostream>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include "../dep/argparse.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::api::v1::audio::ThresholdSensitivity;

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
        .prog("authenticate")
        .description("A tool for authenticating with voice biometrics using SensoryCloud.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument({ "-m", "--model" })
        .help("The model to use for the enrollment.");
    parser.add_argument({ "-u", "--userid" })
        .help("The name of the user ID to query the enrollments for.");
    parser.add_argument({ "-e", "--enrollmentid" })
        .help("The ID of the enrollment to authenticate against.");
    parser.add_argument({ "-l", "--liveness" })
        .action("store_true")
        .help("Whether to conduct a liveness check in addition to the enrollment.");
    parser.add_argument({ "-s", "--sensitivity" })
        .choices({"LOW", "MEDIUM", "HIGH", "HIGHEST"})
        .default_value("HIGH")
        .help("The audio sensitivity level of the model.");
    parser.add_argument({ "-t", "--threshold" })
        .choices(std::vector<std::string>{"LOW", "HIGH"})
        .default_value("HIGH")
        .help("The security threshold for the authentication.");
    parser.add_argument({ "-g", "--group" })
        .action("store_true")
        .help("A flag determining whether the enrollment ID is for an enrollment group.");
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
    const auto MODEL = args.get<std::string>("model");
    const auto USER_ID = args.get<std::string>("userid");
    const auto ENROLLMENT_ID = args.get<std::string>("enrollmentid");
    const auto LIVENESS = args.get<bool>("liveness");
    ThresholdSensitivity SENSITIVITY;
    if (args.get<std::string>("sensitivity") == "LOW")
        SENSITIVITY = ThresholdSensitivity::LOW;
    else if (args.get<std::string>("sensitivity") == "MEDIUM")
        SENSITIVITY = ThresholdSensitivity::MEDIUM;
    else if (args.get<std::string>("sensitivity") == "HIGH")
        SENSITIVITY = ThresholdSensitivity::HIGH;
    else if (args.get<std::string>("sensitivity") == "HIGHEST")
        SENSITIVITY = ThresholdSensitivity::HIGHEST;
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

    // ------ Fetch enrollments for the user -----------------------------------

    if (USER_ID != "") {
        sensory::api::v1::management::GetEnrollmentsResponse enrollment_response;
        status = cloud.management.get_enrollments(&enrollment_response, USER_ID);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get enrollments ("
                << status.error_code() << "): "
                << status.error_message() << std::endl;
            return 1;
        }
        for (auto& enrollment : enrollment_response.enrollments()) {
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
            std::cout << "\tID:            " << enrollment.id()           << std::endl;
            std::cout << "\tReference ID:  " << enrollment.referenceid()  << std::endl;
        }
        return 0;
    }

    // ------ Create the authentication stream ---------------------------------

    // The number of input channels from the microphone. This should always be
    // mono.
    const auto NUM_CHANNELS = 1;
    // The number of bytes per sample, for 16-bit audio, this is 2 bytes.
    const auto SAMPLE_SIZE = 2;
    // The number of bytes in a given chunk of samples.
    const auto BYTES_PER_BLOCK = CHUNK_SIZE * NUM_CHANNELS * SAMPLE_SIZE;

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

    // Create an audio config that describes the format of the audio stream.
    auto audio_config = new sensory::api::v1::audio::AudioConfig;
    audio_config->set_encoding(sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
    audio_config->set_sampleratehertz(SAMPLE_RATE);
    audio_config->set_audiochannelcount(NUM_CHANNELS);
    audio_config->set_languagecode(LANGUAGE);
    // Create the transcribe config with the authentication parameters.
    auto authenticate_config = new ::sensory::api::v1::audio::AuthenticateConfig;
    if (GROUP)
        authenticate_config->set_enrollmentgroupid(ENROLLMENT_ID);
    else
        authenticate_config->set_enrollmentid(ENROLLMENT_ID);
    authenticate_config->set_islivenessenabled(LIVENESS);
    authenticate_config->set_sensitivity(SENSITIVITY);
    authenticate_config->set_security(THRESHOLD);
    // Initialize the stream with the cloud.
    grpc::CompletionQueue queue;
    auto stream = cloud.audio.authenticate(&queue, audio_config, authenticate_config, nullptr, (void*) Events::Finish);

    // start the stream event thread in the background to handle events.
    std::thread audioThread([&stream, &queue, &VERBOSE](){
        // The sample block of audio.
        std::unique_ptr<uint8_t> sample_block(static_cast<uint8_t*>(malloc(BYTES_PER_BLOCK)));
        // A flag determining whether the user has been authenticated.
        bool authenticated(false);

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

        void* tag(nullptr);
        bool ok(false);
        while (queue.Next(&tag, &ok)) {
            if (!ok) continue;
            if (tag == stream) {
                // Respond to the start of stream succeeding. All SensoryCloud
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
                // If the user has been authenticated, close the stream.
                if (authenticated) {
                    stream->getCall()->WritesDone((void*) Events::WritesDone);
                    continue;
                }
                // Read a block of samples from the ADC.
                auto err = Pa_ReadStream(capture, sample_block.get(), CHUNK_SIZE);
                if (err) {
                    describe_pa_error(err);
                    break;
                }
                // Set the audio content for the request and start the write request
                stream->getRequest().set_audiocontent(sample_block.get(), BYTES_PER_BLOCK);
                stream->getCall()->Write(stream->getRequest(), (void*) Events::Write);
            } else if (tag == (void*) Events::Read) {  // Respond to a read event.
                // Log the result of the request to the terminal.
                if (VERBOSE) {  // Verbose output, dump the message to the terminal
                    std::cout << "Response" << std::endl;
                    std::cout << "\tPercent Segment Complete: " << stream->getResponse().percentsegmentcomplete() << std::endl;
                    std::cout << "\tAudio Energy:             " << stream->getResponse().audioenergy()            << std::endl;
                    std::cout << "\tSuccess:                  " << stream->getResponse().success()                << std::endl;
                    std::cout << "\tModel Prompt:             " << stream->getResponse().modelprompt()            << std::endl;
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
                    auto prompt = stream->getResponse().modelprompt().length() > 0 ?
                        "Prompt: \"" + stream->getResponse().modelprompt() + "\"" :
                        "Text-independent model, say anything";
                    std::cout << '\r'
                        << progress[int(stream->getResponse().percentsegmentcomplete() / 10.f)]
                        << prompt << std::flush;
                }
                // Check for successful authentication
                if (stream->getResponse().success()) { // Authentication succeeded, stop reading.
                    authenticated = true;
                    std::cout << std::endl << "Successfully authenticated!";
                } else  // Start the next read request
                    stream->getCall()->Read(&stream->getResponse(), (void*) Events::Read);
            } else if (tag == (void*) Events::Finish) break;
        }
        std::cout << std::endl;

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
        std::cout << "Authentication stream broke ("
            << stream->getStatus().error_code() << "): "
            << stream->getStatus().error_message() << std::endl;
    }

    delete stream;

    return 0;
}
