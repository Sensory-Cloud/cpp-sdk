// The audio service for the SensoryCloud SDK.
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
#include "sensorycloud/calldata.hpp"
#include "sensorycloud/error/stream_errors.hpp"
#include "sensorycloud/util/string_extensions.hpp"

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief SensoryCloud services.
namespace service {

/// @brief A service for audio data.
/// @tparam CredentialStore A key-value store for storing and fetching
/// credentials and tokens.
template<typename CredentialStore>
class AudioService {
 private:
    /// the global configuration for the remote connection
    const ::sensory::Config& config;
    /// the token manager for securing gRPC requests to the server
    ::sensory::token_manager::TokenManager<CredentialStore>& token_manager;
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
        ::sensory::token_manager::TokenManager<CredentialStore>& token_manager_
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
        ::sensory::token_manager::TokenManager<CredentialStore>& token_manager_,
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
    inline ::grpc::Status get_models(
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
    typedef ::sensory::calldata::AsyncResponseReaderCall<
        AudioService<CredentialStore>,
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
    inline GetModelsAsyncCall* get_models(::grpc::CompletionQueue* queue) const {
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
    typedef ::sensory::calldata::CallbackData<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::GetModelsRequest,
        ::sensory::api::v1::audio::GetModelsResponse
    > GetModelsCallbackData;

    /// @brief Fetch a list of the vision models supported by the cloud host.
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `GetModelsCallbackData*`.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the asynchronous call spawned by this call.
    ///
    template<typename Callback>
    inline std::shared_ptr<GetModelsCallbackData> get_models(
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. Setup the call as usual with a bearer token and
        // any application deadlines. This call is initiated as a shared pointer
        // in order to reference count between the parent and child context.
        // This also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<GetModelsCallbackData> call(new GetModelsCallbackData);
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
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param enrollment_config The enrollment configuration for the stream.
    /// _Ownership of the dynamically allocated configuration is implicitly
    /// transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `CreateEnrollmentConfig` message
    /// to the server.
    ///
    inline CreateEnrollmentStream create_enrollment(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::CreateEnrollmentConfig* enrollment_config
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the config.
        enrollment_config->set_allocated_audio(audio_config);
        enrollment_config->set_deviceid(config.get_device_id());
        ::sensory::api::v1::audio::CreateEnrollmentRequest request;
        request.set_allocated_config(enrollment_config);
        // Create the stream and write the initial configuration request.
        auto stream = biometric_stub->CreateEnrollment(context);
        if (stream == nullptr)  // Failed to create stream.
            throw ::sensory::error::NullStreamError("Failed to start CreateEnrollment stream (null pointer).");
        if (!stream->Write(request))  // Failed to write audio config.
            throw ::sensory::error::WriteStreamError("Failed to write audio configuration to CreateEnrollment stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `CreateEnrollment`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncReaderWriterCall<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::CreateEnrollmentRequest,
        ::sensory::api::v1::audio::CreateEnrollmentResponse
    > CreateEnrollmentAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an audio enrollment.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param enrollment_config The enrollment configuration for the stream.
    /// _Ownership of the dynamically allocated configuration is implicitly
    /// transferred to the stream_.
    /// @param init_tag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finish_tag The tag to finish the stream with. Use `nullptr` to
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
    inline CreateEnrollmentAsyncStream* create_enrollment(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::CreateEnrollmentConfig* enrollment_config,
        void* init_tag = nullptr,
        void* finish_tag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new CreateEnrollmentAsyncStream);
        // Set the client context for a unary call.
        token_manager.setup_bidi_client_context(call->context);
        // Create the request with the pointer to the config.
        enrollment_config->set_allocated_audio(audio_config);
        enrollment_config->set_deviceid(config.get_device_id());
        call->request.set_allocated_config(enrollment_config);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        init_tag = init_tag == nullptr ? static_cast<void*>(call) : init_tag;
        call->rpc = biometric_stub->AsyncCreateEnrollment(&call->context, queue, init_tag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finish_tag = finish_tag == nullptr ? static_cast<void*>(call) : finish_tag;
        call->rpc->Finish(&call->status, finish_tag);

        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `CreateEnrollment` calls.
    typedef ::sensory::calldata::AwaitableBidiReactor<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::CreateEnrollmentRequest,
        ::sensory::api::v1::audio::CreateEnrollmentResponse
    > CreateEnrollmentBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an audio enrollment.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param enrollment_config The enrollment configuration for the stream.
    /// _Ownership of the dynamically allocated configuration is implicitly
    /// transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `CreateEnrollmentConfig` message
    /// to the server.
    ///
    template<typename Reactor>
    inline void create_enrollment(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::CreateEnrollmentConfig* enrollment_config
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the request with the pointer to the config.
        enrollment_config->set_allocated_audio(audio_config);
        enrollment_config->set_deviceid(config.get_device_id());
        reactor->request.set_allocated_config(enrollment_config);
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
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param authenticate_config The authentication configuration for the
    /// stream. _Ownership of the dynamically allocated configuration is
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
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::AuthenticateConfig* authenticate_config
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the enrollment config.
        authenticate_config->set_allocated_audio(audio_config);
        ::sensory::api::v1::audio::AuthenticateRequest request;
        request.set_allocated_config(authenticate_config);
        // Create the stream and write the initial configuration request.
        auto stream = biometric_stub->Authenticate(context);
        if (stream == nullptr)  // Failed to create stream.
            throw ::sensory::error::NullStreamError("Failed to start Authenticate stream (null pointer).");
        if (!stream->Write(request))  // Failed to write audio config.
            throw ::sensory::error::WriteStreamError("Failed to write audio configuration to Authenticate stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `Authenticate`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncReaderWriterCall<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::AuthenticateRequest,
        ::sensory::api::v1::audio::AuthenticateResponse
    > AuthenticateAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// enrollment authentication.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param authenticate_config The authentication configuration for the
    /// stream. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @param init_tag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finish_tag The tag to finish the stream with. Use `nullptr` to
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
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::AuthenticateConfig* authenticate_config,
        void* init_tag = nullptr,
        void* finish_tag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new AuthenticateAsyncStream);
        // Set the client context for a unary call.
        token_manager.setup_bidi_client_context(call->context);
        // Create the request with the pointer to the enrollment config.
        authenticate_config->set_allocated_audio(audio_config);
        call->request.set_allocated_config(authenticate_config);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        init_tag = init_tag == nullptr ? static_cast<void*>(call) : init_tag;
        call->rpc = biometric_stub->AsyncAuthenticate(&call->context, queue, init_tag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finish_tag = finish_tag == nullptr ? static_cast<void*>(call) : finish_tag;
        call->rpc->Finish(&call->status, finish_tag);
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `Authenticate` calls.
    typedef ::sensory::calldata::AwaitableBidiReactor<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::AuthenticateRequest,
        ::sensory::api::v1::audio::AuthenticateResponse
    > AuthenticateBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// enrollment authentication.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param authenticate_config The authentication configuration for the
    /// stream. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `AuthenticateConfig` message to
    /// the server.
    ///
    template<typename Reactor>
    inline void authenticate(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::AuthenticateConfig* authenticate_config
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the request with the pointer to the enrollment config.
        authenticate_config->set_allocated_audio(audio_config);
        reactor->request.set_allocated_config(authenticate_config);
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
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param validate_event_config The trigger validation configuration for
    /// the stream. _Ownership of the dynamically allocate configuration is
    /// implicitly transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `ValidateEventConfig` message to
    /// the server.
    ///
    inline ValidateEventStream validate_event(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::ValidateEventConfig* validate_event_config
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the enrollment config.
        validate_event_config->set_allocated_audio(audio_config);
        ::sensory::api::v1::audio::ValidateEventRequest request;
        request.set_allocated_config(validate_event_config);
        // Create the stream and write the initial configuration request.
        auto stream = events_stub->ValidateEvent(context);
        if (stream == nullptr)  // Failed to create stream.
            throw ::sensory::error::NullStreamError("Failed to start ValidateEvent stream (null pointer).");
        if (!stream->Write(request))  // Failed to write audio config.
            throw ::sensory::error::WriteStreamError("Failed to write audio config to ValidateEvent stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `ValidateEvent`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncReaderWriterCall<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::ValidateEventRequest,
        ::sensory::api::v1::audio::ValidateEventResponse
    > ValidateEventAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// audio event validation.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param validate_event_config The trigger validation configuration for
    /// the stream. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @param init_tag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finish_tag The tag to finish the stream with. Use `nullptr` to
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
    inline ValidateEventAsyncStream* validate_event(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::ValidateEventConfig* validate_event_config,
        void* init_tag = nullptr,
        void* finish_tag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new ValidateEventAsyncStream);
        // Set the client context for a bidirectional stream.
        token_manager.setup_bidi_client_context(call->context);
        // Create the request with the pointer to the enrollment config.
        validate_event_config->set_allocated_audio(audio_config);
        call->request.set_allocated_config(validate_event_config);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        init_tag = init_tag == nullptr ? static_cast<void*>(call) : init_tag;
        call->rpc = events_stub->AsyncValidateEvent(&call->context, queue, init_tag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finish_tag = finish_tag == nullptr ? static_cast<void*>(call) : finish_tag;
        call->rpc->Finish(&call->status, finish_tag);
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `ValidateEvent` calls.
    typedef ::sensory::calldata::AwaitableBidiReactor<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::ValidateEventRequest,
        ::sensory::api::v1::audio::ValidateEventResponse
    > ValidateEventBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// audio event validation.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param validate_event_config The trigger validation configuration for
    /// the stream. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `ValidateEventConfig` message to
    /// the server.
    ///
    template<typename Reactor>
    inline void validate_event(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::ValidateEventConfig* validate_event_config
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the request with the pointer to the enrollment config.
        validate_event_config->set_allocated_audio(audio_config);
        reactor->request.set_allocated_config(validate_event_config);
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
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param enrollment_config The enrollment configuration for the stream.
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
    inline CreateEnrolledEventStream create_event_enrollment(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::CreateEnrollmentEventConfig* enrollment_config
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the config.
        enrollment_config->set_allocated_audio(audio_config);
        ::sensory::api::v1::audio::CreateEnrolledEventRequest request;
        request.set_allocated_config(enrollment_config);
        // Create the stream and write the initial configuration request.
        auto stream = events_stub->CreateEnrolledEvent(context);
        if (stream == nullptr)  // Failed to create stream.
            throw ::sensory::error::NullStreamError("Failed to start CreateEnrolledEvent stream (null pointer).");
        if (!stream->Write(request))  // Failed to write audio config.
            throw ::sensory::error::WriteStreamError("Failed to write audio config to CreateEnrolledEvent stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `CreateEnrollment`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncReaderWriterCall<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::CreateEnrolledEventRequest,
        ::sensory::api::v1::audio::CreateEnrollmentResponse
    > CreateEnrolledEventAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an audio enrollment.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param enrollment_config The enrollment configuration for the stream.
    /// Use `newCreateEnrolledEventConfig` to create a new enrollment
    /// config. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @param init_tag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finish_tag The tag to finish the stream with. Use `nullptr` to
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
    inline CreateEnrolledEventAsyncStream* create_event_enrollment(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::CreateEnrollmentEventConfig* enrollment_config,
        void* init_tag = nullptr,
        void* finish_tag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new CreateEnrolledEventAsyncStream);
        // Set the client context for a unary call.
        token_manager.setup_bidi_client_context(call->context);
        // Create the request with the pointer to the config.
        enrollment_config->set_allocated_audio(audio_config);
        call->request.set_allocated_config(enrollment_config);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        init_tag = init_tag == nullptr ? static_cast<void*>(call) : init_tag;
        call->rpc = events_stub->AsyncCreateEnrolledEvent(&call->context, queue, init_tag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finish_tag = finish_tag == nullptr ? static_cast<void*>(call) : finish_tag;
        call->rpc->Finish(&call->status, finish_tag);

        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `CreateEnrolledEvent` calls.
    typedef ::sensory::calldata::AwaitableBidiReactor<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::CreateEnrolledEventRequest,
        ::sensory::api::v1::audio::CreateEnrollmentResponse
    > CreateEnrolledEventBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating an audio enrollment.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param enrollment_config The enrollment configuration for the stream.
    /// Use `newCreateEnrolledEventConfig` to create a new enrollment config.
    /// _Ownership of the dynamically allocated configuration is implicitly
    /// transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `CreateEnrollmentEventConfig`
    /// message to the server.
    ///
    template<typename Reactor>
    inline void create_event_enrollment(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::CreateEnrollmentEventConfig* enrollment_config
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the request with the pointer to the config.
        enrollment_config->set_allocated_audio(audio_config);
        reactor->request.set_allocated_config(enrollment_config);
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
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param validate_config The validation configuration for the
    /// stream. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send audio data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `ValidateEnrolledEventConfig`
    /// message to the server.
    ///
    inline ValidateEnrolledEventStream validate_enrolled_event(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::ValidateEnrolledEventConfig* validate_config
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the enrollment config.
        validate_config->set_allocated_audio(audio_config);
        ::sensory::api::v1::audio::ValidateEnrolledEventRequest request;
        request.set_allocated_config(validate_config);
        // Create the stream and write the initial configuration request.
        auto stream = events_stub->ValidateEnrolledEvent(context);
        if (stream == nullptr)  // Failed to create stream.
            throw ::sensory::error::NullStreamError("Failed to start ValidateEnrolledEvent stream (null pointer).");
        if (!stream->Write(request))  // Failed to write audio config.
            throw ::sensory::error::WriteStreamError("Failed to write audio config to ValidateEnrolledEvent stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `ValidateEnrolledEvent` calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncReaderWriterCall<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::ValidateEnrolledEventRequest,
        ::sensory::api::v1::audio::ValidateEnrolledEventResponse
    > ValidateEnrolledEventAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// event enrollment validation.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param validate_config The validation configuration for the
    /// stream. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @param init_tag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finish_tag The tag to finish the stream with. Use `nullptr` to
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
    inline ValidateEnrolledEventAsyncStream* validate_enrolled_event(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::ValidateEnrolledEventConfig* validate_config,
        void* init_tag = nullptr,
        void* finish_tag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new ValidateEnrolledEventAsyncStream);
        // Set the client context for a unary call.
        token_manager.setup_bidi_client_context(call->context);
        // Create the request with the pointer to the enrollment config.
        validate_config->set_allocated_audio(audio_config);
        call->request.set_allocated_config(validate_config);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        init_tag = init_tag == nullptr ? static_cast<void*>(call) : init_tag;
        call->rpc = events_stub->AsyncValidateEnrolledEvent(&call->context, queue, init_tag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finish_tag = finish_tag == nullptr ? static_cast<void*>(call) : finish_tag;
        call->rpc->Finish(&call->status, finish_tag);
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `ValidateEnrolledEvent` calls.
    typedef ::sensory::calldata::AwaitableBidiReactor<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::ValidateEnrolledEventRequest,
        ::sensory::api::v1::audio::ValidateEnrolledEventResponse
    > ValidateEnrolledEventBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// event enrollment validation.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param validate_config The validation configuration for the
    /// stream. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `ValidateEnrolledEventConfig` message to
    /// the server.
    ///
    template<typename Reactor>
    inline void validate_enrolled_event(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::ValidateEnrolledEventConfig* validate_config
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the request with the pointer to the enrollment config.
        validate_config->set_allocated_audio(audio_config);
        reactor->request.set_allocated_config(validate_config);
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
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param transcribe_config The audio transcription configuration for the
    /// stream. _Ownership of the dynamically allocated configuration is
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
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::TranscribeConfig* transcribe_config
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the enrollment config.
        transcribe_config->set_allocated_audio(audio_config);
        ::sensory::api::v1::audio::TranscribeRequest request;
        request.set_allocated_config(transcribe_config);
        // Create the stream and write the initial configuration request.
        auto stream = transcriptions_stub->Transcribe(context);
        if (stream == nullptr)  // Failed to create stream.
            throw ::sensory::error::NullStreamError("Failed to start Transcribe stream (null pointer).");
        if (!stream->Write(request))  // Failed to write audio config.
            throw ::sensory::error::WriteStreamError("Failed to write audio config to Transcribe stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `Transcribe`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncReaderWriterCall<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::TranscribeRequest,
        ::sensory::api::v1::audio::TranscribeResponse
    > TranscribeAsyncStream;

    /// @brief Open a bidirectional stream to the server that provides a
    /// transcription of the provided audio data.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param transcribe_config The audio transcription configuration for the
    /// stream. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @param init_tag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finish_tag The tag to finish the stream with. Use `nullptr` to
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
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::TranscribeConfig* transcribe_config,
        void* init_tag = nullptr,
        void* finish_tag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new TranscribeAsyncStream);
        // Set the client context for a unary call.
        token_manager.setup_bidi_client_context(call->context);
        // Create the request with the pointer to the enrollment config.
        transcribe_config->set_allocated_audio(audio_config);
        call->request.set_allocated_config(transcribe_config);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        init_tag = init_tag == nullptr ? static_cast<void*>(call) : init_tag;
        call->rpc = transcriptions_stub->AsyncTranscribe(&call->context, queue, init_tag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finish_tag = finish_tag == nullptr ? static_cast<void*>(call) : finish_tag;
        call->rpc->Finish(&call->status, finish_tag);
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `Transcribe` calls.
    typedef ::sensory::calldata::AwaitableBidiReactor<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::TranscribeRequest,
        ::sensory::api::v1::audio::TranscribeResponse
    > TranscribeBidiReactor;

    /// @brief Open a bidirectional stream to the server that provides a
    /// transcription of the provided audio data.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param transcribe_config The audio transcription configuration for the
    /// stream. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    ///
    /// @details
    /// This call will automatically send the `TranscribeConfig` message to the
    /// server.
    ///
    template<typename Reactor>
    inline void transcribe(Reactor* reactor,
        ::sensory::api::v1::audio::AudioConfig* audio_config,
        ::sensory::api::v1::audio::TranscribeConfig* transcribe_config
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the request with the pointer to the enrollment config.
        transcribe_config->set_allocated_audio(audio_config);
        reactor->request.set_allocated_config(transcribe_config);
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
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param model_name The name of the synthesis model to use.
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
        const std::string& model_name,
        const uint32_t& sample_rate,
        const std::string& phrase
    ) const {
        token_manager.setup_bidi_client_context(*context);
        // Create the synthesis configuration object.
        auto synthesis_config = new ::sensory::api::v1::audio::VoiceSynthesisConfig;
        synthesis_config->set_modelname(model_name);
        synthesis_config->set_sampleratehertz(sample_rate);
        // Create the synthesis request.
        ::sensory::api::v1::audio::SynthesizeSpeechRequest request;
        request.set_allocated_config(synthesis_config);
        request.set_phrase(phrase);
        // Create the stream and write the initial configuration request.
        auto stream = synthesis_stub->SynthesizeSpeech(context, request);
        if (stream == nullptr)  // Failed to create stream.
            throw ::sensory::error::NullStreamError("Failed to start SynthesizeSpeech stream (null pointer).");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `SynthesizeSpeech` calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncResponseReaderCall<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::SynthesizeSpeechRequest,
        ::sensory::api::v1::audio::SynthesizeSpeechResponse
    > SynthesizeSpeechAsyncStream;

    /// @brief Open a unidirectional stream from the server that provides a
    /// synthesis of the provided text.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param model_name The name of the synthesis model to use.
    /// @param phrase The text phrase to synthesize into speech.
    /// @param init_tag The tag to initialize the stream with. Use `nullptr` to
    /// use the pointer as the tag.
    /// @param finish_tag The tag to finish the stream with. Use `nullptr` to
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
        const std::string& model_name,
        const uint32_t& sample_rate,
        const std::string& phrase,
        void* init_tag = nullptr,
        void* finish_tag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new SynthesizeSpeechAsyncStream);
        // Set the client context for a unary call.
        token_manager.setup_bidi_client_context(call->context);
        // Create the synthesis configuration object.
        auto synthesis_config = new ::sensory::api::v1::audio::VoiceSynthesisConfig;
        synthesis_config->set_modelname(model_name);
        synthesis_config->set_sampleratehertz(sample_rate);
        // Create the request with the pointer to the synthesis config.
        call->request.set_allocated_config(synthesis_config);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        init_tag = init_tag == nullptr ? static_cast<void*>(call) : init_tag;
        call->rpc = synthesis_stub->AsyncSynthesizeSpeech(&call->context, queue, init_tag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finish_tag = finish_tag == nullptr ? static_cast<void*>(call) : finish_tag;
        call->rpc->Finish(&call->status, finish_tag);
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `SynthesizeSpeech` calls.
    typedef ::sensory::calldata::AwaitableReadReactor<
        AudioService<CredentialStore>,
        ::sensory::api::v1::audio::SynthesizeSpeechResponse
    > SynthesizeSpeechReadReactor;

    /// @brief Open a unidirectional stream from the server that provides a
    /// synthesis of the provided text.
    ///
    /// @tparam Reactor The type of the read reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param audio_config The audio configuration that provides information
    /// about the input audio streams. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @param model_name The name of the synthesis model to use.
    /// @param phrase The text phrase to synthesize into speech.
    ///
    /// @details
    /// This call will automatically send the `VoiceSynthesisConfig` message to
    /// the server.
    ///
    template<typename Reactor>
    inline void synthesize_speech(Reactor* reactor,
        const std::string& model_name,
        const uint32_t& sample_rate,
        const std::string& phrase
    ) const {
        // Setup the context of the reactor for a unidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Create the synthesis configuration object.
        auto synthesis_config = new ::sensory::api::v1::audio::VoiceSynthesisConfig;
        synthesis_config->set_modelname(model_name);
        synthesis_config->set_sampleratehertz(sample_rate);
        reactor->request.set_allocated_config(synthesis_config);
        // Create the stream and write the initial configuration request.
        synthesis_stub->async()->SynthesizeSpeech(&reactor->context, reactor);
        reactor->StartRead(&reactor->response);
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORYCLOUD_SERVICES_AUDIO_SERVICE_HPP_
