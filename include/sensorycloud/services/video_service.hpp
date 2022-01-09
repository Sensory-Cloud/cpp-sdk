// The video service for the Sensory Cloud SDK.
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

#ifndef SENSORY_CLOUD_SERVICES_VIDEO_SERVICE_HPP_
#define SENSORY_CLOUD_SERVICES_VIDEO_SERVICE_HPP_

#include <memory>
#include <string>
#include <utility>
#include "sensorycloud/generated/v1/video/video.pb.h"
#include "sensorycloud/generated/v1/video/video.grpc.pb.h"
#include "sensorycloud/config.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/call_data.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud services.
namespace service {

/// @brief Sensory Cloud video service.
namespace video {

// TODO: implement `compression`?
// TODO: implement `referenceId`?

/// @brief Allocation a create enrollment config for initializing an
/// enrollment creation stream.
///
/// @param modelName The name of the model to use to create the enrollment.
/// Use `getModels()` to fetch a list of available models.
/// @param userID The ID of the user performing the request.
/// @param description The description of the enrollment.
/// @param isLivenessEnabled `true` to perform a liveness check in addition
/// to an enrollment, `false` to perform the enrollment without the liveness
/// check.
/// @param livenessThreshold The liveness threshold for the optional
/// liveness check.
/// @returns A pointer to a new
/// `sensory::api::v1::video::CreateEnrollmentConfig`.
///
inline ::sensory::api::v1::video::CreateEnrollmentConfig* newCreateEnrollmentConfig(
    const std::string& modelName,
    const std::string& userID,
    const std::string& description,
    const bool& isLivenessEnabled,
    const ::sensory::api::v1::video::RecognitionThreshold& livenessThreshold
) {
    auto config = new ::sensory::api::v1::video::CreateEnrollmentConfig;
    config->set_modelname(modelName);
    config->set_userid(userID);
    config->set_description(description);
    config->set_islivenessenabled(isLivenessEnabled);
    config->set_livenessthreshold(livenessThreshold);
    return config;
}

// TODO: implement `compression`?
// TODO: implement `doIncludeToken`?

/// @brief Allocation an authentication config for initializing an
/// authentication stream.
///
/// @param enrollmentID The enrollment ID to authenticate against. This can
/// be either an enrollment ID or a group ID.
/// @param isLivenessEnabled `true` to perform a liveness check before the
/// authentication, `false` to only perform the authentication.
/// @param livenessThreshold The liveness threshold for the optional
/// liveness check.
/// @returns A pointer to a new `sensory::api::v1::video::AuthenticateConfig`.
///
inline ::sensory::api::v1::video::AuthenticateConfig* newAuthenticateConfig(
    const std::string& enrollmentID,
    const bool& isLivenessEnabled,
    const ::sensory::api::v1::video::RecognitionThreshold& livenessThreshold
) {
    auto config = new ::sensory::api::v1::video::AuthenticateConfig;
    config->set_enrollmentid(enrollmentID);
    config->set_islivenessenabled(isLivenessEnabled);
    config->set_livenessthreshold(livenessThreshold);
    return config;
}

/// @brief Allocation a recognition config for initializing a liveness
/// validation stream.
///
/// @param modelName The name of the model to use. Use `getModels()` to
/// fetch a list of available models.
/// @param userID The ID of the user performing the request.
/// @param threshold The threshold of how confident the model has to be to
/// give a positive liveness result.
/// @returns A pointer to a new
/// `sensory::api::v1::video::ValidateRecognitionConfig`.
///
inline ::sensory::api::v1::video::ValidateRecognitionConfig* newValidateRecognitionConfig(
    const std::string& modelName,
    const std::string& userID,
    const ::sensory::api::v1::video::RecognitionThreshold& threshold
) {
    auto config = new ::sensory::api::v1::video::ValidateRecognitionConfig;
    config->set_modelname(modelName);
    config->set_userid(userID);
    config->set_threshold(threshold);
    return config;
}

}  // namespace video

/// @brief A service for video data.
/// @tparam SecureCredentialStore A secure key-value store for storing and
/// fetching credentials and tokens.
template<typename SecureCredentialStore>
class VideoService {
 private:
    /// the global configuration for the remote connection
    const ::sensory::Config& config;
    /// the token manager for securing gRPC requests to the server
    ::sensory::token_manager::TokenManager<SecureCredentialStore>& tokenManager;
    /// the gRPC stub for the video models service
    std::unique_ptr<::sensory::api::v1::video::VideoModels::Stub> modelsStub;
    /// the gRPC stub for the video biometrics service
    std::unique_ptr<::sensory::api::v1::video::VideoBiometrics::Stub> biometricsStub;
    /// the gRPC stub for the video recognition service
    std::unique_ptr<::sensory::api::v1::video::VideoRecognition::Stub> recognitionStub;

