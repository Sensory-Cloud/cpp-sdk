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
#include "sensorycloud/token_manager/token_manager.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud Services.
namespace service {

/// @brief A service for audio data.
/// @tparam SecureCredentialStore a secure CRUD class for storing credentials.
template<typename SecureCredentialStore>
class AudioService {
 private:
    /// the global configuration for the remote connection
    const Config& config;
    /// the token manager for securing gRPC requests to the server
    ::sensory::token_manager::TokenManager<SecureCredentialStore>& tokenManager;
    /// the gRPC stub for the audio models service
    std::unique_ptr<::sensory::api::v1::audio::AudioModels::Stub> models_stub;
    /// the gRPC stub for the audio biometrics service
    std::unique_ptr<::sensory::api::v1::audio::AudioBiometrics::Stub> biometrics_stub;
    /// the gRPC stub for the audio events service
    std::unique_ptr<::sensory::api::v1::audio::AudioEvents::Stub> events_stub;
    /// the gRPC stub for the audio transcriptions service
    std::unique_ptr<::sensory::api::v1::audio::AudioTranscriptions::Stub> transcriptions_stub;

    /// @brief Create a new audio config for an audio streaming application.
    ///
    /// @brief sampleRate The sample rate of the audio stream.
    /// @returns A pointer to a new `::sensory::api::v1::audio::AudioConfig`.
    ///
    /// @details
    /// This function assumes the audio encoding is 16-bit PCM, i.e., linear
    /// 16 (`::sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16`).
    /// This function also assumed the audio stream has a single channel, i.e.,
    /// is monophonic.
    ///
    static inline ::sensory::api::v1::audio::AudioConfig* newAudioConfig(
        const float& sampleRate,
        const std::string& languageCode
    ) {
        // Create the audio config message. gRPC expects a dynamically
        // allocated message and will free the pointer when exiting the scope
        // of the request.
        auto audioConfig = new ::sensory::api::v1::audio::AudioConfig;
        audioConfig->set_encoding(
            ::sensory::api::v1::audio::AudioConfig_AudioEncoding_LINEAR16
        );
        audioConfig->set_sampleratehertz(sampleRate);
        audioConfig->set_audiochannelcount(1);
        audioConfig->set_languagecode(languageCode);
        return audioConfig;
    }

 public:
    /// @brief Initialize a new audio service.
    ///
    /// @param config_ the global configuration for the remote connection
    /// @param tokenManager_ the token manager for requesting Bearer tokens
    ///
    AudioService(
        const Config& config_,
        ::sensory::token_manager::TokenManager<SecureCredentialStore>& tokenManager_
    ) : config(config_),
        tokenManager(tokenManager_),
        models_stub(::sensory::api::v1::audio::AudioModels::NewStub(config.getChannel())),
        biometrics_stub(::sensory::api::v1::audio::AudioBiometrics::NewStub(config.getChannel())),
        events_stub(::sensory::api::v1::audio::AudioEvents::NewStub(config.getChannel())),
        transcriptions_stub(::sensory::api::v1::audio::AudioTranscriptions::NewStub(config.getChannel())) { }

    /// @brief Fetch a list of the audio models supported by the cloud host.
    ///
    /// @param response The response to populate from the RPC.
    /// @returns the status of the synchronous RPC.
    ///
    inline ::grpc::Status getModels(
        ::sensory::api::v1::audio::GetModelsResponse* response
    ) const {
        // Create a context for the client for a unary call.
        ::grpc::ClientContext context;
        config.setupClientContext(context, tokenManager, true);
        // Execute the RPC synchronously and return the status
        return models_stub->GetModels(&context, {}, response);
    }

