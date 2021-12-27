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
#include "sensorycloud/call_data.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud services.
namespace service {

/// @brief A type for encapsulating data for asynchronous calls.
/// @tparam Factory The factory class that will manage the scope of the call.
/// @tparam Request The type of the request message.
/// @tparam Response The type of the response message.
///
/// @details
/// The `Factory` is marked as a friend in order to provide mutable access to
/// private attributes of the structure. This allows instances of `Factory` to
/// mutate to the structure while all external scopes are limited to the
/// immutable interface exposed by the public accessor functions. Instances of
/// `AsyncUnaryCall` are mutable within the scope of `Factory`, but immutable
/// outside of the scope of `Factory`.
///
template<typename Factory, typename Request, typename Response>
struct AsyncUnaryCall {
 private:
    /// The gPRC context that the call is initiated with.
    ::grpc::ClientContext context;
    /// The status of the RPC after the response is processed.
    ::grpc::Status status;
    /// The request to execute in the unary call.
    Request request;
    /// The response to process after the RPC completes.
    Response response;
    /// The reader RPC executing the call.
    std::unique_ptr<::grpc::ClientAsyncResponseReader<Response>> rpc;

    /// @brief Initialize a new call.
    AsyncUnaryCall() { }

    /// @brief Create a copy of this object.
    ///
    /// @param other the other instance to copy data from
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object
    AsyncUnaryCall(const AsyncUnaryCall& other) = delete;

    /// @brief Assign to this object using the `=` operator.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This assignment operator is private to prevent copying of this object.
    ///
    void operator=(const AsyncUnaryCall& other) = delete;

    // Mark the Factory type as a friend to allow it to have write access to
    // the internal types. This allows the parent scope to have mutability, but
    // all other scopes must access data through the immutable `get` interface.
    friend Factory;

 public:
    /// @brief Return the context that the call was created with.
    ///
    /// @returns The gRPC context associated with the call.
    ///
    inline const ::grpc::ClientContext& getContext() const { return context; }

    /// @brief Return the status of the call.
    ///
    /// @returns The gRPC status code and message from the call.
    ///
    inline const ::grpc::Status& getStatus() const { return status; }

    /// @brief Return the request of the call.
    ///
    /// @returns The request message buffer for the call.
    ///
    inline const Request& getRequest() const { return request; }

    /// @brief Return the response of the call.
    ///
    /// @returns The response message buffer for the call.
    ///
    inline const Response& getResponse() const { return response; }
};

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
    typedef AsyncUnaryCall<
        VideoService<SecureCredentialStore>,
        ::sensory::api::v1::video::GetModelsRequest,
        ::sensory::api::v1::video::GetModelsResponse
    > AsyncGetModelsCall;

