// The audio service for the Sensory Cloud SDK.
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

#ifndef SENSORY_CLOUD_SERVICES_AUDIO_SERVICE_HPP_
#define SENSORY_CLOUD_SERVICES_AUDIO_SERVICE_HPP_

#include <string>
#include <memory>
#include "sensorycloud/generated/v1/audio/audio.pb.h"
#include "sensorycloud/generated/v1/audio/audio.grpc.pb.h"
#include "sensorycloud/config.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud Services.
namespace service {

/// @brief A service for audio data.
class AudioService {
 private:
    /// the global configuration for the remote connection
    const Config& config;
    /// the gRPC stub for the audio models service
    std::unique_ptr<api::v1::audio::AudioModels::Stub> models_stub;
    /// the gRPC stub for the audio bio-metrics service
    std::unique_ptr<api::v1::audio::AudioBiometrics::Stub> biometrics_stub;
    /// the gRPC stub for the audio events service
    std::unique_ptr<api::v1::audio::AudioEvents::Stub> events_stub;
    /// the gRPC stub for the audio transcriptions service
    std::unique_ptr<api::v1::audio::AudioTranscriptions::Stub> transcriptions_stub;

 public:
    /// @brief Initialize a new audio service.
    ///
    /// @param config the global configuration for the remote connection
    ///
    explicit AudioService(const Config& config_) : config(config_),
        models_stub(api::v1::audio::AudioModels::NewStub(config.getChannel())),
        biometrics_stub(api::v1::audio::AudioBiometrics::NewStub(config.getChannel())),
        events_stub(api::v1::audio::AudioEvents::NewStub(config.getChannel())),
        transcriptions_stub(api::v1::audio::AudioTranscriptions::NewStub(config.getChannel())) { }

    /// @brief Fetch a list of the audio models supported by the cloud host.
    /// @returns A future to be fulfilled with either a list of available
    /// models, or the network error that occurred
    api::v1::audio::GetModelsResponse getModels() {
        // std::cout << "Requesting audio models from server" << std::endl;

        // Create a context for the client.
        grpc::ClientContext context;
        // Create the request from the parameters.
        api::v1::audio::GetModelsRequest request;
        // Execute the RPC synchronously and get the response
        api::v1::audio::GetModelsResponse response;
        grpc::Status status = models_stub->GetModels(&context, request, &response);
        if (!status.ok()) {  // an error occurred in the RPC
            // std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            throw "GetModels failure";
        }
        return response;
    }

