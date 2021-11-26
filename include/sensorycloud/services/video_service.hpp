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

#include <string>
#include <memory>
#include "sensorycloud/generated/v1/video/video.pb.h"
#include "sensorycloud/generated/v1/video/video.grpc.pb.h"
#include "sensorycloud/config.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud Services.
namespace service {

/// @brief A service for video data.
class VideoService {
 private:
    /// the global configuration for the remote connection
    const Config& config;
    /// the gRPC stub for the video models service
    std::unique_ptr<api::v1::video::VideoModels::Stub> models_stub;
    /// the gRPC stub for the video bio-metrics service
    std::unique_ptr<api::v1::video::VideoBiometrics::Stub> biometrics_stub;
    /// the gRPC stub for the video recognition service
    std::unique_ptr<api::v1::video::VideoRecognition::Stub> recognition_stub;

 public:
    /// @brief Initialize a new video service.
    ///
    /// @param config the global configuration for the remote connection
    ///
    explicit VideoService(const Config& config_) : config(config_),
        models_stub(api::v1::video::VideoModels::NewStub(config.getChannel())),
        biometrics_stub(api::v1::video::VideoBiometrics::NewStub(config.getChannel())),
        recognition_stub(api::v1::video::VideoRecognition::NewStub(config.getChannel())) { }