    /// A type for biometric enrollment streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriter<
            ::sensory::api::v1::audio::CreateEnrollmentRequest,
            ::sensory::api::v1::audio::CreateEnrollmentResponse
        >
    > CreateEnrollmentStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an text-dependent audio enrollment.
    ///
    /// @param modelName The name of the model to use to create the enrollment.
    /// Use `getModels()` to obtain a list of available models.
    /// @param sampleRate The sample rate of the model.
    /// @param langaugeCode the language code of the audio.
    /// @param userID The unique user identifier.
    /// @param description The description of the enrollment
    /// @param isLivenessEnabled `true` to perform a liveness check in addition
    /// to an enrollment, `false` to perform the enrollment without the liveness
    /// check.
    /// @param numUtterances The number of utterances that should be required
    /// for text-dependent enrollments, defaults to 4 if not specified.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server
    ///
    /// @details
    /// This call will automatically send the initial `CreateEnrollmentConfig`
    /// message to the server.
    ///
    inline CreateEnrollmentStream createTextDependentEnrollment(
        const std::string& modelName,
        const int32_t& sampleRate,
        const std::string& languageCode,
        const std::string& userID,
        const std::string& description = "",
        const bool& isLivenessEnabled = false,
        const uint32_t numUtterances = 4
    ) const {
        // Create a context for the client for a bidirectional stream.
        ::grpc::ClientContext context;
        config.setupClientContext(context, tokenManager, true);

        // Create the enrollment config message. gRPC expects a dynamically
        // allocated message and will free the pointer when exiting the scope
        // of the request.
        auto enrollment_config =
            new ::sensory::api::v1::audio::CreateEnrollmentConfig;
        enrollment_config->set_allocated_audio(
            newAudioConfig(sampleRate, languageCode)
        );
        enrollment_config->set_modelname(modelName);
        enrollment_config->set_userid(userID);
        enrollment_config->set_deviceid(config.getDeviceID());
        enrollment_config->set_description(description);
        enrollment_config->set_islivenessenabled(isLivenessEnabled);
        enrollment_config->set_enrollmentnumutterances(numUtterances);

        // Create the request with the pointer to the enrollment config.
        ::sensory::api::v1::audio::CreateEnrollmentRequest request;
        request.set_allocated_config(enrollment_config);

        // Create the stream and write the initial configuration request.
        CreateEnrollmentStream stream =
            biometrics_stub->CreateEnrollment(&context);
        stream->Write(request);
        return stream;
    }

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating a text-independent audio enrollment.
    ///
    /// @param modelName The name of the model to use to create the enrollment.
    /// Use `getModels()` to obtain a list of available models.
    /// @param sampleRate The sample rate of the model.
    /// @param langaugeCode the language code of the audio.
    /// @param userID The unique user identifier.
    /// @param description The description of the enrollment
    /// @param isLivenessEnabled `true` to perform a liveness check in addition
    /// to an enrollment, `false` to perform the enrollment without the liveness
    /// check.
    /// @param enrollmentDuration The duration in seconds for text-independent
    /// enrollments, defaults to 12.5 without liveness enabled and 8 with
    /// liveness enabled.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server
    ///
    /// @details
    /// This call will automatically send the initial `CreateEnrollmentConfig`
    /// message to the server.
    ///
    inline CreateEnrollmentStream createTextIndependentEnrollment(
        const std::string& modelName,
        const int32_t& sampleRate,
        const std::string& languageCode,
        const std::string& userID,
        const std::string& description = "",
        const bool& isLivenessEnabled = false,
        const float enrollmentDuration = -1.f
    ) const {
        // Create a context for the client for a bidirectional stream.
        ::grpc::ClientContext context;
        config.setupClientContext(context, tokenManager, true);

        // Create the enrollment config message. gRPC expects a dynamically
        // allocated message and will free the pointer when exiting the scope
        // of the request.
        auto enrollment_config =
            new ::sensory::api::v1::audio::CreateEnrollmentConfig;
        enrollment_config->set_allocated_audio(
            newAudioConfig(sampleRate, languageCode)
        );
        enrollment_config->set_modelname(modelName);
        enrollment_config->set_userid(userID);
        enrollment_config->set_deviceid(config.getDeviceID());
        enrollment_config->set_description(description);
        enrollment_config->set_islivenessenabled(isLivenessEnabled);
        if (enrollmentDuration < 0.f)
            enrollment_config->set_enrollmentduration(isLivenessEnabled ? 8.f : 12.5f);
        else
            enrollment_config->set_enrollmentduration(enrollmentDuration);

        // Create the request with the pointer to the enrollment config.
        ::sensory::api::v1::audio::CreateEnrollmentRequest request;
        request.set_allocated_config(enrollment_config);

        // Create the stream and write the initial configuration request.
        CreateEnrollmentStream stream =
            biometrics_stub->CreateEnrollment(&context);
        stream->Write(request);
        return stream;
    }

