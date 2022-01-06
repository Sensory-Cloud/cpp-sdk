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

/// @brief Create a new audio config for an audio streaming application.
///
/// @param encoding The encoding of the samples in the byte-stream.
/// @param sampleRateHertz The sample rate of the audio stream.
/// @param audioChannelCount The number of audio channels in the audio.
/// @param languageCode The language code for the speech in the audio.
/// @returns A pointer to a new `::sensory::api::v1::audio::AudioConfig`.
///
inline ::sensory::api::v1::audio::AudioConfig* newAudioConfig(
    const ::sensory::api::v1::audio::AudioConfig_AudioEncoding& encoding,
    const float& sampleRateHertz,
    const uint32_t& audioChannelCount,
    const std::string& languageCode
) {
    auto audioConfig = new ::sensory::api::v1::audio::AudioConfig;
    audioConfig->set_encoding(encoding);
    audioConfig->set_sampleratehertz(sampleRateHertz);
    audioConfig->set_audiochannelcount(audioChannelCount);
    audioConfig->set_languagecode(languageCode);
    return audioConfig;
}

/// @brief Allocate a new configuration object for enrollment creation.
///
/// @param modelName The name of the model to use to create the enrollment.
/// @param userID The ID of the user making the request.
/// @param description The description of the enrollment.
/// @param isLivenessEnabled `true` to perform a liveness check in addition
/// to an enrollment, `false` to perform the enrollment without the liveness
/// check.
/// @param enrollmentDuration The duration in seconds for text-independent
/// enrollments, defaults to \f$12.5\f$ without liveness enabled and \f$8\f$
/// with liveness enabled.
/// @param numUtterances The number of utterances that should be required
/// for text-dependent enrollments, defaults to \f$4\f$ if not specified.
///
/// @returns A pointer to the `CreateEnrollmentConfig` object.
///
/// @throws std::runtime_error if `numUtterances` and `enrollmentDuration`
/// are both specified. For _text-independent_ models, an enrollment
/// duration can be specified, but the number of utterances do not apply.
/// Conversely, for _text-dependent_ enrollments, a number of utterances
/// may be provided, but an enrollment duration does not apply.
///
/// @details
/// The enrollment duration for text-independent enrollments controls the
/// maximal amount of time allowed for authentication.
///
/// The number of utterances for text-dependent enrollments controls the
/// number of uttered phrases that must be emitted to authenticate.
///
inline ::sensory::api::v1::audio::CreateEnrollmentConfig* newCreateEnrollmentConfig(
    const std::string& modelName,
    const std::string& userID,
    const std::string& description,
    const bool& isLivenessEnabled,
    const float enrollmentDuration = 0.f,
    const int32_t numUtterances = 0
) {
    // The number of utterances and the enrollment duration cannot both be
    // specified in the message, so check for sentinel "null" values and if
    // both are provided, throw an error. This check is conducted before
    // allocation of the config to prevent memory leaks.
    if (enrollmentDuration > 0 && numUtterances > 0)
        throw std::runtime_error("enrollmentDuration and numUterrances cannot both be specified.");
    auto config = new ::sensory::api::v1::audio::CreateEnrollmentConfig;
    config->set_modelname(modelName);
    config->set_userid(userID);
    config->set_description(description);
    config->set_islivenessenabled(isLivenessEnabled);
    if (enrollmentDuration > 0)  // enrollment duration provided
        config->set_enrollmentduration(enrollmentDuration);
    else if (numUtterances > 0)  // number of utterances provided
        config->set_enrollmentnumutterances(numUtterances);
    return config;
}

/// @brief Allocate a new configuration object for enrollment authentication.
///
/// @param enrollmentID The enrollment ID to authenticate against. This can
/// be either an enrollment ID or a group ID.
/// @param isLivenessEnabled `true` to perform a liveness check before the
/// authentication, `false` to only perform the authentication.
/// @param sensitivity The sensitivity of the model.
/// @param security The security level of the model.
/// @returns A pointer to the `AuthenticateConfig` object.
///
inline ::sensory::api::v1::audio::AuthenticateConfig* newAuthenticateConfig(
    const std::string& enrollmentID,
    const bool& isLivenessEnabled,
    const ::sensory::api::v1::audio::ThresholdSensitivity& sensitivity,
    const ::sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity& security
) {
    auto config = new ::sensory::api::v1::audio::AuthenticateConfig;
    config->set_enrollmentid(enrollmentID);
    config->set_islivenessenabled(isLivenessEnabled);
    config->set_sensitivity(sensitivity);
    config->set_security(security);
    return config;
}