    /// a type for bio-metric enrollment streams
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::audio::CreateEnrollmentRequest,
            ::sensory::api::v1::audio::CreateEnrollmentResponse
        >
    > CreateEnrollmentStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an audio enrollment.
    ///
    /// @param modelName The name of the model to validate
    /// @param sampleRate Sample rate of model to validate
    /// @param userID Unique user identifier
    /// @param description User supplied description of the enrollment
    /// @param isLivenessEnabled Verifies liveness during the enrollment process
    /// @param numUtterances Sets how many utterances should be required
    /// for text-dependent enrollments, defaults to 4 if not specified. This
    /// parameter should be left `nil` for text-independent enrollments
    /// @param enrollmentDuration: Sets the duration in seconds for
    /// text-independent enrollments, defaults to 12.5 without liveness
    /// enabled and 8 with liveness enabled. This parameter should be left
    /// `nil` for text-dependent enrollments
    /// @param onStreamReceive: Handler function to handle response sent from
    /// the server
    /// @throws `NetworkError` if an error occurs while processing the cached
    /// server url
    /// @throws `NetworkError.notInitialized` if `Config.deviceID` has not
    /// been set
    /// @returns Bidirectional stream that can be used to send audio data to
    /// the server
    /// @details
    /// This call will automatically send the initial `AudioConfig` message to
    /// the server.
    ///
    template<typename T>
    CreateEnrollmentStream createEnrollment(
        const std::string& modelName,
        const int32_t& sampleRate,
        const std::string& userID,
        const T& onStreamReceive,
        const std::string& description = "",
        const bool& isLivenessEnabled = false,
        const uint32_t* numUtterances = nullptr,
        const float* enrollmentDuration = nullptr
    ) {
        std::cout << "Starting audio enrollment stream" << std::endl;

        // Create a context for the client.
        grpc::ClientContext context;
        const auto call = biometrics_stub->CreateEnrollment(&context, onStreamReceive);

        // Send initial config message
        api::v1::audio::AudioConfig audioConfig;
        audioConfig.set_encoding(api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
        audioConfig.set_sampleratehertz(sampleRate);
        audioConfig.set_audiochannelcount(1);
        audioConfig.set_languagecode(config.languageCode);

        api::v1::audio::CreateEnrollmentConfig enrollment_config;
        // TODO: should the audio config be allocated dynamically?
        enrollment_config.set_allocated_audio(&audioConfig);
        enrollment_config.set_modelname(modelName);
        enrollment_config.set_userid(userID);
        enrollment_config.set_deviceid(config.deviceID);
        enrollment_config.set_description(description);
        enrollment_config.set_islivenessenabled(isLivenessEnabled);
        if (numUtterances != nullptr)
            enrollment_config.set_enrollmentnumutterances(*numUtterances);
        else if (enrollmentDuration != nullptr)
            enrollment_config.set_enrollmentduration(*enrollmentDuration);

        api::v1::audio::CreateEnrollmentRequest request;
        // TODO: should the config be allocated dynamically?
        request.set_allocated_config(&enrollment_config);

        call.Write(request);
        return call;
    }

    /// a type for bio-metric authentication streams
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::audio::AuthenticateRequest,
            ::sensory::api::v1::audio::AuthenticateResponse
        >
    > AuthenticateStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// authentication.
    ///
    /// @param enrollmentID Enrollment to authenticate against
    /// @param groupID enrollment group ID to authenticate against, enrollmentID will overwrite groupID if both are passed in
    /// @param sampleRate Sample rate of model to validate
    /// @param isLivenessEnabled Specifies if the authentication should include a liveness check
    /// @param onStreamReceive Handler function to handle response sent form the server
    /// @throws `NetworkError` if an error occurs while processing the cached server url
    /// @returns Bidirectional stream that can be used to send audio data to the server
    /// This call will automatically send the initial `AudioConfig` message to the server
    ///
    template<typename T>
    AuthenticateStream streamAuthenticate(
        const std::string& enrollmentID,
        const std::string& groupID,
        const int32_t& sampleRate,
        const bool& isLivenessEnabled,
        const T& onStreamReceive
    ) {
        std::cout << "Starting audio authentication stream" << std::endl;

        // Create a context for the client.
        grpc::ClientContext context;
        const auto call = biometrics_stub->Authenticate(&context, onStreamReceive);

        // Send initial config message
        api::v1::audio::AudioConfig audioConfig;
        audioConfig.set_encoding(api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
        audioConfig.set_sampleratehertz(sampleRate);
        audioConfig.set_audiochannelcount(1);
        audioConfig.set_languagecode(config.languageCode);

        api::v1::audio::AuthenticateConfig authenticatecConfig;
        // TODO: should the config be allocated dynamically?
        authenticatecConfig.set_allocated_audio(&audioConfig);
        authenticatecConfig.set_enrollmentid(groupID.empty() ? enrollmentID : groupID);
        authenticatecConfig.set_islivenessenabled(isLivenessEnabled);

        api::v1::audio::AuthenticateRequest request;
        // TODO: should the config be allocated dynamically?
        request.set_allocated_config(&authenticatecConfig);

        call.Write(request);
        return call;
    }

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// authentication against an audio enrollment.
    ///
    /// @param enrollmentID Enrollment to authenticate against
    /// @param sampleRate Sample rate of model to validate
    /// @param isLivenessEnabled Specifies if the authentication should also
    /// include a liveness check
    /// @param onStreamReceive Handler function to handle response sent form
    /// the server
    /// @throws `NetworkError` if an error occurs while processing the cached
    /// server url
    /// @returns Bidirectional stream that can be used to send audio data to
    /// the server
    /// @details
    /// This call will automatically send the initial `AudioConfig` message to
    /// the server
    ///
    template<typename T>
    inline AuthenticateStream authenticateEnrollment(
        const std::string& enrollmentID,
        const int32_t& sampleRate,
        const bool& isLivenessEnabled,
        const T& onStreamReceive
    ) {
        return streamAuthenticate(enrollmentID, "", sampleRate, isLivenessEnabled, onStreamReceive);
    }

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// authentication against an audio enrollment group
    ///
    /// @param groupID Enrollment group to authenticate against
    /// @param sampleRate Sample rate of model to validate
    /// @param isLivenessEnabled Specifies if the authentication should also
    /// include a liveness check
    /// @param onStreamReceive Handler function to handle response sent form
    /// the server
    /// @throws `NetworkError` if an error occurs while processing the cached
    /// server url
    /// @returns Bidirectional stream that can be used to send audio data to
    /// the server
    /// @details
    /// This call will automatically send the initial `AudioConfig` message to
    /// the server
    ///
    template<typename T>
    inline AuthenticateStream authenticateGroup(
        const std::string& groupID,
        const int32_t& sampleRate,
        const bool& isLivenessEnabled,
        const T& onStreamReceive
    ) {
        return streamAuthenticate("", groupID, sampleRate, isLivenessEnabled, onStreamReceive);
    }

    /// a type for trigger validation streams
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::audio::ValidateEventRequest,
            ::sensory::api::v1::audio::ValidateEventResponse
        >
    > ValidateTriggerStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// audio event validation
    ///
    /// @param modelName Name of model to validate
    /// @param sampleRate Sample rate of model to validate
    /// @param userID Unique user identifier
    /// @param sensitivity How sensitive the model should be to false accepts
    /// @param onStreamReceive Handler function to handle response sent form
    /// the server
    /// @throws `NetworkError` if an error occurs while processing the cached
    /// server url
    /// @returns Bidirectional stream that can be used to send audio data to
    /// the server
    /// @details
    /// This call will automatically send the initial `AudioConfig` message
    /// to the server
    ///
    template<typename T>
    ValidateTriggerStream validateTrigger(
        const std::string& modelName,
        const int32_t& sampleRate,
        const std::string& userID,
        const api::v1::audio::ThresholdSensitivity& sensitivity,
        const T& onStreamReceive
    ) {
        std::cout << "Requesting validate trigger stream from server" << std::endl;

        // Create a context for the client.
        grpc::ClientContext context;
        const auto call = events_stub->ValidateEvent(&context, onStreamReceive);

        // Send initial config message
        api::v1::audio::AudioConfig audioConfig;
        audioConfig.set_encoding(api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
        audioConfig.set_sampleratehertz(sampleRate);
        audioConfig.set_audiochannelcount(1);
        audioConfig.set_languagecode(config.languageCode);

        api::v1::audio::ValidateEventConfig validateEventConfig;
        // TODO: should the config be allocated dynamically?
        validateEventConfig.set_allocated_audio(&audioConfig);
        validateEventConfig.set_modelname(modelName);
        validateEventConfig.set_userid(userID);
        validateEventConfig.set_sensitivity(sensitivity);

        api::v1::audio::ValidateEventRequest request;
        // TODO: should the config be allocated dynamically?
        request.set_allocated_config(&validateEventConfig);

        call.Write(request);
        return call;
    }

    /// a type for bio-metric transcription streams
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::audio::TranscribeRequest,
            ::sensory::api::v1::audio::TranscribeResponse
        >
    > TranscribeAudioStream;

    /// @brief Open a bidirectional stream to the server that provides a
    /// transcription of the provided audio data.
    ///
    /// @param modelName Name of model to validate
    /// @param sampleRate Sample rate of model to validate
    /// @param userID Unique user identifier
    /// @param onStreamReceive Handler function to handle response sent form
    /// the server
    /// @throws `NetworkError` if an error occurs while processing the cached
    /// server url
    /// @returns Bidirectional stream that can be used to send audio data to
    /// the server
    /// @details
    /// This call will automatically send the initial `AudioConfig` message to
    /// the server
    ///
    template<typename T>
    TranscribeAudioStream transcribeAudio(
        const std::string& modelName,
        const int32_t& sampleRate,
        const std::string& userID,
        const T& onStreamReceive
    ) {
        std::cout << "Requesting to transcribe audio" << std::endl;

        // Create a context for the client.
        grpc::ClientContext context;
        const auto call = transcriptions_stub->Transcribe(&context, onStreamReceive);

        // Send initial config message
        api::v1::audio::AudioConfig audioConfig;
        audioConfig.set_encoding(api::v1::audio::AudioConfig_AudioEncoding_LINEAR16);
        audioConfig.set_sampleratehertz(sampleRate);
        audioConfig.set_audiochannelcount(1);
        audioConfig.set_languagecode(config.languageCode);

        api::v1::audio::TranscribeConfig transcribeConfig;
        // TODO: should the config be allocated dynamically?
        transcribeConfig.set_allocated_audio(&audioConfig);
        transcribeConfig.set_modelname(modelName);
        transcribeConfig.set_userid(userID);

        api::v1::audio::TranscribeRequest request;
        // TODO: should the config be allocated dynamically?
        request.set_allocated_config(&transcribeConfig);

        call.Write(request);
        return call;
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORY_CLOUD_SERVICES_AUDIO_SERVICE_HPP_
