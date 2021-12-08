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

#include <memory>
#include <string>
#include <utility>
#include "sensorycloud/generated/v1/audio/audio.pb.h"
#include "sensorycloud/generated/v1/audio/audio.grpc.pb.h"
#include "sensorycloud/config.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/call_data.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud services.
namespace service {

/// @brief A service for audio data.
/// @tparam SecureCredentialStore A secure key-value store for storing and
/// fetching credentials and tokens.
template<typename SecureCredentialStore>
class AudioService {
 private:
    /// the global configuration for the remote connection
    const ::sensory::Config& config;
    /// the token manager for securing gRPC requests to the server
    ::sensory::token_manager::TokenManager<SecureCredentialStore>& tokenManager;
    /// the gRPC stub for the audio models service
    std::unique_ptr<::sensory::api::v1::audio::AudioModels::Stub> modelsStub;
    /// the gRPC stub for the audio biometrics service
    std::unique_ptr<::sensory::api::v1::audio::AudioBiometrics::Stub> biometricStub;
    /// the gRPC stub for the audio events service
    std::unique_ptr<::sensory::api::v1::audio::AudioEvents::Stub> eventsStub;
    /// the gRPC stub for the audio transcriptions service
    std::unique_ptr<::sensory::api::v1::audio::AudioTranscriptions::Stub> transcriptionsStub;

    /// @brief Create a copy of this object.
    ///
    /// @param other the other instance to copy data from
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object
    AudioService(const AudioService& other) = delete;

    /// @brief Assign to this object using the `=` operator.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This assignment operator is private to prevent copying of this object.
    ///
    void operator=(const AudioService& other) = delete;

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
    /// @param config_ The global configuration for the remote connection.
    /// @param tokenManager_ The token manager for requesting Bearer tokens.
    ///
    AudioService(
        const ::sensory::Config& config_,
        ::sensory::token_manager::TokenManager<SecureCredentialStore>& tokenManager_
    ) : config(config_),
        tokenManager(tokenManager_),
        modelsStub(::sensory::api::v1::audio::AudioModels::NewStub(config.getChannel())),
        biometricStub(::sensory::api::v1::audio::AudioBiometrics::NewStub(config.getChannel())),
        eventsStub(::sensory::api::v1::audio::AudioEvents::NewStub(config.getChannel())),
        transcriptionsStub(::sensory::api::v1::audio::AudioTranscriptions::NewStub(config.getChannel())) { }

    // ----- Get Models --------------------------------------------------------

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
        config.setupUnaryClientContext(context, tokenManager);
        // Execute the RPC synchronously and return the status
        return modelsStub->GetModels(&context, {}, response);
    }

    /// A type for asynchronous model fetching.
    typedef std::unique_ptr<
        ::grpc::ClientAsyncResponseReader<
            ::sensory::api::v1::audio::GetModelsResponse
        >
    > AsyncGetModelsReader;

    /// @brief Fetch a list of the vision models supported by the cloud host.
    ///
    /// @param response The response to populate from the RPC.
    /// @returns The status of the synchronous RPC.
    ///
    inline AsyncGetModelsReader asyncGetModels(
        ::grpc::CompletionQueue* queue
    ) const {
        // Create a context for the client for a unary call.
        // TODO: this will result in a memory leak. Update to remove the memory
        // link by persisting a stack allocated context past the scope of this
        // call to setup the stream.
        auto context(new ::grpc::ClientContext);
        config.setupUnaryClientContext(*context, tokenManager);
        // Execute the RPC synchronously and return the status
        return modelsStub->AsyncGetModels(context, {}, queue);
    }

    /// @brief A type for encapsulating data for asynchronous `GetModels` calls.
    typedef ::sensory::CallData<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::GetModelsRequest,
        ::sensory::api::v1::audio::GetModelsResponse
    > GetModelsCallData;

    /// @brief Fetch a list of the vision models supported by the cloud host.
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `GetModelsCallData*`.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the asynchronous call spawned by this call.
    ///
    template<typename Callback>
    inline std::shared_ptr<GetModelsCallData> asyncGetModels(
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. Setup the call as usual with a bearer token and
        // any application deadlines. This call is initiated as a shared pointer
        // in order to reference count between the parent and child context.
        // This also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<GetModelsCallData> call(new GetModelsCallData);
        config.setupUnaryClientContext(call->context, tokenManager);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        modelsStub->async()->GetModels(
            &call->context,
            &call->request,
            &call->response,
            [call, callback](::grpc::Status status) {
                // Copy the status to the call.
                call->status = std::move(status);
                // Call the callback function with a raw pointer because
                // ownership is not being transferred.
                callback(call.get());
                // Mark the call as done for any awaiting process.
                call->isDone = true;
            });
        return call;
    }