/// @brief Allocate a new configuration object for trigger validation.
///
/// @param modelName The name of the model to use to validate the trigger.
/// @param userID The ID of the user making the request.
/// @param sensitivity The sensitivity of the model.
/// @returns A pointer to the `ValidateEventConfig` object.
///
inline ::sensory::api::v1::audio::ValidateEventConfig* newValidateEventConfig(
    const std::string& modelName,
    const std::string& userID,
    const sensory::api::v1::audio::ThresholdSensitivity& sensitivity
) {
    auto config = new ::sensory::api::v1::audio::ValidateEventConfig;
    config->set_modelname(modelName);
    config->set_userid(userID);
    config->set_sensitivity(sensitivity);
    return config;
}

/// @brief Allocate a new configuration object for audio transcription.
///
/// @param modelName The name of the model to use to transcribe the audio.
/// @param userID The ID of the user making the request.
/// @returns A pointer to the `TranscribeConfig` object.
///
inline ::sensory::api::v1::audio::TranscribeConfig* newTranscribeConfig(
    const std::string& modelName,
    const std::string& userID
) {
    auto config = new ::sensory::api::v1::audio::TranscribeConfig;
    config->set_modelname(modelName);
    config->set_userid(userID);
    return config;
}

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

    /// @brief A type for encapsulating data for asynchronous `GetModels` calls
    /// based on CompletionQueue event loops.
    typedef AsyncResponseReaderCall<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::GetModelsRequest,
        ::sensory::api::v1::audio::GetModelsResponse
    > GetModelsAsyncCall;

    /// @brief Fetch a list of the vision models supported by the cloud host.
    ///
    /// @param queue The completion queue handling the event-loop processing.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    inline GetModelsAsyncCall* getModels(::grpc::CompletionQueue* queue) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new GetModelsAsyncCall);
        // Set the client context for a unary call.
        config.setupUnaryClientContext(call->context, tokenManager);
        // Start the asynchronous RPC with the call's context and queue.
        call->rpc = modelsStub->AsyncGetModels(&call->context, call->request, queue);
        // Finish the RPC to tell it where the response and status buffers are
        // located within the call object. Use the address of the call as the
        // tag for identifying the call in the event-loop.
        call->rpc->Finish(&call->response, &call->status, static_cast<void*>(call));
        // Return the pointer to the call. This both transfers the ownership
        // of the instance to the caller, and provides the caller with an
        // identifier for detecting the result of this call in the completion
        // queue.
        return call;
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
    inline std::shared_ptr<GetModelsCallData> getModels(
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
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `newAudioConfig` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param enrollmentConfig The enrollment configuration for the stream.
    /// Use `newCreateEnrollmentConfig` to create a new enrollment config.
    /// _Ownership of the dynamically allocated configuration is implicitly
    /// transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `CreateEnrollmentConfig` message
    /// to the server.
    ///
    inline CreateEnrollmentStream createEnrollment(
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::CreateEnrollmentConfig* enrollmentConfig
    ) const {
        // Create a context for the client for a bidirectional stream.
        // TODO: will the stream automatically free this dynamically allocated
        // context?
        auto context = new ::grpc::ClientContext;
        config.setupBidiClientContext(*context, tokenManager);
        // Create the request with the pointer to the config.
        enrollmentConfig->set_allocated_audio(audioConfig);
        enrollmentConfig->set_deviceid(config.getDeviceID());
        ::sensory::api::v1::audio::CreateEnrollmentRequest request;
        request.set_allocated_config(enrollmentConfig);
        // Create the stream and write the initial configuration request.
        CreateEnrollmentStream stream = biometricStub->CreateEnrollment(context);
        stream->Write(request);
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `CreateEnrollment`
    /// calls based on CompletionQueue event loops.
    typedef AsyncReaderWriterCall<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::CreateEnrollmentRequest,
        ::sensory::api::v1::audio::CreateEnrollmentResponse
    > CreateEnrollmentAsyncCall;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an audio enrollment.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `newAudioConfig` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param enrollmentConfig The enrollment configuration for the stream.
    /// Use `newCreateEnrollmentConfig` to create a new enrollment config.
    /// _Ownership of the dynamically allocated configuration is implicitly
    /// transferred to the stream_.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    /// @details
    /// This call will **NOT** automatically send the `CreateEnrollmentConfig`
    /// message to the server, but will buffer it in the message for later
    /// transmission.
    ///
    inline CreateEnrollmentAsyncCall* createEnrollment(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::CreateEnrollmentConfig* enrollmentConfig
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new CreateEnrollmentAsyncCall);
        // Set the client context for a unary call.
        config.setupBidiClientContext(call->context, tokenManager);
        // Create the request with the pointer to the config.
        enrollmentConfig->set_allocated_audio(audioConfig);
        enrollmentConfig->set_deviceid(config.getDeviceID());
        call->request.set_allocated_config(enrollmentConfig);
        // Start the asynchronous RPC with the call's context and queue.
        call->rpc = biometricStub->AsyncCreateEnrollment(&call->context, queue, static_cast<void*>(call));

        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `CreateEnrollment` calls.
    typedef ::sensory::AwaitableBidiReactor<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::CreateEnrollmentRequest,
        ::sensory::api::v1::audio::CreateEnrollmentResponse
    > CreateEnrollmentBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an audio enrollment.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `newAudioConfig` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param enrollmentConfig The enrollment configuration for the stream.
    /// Use `newCreateEnrollmentConfig` to create a new enrollment config.
    /// _Ownership of the dynamically allocated configuration is implicitly
    /// transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `CreateEnrollmentConfig` message
    /// to the server.
    ///
    template<typename Reactor>
    inline void createEnrollment(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::CreateEnrollmentConfig* enrollmentConfig
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(reactor->context, tokenManager);
        // Create the request with the pointer to the config.
        enrollmentConfig->set_allocated_audio(audioConfig);
        enrollmentConfig->set_deviceid(config.getDeviceID());
        reactor->request.set_allocated_config(enrollmentConfig);
        // Create the stream and write the initial configuration request.
        biometricStub->async()->CreateEnrollment(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
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
    /// enrollment authentication.
    ///
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `newAudioConfig` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param authenticateConfig The authentication configuration for the
    /// stream. Use `newAuthenticateConfig` to create a new authentication
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `AuthenticateConfig` message to
    /// the server.
    ///
    inline AuthenticateStream authenticate(
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::AuthenticateConfig* authenticateConfig
    ) const {
        // Create a context for the client for a bidirectional stream.
        // TODO: will the stream automatically free this dynamically allocated
        // context?
        auto context = new ::grpc::ClientContext;
        config.setupBidiClientContext(*context, tokenManager);
        // Create the request with the pointer to the enrollment config.
        authenticateConfig->set_allocated_audio(audioConfig);
        ::sensory::api::v1::audio::AuthenticateRequest request;
        request.set_allocated_config(authenticateConfig);
        // Create the stream and write the initial configuration request.
        AuthenticateStream stream = biometricStub->Authenticate(context);
        stream->Write(request);
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `Authenticate`
    /// calls based on CompletionQueue event loops.
    typedef AsyncReaderWriterCall<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::AuthenticateRequest,
        ::sensory::api::v1::audio::AuthenticateResponse
    > AuthenticateAsyncCall;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// enrollment authentication.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `newAudioConfig` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param authenticateConfig The authentication configuration for the
    /// stream. Use `newAuthenticateConfig` to create a new authentication
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    /// @details
    /// This call will **NOT** automatically send the `AuthenticateConfig`
    /// message to the server, but will buffer it in the message for later
    /// transmission.
    ///
    inline AuthenticateAsyncCall* authenticate(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::AuthenticateConfig* authenticateConfig
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new AuthenticateAsyncCall);
        // Set the client context for a unary call.
        config.setupBidiClientContext(call->context, tokenManager);
        // Create the request with the pointer to the enrollment config.
        authenticateConfig->set_allocated_audio(audioConfig);
        call->request.set_allocated_config(authenticateConfig);
        // Start the asynchronous RPC with the call's context and queue.
        call->rpc = biometricStub->AsyncAuthenticate(&call->context, queue, static_cast<void*>(call));
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `Authenticate` calls.
    typedef ::sensory::AwaitableBidiReactor<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::AuthenticateRequest,
        ::sensory::api::v1::audio::AuthenticateResponse
    > AuthenticateBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// enrollment authentication.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `newAudioConfig` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param authenticateConfig The authentication configuration for the
    /// stream. Use `newAuthenticateConfig` to create a new authentication
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `AuthenticateConfig` message to
    /// the server.
    ///
    template<typename Reactor>
    inline void authenticate(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::AuthenticateConfig* authenticateConfig
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(reactor->context, tokenManager);
        // Create the request with the pointer to the enrollment config.
        authenticateConfig->set_allocated_audio(audioConfig);
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
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `newAudioConfig` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param validateEventConfig The trigger validation configuration for the
    /// stream. Use `newValidateEventConfig` to create a new trigger validation
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `ValidateEventConfig` message to
    /// the server.
    ///
    inline ValidateTriggerStream validateTrigger(
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::ValidateEventConfig* validateEventConfig
    ) const {
        // Create a context for the client for a bidirectional stream.
        // TODO: will the stream automatically free this dynamically allocated
        // context?
        auto context = new ::grpc::ClientContext;
        config.setupBidiClientContext(*context, tokenManager);
        // Create the request with the pointer to the enrollment config.
        validateEventConfig->set_allocated_audio(audioConfig);
        ::sensory::api::v1::audio::ValidateEventRequest request;
        request.set_allocated_config(validateEventConfig);
        // Create the stream and write the initial configuration request.
        ValidateTriggerStream stream = eventsStub->ValidateEvent(context);
        stream->Write(request);
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `ValidateEvent`
    /// calls based on CompletionQueue event loops.
    typedef AsyncReaderWriterCall<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::ValidateEventRequest,
        ::sensory::api::v1::audio::ValidateEventResponse
    > ValidateEventAsyncCall;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// audio event validation.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `newAudioConfig` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param validateEventConfig The trigger validation configuration for the
    /// stream. Use `newValidateEventConfig` to create a new trigger validation
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    /// @details
    /// This call will **NOT** automatically send the `ValidateEventConfig`
    /// message to the server, but will buffer it in the message for later
    /// transmission.
    ///
    inline ValidateEventAsyncCall* validateTrigger(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::ValidateEventConfig* validateEventConfig
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new ValidateEventAsyncCall);
        // Set the client context for a bidirectional stream.
        config.setupBidiClientContext(call->context, tokenManager);
        // Create the request with the pointer to the enrollment config.
        validateEventConfig->set_allocated_audio(audioConfig);
        call->request.set_allocated_config(validateEventConfig);
        // Start the asynchronous RPC with the call's context and queue.
        call->rpc = eventsStub->AsyncValidateEvent(&call->context, queue, static_cast<void*>(call));
        return call;
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
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `newAudioConfig` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param validateEventConfig The trigger validation configuration for the
    /// stream. Use `newValidateEventConfig` to create a new trigger validation
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `ValidateEventConfig` message to
    /// the server.
    ///
    template<typename Reactor>
    inline void validateTrigger(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::ValidateEventConfig* validateEventConfig
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(reactor->context, tokenManager);
        // Create the request with the pointer to the enrollment config.
        validateEventConfig->set_allocated_audio(audioConfig);
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
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `newAudioConfig` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param transcribeConfig The audio transcription configuration for the
    /// stream. Use `newTranscribeConfig` to create a new audio transcription
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `TranscribeConfig` message to the
    /// server.
    ///
    inline TranscribeAudioStream transcribeAudio(
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::TranscribeConfig* transcribeConfig
    ) const {
        // Create a context for the client for a bidirectional stream.
        // TODO: will the stream automatically free this dynamically allocated
        // context?
        auto context = new ::grpc::ClientContext;
        config.setupBidiClientContext(*context, tokenManager);
        // Create the request with the pointer to the enrollment config.
        transcribeConfig->set_allocated_audio(audioConfig);
        ::sensory::api::v1::audio::TranscribeRequest request;
        request.set_allocated_config(transcribeConfig);
        // Create the stream and write the initial configuration request.
        TranscribeAudioStream stream = transcriptionsStub->Transcribe(context);
        stream->Write(request);
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `Transcribe`
    /// calls based on CompletionQueue event loops.
    typedef AsyncReaderWriterCall<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::TranscribeRequest,
        ::sensory::api::v1::audio::TranscribeResponse
    > TranscribeAsyncCall;

    /// @brief Open a bidirectional stream to the server that provides a
    /// transcription of the provided audio data.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `newAudioConfig` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param transcribeConfig The audio transcription configuration for the
    /// stream. Use `newTranscribeConfig` to create a new audio transcription
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    /// @details
    /// This call will **NOT** automatically send the `ValidateEventConfig`
    /// message to the server, but will buffer it in the message for later
    /// transmission.
    ///
    inline TranscribeAsyncCall* transcribeAudio(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::TranscribeConfig* transcribeConfig
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new TranscribeAsyncCall);
        // Set the client context for a unary call.
        config.setupBidiClientContext(call->context, tokenManager);
        // Create the request with the pointer to the enrollment config.
        transcribeConfig->set_allocated_audio(audioConfig);
        call->request.set_allocated_config(transcribeConfig);
        // Start the asynchronous RPC with the call's context and queue.
        call->rpc = transcriptionsStub->AsyncTranscribe(&call->context, queue, static_cast<void*>(call));
        return call;
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
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `newAudioConfig` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param transcribeConfig The audio transcription configuration for the
    /// stream. Use `newTranscribeConfig` to create a new audio transcription
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `TranscribeConfig` message to the
    /// server.
    ///
    template<typename Reactor>
    inline void transcribeAudio(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::TranscribeConfig* transcribeConfig
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(reactor->context, tokenManager);
        // Create the request with the pointer to the enrollment config.
        transcribeConfig->set_allocated_audio(audioConfig);
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
