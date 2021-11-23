// The OAuth service for the Sensory Cloud SDK.
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

#ifndef SENSORY_CLOUD_SERVICES_OAUTH_SERVICE_HPP_
#define SENSORY_CLOUD_SERVICES_OAUTH_SERVICE_HPP_

#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include "sensorycloud/generated/oauth/oauth.pb.h"
#include "sensorycloud/generated/oauth/oauth.grpc.pb.h"
#include "sensorycloud/generated/v1/management/device.pb.h"
#include "sensorycloud/generated/v1/management/device.grpc.pb.h"
#include "sensorycloud/config.hpp"
#include "sensorycloud/services/network_error.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud Services.
namespace service {

/// @brief A service for handling device and user authentication.
class OAuthService {
 private:
    /// The gRPC stub for the device service
    std::unique_ptr<api::v1::management::DeviceService::Stub> device_stub;
    /// The gRPC stub for the OAuth service
    std::unique_ptr<api::oauth::OauthService::Stub> oauth_stub;

 public:
    /// @brief Initialize a new OAuth service.
    ///
    /// @param channel TODO
    ///
    OAuthService(std::shared_ptr<grpc::Channel> channel) :
        device_stub(api::v1::management::DeviceService::NewStub(channel)),
        oauth_stub(api::oauth::OauthService::NewStub(channel)) { }

    /// @brief Create a new device enrollment.
    ///
    /// @param name Name of the enrolling device
    /// @param credential Credential string to authenticate that this device
    /// is allowed to enroll
    /// @param clientID ClientID to use for OAuth token generation
    /// @param clientSecret Client Secret to use for OAuth token generation
    /// @returns A future to be fulfilled with either the enrolled device, or
    /// the network error that occurred
    ///
    /// @details
    /// The credential string authenticates that this device is allowed to
    /// enroll. Depending on the server configuration the credential string
    /// may be one of multiple values:
    /// -   An empty string if no authentication is configured on the server
    /// -   A shared secret (password)
    /// -   A signed JWT
    ///
    /// `TokenManager` may be used for securely generating a clientID and
    /// clientSecret for this call
    ///
    /// This call will fail with `NetworkError.notInitialized` if
    /// `Config.deviceID` or `Config.tenantID` has not been set
    ///
    api::v1::management::DeviceResponse enrollDevice(
        const Config& config,
        const std::string& name,
        const std::string& credential,
        const std::string& clientID,
        const std::string& clientSecret
    ) {
        // std::cout << "Enrolling device: " << name << std::endl;
        // Ensure that the configuration specifies a cloud host and has a valid
        // tenant ID and device ID before proceeding with the RPC.
        if (!config.hasCloudHost() || !config.isValid())
            throw NetworkError(NetworkError::Code::NotInitialized);
        // Create a context for the client.
        grpc::ClientContext context;
        // Create the request from the parameters.
        api::v1::management::EnrollDeviceRequest request;
        request.set_name(name);
        request.set_deviceid(config.deviceID);
        request.set_tenantid(config.tenantID);
        api::common::GenericClient clientRequest;
        clientRequest.set_clientid(clientID);
        clientRequest.set_secret(clientSecret);
        request.set_allocated_client(&clientRequest);
        request.set_credential(credential);
        // Execute the RPC synchronously and get the response
        api::v1::management::DeviceResponse response;
        grpc::Status status = device_stub->EnrollDevice(&context, request, &response);
        if (!status.ok()) {  // an error occurred in the RPC
            // std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            throw "EnrollDevice failure";
        }
        return response;
    }

    api::v1::management::DeviceResponse getWhoAmI(const Config& config) {
        // NSLog("Getting WhoAmI")
        if (config.getCloudHost() == nullptr) {
            throw "NetworkError.notInitialized";
        }
        grpc::ClientContext context;
        api::v1::management::DeviceGetWhoAmIRequest request;
        api::v1::management::DeviceResponse response;
        grpc::Status status = device_stub->GetWhoAmI(&context, request, &response);
        if (!status.ok()) {  // an error occurred in the RPC
            // std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            throw "WhoAmI failure";
        }
        return response;
    }

    /// @brief Request a new OAuth token from the server.
    ///
    /// @param clientID: Client id to use in token request
    /// @param secret: Client secret to use in token request
    /// @returns Future to be fulfilled with the new access token, or the
    ///     network error that occurred
    ///
    api::common::TokenResponse getToken(
        const Config& config,
        const std::string& clientID,
        const std::string& secret
    ) {
        // NSLog("Requesting OAuth Token with clientID %@", clientID)
        if (config.getCloudHost() == nullptr) {
            throw "NetworkError.notInitialized";
        }
        // Create a context for the client.
        grpc::ClientContext context;
        // Create the token request from the function parameters.
        api::oauth::TokenRequest request;
        request.set_clientid(clientID);
        request.set_secret(secret);
        // Execute the remote procedure call
        api::common::TokenResponse response;
        grpc::Status status = oauth_stub->GetToken(&context, request, &response);
        if (!status.ok()) {  // an error occurred in the RPC
            // std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            throw "GetToken failure";
        }
        return response;
    }

    /// TODO
    ///
    /// @returns TODO
    ///
    // std::unique_ptr<api::oauth::OauthService::Stub> getOAuthClient() {
    //     return oauth_stub;
    // }

    /// TODO
    ///
    /// @returns TODO
    ///
    // std::unique_ptr<api::v1::management::DeviceService::Stub> getEnrollmentClient() {
    //     return device_stub;
    // }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORY_CLOUD_SERVICES_OAUTH_SERVICE_HPP_