    // ----- Create Enrollment -------------------------------------------------

    /// A type for biometric enrollment streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriter<
            ::sensory::api::v1::audio::CreateEnrollmentRequest,
            ::sensory::api::v1::audio::CreateEnrollmentResponse
        >
    > CreateEnrollmentStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an audio enrollment.
    ///
    /// @param modelName The name of the model to use to create the enrollment.
    /// Use `getModels()` to obtain a list of available models.
    /// @param sampleRate The sample rate of the audio stream.
    /// @param langaugeCode The language code of the audio stream.
    /// @param userID The ID of the user making the request.
    /// @param description The description of the enrollment.
    /// @param isLivenessEnabled `true` to perform a liveness check in addition
    /// to an enrollment, `false` to perform the enrollment without the liveness
    /// check.
    /// @param enrollmentDuration The duration in seconds for text-independent
    /// enrollments, defaults to 12.5 without liveness enabled and 8 with
    /// liveness enabled.
    /// @param numUtterances The number of utterances that should be required
    /// for text-dependent enrollments, defaults to 4 if not specified.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the initial `CreateEnrollmentConfig`
    /// message to the server.
    ///
    /// Note that for text-_independent_ models, an enrollment duration can be
    /// specified, but the number of utterances do not apply. Conversely, for
    /// text-_dependent_ enrollments, a number of utterances may be provided,
    /// but an enrollment duration does not apply.
    ///
    /// The enrollment duration for text-independent enrollments controls the
    /// maximal amount of time allowed for authentication.
    ///
    /// The number of utterances for text-dependent enrollments controls the
    /// number of uttered text fragments that must be emitted to authenticate.
    ///
    inline CreateEnrollmentStream createEnrollment(
        const std::string& modelName,
        const int32_t& sampleRate,
        const std::string& languageCode,
        const std::string& userID,
        const std::string& description = "",
        const bool& isLivenessEnabled = false,
        const float enrollmentDuration = -1.f,
        const int32_t numUtterances = -1
    ) const {
        // Create a context for the client for a bidirectional stream.
        // TODO: will the stream automatically free this dynamically allocated
        // context?
        auto context = new ::grpc::ClientContext;
        config.setupBidiClientContext(*context, tokenManager);

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
        // Set the model dependent metadata based on the passed in values. The
        // number of utterances and the enrollment duration cannot both be
        // specified in the message, so check for sentinel "null" values and if
        // both are provided, throw an error. Otherwise, only set the parameter
        // that was specified. The sentinel value for both is any negative
        // number. If neither is specified, provide neither and fall back on the
        // default values used on the server.
        if (enrollmentDuration > 0.f && numUtterances > 0)
            throw std::runtime_error("enrollmentDuration and numUterrances cannot both be specified.");
        else if (enrollmentDuration > 0.f)
            enrollment_config->set_enrollmentduration(isLivenessEnabled ? 8.f : 12.5f);
        else if (numUtterances > 0)
            enrollment_config->set_enrollmentnumutterances(numUtterances);

        // Create the request with the pointer to the enrollment config.
        ::sensory::api::v1::audio::CreateEnrollmentRequest request;
        request.set_allocated_config(enrollment_config);

        // Create the stream and write the initial configuration request.
        CreateEnrollmentStream stream =
            biometricStub->CreateEnrollment(context);
        stream->Write(request);
        return stream;
    }