    /// @brief Fetch a list of the vision models supported by the cloud host.
    ///
    /// @param queue The completion queue handling the event-loop processing.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    inline AsyncGetModelsCall* asyncGetModels(
        ::grpc::CompletionQueue* queue
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new AsyncGetModelsCall);
        // Set the client context for a unary call.
        config.setupUnaryClientContext(call->context, tokenManager);
        // Start the asynchronous RPC with the call's context and queue.
        call->rpc = modelsStub->AsyncGetModels(&call->context, call->request, queue);
        // Finish the RPC to tell it where the response and status buffers are
        // located within the call object. Use the address of the call as the
        // tag for identifying the call in the event-loop.
        call->rpc->Finish(&call->response, &call->status, (void*) call);
        // Return the pointer to the call. This both transfers the ownership of
        // the instance to the caller, and provides the caller with an
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
            ::sensory::api::v1::video::CreateEnrollmentRequest,
            ::sensory::api::v1::video::CreateEnrollmentResponse
        >
    > CreateEnrollmentStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating a video enrollment.
    ///
    /// @param modelName The name of the model to use to create the enrollment.
    /// Use `getModels()` to obtain a list of available models.
    /// @param userID The ID of the user performing the request.
    /// @param description The description of the enrollment.
    /// @param isLivenessEnabled `true` to perform a liveness check in addition
    /// to an enrollment, `false` to perform the enrollment without the liveness
    /// check.
    /// @param livenessThreshold The liveness threshold for the optional
    /// liveness check.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the initial `CreateEnrollmentConfig`
    /// message to the server.
    ///
    inline CreateEnrollmentStream createEnrollment(
        const std::string& modelName,
        const std::string& userID,
        const std::string& description = "",
        const bool& isLivenessEnabled = false,
        const ::sensory::api::v1::video::RecognitionThreshold& livenessThreshold =
            ::sensory::api::v1::video::RecognitionThreshold::LOW
    ) const {
        // Create a context for the client for a bidirectional stream.
        // TODO: will the stream automatically free this dynamically allocated
        // context?
        auto context = new ::grpc::ClientContext;
        config.setupBidiClientContext(*context, tokenManager);

        // Create the initial config message. gRPC expects a dynamically
        // allocated message and will free the pointer when exiting the scope
        // of the request.
        auto enrollment_config =
            new ::sensory::api::v1::video::CreateEnrollmentConfig;
        enrollment_config->set_modelname(modelName);
        enrollment_config->set_userid(userID);
        enrollment_config->set_deviceid(config.getDeviceID());
        enrollment_config->set_description(description);
        enrollment_config->set_islivenessenabled(isLivenessEnabled);
        enrollment_config->set_livenessthreshold(livenessThreshold);

        // Create the request with the pointer to the allocated config.
        ::sensory::api::v1::video::CreateEnrollmentRequest request;
        request.set_allocated_config(enrollment_config);

        // Create the stream and write the initial configuration request.
        CreateEnrollmentStream stream =
            biometricsStub->CreateEnrollment(context);
        stream->Write(request);
        return stream;
    }




    /// A type for asynchronous model fetching.
    typedef std::unique_ptr<
        ::grpc::ClientAsyncReaderWriter<
            ::sensory::api::v1::video::CreateEnrollmentRequest,
            ::sensory::api::v1::video::CreateEnrollmentResponse
        >
    > AsyncCreateEnrollmentReaderWriter;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating a video enrollment.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param modelName The name of the model to use to create the enrollment.
    /// Use `getModels()` to obtain a list of available models.
    /// @param userID The ID of the user performing the request.
    /// @param description The description of the enrollment.
    /// @param isLivenessEnabled `true` to perform a liveness check in addition
    /// to an enrollment, `false` to perform the enrollment without the liveness
    /// check.
    /// @param livenessThreshold The liveness threshold for the optional
    /// liveness check.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the initial `CreateEnrollmentConfig`
    /// message to the server.
    ///
    inline AsyncCreateEnrollmentReaderWriter asyncCreateEnrollment(
        ::grpc::CompletionQueue* queue,
        const std::string& modelName,
        const std::string& userID,
        const std::string& description = "",
        const bool& isLivenessEnabled = false,
        const ::sensory::api::v1::video::RecognitionThreshold& livenessThreshold =
            ::sensory::api::v1::video::RecognitionThreshold::LOW
    ) const {
        // Create a context for the client for a unary call.
        // TODO: this will result in a memory leak. Update to remove the memory
        // link by persisting a stack allocated context past the scope of this
        // call to setup the stream.
        auto context(new ::grpc::ClientContext);
        config.setupBidiClientContext(*context, tokenManager);
        // Create the initial config message. gRPC expects a dynamically
        // allocated message and will free the pointer when exiting the scope
        // of the request.
        auto enrollment_config =
            new ::sensory::api::v1::video::CreateEnrollmentConfig;
        enrollment_config->set_deviceid(config.getDeviceID());
        enrollment_config->set_modelname(modelName);
        enrollment_config->set_userid(userID);
        enrollment_config->set_description(description);
        enrollment_config->set_islivenessenabled(isLivenessEnabled);
        enrollment_config->set_livenessthreshold(livenessThreshold);
        // Start the stream with the context in the reactor and a pointer to
        // reactor for callbacks.
        auto rpc = biometricsStub->AsyncCreateEnrollment(context, queue, (void*) 1);

        // Update the request buffer in the reactor with the allocated config.
        // reactor->request.set_allocated_config(enrollment_config);

        return rpc;
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
    /// @param modelName The name of the model to use to create the enrollment.
    /// Use `getModels()` to obtain a list of available models.
    /// @param userID The ID of the user performing the request.
    /// @param description The description of the enrollment.
    /// @param isLivenessEnabled `true` to perform a liveness check in addition
    /// to an enrollment, `false` to perform the enrollment without the liveness
    /// check.
    /// @param livenessThreshold The liveness threshold for the optional
    /// liveness check.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the initial `CreateEnrollmentConfig`
    /// message to the server.
    ///
    template<typename Reactor>
    inline void asyncCreateEnrollment(Reactor* reactor,
        const std::string& modelName,
        const std::string& userID,
        const std::string& description = "",
        const bool& isLivenessEnabled = false,
        const ::sensory::api::v1::video::RecognitionThreshold& livenessThreshold =
            ::sensory::api::v1::video::RecognitionThreshold::LOW
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(reactor->context, tokenManager);
        // Create the initial config message. gRPC expects a dynamically
        // allocated message and will free the pointer when exiting the scope
        // of the request.
        auto enrollment_config =
            new ::sensory::api::v1::video::CreateEnrollmentConfig;
        enrollment_config->set_deviceid(config.getDeviceID());
        enrollment_config->set_modelname(modelName);
        enrollment_config->set_userid(userID);
        enrollment_config->set_description(description);
        enrollment_config->set_islivenessenabled(isLivenessEnabled);
        enrollment_config->set_livenessthreshold(livenessThreshold);
        // Update the request buffer in the reactor with the allocated config.
        reactor->request.set_allocated_config(enrollment_config);
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
    /// @param enrollmentID The enrollment ID to authenticate against. This can
    /// be either an enrollment ID or a group ID.
    /// @param isLivenessEnabled `true` to perform a liveness check before the
    /// authentication, `false` to only perform the authentication.
    /// @param livenessThreshold The liveness threshold for the optional
    /// liveness check.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the initial `AuthenticateConfig`
    /// message to the server.
    ///
    inline AuthenticateStream authenticate(
        const std::string& enrollmentID,
        const bool& isLivenessEnabled = false,
        const ::sensory::api::v1::video::RecognitionThreshold& livenessThreshold =
            ::sensory::api::v1::video::RecognitionThreshold::LOW
    ) const {
        // Create a context for the client for a bidirectional stream.
        // TODO: will the stream automatically free this dynamically allocated
        // context?
        auto context = new ::grpc::ClientContext;
        config.setupBidiClientContext(*context, tokenManager);

        // Create the initial config message. gRPC expects a dynamically
        // allocated message and will free the pointer when exiting the scope
        // of the request.
        auto authenticate_config =
            new ::sensory::api::v1::video::AuthenticateConfig;
        authenticate_config->set_enrollmentid(enrollmentID);
        authenticate_config->set_islivenessenabled(isLivenessEnabled);
        authenticate_config->set_livenessthreshold(livenessThreshold);

        // Create the request with the pointer to the allocated config.
        ::sensory::api::v1::video::AuthenticateRequest request;
        request.set_allocated_config(authenticate_config);

        // Create the stream and write the initial configuration request.
        AuthenticateStream stream = biometricsStub->Authenticate(context);
        stream->Write(request);
        return stream;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `Authenticate` calls.
    typedef ::sensory::AwaitableBidiReactor<
        VideoService<SecureCredentialStore>,
        ::sensory::api::v1::video::AuthenticateRequest,
        ::sensory::api::v1::video::AuthenticateResponse
    > AuthorizeBidiReactor;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// video authentication.
    ///
    /// @tparam Reactor The type of the reactor for handling callbacks.
    /// @param reactor The reactor for receiving callbacks and managing the
    /// context of the stream.
    /// @param enrollmentID The enrollment ID to authenticate against. This can
    /// be either an enrollment ID or a group ID.
    /// @param isLivenessEnabled `true` to perform a liveness check before the
    /// authentication, `false` to only perform the authentication.
    /// @param livenessThreshold The liveness threshold for the optional
    /// liveness check.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the initial `CreateEnrollmentConfig`
    /// message to the server.
    ///
    template<typename Reactor>
    inline void asyncAuthenticate(Reactor* reactor,
        const std::string& enrollmentID,
        const bool& isLivenessEnabled = false,
        const ::sensory::api::v1::video::RecognitionThreshold& livenessThreshold =
            ::sensory::api::v1::video::RecognitionThreshold::LOW
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(reactor->context, tokenManager);
        // Create the initial config message. gRPC expects a dynamically
        // allocated message and will free the pointer when exiting the scope
        // of the request.
        auto authenticate_config =
            new ::sensory::api::v1::video::AuthenticateConfig;
        authenticate_config->set_enrollmentid(enrollmentID);
        authenticate_config->set_islivenessenabled(isLivenessEnabled);
        authenticate_config->set_livenessthreshold(livenessThreshold);
        // Update the request buffer in the reactor with the allocated config.
        reactor->request.set_allocated_config(authenticate_config);
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
    /// @param modelName The name of the model to use. Use `getModels()` to
    /// obtain a list of available models.
    /// @param userID The ID of the user performing the request.
    /// @param threshold The threshold of how confident the model has to be to
    /// give a positive liveness result.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the initial
    /// `ValidateRecognitionConfig` message to the server.
    ///
    inline ValidateLivenessStream validateLiveness(
        const std::string& modelName,
        const std::string& userID,
        const ::sensory::api::v1::video::RecognitionThreshold& threshold
    ) const {
        // Create a context for the client for a bidirectional stream.
        // TODO: will the stream automatically free this dynamically allocated
        // context?
        auto context = new ::grpc::ClientContext;
        config.setupBidiClientContext(*context, tokenManager);

        // Create the initial config message. gRPC expects a dynamically
        // allocated message and will free the pointer when exiting the scope
        // of the request.
        auto recognition_config =
            new ::sensory::api::v1::video::ValidateRecognitionConfig;
        recognition_config->set_modelname(modelName);
        recognition_config->set_userid(userID);
        recognition_config->set_threshold(threshold);

        // Create the request with the pointer to the allocated config.
        ::sensory::api::v1::video::ValidateRecognitionRequest request;
        request.set_allocated_config(recognition_config);

        // Create the stream and write the initial configuration request.
        ValidateLivenessStream stream =
            recognitionStub->ValidateLiveness(context);
        stream->Write(request);
        return stream;
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
    /// @param modelName The name of the model to use. Use `getModels()` to
    /// obtain a list of available models.
    /// @param userID The ID of the user performing the request.
    /// @param threshold The threshold of how confident the model has to be to
    /// give a positive liveness result.
    /// @returns A bidirectional stream that can be used to send video data to
    /// the server.
    ///
    /// @details
    /// This call will automatically send the initial `CreateEnrollmentConfig`
    /// message to the server.
    ///
    template<typename Reactor>
    inline void asyncValidateLiveness(Reactor* reactor,
        const std::string& modelName,
        const std::string& userID,
        const ::sensory::api::v1::video::RecognitionThreshold& threshold
    ) const {
        // Setup the context of the reactor for a bidirectional stream. This
        // will add the Bearer token to the header of the RPC.
        config.setupBidiClientContext(reactor->context, tokenManager);
        // Create the initial config message. gRPC expects a dynamically
        // allocated message and will free the pointer when exiting the scope
        // of the request.
        auto recognition_config =
            new ::sensory::api::v1::video::ValidateRecognitionConfig;
        recognition_config->set_modelname(modelName);
        recognition_config->set_userid(userID);
        recognition_config->set_threshold(threshold);
        // Update the request buffer in the reactor with the allocated config.
        reactor->request.set_allocated_config(recognition_config);
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