    /// @brief Create a copy of this object.
    ///
    /// @param other the other instance to copy data from
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object
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
    /// @param tokenManager_ The token manager for requesting Bearer tokens.
    ///
    VideoService(
        const ::sensory::Config& config_,
        ::sensory::token_manager::TokenManager<SecureCredentialStore>& tokenManager_
    ) : config(config_),
        tokenManager(tokenManager_),
        modelsStub(::sensory::api::v1::video::VideoModels::NewStub(config.getChannel())),
        biometricsStub(::sensory::api::v1::video::VideoBiometrics::NewStub(config.getChannel())),
        recognitionStub(::sensory::api::v1::video::VideoRecognition::NewStub(config.getChannel())) { }

    // ----- Get Models --------------------------------------------------------

    /// @brief Fetch a list of the vision models supported by the cloud host.
    ///
    /// @param response The response to populate from the RPC.
    /// @returns The status of the synchronous RPC.
    ///
    inline ::grpc::Status getModels(
        ::sensory::api::v1::video::GetModelsResponse* response
    ) const {
        // Create a context for the client for a unary call.
        ::grpc::ClientContext context;
        config.setupUnaryClientContext(context, tokenManager);
        // Execute the RPC synchronously and return the status
        return modelsStub->GetModels(&context, {}, response);
    }

    /// @brief A type for encapsulating data for asynchronous `GetModels` calls
    /// based on CompletionQueue event loops.
    typedef ::sensory::AsyncResponseReaderCall<
        VideoService<SecureCredentialStore>,
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

    /// @brief A type for encapsulating data for asynchronous `GetModels` calls
    /// based on callback/reactor patterns.
    typedef ::sensory::CallData<
        VideoService<SecureCredentialStore>,
        ::sensory::api::v1::video::GetModelsRequest,
        ::sensory::api::v1::video::GetModelsResponse
    > GetModelsCallData;