    /// A type for biometric authentication streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriter<
            ::sensory::api::v1::audio::AuthenticateRequest,
            ::sensory::api::v1::audio::AuthenticateResponse
        >
    > AuthenticateStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// authentication.
    ///
    /// @param enrollmentID The enrollment ID to authenticate against. This can
    /// be either an enrollment ID or a group ID.
    /// @param sampleRate The sample rate of the model.
    /// @param langaugeCode the language code of the audio.
    /// @param isLivenessEnabled `true` to perform a liveness check before the
    /// authentication, `false` to only perform the authentication.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the initial `AuthenticateConfig`
    /// message to the server.
    ///
    inline AuthenticateStream authenticate(
        const std::string& enrollmentID,
        const int32_t& sampleRate,
        const std::string& languageCode,
        const bool& isLivenessEnabled = false
    ) const {
        // Create a context for the client for a bidirectional stream.
        ::grpc::ClientContext context;
        config.setupClientContext(context, tokenManager, true);

        // Create the authenticate config message. gRPC expects a dynamically
        // allocated message and will free the pointer when exiting the scope
        // of the request.
        auto authenticateConfig =
            new ::sensory::api::v1::audio::AuthenticateConfig;
        authenticateConfig->set_allocated_audio(
            newAudioConfig(sampleRate, languageCode)
        );
        authenticateConfig->set_enrollmentid(enrollmentID);
        authenticateConfig->set_islivenessenabled(isLivenessEnabled);

        // Create the request with the pointer to the enrollment config.
        ::sensory::api::v1::audio::AuthenticateRequest request;
        request.set_allocated_config(authenticateConfig);

        // Create the stream and write the initial configuration request.
        AuthenticateStream stream =
            biometrics_stub->Authenticate(&context);
        stream->Write(request);
        return stream;
    }

    /// A type for trigger validation streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriter<
            ::sensory::api::v1::audio::ValidateEventRequest,
            ::sensory::api::v1::audio::ValidateEventResponse
        >
    > ValidateTriggerStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// audio event validation.
    ///
    /// @param modelName The name of the model to use to validate the trigger.
    /// Use `getModels()` to obtain a list of available models.
    /// @param sampleRate The sample rate of the model.
    /// @param langaugeCode the language code of the audio.
    /// @param userID The unique user identifier.
    /// @param sensitivity How sensitive the model should be to false accepts.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the initial `ValidateEventConfig`
    /// message to the server.
    ///
    inline ValidateTriggerStream validateTrigger(
        const std::string& modelName,
        const int32_t& sampleRate,
        const std::string& languageCode,
        const std::string& userID,
        const sensory::api::v1::audio::ThresholdSensitivity& sensitivity
    ) const {
        // Create a context for the client for a bidirectional stream.
        ::grpc::ClientContext context;
        config.setupClientContext(context, tokenManager, true);

        // Create the validate event message. gRPC expects a dynamically
        // allocated message and will free the pointer when exiting the scope
        // of the request.
        auto validateEventConfig =
            new ::sensory::api::v1::audio::ValidateEventConfig;
        validateEventConfig->set_allocated_audio(
            newAudioConfig(sampleRate, languageCode)
        );
        validateEventConfig->set_modelname(modelName);
        validateEventConfig->set_userid(userID);
        validateEventConfig->set_sensitivity(sensitivity);

        // Create the request with the pointer to the enrollment config.
        ::sensory::api::v1::audio::ValidateEventRequest request;
        request.set_allocated_config(validateEventConfig);

        // Create the stream and write the initial configuration request.
        ValidateTriggerStream stream =
            events_stub->ValidateEvent(&context);
        stream->Write(request);
        return stream;
    }

    /// a type for biometric transcription streams
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriter<
            ::sensory::api::v1::audio::TranscribeRequest,
            ::sensory::api::v1::audio::TranscribeResponse
        >
    > TranscribeAudioStream;

    /// @brief Open a bidirectional stream to the server that provides a
    /// transcription of the provided audio data.
    ///
    /// @param modelName The name of the model to use to transcribe the audio.
    /// Use `getModels()` to obtain a list of available models.
    /// @param sampleRate The sample rate of the model.
    /// @param langaugeCode the language code of the audio.
    /// @param userID The unique user identifier.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the initial `TranscribeConfig`
    /// message to the server.
    ///
    inline TranscribeAudioStream transcribeAudio(
        const std::string& modelName,
        const int32_t& sampleRate,
        const std::string& languageCode,
        const std::string& userID
    ) const {
        // Create a context for the client for a bidirectional stream.
        ::grpc::ClientContext context;
        config.setupClientContext(context, tokenManager, true);

        // Create the transcribe audio message. gRPC expects a dynamically
        // allocated message and will free the pointer when exiting the scope
        // of the request.
        auto transcribeConfig = new ::sensory::api::v1::audio::TranscribeConfig;
        transcribeConfig->set_allocated_audio(
            newAudioConfig(sampleRate, languageCode)
        );
        transcribeConfig->set_modelname(modelName);
        transcribeConfig->set_userid(userID);

        // Create the request with the pointer to the enrollment config.
        ::sensory::api::v1::audio::TranscribeRequest request;
        request.set_allocated_config(transcribeConfig);

        // Create the stream and write the initial configuration request.
        TranscribeAudioStream stream =
            transcriptions_stub->Transcribe(&context);
        stream->Write(request);
        return stream;
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORY_CLOUD_SERVICES_AUDIO_SERVICE_HPP_
