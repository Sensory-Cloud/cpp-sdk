// The audio service for the SensoryCloud SDK.
//
// Copyright (c) 2021 Sensory, Inc.
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

#ifndef SENSORYCLOUD_SERVICES_AUDIO_SERVICE_HPP_
#define SENSORYCLOUD_SERVICES_AUDIO_SERVICE_HPP_

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include "sensorycloud/generated/v1/audio/audio.pb.h"
#include "sensorycloud/generated/v1/audio/audio.grpc.pb.h"
#include "sensorycloud/config.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/call_data.hpp"
#include "sensorycloud/services/errors.hpp"

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief SensoryCloud services.
namespace service {

/// @brief SensoryCloud audio service.
namespace audio {

/// @brief Create a new audio config for an audio streaming application.
///
/// @param encoding The encoding of the samples in the byte-stream.
/// @param sample_rate_hertz The sample rate of the audio stream.
/// @param audio_channel_count The number of audio channels in the audio.
/// @param language_code The language code for the speech in the audio.
/// @returns A pointer to a new `::sensory::api::v1::audio::AudioConfig`.
///
inline ::sensory::api::v1::audio::AudioConfig* new_audio_config(
    const ::sensory::api::v1::audio::AudioConfig_AudioEncoding& encoding,
    const float& sample_rate_hertz,
    const uint32_t& audio_channel_count,
    const std::string& language_code
) {
    auto config = new ::sensory::api::v1::audio::AudioConfig;
    config->set_encoding(encoding);
    config->set_sampleratehertz(sample_rate_hertz);
    config->set_audiochannelcount(audio_channel_count);
    config->set_languagecode(language_code);
    return config;
}

// TODO: implement `referenceId` for CreateEnrollmentConfig?

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
inline ::sensory::api::v1::audio::CreateEnrollmentConfig* new_create_enrollment_config(
    const std::string& modelName,
    const std::string& userID,
    const std::string& description,
    const bool& isLivenessEnabled,
    const float enrollmentDuration = 0.f,
    const uint32_t numUtterances = 0
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

// TODO: Implement `doIncludeToken` for AuthenticateConfig?

/// @brief Allocate a new configuration object for enrollment authentication.
///
/// @param enrollmentID The enrollment ID to authenticate against. This can
/// be either an enrollment ID or a group ID.
/// @param isLivenessEnabled `true` to perform a liveness check before the
/// authentication, `false` to only perform the authentication.
/// @param sensitivity The sensitivity of the model.
/// @param security The security level of the model.
/// @param isEnrollmentGroup `true` if the enrollment ID references an
/// enrollment group, `false` is the enrollment ID references a single
/// enrollment.
/// @returns A pointer to the `AuthenticateConfig` object.
///
inline ::sensory::api::v1::audio::AuthenticateConfig* new_authenticate_config(
    const std::string& enrollmentID,
    const bool& isLivenessEnabled,
    const ::sensory::api::v1::audio::ThresholdSensitivity& sensitivity,
    const ::sensory::api::v1::audio::AuthenticateConfig_ThresholdSecurity& security,
    const bool& isEnrollmentGroup = false
) {
    auto config = new ::sensory::api::v1::audio::AuthenticateConfig;
    if (isEnrollmentGroup)
        config->set_enrollmentgroupid(enrollmentID);
    else
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
inline ::sensory::api::v1::audio::ValidateEventConfig* new_validate_event_config(
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

// TODO: implement `referenceId` for CreateEnrollmentEventConfig?

/// @brief Allocate a new configuration object for creating enrolled events.
///
/// @param modelName The name of the model to use to create the enrollment.
/// @param userID The ID of the user making the request.
/// @param description The description of the enrollment.
/// @param enrollmentDuration The duration in seconds for text-independent
/// enrollments, defaults to \f$12.5\f$ without liveness enabled and \f$8\f$
/// with liveness enabled.
/// @param numUtterances The number of utterances that should be required
/// for text-dependent enrollments, defaults to \f$4\f$ if not specified.
///
/// @returns A pointer to the `CreateEnrollmentEventConfig` object.
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
inline ::sensory::api::v1::audio::CreateEnrollmentEventConfig* new_create_enrollment_event_config(
    const std::string& modelName,
    const std::string& userID,
    const std::string& description,
    const float enrollmentDuration = 0.f,
    const uint32_t numUtterances = 0
) {
    // The number of utterances and the enrollment duration cannot both be
    // specified in the message, so check for sentinel "null" values and if
    // both are provided, throw an error. This check is conducted before
    // allocation of the config to prevent memory leaks.
    if (enrollmentDuration > 0 && numUtterances > 0)
        throw std::runtime_error("enrollmentDuration and numUterrances cannot both be specified.");
    auto config = new ::sensory::api::v1::audio::CreateEnrollmentEventConfig;
    config->set_modelname(modelName);
    config->set_userid(userID);
    config->set_description(description);
    if (enrollmentDuration > 0)  // enrollment duration provided
        config->set_enrollmentduration(enrollmentDuration);
    else if (numUtterances > 0)  // number of utterances provided
        config->set_enrollmentnumutterances(numUtterances);
    return config;
}

/// @brief Allocate a new configuration object for enrolled event validation.
///
/// @param enrollmentID The enrollment ID to validate against. This can
/// be either an enrollment ID or a group ID.
/// @param sensitivity The sensitivity of the model.
/// @param isEnrollmentGroup `true` if the enrollment ID references an
/// enrollment group, `false` is the enrollment ID references a single
/// enrollment.
/// @returns A pointer to the `ValidateEnrolledEventConfig` object.
///
inline ::sensory::api::v1::audio::ValidateEnrolledEventConfig* new_validate_enrolled_event_config(
    const std::string& enrollmentID,
    const ::sensory::api::v1::audio::ThresholdSensitivity& sensitivity,
    const bool& isEnrollmentGroup = false
) {
    auto config = new ::sensory::api::v1::audio::ValidateEnrolledEventConfig;
    if (isEnrollmentGroup)
        config->set_enrollmentgroupid(enrollmentID);
    else
        config->set_enrollmentid(enrollmentID);
    config->set_sensitivity(sensitivity);
    return config;
}

/// @brief Allocate a new configuration object for audio transcription.
///
/// @param modelName The name of the model to use to transcribe the audio.
/// @param userID The ID of the user making the request.
/// @returns A pointer to the `TranscribeConfig` object.
///
inline ::sensory::api::v1::audio::TranscribeConfig* new_transcribe_config(
    const std::string& modelName,
    const std::string& userID
) {
    auto config = new ::sensory::api::v1::audio::TranscribeConfig;
    config->set_modelname(modelName);
    config->set_userid(userID);
    return config;
}

/// @brief A structure that aggregates and stores transcription responses.
/// @details
/// This class can maintain the full transcript returned from the server's
/// windows responses.
class TranscriptAggregator {
 private:
    /// An internal buffer of the complete transcript from the server.
    std::vector<::sensory::api::v1::audio::TranscribeWord> word_list;

 public:
    /// @brief Process a single sliding-window response from the server.
    ///
    /// @param response The current word list from the server.
    ///
    void process_response(::sensory::api::v1::audio::TranscribeWordResponse* response) {
        // If nothing is returned, do nothing
        if (response == nullptr || !response->words().size()) return;
        // Grow the list of words if the last word index has increased past the
        // size of the transcription buffer.
        if (response->lastwordindex() >= word_list.size())
            word_list.resize(response->lastwordindex() + 1);
        // Loop through returned words and set the returned value at the
        // specified index in the transcript.
        for (const auto& word : response->words())
            word_list[word.wordindex()] = word;
        // Shrink the word list if the incoming transcript is smaller.
        if (response->lastwordindex() < word_list.size() - 1)
            word_list.erase(word_list.begin() + response->lastwordindex() + 1, word_list.end());
    }

    /// @brief Return a constant reference to the complete transcript.
    ///
    /// @returns A vector with the transcribed words and associated metadata.
    ///
    const std::vector<::sensory::api::v1::audio::TranscribeWord>& get_word_list() const {
        return word_list;
    }

    /// @brief Return the full transcript as computed from the current word list.
    ///
    /// @returns An imploded string representation of the underlying word list.
    ///
    std::string get_transcript() const {
        // If there are no words, then there is no transcript.
        if (word_list.size() == 0) return "";
        // Iterate over the word responses to accumulate the transcript.
        std::string transcript = "";
        for (const auto& word : word_list)
            transcript += " " + word.word();
        // Remove the extra space at the front of the transcript.
        transcript.erase(0, 1);
        return transcript;
    }
};

}  // namespace audio

/// @brief A service for audio data.
/// @tparam SecureCredentialStore A secure key-value store for storing and
/// fetching credentials and tokens.
template<typename SecureCredentialStore>
class AudioService {
 private:
    /// the global configuration for the remote connection
    const ::sensory::Config& config;
    /// the token manager for securing gRPC requests to the server
    ::sensory::token_manager::TokenManager<SecureCredentialStore>& token_manager;
    /// the gRPC stub for the audio models service
    std::unique_ptr<::sensory::api::v1::audio::AudioModels::StubInterface> models_stub;
    /// the gRPC stub for the audio biometrics service
    std::unique_ptr<::sensory::api::v1::audio::AudioBiometrics::StubInterface> biometric_stub;
    /// the gRPC stub for the audio events service
    std::unique_ptr<::sensory::api::v1::audio::AudioEvents::StubInterface> events_stub;
    /// the gRPC stub for the audio transcriptions service
    std::unique_ptr<::sensory::api::v1::audio::AudioTranscriptions::StubInterface> transcriptions_stub;
    /// the gRPC stub for the audio synthesis service
    std::unique_ptr<::sensory::api::v1::audio::AudioSynthesis::StubInterface> synthesis_stub;

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
    /// @param token_manager_ The token manager for requesting Bearer tokens.
    ///
    AudioService(
        const ::sensory::Config& config_,
        ::sensory::token_manager::TokenManager<SecureCredentialStore>& token_manager_
    ) : config(config_),
        token_manager(token_manager_),
        models_stub(::sensory::api::v1::audio::AudioModels::NewStub(config.get_channel())),
        biometric_stub(::sensory::api::v1::audio::AudioBiometrics::NewStub(config.get_channel())),
        events_stub(::sensory::api::v1::audio::AudioEvents::NewStub(config.get_channel())),
        transcriptions_stub(::sensory::api::v1::audio::AudioTranscriptions::NewStub(config.get_channel())),
        synthesis_stub(::sensory::api::v1::audio::AudioSynthesis::NewStub(config.get_channel())) { }

    /// @brief Initialize a new audio service.
    ///
    /// @param config_ The global configuration for the remote connection.
    /// @param token_manager_ The token manager for requesting Bearer tokens.
    /// @param models_stub_ The models service stub to initialize the service
    /// with.
    /// @param biometric_stub_ The biometrics service stub to initialize the
    /// service with.
    /// @param events_stub_ The events service stub to initialize the service
    /// with.
    /// @param transcriptions_stub_ The transcription service stub to initialize
    /// the service with.
    /// @param synthesis_stub_ The synthesis service stub to initialize the
    /// service with.
    ///
    AudioService(
        const ::sensory::Config& config_,
        ::sensory::token_manager::TokenManager<SecureCredentialStore>& token_manager_,
        ::sensory::api::v1::audio::AudioModels::StubInterface* models_stub_,
        ::sensory::api::v1::audio::AudioBiometrics::StubInterface* biometric_stub_,
        ::sensory::api::v1::audio::AudioEvents::StubInterface* events_stub_,
        ::sensory::api::v1::audio::AudioTranscriptions::StubInterface* transcriptions_stub_,
        ::sensory::api::v1::audio::AudioSynthesis::StubInterface* synthesis_stub_
    ) : config(config_),
        token_manager(token_manager_),
        models_stub(models_stub_),
        biometric_stub(biometric_stub_),
        events_stub(events_stub_),
        transcriptions_stub(transcriptions_stub_),
        synthesis_stub(synthesis_stub_) { }

    /// @brief Return the cloud configuration associated with this service.
    ///
    /// @returns the configuration used by this service.
    ///
    inline const ::sensory::Config& get_config() const { return config; }

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
        token_manager.setup_unary_client_context(context);
        // Execute the RPC synchronously and return the status
        return models_stub->GetModels(&context, {}, response);
    }

    /// @brief A type for encapsulating data for asynchronous `GetModels` calls
    /// based on CompletionQueue event loops.
    typedef ::sensory::AsyncResponseReaderCall<
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
        token_manager.setup_unary_client_context(call->context);
        // Start the asynchronous RPC with the call's context and queue.
        call->rpc = models_stub->AsyncGetModels(&call->context, call->request, queue);
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
        token_manager.setup_unary_client_context(call->context);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        models_stub->async()->GetModels(
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
                call->setIsDone();
            });
        return call;
    }

