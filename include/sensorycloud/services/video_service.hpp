// The video service for the SensoryCloud SDK.
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

#ifndef SENSORYCLOUD_SERVICES_VIDEO_SERVICE_HPP_
#define SENSORYCLOUD_SERVICES_VIDEO_SERVICE_HPP_

#include <memory>
#include <string>
#include <utility>
#include "sensorycloud/generated/v1/video/video.pb.h"
#include "sensorycloud/generated/v1/video/video.grpc.pb.h"
#include "sensorycloud/config.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/calldata.hpp"
#include "sensorycloud/error/stream_errors.hpp"

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief SensoryCloud services.
namespace service {

/// @brief A service for video data.
/// @tparam CredentialStore A key-value store for storing and fetching
/// credentials and tokens.
template<typename CredentialStore>
class VideoService {
 private:
    /// the global configuration for the remote connection
    const ::sensory::Config& config;
    /// the token manager for securing gRPC requests to the server
    ::sensory::token_manager::TokenManager<CredentialStore>& token_manager;
    /// the gRPC stub for the video models service
    std::unique_ptr<::sensory::api::v1::video::VideoModels::StubInterface> models_stub;
    /// the gRPC stub for the video biometrics service
    std::unique_ptr<::sensory::api::v1::video::VideoBiometrics::StubInterface> biometrics_stub;
    /// the gRPC stub for the video recognition service
    std::unique_ptr<::sensory::api::v1::video::VideoRecognition::StubInterface> recognition_stub;

    /// @brief Create a copy of this object.
    ///
    /// @param other the other instance to copy data from
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object.
    ///
    VideoService(const VideoService& other) = delete;

    /// @brief Assign to this object using the `=` operator.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This assignment operator is private to prevent copying of this object.
    ///
    void operator=(const VideoService& other) = delete;

 public:
    /// @brief Initialize a new video service.
    ///
    /// @param config_ The global configuration for the remote connection.
    /// @param token_manager_ The token manager for requesting Bearer tokens.
    ///
    VideoService(
        const ::sensory::Config& config_,
        ::sensory::token_manager::TokenManager<CredentialStore>& token_manager_
    ) : config(config_),
        token_manager(token_manager_),
        models_stub(::sensory::api::v1::video::VideoModels::NewStub(config.get_channel())),
        biometrics_stub(::sensory::api::v1::video::VideoBiometrics::NewStub(config.get_channel())),
        recognition_stub(::sensory::api::v1::video::VideoRecognition::NewStub(config.get_channel())) { }

    /// @brief Initialize a new video service.
    ///
    /// @param config_ The global configuration for the remote connection.
    /// @param token_manager_ The token manager for requesting Bearer tokens.
    /// @param models_stub_ The models service stub to initialize the service
    /// with.
    /// @param biometrics_stub_ The biometrics service stub to initialize the
    /// service with.
    /// @param recognition_stub The recognition service stub to initialize the
    /// service with.
    ///
    VideoService(
        const ::sensory::Config& config_,
        ::sensory::token_manager::TokenManager<CredentialStore>& token_manager_,
        ::sensory::api::v1::video::VideoModels::StubInterface* models_stub_,
        ::sensory::api::v1::video::VideoBiometrics::StubInterface* biometrics_stub_,
        ::sensory::api::v1::video::VideoRecognition::StubInterface* recognition_stub_
    ) : config(config_),
        token_manager(token_manager_),
        models_stub(models_stub_),
        biometrics_stub(biometrics_stub_),
        recognition_stub(recognition_stub_) { }

    /// @brief Return the cloud configuration associated with this service.
    ///
    /// @returns the configuration used by this service.
    ///
    inline const ::sensory::Config& get_config() const { return config; }

    // ----- Get Models --------------------------------------------------------