    // ----- Authenticate ------------------------------------------------------

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
    /// @param sampleRate The sample rate of the audio stream.
    /// @param langaugeCode The language code of the audio stream.
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
        const bool& isLivenessEnabled = false,
        const ::sensory::api::v1::audio::ThresholdSensitivity& sensitivity =
            ::sensory::api::v1::audio::ThresholdSensitivity::LOW,
        const ::sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity& security =
            ::sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW
    ) const {
        // Create a context for the client for a bidirectional stream.
        // TODO: will the stream automatically free this dynamically allocated
        // context?
        auto context = new ::grpc::ClientContext;
        config.setupBidiClientContext(*context, tokenManager);

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
        authenticateConfig->set_sensitivity(sensitivity);
        authenticateConfig->set_security(security);

        // Create the request with the pointer to the enrollment config.
        ::sensory::api::v1::audio::AuthenticateRequest request;
        request.set_allocated_config(authenticateConfig);

        // Create the stream and write the initial configuration request.
        AuthenticateStream stream =
            biometricStub->Authenticate(context);
        stream->Write(request);
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `Authenticate` calls.
    typedef ::sensory::AwaitableBidiReactor<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::AuthenticateRequest,
        ::sensory::api::v1::audio::AuthenticateResponse
    > AuthenticateBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// audio event validation.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param enrollmentID The enrollment ID to authenticate against. This can
    /// be either an enrollment ID or a group ID.
    /// @param sampleRate The sample rate of the audio stream.
    /// @param langaugeCode The language code of the audio stream.
    /// @param isLivenessEnabled `true` to perform a liveness check before the
    /// authentication, `false` to only perform the authentication.
    ///
    /// @details
    /// This call will automatically send the initial `AuthenticateConfig`
    /// message to the server.
    ///
    template<typename Reactor>
    inline void asyncAuthenticate(Reactor* reactor,
        const std::string& enrollmentID,
        const int32_t& sampleRate,
        const std::string& languageCode,
        const bool& isLivenessEnabled = false,
        const ::sensory::api::v1::audio::ThresholdSensitivity& sensitivity =
            ::sensory::api::v1::audio::ThresholdSensitivity::LOW,
        const ::sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity& security =
            ::sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity_LOW
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(reactor->context, tokenManager);

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
        authenticateConfig->set_sensitivity(sensitivity);
        authenticateConfig->set_security(security);

        // Create the request with the pointer to the enrollment config.
        reactor->request.set_allocated_config(authenticateConfig);

        // Create the stream and write the initial configuration request.
        biometricStub->async()->Authenticate(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }

    // ----- Validate Trigger --------------------------------------------------

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
    /// @param sampleRate The sample rate of the audio stream.
    /// @param langaugeCode The language code of the audio stream.
    /// @param userID The ID of the user making the request.
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
        // TODO: will the stream automatically free this dynamically allocated
        // context?
        auto context = new ::grpc::ClientContext;
        config.setupBidiClientContext(*context, tokenManager);

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
            eventsStub->ValidateEvent(context);
        stream->Write(request);
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `ValidateEvent` calls.
    typedef ::sensory::AwaitableBidiReactor<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::ValidateEventRequest,
        ::sensory::api::v1::audio::ValidateEventResponse
    > ValidateEventBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// audio event validation.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param modelName The name of the model to use to validate the trigger.
    /// Use `getModels()` to obtain a list of available models.
    /// @param sampleRate The sample rate of the audio stream.
    /// @param langaugeCode The language code of the audio stream.
    /// @param userID The ID of the user making the request.
    /// @param sensitivity How sensitive the model should be to false accepts.
    ///
    /// @details
    /// This call will automatically send the initial `ValidateEventConfig`
    /// message to the server.
    ///
    template<typename Reactor>
    inline void asyncValidateTrigger(Reactor* reactor,
        const std::string& modelName,
        const int32_t& sampleRate,
        const std::string& languageCode,
        const std::string& userID,
        const sensory::api::v1::audio::ThresholdSensitivity& sensitivity
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(reactor->context, tokenManager);

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
        reactor->request.set_allocated_config(validateEventConfig);

        // Create the stream and write the initial configuration request.
        eventsStub->async()->ValidateEvent(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }

    // ----- Transcribe Audio --------------------------------------------------

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
    /// @param sampleRate The sample rate of the audio stream.
    /// @param langaugeCode The language code of the audio stream.
    /// @param userID The ID of the user making the request.
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
        // TODO: will the stream automatically free this dynamically allocated
        // context?
        auto context = new ::grpc::ClientContext;
        config.setupBidiClientContext(*context, tokenManager);

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
            transcriptionsStub->Transcribe(context);
        stream->Write(request);
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `Transcribe` calls.
    typedef ::sensory::AwaitableBidiReactor<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::TranscribeRequest,
        ::sensory::api::v1::audio::TranscribeResponse
    > TranscribeBidiReactor;

    /// @brief Open a bidirectional stream to the server that provides a
    /// transcription of the provided audio data.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param modelName The name of the model to use to transcribe the audio.
    /// Use `getModels()` to obtain a list of available models.
    /// @param sampleRate The sample rate of the audio stream.
    /// @param langaugeCode The language code of the audio stream.
    /// @param userID The ID of the user making the request.
    ///
    /// @details
    /// This call will automatically send the initial `TranscribeConfig`
    /// message to the server.
    ///
    template<typename Reactor>
    inline void asyncTranscribeAudio(Reactor* reactor,
        const std::string& modelName,
        const int32_t& sampleRate,
        const std::string& languageCode,
        const std::string& userID
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(reactor->context, tokenManager);

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
        reactor->request.set_allocated_config(transcribeConfig);

        // Create the stream and write the initial configuration request.
        transcriptionsStub->async()->Transcribe(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORY_CLOUD_SERVICES_AUDIO_SERVICE_HPP_