    // ----- Create Enrollment -------------------------------------------------

    /// A type for biometric enrollment streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::audio::CreateEnrollmentRequest,
            ::sensory::api::v1::audio::CreateEnrollmentResponse
        >
    > CreateEnrollmentStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an audio enrollment.
    ///
    /// @param context the gRPC context for the stream
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param enrollmentConfig The enrollment configuration for the stream.
    /// Use `new_create_enrollment_config` to create a new enrollment config.
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
        ::grpc::ClientContext* context,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::CreateEnrollmentConfig* enrollmentConfig
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the config.
        enrollmentConfig->set_allocated_audio(audioConfig);
        enrollmentConfig->set_deviceid(config.get_device_id());
        ::sensory::api::v1::audio::CreateEnrollmentRequest request;
        request.set_allocated_config(enrollmentConfig);
        // Create the stream and write the initial configuration request.
        auto stream = biometric_stub->CreateEnrollment(context);
        if (stream == nullptr)  // Failed to create stream.
            throw NullStreamError("Failed to start CreateEnrollment stream (null pointer).");
        if (!stream->Write(request))  // Failed to write audio config.
            throw WriteStreamError("Failed to write audio configuration to CreateEnrollment stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `CreateEnrollment`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::AsyncReaderWriterCall<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::CreateEnrollmentRequest,
        ::sensory::api::v1::audio::CreateEnrollmentResponse
    > CreateEnrollmentAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an audio enrollment.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param enrollmentConfig The enrollment configuration for the stream.
    /// Use `new_create_enrollment_config` to create a new enrollment config.
    /// _Ownership of the dynamically allocated configuration is implicitly
    /// transferred to the stream_.
    /// @param initTag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finishTag The tag to finish the stream with. Use `nullptr` to
    /// use the pointer as the tag.
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
    inline CreateEnrollmentAsyncStream* createEnrollment(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::CreateEnrollmentConfig* enrollmentConfig,
        void* initTag = nullptr,
        void* finishTag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new CreateEnrollmentAsyncStream);
        // Set the client context for a unary call.
        token_manager.setup_bidi_client_context(call->context);
        // Create the request with the pointer to the config.
        enrollmentConfig->set_allocated_audio(audioConfig);
        enrollmentConfig->set_deviceid(config.get_device_id());
        call->request.set_allocated_config(enrollmentConfig);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        initTag = initTag == nullptr ? static_cast<void*>(call) : initTag;
        call->rpc = biometric_stub->AsyncCreateEnrollment(&call->context, queue, initTag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finishTag = finishTag == nullptr ? static_cast<void*>(call) : finishTag;
        call->rpc->Finish(&call->status, finishTag);

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
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param enrollmentConfig The enrollment configuration for the stream.
    /// Use `new_create_enrollment_config` to create a new enrollment config.
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
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the request with the pointer to the config.
        enrollmentConfig->set_allocated_audio(audioConfig);
        enrollmentConfig->set_deviceid(config.get_device_id());
        reactor->request.set_allocated_config(enrollmentConfig);
        // Create the stream and write the initial configuration request.
        biometric_stub->async()->CreateEnrollment(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }

    // ----- Authenticate ------------------------------------------------------

    /// A type for biometric authentication streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::audio::AuthenticateRequest,
            ::sensory::api::v1::audio::AuthenticateResponse
        >
    > AuthenticateStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// enrollment authentication.
    ///
    /// @param context the gRPC context for the stream
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param authenticateConfig The authentication configuration for the
    /// stream. Use `new_authenticate_config` to create a new authentication
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
        ::grpc::ClientContext* context,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::AuthenticateConfig* authenticateConfig
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the enrollment config.
        authenticateConfig->set_allocated_audio(audioConfig);
        ::sensory::api::v1::audio::AuthenticateRequest request;
        request.set_allocated_config(authenticateConfig);
        // Create the stream and write the initial configuration request.
        auto stream = biometric_stub->Authenticate(context);
        if (stream == nullptr)  // Failed to create stream.
            throw NullStreamError("Failed to start Authenticate stream (null pointer).");
        if (!stream->Write(request))  // Failed to write audio config.
            throw WriteStreamError("Failed to write audio configuration to Authenticate stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `Authenticate`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::AsyncReaderWriterCall<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::AuthenticateRequest,
        ::sensory::api::v1::audio::AuthenticateResponse
    > AuthenticateAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// enrollment authentication.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param authenticateConfig The authentication configuration for the
    /// stream. Use `new_authenticate_config` to create a new authentication
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @param initTag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finishTag The tag to finish the stream with. Use `nullptr` to
    /// use the pointer as the tag.
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
    inline AuthenticateAsyncStream* authenticate(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::AuthenticateConfig* authenticateConfig,
        void* initTag = nullptr,
        void* finishTag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new AuthenticateAsyncStream);
        // Set the client context for a unary call.
        token_manager.setup_bidi_client_context(call->context);
        // Create the request with the pointer to the enrollment config.
        authenticateConfig->set_allocated_audio(audioConfig);
        call->request.set_allocated_config(authenticateConfig);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        initTag = initTag == nullptr ? static_cast<void*>(call) : initTag;
        call->rpc = biometric_stub->AsyncAuthenticate(&call->context, queue, initTag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finishTag = finishTag == nullptr ? static_cast<void*>(call) : finishTag;
        call->rpc->Finish(&call->status, finishTag);
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
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param authenticateConfig The authentication configuration for the
    /// stream. Use `new_authenticate_config` to create a new authentication
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
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the request with the pointer to the enrollment config.
        authenticateConfig->set_allocated_audio(audioConfig);
        reactor->request.set_allocated_config(authenticateConfig);
        // Create the stream and write the initial configuration request.
        biometric_stub->async()->Authenticate(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }

    // ----- Validate Event ----------------------------------------------------

    /// A type for trigger validation streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::audio::ValidateEventRequest,
            ::sensory::api::v1::audio::ValidateEventResponse
        >
    > ValidateEventStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// audio event validation.
    ///
    /// @param context the gRPC context for the stream
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param validateEventConfig The trigger validation configuration for the
    /// stream. Use `new_validate_event_config` to create a new trigger validation
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `ValidateEventConfig` message to
    /// the server.
    ///
    inline ValidateEventStream validateEvent(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::ValidateEventConfig* validateEventConfig
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the enrollment config.
        validateEventConfig->set_allocated_audio(audioConfig);
        ::sensory::api::v1::audio::ValidateEventRequest request;
        request.set_allocated_config(validateEventConfig);
        // Create the stream and write the initial configuration request.
        auto stream = events_stub->ValidateEvent(context);
        if (stream == nullptr)  // Failed to create stream.
            throw NullStreamError("Failed to start ValidateEvent stream (null pointer).");
        if (!stream->Write(request))  // Failed to write audio config.
            throw WriteStreamError("Failed to write audio config to ValidateEvent stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `ValidateEvent`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::AsyncReaderWriterCall<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::ValidateEventRequest,
        ::sensory::api::v1::audio::ValidateEventResponse
    > ValidateEventAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// audio event validation.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param validateEventConfig The trigger validation configuration for the
    /// stream. Use `new_validate_event_config` to create a new trigger validation
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @param initTag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finishTag The tag to finish the stream with. Use `nullptr` to
    /// use the pointer as the tag.
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
    inline ValidateEventAsyncStream* validateEvent(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::ValidateEventConfig* validateEventConfig,
        void* initTag = nullptr,
        void* finishTag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new ValidateEventAsyncStream);
        // Set the client context for a bidirectional stream.
        token_manager.setup_bidi_client_context(call->context);
        // Create the request with the pointer to the enrollment config.
        validateEventConfig->set_allocated_audio(audioConfig);
        call->request.set_allocated_config(validateEventConfig);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        initTag = initTag == nullptr ? static_cast<void*>(call) : initTag;
        call->rpc = events_stub->AsyncValidateEvent(&call->context, queue, initTag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finishTag = finishTag == nullptr ? static_cast<void*>(call) : finishTag;
        call->rpc->Finish(&call->status, finishTag);
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
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param validateEventConfig The trigger validation configuration for the
    /// stream. Use `new_validate_event_config` to create a new trigger validation
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `ValidateEventConfig` message to
    /// the server.
    ///
    template<typename Reactor>
    inline void validateEvent(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::ValidateEventConfig* validateEventConfig
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the request with the pointer to the enrollment config.
        validateEventConfig->set_allocated_audio(audioConfig);
        reactor->request.set_allocated_config(validateEventConfig);
        // Create the stream and write the initial configuration request.
        events_stub->async()->ValidateEvent(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }

    // ----- Create Enrolled Event ---------------------------------------------

    /// A type for biometric enrollment streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::audio::CreateEnrolledEventRequest,
            ::sensory::api::v1::audio::CreateEnrollmentResponse
        >
    > CreateEnrolledEventStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an audio enrollment.
    ///
    /// @param context the gRPC context for the stream
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param enrollmentConfig The enrollment configuration for the stream.
    /// Use `newCreateEnrolledEventConfig` to create a new enrollment
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `CreateEnrollmentEventConfig`
    /// message to the server.
    ///
    inline CreateEnrolledEventStream createEventEnrollment(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::CreateEnrollmentEventConfig* enrollmentConfig
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the config.
        enrollmentConfig->set_allocated_audio(audioConfig);
        ::sensory::api::v1::audio::CreateEnrolledEventRequest request;
        request.set_allocated_config(enrollmentConfig);
        // Create the stream and write the initial configuration request.
        auto stream = events_stub->CreateEnrolledEvent(context);
        if (stream == nullptr)  // Failed to create stream.
            throw NullStreamError("Failed to start CreateEnrolledEvent stream (null pointer).");
        if (!stream->Write(request))  // Failed to write audio config.
            throw WriteStreamError("Failed to write audio config to CreateEnrolledEvent stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `CreateEnrollment`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::AsyncReaderWriterCall<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::CreateEnrolledEventRequest,
        ::sensory::api::v1::audio::CreateEnrollmentResponse
    > CreateEnrolledEventAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an audio enrollment.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param enrollmentConfig The enrollment configuration for the stream.
    /// Use `newCreateEnrolledEventConfig` to create a new enrollment
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @param initTag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finishTag The tag to finish the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    /// @details
    /// This call will **NOT** automatically send the
    /// `CreateEnrollmentEventConfig` message to the server, but will buffer
    /// it in the message for later transmission.
    ///
    inline CreateEnrolledEventAsyncStream* createEventEnrollment(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::CreateEnrollmentEventConfig* enrollmentConfig,
        void* initTag = nullptr,
        void* finishTag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new CreateEnrolledEventAsyncStream);
        // Set the client context for a unary call.
        token_manager.setup_bidi_client_context(call->context);
        // Create the request with the pointer to the config.
        enrollmentConfig->set_allocated_audio(audioConfig);
        call->request.set_allocated_config(enrollmentConfig);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        initTag = initTag == nullptr ? static_cast<void*>(call) : initTag;
        call->rpc = events_stub->AsyncCreateEnrolledEvent(&call->context, queue, initTag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finishTag = finishTag == nullptr ? static_cast<void*>(call) : finishTag;
        call->rpc->Finish(&call->status, finishTag);

        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `CreateEnrolledEvent` calls.
    typedef ::sensory::AwaitableBidiReactor<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::CreateEnrolledEventRequest,
        ::sensory::api::v1::audio::CreateEnrollmentResponse
    > CreateEnrolledEventBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an audio enrollment.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param enrollmentConfig The enrollment configuration for the stream.
    /// Use `newCreateEnrolledEventConfig` to create a new enrollment config.
    /// _Ownership of the dynamically allocated configuration is implicitly
    /// transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `CreateEnrollmentEventConfig`
    /// message to the server.
    ///
    template<typename Reactor>
    inline void createEventEnrollment(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::CreateEnrollmentEventConfig* enrollmentConfig
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the request with the pointer to the config.
        enrollmentConfig->set_allocated_audio(audioConfig);
        reactor->request.set_allocated_config(enrollmentConfig);
        // Create the stream and write the initial configuration request.
        events_stub->async()->CreateEnrolledEvent(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }

    // ----- Validate Enrolled Event -------------------------------------------

    /// A type for event validation streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::audio::ValidateEnrolledEventRequest,
            ::sensory::api::v1::audio::ValidateEnrolledEventResponse
        >
    > ValidateEnrolledEventStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// event enrollment validation.
    ///
    /// @param context the gRPC context for the stream
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param validateConfig The validation configuration for the
    /// stream. Use `new_validate_enrolled_event_config` to create a new validation
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `ValidateEnrolledEventConfig`
    /// message to the server.
    ///
    inline ValidateEnrolledEventStream validateEnrolledEvent(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::ValidateEnrolledEventConfig* validateConfig
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the enrollment config.
        validateConfig->set_allocated_audio(audioConfig);
        ::sensory::api::v1::audio::ValidateEnrolledEventRequest request;
        request.set_allocated_config(validateConfig);
        // Create the stream and write the initial configuration request.
        auto stream = events_stub->ValidateEnrolledEvent(context);
        if (stream == nullptr)  // Failed to create stream.
            throw NullStreamError("Failed to start ValidateEnrolledEvent stream (null pointer).");
        if (!stream->Write(request))  // Failed to write audio config.
            throw WriteStreamError("Failed to write audio config to ValidateEnrolledEvent stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `ValidateEnrolledEvent` calls based on CompletionQueue event loops.
    typedef ::sensory::AsyncReaderWriterCall<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::ValidateEnrolledEventRequest,
        ::sensory::api::v1::audio::ValidateEnrolledEventResponse
    > ValidateEnrolledEventAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// event enrollment validation.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param validateConfig The validation configuration for the
    /// stream. Use `new_validate_enrolled_event_config` to create a new validation
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @param initTag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finishTag The tag to finish the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    /// @details
    /// This call will **NOT** automatically send the `ValidateEnrolledEventConfig`
    /// message to the server, but will buffer it in the message for later
    /// transmission.
    ///
    inline ValidateEnrolledEventAsyncStream* validateEnrolledEvent(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::ValidateEnrolledEventConfig* validateConfig,
        void* initTag = nullptr,
        void* finishTag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new ValidateEnrolledEventAsyncStream);
        // Set the client context for a unary call.
        token_manager.setup_bidi_client_context(call->context);
        // Create the request with the pointer to the enrollment config.
        validateConfig->set_allocated_audio(audioConfig);
        call->request.set_allocated_config(validateConfig);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        initTag = initTag == nullptr ? static_cast<void*>(call) : initTag;
        call->rpc = events_stub->AsyncValidateEnrolledEvent(&call->context, queue, initTag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finishTag = finishTag == nullptr ? static_cast<void*>(call) : finishTag;
        call->rpc->Finish(&call->status, finishTag);
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `ValidateEnrolledEvent` calls.
    typedef ::sensory::AwaitableBidiReactor<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::ValidateEnrolledEventRequest,
        ::sensory::api::v1::audio::ValidateEnrolledEventResponse
    > ValidateEnrolledEventBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// event enrollment validation.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param validateConfig The validation configuration for the
    /// stream. Use `new_validate_enrolled_event_config` to create a new validation
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `ValidateEnrolledEventConfig` message to
    /// the server.
    ///
    template<typename Reactor>
    inline void validateEnrolledEvent(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::ValidateEnrolledEventConfig* validateConfig
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the request with the pointer to the enrollment config.
        validateConfig->set_allocated_audio(audioConfig);
        reactor->request.set_allocated_config(validateConfig);
        // Create the stream and write the initial configuration request.
        events_stub->async()->ValidateEnrolledEvent(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }

    // ----- Transcribe Audio --------------------------------------------------

    /// a type for speech transcription streams
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::audio::TranscribeRequest,
            ::sensory::api::v1::audio::TranscribeResponse
        >
    > TranscribeStream;

    /// @brief Open a bidirectional stream to the server that provides a
    /// transcription of the provided audio data.
    ///
    /// @param context the gRPC context for the stream
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param transcribeConfig The audio transcription configuration for the
    /// stream. Use `new_transcribe_config` to create a new audio transcription
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `TranscribeConfig` message to the
    /// server.
    ///
    inline TranscribeStream transcribe(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::TranscribeConfig* transcribeConfig
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the enrollment config.
        transcribeConfig->set_allocated_audio(audioConfig);
        ::sensory::api::v1::audio::TranscribeRequest request;
        request.set_allocated_config(transcribeConfig);
        // Create the stream and write the initial configuration request.
        auto stream = transcriptions_stub->Transcribe(context);
        if (stream == nullptr)  // Failed to create stream.
            throw NullStreamError("Failed to start Transcribe stream (null pointer).");
        if (!stream->Write(request))  // Failed to write audio config.
            throw WriteStreamError("Failed to write audio config to Transcribe stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `Transcribe`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::AsyncReaderWriterCall<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::TranscribeRequest,
        ::sensory::api::v1::audio::TranscribeResponse
    > TranscribeAsyncStream;

    /// @brief Open a bidirectional stream to the server that provides a
    /// transcription of the provided audio data.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audioConfig The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param transcribeConfig The audio transcription configuration for the
    /// stream. Use `new_transcribe_config` to create a new audio transcription
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @param initTag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finishTag The tag to finish the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    /// @details
    /// This call will **NOT** automatically send the `TranscribeConfig`
    /// message to the server, but will buffer it in the message for later
    /// transmission.
    ///
    inline TranscribeAsyncStream* transcribe(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::TranscribeConfig* transcribeConfig,
        void* initTag = nullptr,
        void* finishTag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new TranscribeAsyncStream);
        // Set the client context for a unary call.
        token_manager.setup_bidi_client_context(call->context);
        // Create the request with the pointer to the enrollment config.
        transcribeConfig->set_allocated_audio(audioConfig);
        call->request.set_allocated_config(transcribeConfig);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        initTag = initTag == nullptr ? static_cast<void*>(call) : initTag;
        call->rpc = transcriptions_stub->AsyncTranscribe(&call->context, queue, initTag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finishTag = finishTag == nullptr ? static_cast<void*>(call) : finishTag;
        call->rpc->Finish(&call->status, finishTag);
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
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param transcribeConfig The audio transcription configuration for the
    /// stream. Use `new_transcribe_config` to create a new audio transcription
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `TranscribeConfig` message to the
    /// server.
    ///
    template<typename Reactor>
    inline void transcribe(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audioConfig,
        ::sensory::api::v1::audio::TranscribeConfig* transcribeConfig
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the request with the pointer to the enrollment config.
        transcribeConfig->set_allocated_audio(audioConfig);
        reactor->request.set_allocated_config(transcribeConfig);
        // Create the stream and write the initial configuration request.
        transcriptions_stub->async()->Transcribe(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }

    // ----- Synthesize Speech -------------------------------------------------

    /// a type for speech synthesis streams
    typedef std::unique_ptr<
        ::grpc::ClientReaderInterface<
            ::sensory::api::v1::audio::SynthesizeSpeechResponse
        >
    > SynthesizeSpeechStream;

    /// @brief Open a unidirectional stream from the server that provides a
    /// synthesis of the provided text.
    ///
    /// @param context the gRPC context for the stream
    /// @param audio_config The audio configuration that provides information
    /// about the output audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param voice The style of voice to use for the synthesis
    /// @param phrase The text phrase to synthesize into speech.
    /// @returns A unidirectional stream that can be used to receive audio data
    /// from the server.
    ///
    /// @details
    /// This call will automatically send the `VoiceSynthesisConfig` message
    /// to the server.
    ///
    inline SynthesizeSpeechStream synthesize_speech(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        const std::string& voice,
        const std::string& phrase
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the synthesis configuration object.
        auto synthesis_config = new ::sensory::api::v1::audio::VoiceSynthesisConfig;
        synthesis_config->set_voice(voice);
        synthesis_config->set_allocated_audio(audio_config);
        // Create the synthesis request.
        ::sensory::api::v1::audio::SynthesizeSpeechRequest request;
        request.set_allocated_config(synthesis_config);
        request.set_phrase(phrase);
        // Create the stream and write the initial configuration request.
        auto stream = synthesis_stub->SynthesizeSpeech(context, request);
        if (stream == nullptr)  // Failed to create stream.
            throw NullStreamError("Failed to start SynthesizeSpeech stream (null pointer).");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `SynthesizeSpeech` calls based on CompletionQueue event loops.
    typedef ::sensory::AsyncResponseReaderCall<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::SynthesizeSpeechRequest,
        ::sensory::api::v1::audio::SynthesizeSpeechResponse
    > SynthesizeSpeechAsyncStream;

    /// @brief Open a unidirectional stream from the server that provides a
    /// synthesis of the provided text.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audio_config The audio configuration that provides information
    /// about the output audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param voice The style of voice to use for the synthesis
    /// @param phrase The text phrase to synthesize into speech.
    /// @param initTag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finishTag The tag to finish the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    /// @details
    /// This call will **NOT** automatically send the `VoiceSynthesisConfig`
    /// message to the server, but will buffer it in the message for later
    /// transmission.
    ///
    inline SynthesizeSpeechAsyncStream* synthesize_speech(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        const std::string& voice,
        const std::string& phrase,
        void* initTag = nullptr,
        void* finishTag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new SynthesizeSpeechAsyncStream);
        // Set the client context for a unary call.
        token_manager.setup_bidi_client_context(call->context);
        // Create the synthesis configuration object.
        auto synthesis_config = new ::sensory::api::v1::audio::VoiceSynthesisConfig;
        synthesis_config->set_voice(voice);
        synthesis_config->set_allocated_audio(audio_config);
        // Create the request with the pointer to the synthesis config.
        call->request.set_allocated_config(synthesis_config);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        initTag = initTag == nullptr ? static_cast<void*>(call) : initTag;
        call->rpc = synthesis_stub->AsyncSynthesizeSpeech(&call->context, queue, initTag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finishTag = finishTag == nullptr ? static_cast<void*>(call) : finishTag;
        call->rpc->Finish(&call->status, finishTag);
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `SynthesizeSpeech` calls.
    typedef ::sensory::AwaitableReadReactor<
        AudioService<SecureCredentialStore>,
        ::sensory::api::v1::audio::SynthesizeSpeechResponse
    > SynthesizeSpeechReadReactor;

    /// @brief Open a unidirectional stream from the server that provides a
    /// synthesis of the provided text.
    ///
    /// @tparam Reactor The type of the read reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. Use `new_audio_config` to generate the
    /// audio config. _Ownership of the dynamically allocated configuration
    /// is implicitly transferred to the stream_.
    /// @param voice The style of voice to use for the synthesis
    /// @param phrase The text phrase to synthesize into speech.
    ///
    /// @details
    /// This call will automatically send the `VoiceSynthesisConfig` message to
    /// the server.
    ///
    template<typename Reactor>
    inline void synthesize_speech(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        const std::string& voice,
        const std::string& phrase
    ) const {
        // Setup the context of the reactor for a unidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the synthesis configuration object.
        auto synthesis_config = new ::sensory::api::v1::audio::VoiceSynthesisConfig;
        synthesis_config->set_voice(voice);
        synthesis_config->set_allocated_audio(audio_config);
        reactor->request.set_allocated_config(synthesis_config);
        // Create the stream and write the initial configuration request.
        synthesis_stub->async()->SynthesizeSpeech(&reactor->context, reactor);
        reactor->StartRead(&reactor->response);
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORYCLOUD_SERVICES_AUDIO_SERVICE_HPP_