    /// @brief Fetch a list of the vision models supported by the cloud host.
    /// @returns A future to be fulfilled with either a list of available
    /// models, or the network error that occurred
    api::v1::video::GetModelsResponse getModels() {
        // std::cout << "Requesting video models from server" << std::endl;
        // Create a context for the client.
        grpc::ClientContext context;
        // Create the request from the parameters.
        api::v1::video::GetModelsRequest request;
        // Execute the RPC synchronously and get the response
        api::v1::video::GetModelsResponse response;
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
            ::sensory::api::v1::video::CreateEnrollmentRequest,
            ::sensory::api::v1::video::CreateEnrollmentResponse
        >
    > CreateEnrollmentStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// creating a video enrollment
    ///
    /// @param modelName Name of model to create enrollment for
    /// @param userID: Unique user identifier
    /// @param onStreamReceive: Handler function to handle responses sent from
    /// the server
    /// @param description: User supplied description of the enrollment
    /// @param isLivenessEnabled: Determines if a liveness check should be
    /// conducted as well as an enrollment
    /// @param livenessThreshold: Liveness threshold for the potential liveness
    /// check
    /// @throws `NetworkError` if an error occurs while processing the cached
    /// server url
    /// @returns Bidirectional stream that can be used to send video data to
    /// the server
    ///
    /// @details
    /// This call will automatically send the initial `videoConfig` message to
    /// the server.
    ///
    template<typename T>
    CreateEnrollmentStream createEnrollment(
        const std::string& modelName,
        const std::string& userID,
        const T& onStreamReceive,
        const std::string& description = "",
        const bool& isLivenessEnabled = false,
        const api::v1::video::RecognitionThreshold& livenessThreshold =
            api::v1::video::RecognitionThreshold::LOW
    ) {
        std::cout << "Starting video enrollment stream" << std::endl;

        // Create a context for the client.
        grpc::ClientContext context;
        const auto call = biometrics_stub->CreateEnrollment(&context, onStreamReceive);

        // Send initial config message
        api::v1::video::CreateEnrollmentConfig enrollment_config;
        enrollment_config.set_modelname(modelName);
        enrollment_config.set_userid(userID);
        enrollment_config.set_deviceid(config.getDeviceID());
        enrollment_config.set_description(description);
        enrollment_config.set_islivenessenabled(isLivenessEnabled);
        enrollment_config.set_livenessthreshold(livenessThreshold);

        api::v1::video::CreateEnrollmentRequest request;
        // TODO: is it better to use a dynamic allocation? Since the scope of
        // this function will end before the scope of the "call" resolves, the
        // config will be de-allocated and may result in a segmentation fault.
        request.set_allocated_config(&enrollment_config);

        call.Write(request);
        return call;
    }

    /// a type for bio-metric authentication streams
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::video::AuthenticateRequest,
            ::sensory::api::v1::video::AuthenticateResponse
        >
    > AuthenticateStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// video authentication.
    ///
    /// @param enrollmentID Enrollment to authenticate against
    /// @param isLivenessEnabled Determines if a liveness check should be
    /// conducted as well as an enrollment
    /// @param livenessThreshold Liveness threshold for the potential liveness
    /// check
    /// @param onStreamReceive Handler function to handle responses sent from
    /// the server
    /// @throws `NetworkError` if an error occurs while processing the cached
    /// server url
    /// @returns Bidirectional stream that can be used to send audio data to
    /// the server
    ///
    /// @details
    /// This call will automatically send the initial `VideoConfig` message to
    /// the server.
    ///
    template<typename T>
    AuthenticateStream authenticate(
        const std::string& enrollmentID,
        const T& onStreamReceive,
        const bool& isLivenessEnabled = false,
        const api::v1::video::RecognitionThreshold& livenessThreshold =
            api::v1::video::RecognitionThreshold::LOW
    ) {
        // std::cout << "Starting video authentication stream" << std::endl;

        // Create a context for the client.
        grpc::ClientContext context;
        const auto call = biometrics_stub->Authenticate(&context, onStreamReceive);

        // Send initial config message
        api::v1::video::AuthenticateConfig authenticate_config;
        authenticate_config.set_enrollmentid(enrollmentID);
        authenticate_config.set_islivenessenabled(isLivenessEnabled);
        authenticate_config.set_livenessthreshold(livenessThreshold);

        api::v1::video::AuthenticateRequest request;
        // TODO: is it better to use a dynamic allocation? Since the scope of
        // this function will end before the scope of the "call" resolves, the
        // config will be de-allocated and may result in a segmentation fault.
        request.set_allocated_config(&authenticate_config);

        call.Write(request);
        return call;
    }

    /// a type for face liveness validation streams
    typedef std::unique_ptr<
        ::grpc::ClientReaderWriterInterface<
            ::sensory::api::v1::video::ValidateRecognitionRequest,
            ::sensory::api::v1::video::LivenessRecognitionResponse
        >
    > ValidateLivenessStream;

    /// @brief Open a bidirectional stream to the server for the purpose of
    /// validating the liveness of an image stream.
    ///
    /// @param modelName Name of the model to use
    /// @param userID Unique user identifier
    /// @param threshold Threshold of how confident the model has to be to give
    /// a positive liveness result
    /// @param onStreamReceive Handler function to handle responses sent from
    /// the server
    /// @throws `NetworkError` if an error occurs while processing the cached
    /// server URL
    /// @returns Bidirectional stream that can be used to push image data to
    /// the server
    ///
    /// @details
    /// This call will automatically send the initial `VideoConfig` message to
    /// the server.
    ///
    template<typename T>
    ValidateLivenessStream validateLiveness(
        const std::string& modelName,
        const std::string& userID,
        const api::v1::video::RecognitionThreshold& threshold,
        const T& onStreamReceive
    ) {
        // std::cout << "Requesting Liveness stream from server" << std::endl;

        // Create a context for the client.
        grpc::ClientContext context;
        const auto call = recognition_stub->ValidateLiveness(&context, onStreamReceive);

        // Send initial config message
        api::v1::video::ValidateRecognitionConfig recognition_config;
        recognition_config.set_modelname(modelName);
        recognition_config.set_userid(userID);
        recognition_config.set_threshold(threshold);

        api::v1::video::ValidateRecognitionRequest request;
        // TODO: is it better to use a dynamic allocation? Since the scope of
        // this function will end before the scope of the "call" resolves, the
        // config will be de-allocated and may result in a segmentation fault.
        request.set_allocated_config(&recognition_config);

        call.Write(request);
        return call;
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORY_CLOUD_SERVICES_VIDEO_SERVICE_HPP_