    /// @brief Fetch a list of the vision models supported by the cloud host.
    ///
    /// @tparam Callback the type of the callback function. The callback should
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
                call->setIsDone();
            });
        return call;
    }

    // ----- Create Enrollment -------------------------------------------------

    /// A type for biometric enrollment streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriter<
            ::sensory::api::v1::video::CreateEnrollmentRequest,
            ::sensory::api::v1::video::CreateEnrollmentResponse
        >
    > CreateEnrollmentStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating a video enrollment.
    ///
    /// @param context the gRPC context for the stream
    /// @param enrollmentConfig The enrollment configuration for the stream.
    /// Use `newCreateEnrollmentConfig` to create a new enrollment config.
    /// _Ownership of the dynamically allocated configuration is implicitly
    /// transferred to the stream_.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `CreateEnrollmentConfig` message
    /// to the server.
    ///
    inline CreateEnrollmentStream createEnrollment(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::video::CreateEnrollmentConfig* enrollmentConfig
    ) const {
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(*context, tokenManager);
        // Create the request with the pointer to the allocated config.
        enrollmentConfig->set_deviceid(config.getDeviceID());
        ::sensory::api::v1::video::CreateEnrollmentRequest request;
        request.set_allocated_config(enrollmentConfig);
        // Create the stream and write the initial configuration request.
        auto stream = biometricsStub->CreateEnrollment(context);
        stream->Write(request);
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `CreateEnrollment`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::AsyncReaderWriterCall<
        VideoService<SecureCredentialStore>,
        ::sensory::api::v1::video::CreateEnrollmentRequest,
        ::sensory::api::v1::video::CreateEnrollmentResponse
    > CreateEnrollmentAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating a video enrollment.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param enrollmentConfig The enrollment configuration for the stream.
    /// Use `newCreateEnrollmentConfig` to create a new enrollment config.
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
        ::sensory::api::v1::video::CreateEnrollmentConfig* enrollmentConfig,
        void* initTag = nullptr,
        void* finishTag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new CreateEnrollmentAsyncStream);
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(call->context, tokenManager);
        // Update the request with the pointer to the allocated config.
        enrollmentConfig->set_deviceid(config.getDeviceID());
        call->request.set_allocated_config(enrollmentConfig);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        initTag = initTag == nullptr ? static_cast<void*>(call) : initTag;
        call->rpc = biometricsStub->AsyncCreateEnrollment(&call->context, queue, initTag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finishTag = finishTag == nullptr ? static_cast<void*>(call) : finishTag;
        call->rpc->Finish(&call->status, finishTag);
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `CreateEnrollment` calls.
    typedef ::sensory::AwaitableBidiReactor<
        VideoService<SecureCredentialStore>,
        ::sensory::api::v1::video::CreateEnrollmentRequest,
        ::sensory::api::v1::video::CreateEnrollmentResponse
    > CreateEnrollmentBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating a video enrollment.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param enrollmentConfig The enrollment configuration for the stream.
    /// Use `newCreateEnrollmentConfig` to create a new enrollment config.
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
    inline void createEnrollment(Reactor* reactor,
        ::sensory::api::v1::video::CreateEnrollmentConfig* enrollmentConfig
    ) const {
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(reactor->context, tokenManager);
        // Update the request with the pointer to the allocated config.
        enrollmentConfig->set_deviceid(config.getDeviceID());
        reactor->request.set_allocated_config(enrollmentConfig);
        // Start the stream with the context in the reactor and a pointer to
        // reactor for callbacks.
        biometricsStub->async()->CreateEnrollment(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }

    // ----- Authenticate ------------------------------------------------------

    /// A type for biometric authentication streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriter<
            ::sensory::api::v1::video::AuthenticateRequest,
            ::sensory::api::v1::video::AuthenticateResponse
        >
    > AuthenticateStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// video authentication.
    ///
    /// @param context the gRPC context for the stream
    /// @param authenticateConfig The authentication configuration for the
    /// stream. Use `newAuthenticateConfig` to create a new authentication
    /// config. _Ownership of the dynamically allocated configuration is
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
        ::sensory::api::v1::video::AuthenticateConfig* authenticateConfig
    ) const {
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(*context, tokenManager);
        // Create the request with the pointer to the allocated config.
        ::sensory::api::v1::video::AuthenticateRequest request;
        request.set_allocated_config(authenticateConfig);
        // Create the stream and write the initial configuration request.
        auto stream = biometricsStub->Authenticate(context);
        stream->Write(request);
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `Authenticate`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::AsyncReaderWriterCall<
        VideoService<SecureCredentialStore>,
        ::sensory::api::v1::video::AuthenticateRequest,
        ::sensory::api::v1::video::AuthenticateResponse
    > AuthenticateAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// video authentication.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param authenticateConfig The authentication configuration for the
    /// stream. Use `newAuthenticateConfig` to create a new authentication
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
        ::sensory::api::v1::video::AuthenticateConfig* authenticateConfig,
        void* initTag = nullptr,
        void* finishTag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new AuthenticateAsyncStream);
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(call->context, tokenManager);
        // Update the request with the pointer to the allocated config.
        call->request.set_allocated_config(authenticateConfig);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        initTag = initTag == nullptr ? static_cast<void*>(call) : initTag;
        call->rpc = biometricsStub->AsyncAuthenticate(&call->context, queue, initTag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finishTag = finishTag == nullptr ? static_cast<void*>(call) : finishTag;
        call->rpc->Finish(&call->status, finishTag);
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `Authenticate` calls.
    typedef ::sensory::AwaitableBidiReactor<
        VideoService<SecureCredentialStore>,
        ::sensory::api::v1::video::AuthenticateRequest,
        ::sensory::api::v1::video::AuthenticateResponse
    > AuthenticateBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// video authentication.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param authenticateConfig The authentication configuration for the
    /// stream. Use `newAuthenticateConfig` to create a new authentication
    /// config. _Ownership of the dynamically allocated configuration is
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
        ::sensory::api::v1::video::AuthenticateConfig* authenticateConfig
    ) const {
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(reactor->context, tokenManager);
        // Update the request with the pointer to the allocated config.
        reactor->request.set_allocated_config(authenticateConfig);
        // Start the stream with the context in the reactor and a pointer to
        // reactor for callbacks.
        biometricsStub->async()->Authenticate(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }

    // ----- Validate Liveness -------------------------------------------------

    /// A type for face liveness validation streams.
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriter<
            ::sensory::api::v1::video::ValidateRecognitionRequest,
            ::sensory::api::v1::video::LivenessRecognitionResponse
        >
    > ValidateLivenessStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// validating the liveness of an image stream.
    ///
    /// @param context the gRPC context for the stream
    /// @param validateRecognitionConfig The recognition validation
    /// configuration for the stream. Use `newValidateRecognitionConfig` to
    /// create a new recognition validation config. _Ownership of the
    /// dynamically allocated configuration is implicitly transferred to the
    /// stream_.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `ValidateRecognitionConfig`
    /// message to the server.
    ///
    inline ValidateLivenessStream validateLiveness(
        ::grpc::ClientContext* context,
        ::sensory::api::v1::video::ValidateRecognitionConfig* recognitionConfig
    ) const {
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(*context, tokenManager);
        // Create the request with the pointer to the allocated config.
        ::sensory::api::v1::video::ValidateRecognitionRequest request;
        // Update the request with the pointer to the allocated config.
        request.set_allocated_config(recognitionConfig);
        // Create the stream and write the initial configuration request.
        auto stream = recognitionStub->ValidateLiveness(context);
        stream->Write(request);
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous `ValidateLiveness`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::AsyncReaderWriterCall<
        VideoService<SecureCredentialStore>,
        ::sensory::api::v1::video::ValidateRecognitionRequest,
        ::sensory::api::v1::video::LivenessRecognitionResponse
    > ValidateLivenessAsyncStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// validating the liveness of an image stream.
    ///
    /// @param queue The `::grpc::CompletionQueue` instance for handling
    /// asynchronous callbacks.
    /// @param validateRecognitionConfig The recognition validation
    /// configuration for the stream. Use `newValidateRecognitionConfig` to
    /// create a new recognition validation config. _Ownership of the
    /// dynamically allocated configuration is implicitly transferred to the
    /// stream_.
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
    /// `ValidateRecognitionConfig` message to the server, but will buffer it
    /// in the message for later transmission.
    ///
    inline ValidateLivenessAsyncStream* validateLiveness(
        ::grpc::CompletionQueue* queue,
        ::sensory::api::v1::video::ValidateRecognitionConfig* recognitionConfig,
        void* initTag = nullptr,
        void* finishTag = nullptr
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new ValidateLivenessAsyncStream);
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(call->context, tokenManager);
        // Update the request with the pointer to the allocated config.
        call->request.set_allocated_config(recognitionConfig);
        // Start the asynchronous RPC with the call's context and queue. If the
        // initial tag is a nullptr, assign it to the call pointer.
        initTag = initTag == nullptr ? static_cast<void*>(call) : initTag;
        call->rpc = recognitionStub->AsyncValidateLiveness(&call->context, queue, initTag);
        // Finish the call to set the output status. If the finish tag is a
        // nullptr, assign it to the call pointer.
        finishTag = finishTag == nullptr ? static_cast<void*>(call) : finishTag;
        call->rpc->Finish(&call->status, finishTag);
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `ValidateLiveness` calls.
    typedef ::sensory::AwaitableBidiReactor<
        VideoService<SecureCredentialStore>,
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
    /// configuration for the stream. Use `newValidateRecognitionConfig` to
    /// create a new recognition validation config. _Ownership of the
    /// dynamically allocated configuration is implicitly transferred to the
    /// stream_.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the `ValidateRecognitionConfig`
    /// message to the server.
    ///
    template<typename Reactor>
    inline void validateLiveness(Reactor* reactor,
        ::sensory::api::v1::video::ValidateRecognitionConfig* recognitionConfig
    ) const {
        // Setup the context of the call for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(reactor->context, tokenManager);
        // Update the request with the pointer to the allocated config.
        reactor->request.set_allocated_config(recognitionConfig);
        // Start the stream with the context in the reactor and a pointer to
        // reactor for callbacks.
        recognitionStub->async()->ValidateLiveness(&reactor->context, reactor);
        reactor->StartWrite(&reactor->request);
        reactor->StartRead(&reactor->response);
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORY_CLOUD_SERVICES_VIDEO_SERVICE_HPP_
