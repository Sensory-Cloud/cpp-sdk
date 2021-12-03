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

#include <memory>
#include <string>
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
#include "sensorycloud/call_data.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud services.
namespace service {

/// @brief A service for handling device and user authentication.
class OAuthService {
 private:
    /// the global configuration for the remote connection
    const ::sensory::Config& config;
    /// The gRPC stub for the device service
    std::unique_ptr<::sensory::api::v1::management::DeviceService::Stub> deviceStub;
    /// The gRPC stub for the OAuth service
    std::unique_ptr<::sensory::api::oauth::OauthService::Stub> oauthStub;

    /// @brief Create a copy of this object.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object.
    ///
    OAuthService(const OAuthService& other);

    /// @brief Assign to this object using the `=` operator.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This assignment operator is private to prevent copying of this object.
    ///
    void operator=(const OAuthService& other) = delete;

 public:
    /// @brief Initialize a new OAuth service.
    ///
    /// @param config_ The global configuration for the remote connection.
    ///
    explicit OAuthService(const ::sensory::Config& config_) : config(config_),
        deviceStub(::sensory::api::v1::management::DeviceService::NewStub(config.getChannel())),
        oauthStub(::sensory::api::oauth::OauthService::NewStub(config.getChannel())) { }

    // ----- Register Device ---------------------------------------------------

    /// @brief Register a new device with the Sensory Cloud service.
    ///
    /// @param response The device response to store the result of the RPC into.
    /// @param name The friendly name of the device that is being registered.
    /// @param credential A credential string to authenticate that this device
    /// is allowed to register.
    /// @param clientID The client ID to use for OAuth token generation.
    /// @param clientSecret The client secret to use for OAuth token generation.
    /// @returns The status of the synchronous gRPC call.
    ///
    /// @details
    /// The credential string authenticates that this device is allowed to
    /// register. Depending on the server configuration the credential string
    /// may be one of multiple values:
    /// -   An empty string if no authentication is configured on the server,
    /// -   a shared secret (password), or
    /// -   a signed JWT.
    ///
    ::grpc::Status registerDevice(
        ::sensory::api::v1::management::DeviceResponse* response,
        const std::string& name,
        const std::string& credential,
        const std::string& clientID,
        const std::string& clientSecret
    ) {
        // Create a context for the client. Most requests require the existence
        // of a authorization Bearer token, but this request does not.
        ::grpc::ClientContext context;
        // Create the request from the parameters.
        ::sensory::api::v1::management::EnrollDeviceRequest request;
        request.set_deviceid(config.getDeviceID());
        request.set_tenantid(config.getTenantID());
        request.set_name(name);
        request.set_credential(credential);
        auto clientRequest = new ::sensory::api::common::GenericClient;
        clientRequest->set_clientid(clientID);
        clientRequest->set_secret(clientSecret);
        request.set_allocated_client(clientRequest);
        // Execute the RPC synchronously and return the status
        return deviceStub->EnrollDevice(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous `EnrollDevice`
    /// calls.
    typedef ::sensory::CallData<
        OAuthService,
        ::sensory::api::v1::management::EnrollDeviceRequest,
        ::sensory::api::v1::management::DeviceResponse
    > RegisterDeviceCallData;

    /// @brief Register a new device with the Sensory Cloud service.
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `RegisterDeviceCallData*`.
    /// @param name The friendly name of the device that is being registered.
    /// @param credential A credential string to authenticate that this device
    /// is allowed to register.
    /// @param clientID The client ID to use for OAuth token generation.
    /// @param clientSecret The client secret to use for OAuth token generation.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the asynchronous call spawned by this call.
    ///
    /// @details
    /// The credential string authenticates that this device is allowed to
    /// register. Depending on the server configuration the credential string
    /// may be one of multiple values:
    /// -   An empty string if no authentication is configured on the server,
    /// -   a shared secret (password), or
    /// -   a signed JWT.
    ///
    template<typename Callback>
    inline std::shared_ptr<RegisterDeviceCallData> asyncRegisterDevice(
        const std::string& name,
        const std::string& credential,
        const std::string& clientID,
        const std::string& clientSecret,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<RegisterDeviceCallData>
            call(new RegisterDeviceCallData);
        call->request.set_deviceid(config.getDeviceID());
        call->request.set_tenantid(config.getTenantID());
        call->request.set_name(name);
        call->request.set_credential(credential);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        deviceStub->async()->EnrollDevice(
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

    // ----- Get Token ---------------------------------------------------------

    /// @brief Request a new OAuth token from the server.
    ///
    /// @param response The token response to store the result of the RPC into.
    /// @param clientID The client ID to use for OAuth token generation.
    /// @param clientSecret The client secret to use for OAuth token generation.
    /// @returns The status of the synchronous gRPC call.
    ///
    ::grpc::Status getToken(
        ::sensory::api::common::TokenResponse* response,
        const std::string& clientID,
        const std::string& clientSecret
    ) {
        // Create a context for the client. Most requests require the existence
        // of a authorization Bearer token, but this request does not.
        ::grpc::ClientContext context;
        // Create the token request from the function parameters.
        ::sensory::api::oauth::TokenRequest request;
        request.set_clientid(clientID);
        request.set_secret(clientSecret);
        // Execute the remote procedure call synchronously
        return oauthStub->GetToken(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous `GetToken` calls.
    typedef ::sensory::CallData<
        OAuthService,
        ::sensory::api::oauth::TokenRequest,
        ::sensory::api::common::TokenResponse
    > GetTokenCallData;

    /// @brief Register a new device with the Sensory Cloud service.
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `GetTokenCallData*`.
    /// @param clientID The client ID to use for OAuth token generation.
    /// @param clientSecret The client secret to use for OAuth token generation.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the asynchronous call spawned by this call.
    ///
    template<typename Callback>
    inline std::shared_ptr<GetTokenCallData> asyncGetToken(
        const std::string& clientID,
        const std::string& clientSecret,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<GetTokenCallData>
            call(new GetTokenCallData);
        call->request.set_clientid(clientID);
        call->request.set_secret(clientSecret);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        oauthStub->async()->GetToken(
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
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORY_CLOUD_SERVICES_OAUTH_SERVICE_HPP_
