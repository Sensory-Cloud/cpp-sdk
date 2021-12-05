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

    /// A type for asynchronous model fetching.
    typedef std::unique_ptr<
        ::grpc::ClientAsyncResponseReader<
            ::sensory::api::v1::video::GetModelsResponse
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

    /// @brief A type for encapsulating data for asynchronous
    /// `CreateEnrollment` calls.
    typedef ::sensory::BidiReactor<
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
    typedef ::sensory::BidiReactor<
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
    typedef ::sensory::BidiReactor<
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