    /// @brief Fetch a list of the vision models supported by the cloud host.
    ///
    /// @param response The response to populate from the RPC.
    /// @returns The status of the synchronous RPC.
    ///
    inline ::grpc::Status get_models(
        ::sensory::api::v1::video::GetModelsResponse* response
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
        VideoService<CredentialStore>,
        ::sensory::api::v1::video::GetModelsRequest,
        ::sensory::api::v1::video::GetModelsResponse
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

    /// @brief A type for encapsulating data for asynchronous `GetModels` calls
    /// based on callback/reactor patterns.
    typedef ::sensory::calldata::CallbackData<
        VideoService<CredentialStore>,
        ::sensory::api::v1::video::GetModelsRequest,
        ::sensory::api::v1::video::GetModelsResponse
    > GetModelsCallbackData;

    /// @brief Fetch a list of the vision models supported by the cloud host.
    ///
    /// @tparam Callback the type of the callback function. The callback should
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
            ::sensory::api::v1::video::CreateEnrollmentRequest,
            ::sensory::api::v1::video::CreateEnrollmentResponse
        >
    > CreateEnrollmentStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating a video enrollment.
    ///
    /// @param context the gRPC context for the stream
    /// @param enrollment_config The enrollment configuration for the stream.
    /// _Ownership of the dynamically allocated configuration is implicitly
    /// transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `CreateEnrollmentConfig` message
    /// to the server.
    ///
    inline CreateEnrollmentStream create_enrollment(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::video::CreateEnrollmentConfig* enrollment_config
    ) const {
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the allocated config.
        enrollment_config->set_deviceid(config.get_device_id());
        ::sensory::api::v1::video::CreateEnrollmentRequest request;
        request.set_allocated_config(enrollment_config);
        // Create the stream and write the initial configuration request.
        auto stream = biometrics_stub->CreateEnrollment(context);
        if (stream == nullptr)  // Failed to create stream.
            throw ::sensory::error::NullStreamError("Failed to start CreateEnrollment stream (null pointer).");
        if (!stream->Write(request))  // Failed to write video config.
            throw ::sensory::error::WriteStreamError("Failed to write video configuration to CreateEnrollment stream.");
        // Successfully created stream, implicitly cast it to a unique pointer.
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `CreateEnrollment`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncReaderWriterCall<
        VideoService<CredentialStore>,
        ::sensory::api::v1::video::CreateEnrollmentRequest,
        ::sensory::api::v1::video::CreateEnrollmentResponse
    > CreateEnrollmentAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating a video enrollment.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
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
        ::sensory::api::v1::video::CreateEnrollmentConfig* enrollment_config,
        void* init_tag = nullptr,
        void* finish_tag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new CreateEnrollmentAsyncStream);
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(call->context);
        // Update the request with the pointer to the allocated config.
        enrollment_config->set_deviceid(config.get_device_id());
        call->request.set_allocated_config(enrollment_config);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        init_tag = init_tag == nullptr ? static_cast<void*>(call) : init_tag;
        call->rpc = biometrics_stub->AsyncCreateEnrollment(&call->context, queue, init_tag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finish_tag = finish_tag == nullptr ? static_cast<void*>(call) : finish_tag;
        call->rpc->Finish(&call->status, finish_tag);
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `CreateEnrollment` calls.
    typedef ::sensory::calldata::AwaitableBidiReactor<
        VideoService<CredentialStore>,
        ::sensory::api::v1::video::CreateEnrollmentRequest,
        ::sensory::api::v1::video::CreateEnrollmentResponse
    > CreateEnrollmentBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating a video enrollment.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param enrollment_config The enrollment configuration for the stream.
    /// _Ownership of the dynamically allocated configuration is implicitly
    /// transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `CreateEnrollmentConfig` message
    /// to the server.
    ///
    template<typename Reactor>
    inline void create_enrollment(Reactor* reactor,
        ::sensory::api::v1::video::CreateEnrollmentConfig* enrollment_config
    ) const {
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Update the request with the pointer to the allocated config.
        enrollment_config->set_deviceid(config.get_device_id());
        reactor->request.set_allocated_config(enrollment_config);
        // Start the stream with the context in the reactor and a pointer to
        // reactor for callbacks.
        biometrics_stub->async()->CreateEnrollment(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }

    // ----- Authenticate ------------------------------------------------------

    /// A type for biometric authentication streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::video::AuthenticateRequest,
            ::sensory::api::v1::video::AuthenticateResponse
        >
    > AuthenticateStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// video authentication.
    ///
    /// @param context the gRPC context for the stream
    /// @param authenticate_config The authentication configuration for the
    /// stream. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `AuthenticateConfig` message to
    /// the server.
    ///
    inline AuthenticateStream authenticate(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::video::AuthenticateConfig* authenticate_config
    ) const {
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the allocated config.
        ::sensory::api::v1::video::AuthenticateRequest request;
        request.set_allocated_config(authenticate_config);
        // Create the stream and write the initial configuration request.
        auto stream = biometrics_stub->Authenticate(context);
        if (stream == nullptr)
            throw ::sensory::error::NullStreamError("Authenticate stream returned null pointer!");
        if (!stream->Write(request))  // Failed to write video config.
            throw ::sensory::error::WriteStreamError("Failed to write video configuration to Authenticate stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `Authenticate`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncReaderWriterCall<
        VideoService<CredentialStore>,
        ::sensory::api::v1::video::AuthenticateRequest,
        ::sensory::api::v1::video::AuthenticateResponse
    > AuthenticateAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// video authentication.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
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
        ::sensory::api::v1::video::AuthenticateConfig* authenticate_config,
        void* init_tag = nullptr,
        void* finish_tag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new AuthenticateAsyncStream);
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(call->context);
        // Update the request with the pointer to the allocated config.
        call->request.set_allocated_config(authenticate_config);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        init_tag = init_tag == nullptr ? static_cast<void*>(call) : init_tag;
        call->rpc = biometrics_stub->AsyncAuthenticate(&call->context, queue, init_tag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finish_tag = finish_tag == nullptr ? static_cast<void*>(call) : finish_tag;
        call->rpc->Finish(&call->status, finish_tag);
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `Authenticate` calls.
    typedef ::sensory::calldata::AwaitableBidiReactor<
        VideoService<CredentialStore>,
        ::sensory::api::v1::video::AuthenticateRequest,
        ::sensory::api::v1::video::AuthenticateResponse
    > AuthenticateBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// video authentication.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param authenticate_config The authentication configuration for the
    /// stream. _Ownership of the dynamically allocated configuration is
    /// implicitly transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `AuthenticateConfig` message to
    /// the server.
    ///
    template<typename Reactor>
    inline void authenticate(Reactor* reactor,
        ::sensory::api::v1::video::AuthenticateConfig* authenticate_config
    ) const {
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Update the request with the pointer to the allocated config.
        reactor->request.set_allocated_config(authenticate_config);
        // Start the stream with the context in the reactor and a pointer to
        // reactor for callbacks.
        biometrics_stub->async()->Authenticate(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }

    // ----- Validate Liveness -------------------------------------------------

    /// A type for face liveness validation streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::video::ValidateRecognitionRequest,
            ::sensory::api::v1::video::LivenessRecognitionResponse
        >
    > ValidateLivenessStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// validating the liveness of an image stream.
    ///
    /// @param context the gRPC context for the stream
    /// @param validateRecognitionConfig The recognition validation
    /// configuration for the stream. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `ValidateRecognitionConfig`
    /// message to the server.
    ///
    inline ValidateLivenessStream validate_liveness(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::video::ValidateRecognitionConfig* recognition_config
    ) const {
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(*context);
        // Create the request with the pointer to the allocated config.
        ::sensory::api::v1::video::ValidateRecognitionRequest request;
        request.set_allocated_config(recognition_config);
        // Create the stream and write the initial configuration request.
        auto stream = recognition_stub->ValidateLiveness(context);
        if (stream == nullptr)
            throw ::sensory::error::NullStreamError("Authenticate stream returned null pointer!");
        if (!stream->Write(request))  // Failed to write video config.
            throw ::sensory::error::WriteStreamError("Failed to write video configuration to ValidateLiveness stream.");
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `ValidateLiveness`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncReaderWriterCall<
        VideoService<CredentialStore>,
        ::sensory::api::v1::video::ValidateRecognitionRequest,
        ::sensory::api::v1::video::LivenessRecognitionResponse
    > ValidateLivenessAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// validating the liveness of an image stream.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param validateRecognitionConfig The recognition validation
    /// configuration for the stream. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
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
    /// `ValidateRecognitionConfig` message to the server, but will buffer it
    /// in the message for later transmission.
    ///
    inline ValidateLivenessAsyncStream* validate_liveness(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::video::ValidateRecognitionConfig* recognition_config,
        void* init_tag = nullptr,
        void* finish_tag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new ValidateLivenessAsyncStream);
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(call->context);
        // Update the request with the pointer to the allocated config.
        call->request.set_allocated_config(recognition_config);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        init_tag = init_tag == nullptr ? static_cast<void*>(call) : init_tag;
        call->rpc = recognition_stub->AsyncValidateLiveness(&call->context, queue, init_tag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finish_tag = finish_tag == nullptr ? static_cast<void*>(call) : finish_tag;
        call->rpc->Finish(&call->status, finish_tag);
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `ValidateLiveness` calls.
    typedef ::sensory::calldata::AwaitableBidiReactor<
        VideoService<CredentialStore>,
        ::sensory::api::v1::video::ValidateRecognitionRequest,
        ::sensory::api::v1::video::LivenessRecognitionResponse
    > ValidateLivenessBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// validating the liveness of an image stream.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param validateRecognitionConfig The recognition validation
    /// configuration for the stream. _Ownership of the dynamically allocated
    /// configuration is implicitly transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `ValidateRecognitionConfig`
    /// message to the server.
    ///
    template<typename Reactor>
    inline void validate_liveness(Reactor* reactor,
        ::sensory::api::v1::video::ValidateRecognitionConfig* recognition_config
    ) const {
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        token_manager.setup_bidi_client_context(reactor->context);
        // Update the request with the pointer to the allocated config.
        reactor->request.set_allocated_config(recognition_config);
        // Start the stream with the context in the reactor and a pointer to
        // reactor for callbacks.
        recognition_stub->async()->ValidateLiveness(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORYCLOUD_SERVICES_VIDEO_SERVICE_HPP_
