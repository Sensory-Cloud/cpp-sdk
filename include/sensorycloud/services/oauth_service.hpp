// The OAuth service for the SensoryCloud SDK.
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

#ifndef SENSORYCLOUD_SERVICES_OAUTH_SERVICE_HPP_
#define SENSORYCLOUD_SERVICES_OAUTH_SERVICE_HPP_

#include <memory>
#include <string>
#include <utility>
#include "sensorycloud/generated/oauth/oauth.pb.h"
#include "sensorycloud/generated/oauth/oauth.grpc.pb.h"
#include "sensorycloud/generated/v1/management/device.pb.h"
#include "sensorycloud/generated/v1/management/device.grpc.pb.h"
#include "sensorycloud/config.hpp"
#include "sensorycloud/calldata.hpp"

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief SensoryCloud services.
namespace service {

/// @brief A service for handling device and user authentication.
class OAuthService {
 private:
    /// the global configuration for the remote connection
    const ::sensory::Config& config;
    /// The gRPC stub for the device service
    std::unique_ptr<::sensory::api::v1::management::DeviceService::StubInterface> device_stub;
    /// The gRPC stub for the OAuth service
    std::unique_ptr<::sensory::api::oauth::OauthService::StubInterface> oauth_stub;

    /// @brief Create a copy of this object.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object.
    ///
    OAuthService(const OAuthService& other) = delete;

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
        device_stub(::sensory::api::v1::management::DeviceService::NewStub(config.get_channel())),
        oauth_stub(::sensory::api::oauth::OauthService::NewStub(config.get_channel())) { }

    /// @brief Initialize a new OAuth service.
    ///
    /// @param config_ The global configuration for the remote connection.
    /// @param device_stub_ The device service stub to initialize the service with.
    /// @param oauth_stub_ The OAuth service stub to initialize the service with.
    ///
    explicit OAuthService(
        const ::sensory::Config& config_,
        ::sensory::api::v1::management::DeviceService::StubInterface* device_stub_,
        ::sensory::api::oauth::OauthService::StubInterface* oauth_stub_
    ) : config(config_), device_stub(device_stub_), oauth_stub(oauth_stub_) { }

    /// @brief Return the cloud configuration associated with this service.
    ///
    /// @returns the configuration used by this service.
    ///
    inline const ::sensory::Config& get_config() const { return config; }

    // ----- Register Device ---------------------------------------------------

    /// @brief Register a new device with the SensoryCloud service.
    ///
    /// @param response The device response to store the result of the RPC into.
    /// @param name The friendly name of the device that is being registered.
    /// @param credential A credential string to authenticate that this device
    /// is allowed to register.
    /// @param client_id The client ID to use for OAuth token generation.
    /// @param client_secret The client secret to use for OAuth token generation.
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
    ::grpc::Status register_device(
        ::sensory::api::v1::management::DeviceResponse* response,
        const std::string& name,
        const std::string& credential,
        const std::string& client_id,
        const std::string& client_secret
    ) {
        // Create a context for the client. Most requests require the existence
        // of a authorization Bearer token, but this request does not.
        ::grpc::ClientContext context;
        // Create the request from the parameters.
        ::sensory::api::v1::management::EnrollDeviceRequest request;
        request.set_deviceid(config.get_device_id());
        request.set_tenantid(config.get_tenant_id());
        request.set_name(name);
        request.set_credential(credential);
        auto client_request = new ::sensory::api::common::GenericClient;
        client_request->set_clientid(client_id);
        client_request->set_secret(client_secret);
        request.set_allocated_client(client_request);
        // Execute the RPC synchronously and return the status
        return device_stub->EnrollDevice(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous `GetToken` calls
    /// based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncResponseReaderCall<
        OAuthService,
        ::sensory::api::v1::management::EnrollDeviceRequest,
        ::sensory::api::v1::management::DeviceResponse
    > RegisterDeviceAsyncCall;

    /// @brief Register a new device with the SensoryCloud service.
    ///
    /// @param queue The completion queue handling the event-loop processing.
    /// @param name The friendly name of the device that is being registered.
    /// @param credential A credential string to authenticate that this device
    /// is allowed to register.
    /// @param client_id The client ID to use for OAuth token generation.
    /// @param client_secret The client secret to use for OAuth token generation.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    /// @details
    /// The credential string authenticates that this device is allowed to
    /// register. Depending on the server configuration the credential string
    /// may be one of multiple values:
    /// -   An empty string if no authentication is configured on the server,
    /// -   a shared secret (password), or
    /// -   a signed JWT.
    ///
    inline RegisterDeviceAsyncCall* register_device(
        ::grpc::CompletionQueue* queue,
        const std::string& name,
        const std::string& credential,
        const std::string& client_id,
        const std::string& client_secret
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new RegisterDeviceAsyncCall);
        // Start the asynchronous RPC with the call's context and queue.
        call->request.set_deviceid(config.get_device_id());
        call->request.set_tenantid(config.get_tenant_id());
        call->request.set_name(name);
        call->request.set_credential(credential);
        auto client_request = new ::sensory::api::common::GenericClient;
        client_request->set_clientid(client_id);
        client_request->set_secret(client_secret);
        call->request.set_allocated_client(client_request);
        call->rpc = device_stub->AsyncEnrollDevice(&call->context, call->request, queue);
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

    /// @brief A type for encapsulating data for asynchronous `EnrollDevice`
    /// calls.
    typedef ::sensory::calldata::CallbackData<
        OAuthService,
        ::sensory::api::v1::management::EnrollDeviceRequest,
        ::sensory::api::v1::management::DeviceResponse
    > RegisterDeviceCallbackData;

    /// @brief Register a new device with the SensoryCloud service.
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `RegisterDeviceCallbackData*`.
    /// @param name The friendly name of the device that is being registered.
    /// @param credential A credential string to authenticate that this device
    /// is allowed to register.
    /// @param client_id The client ID to use for OAuth token generation.
    /// @param client_secret The client secret to use for OAuth token generation.
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
    inline std::shared_ptr<RegisterDeviceCallbackData> register_device(
        const std::string& name,
        const std::string& credential,
        const std::string& client_id,
        const std::string& client_secret,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<RegisterDeviceCallbackData>
            call(new RegisterDeviceCallbackData);
        call->request.set_deviceid(config.get_device_id());
        call->request.set_tenantid(config.get_tenant_id());
        call->request.set_name(name);
        call->request.set_credential(credential);
        auto client_request = new ::sensory::api::common::GenericClient;
        client_request->set_clientid(client_id);
        client_request->set_secret(client_secret);
        call->request.set_allocated_client(client_request);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        device_stub->async()->EnrollDevice(
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

    // ----- Renew Credential --------------------------------------------------

    /// @brief Renew a device's credential with the SensoryCloud service.
    ///
    /// @param response The device response to store the result of the RPC into.
    /// @param credential A credential string to authenticate that this device
    /// is allowed to renew its credentials.
    /// @param client_id The client ID associated with the device.
    /// @returns The status of the synchronous gRPC call.
    ///
    /// @details
    /// The credential string authenticates that this device is allowed to
    /// renew. Depending on the server configuration the credential string
    /// may be one of multiple values:
    /// -   An empty string if no authentication is configured on the server,
    /// -   a shared secret (password), or
    /// -   a signed JWT.
    ///
    ::grpc::Status renew_device_credential(
        ::sensory::api::v1::management::DeviceResponse* response,
        const std::string& credential,
        const std::string& client_id
    ) {
        // Create a context for the client. Most requests require the existence
        // of a authorization Bearer token, but this request does not.
        ::grpc::ClientContext context;
        // Create the request from the parameters.
        ::sensory::api::v1::management::RenewDeviceCredentialRequest request;
        request.set_deviceid(config.get_device_id());
        request.set_tenantid(config.get_tenant_id());
        request.set_credential(credential);
        request.set_clientid(client_id);
        // Execute the RPC synchronously and return the status
        return device_stub->RenewDeviceCredential(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous `GetToken` calls
    /// based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncResponseReaderCall<
        OAuthService,
        ::sensory::api::v1::management::RenewDeviceCredentialRequest,
        ::sensory::api::v1::management::DeviceResponse
    > RenewCredentialAsyncCall;

    /// @brief Renew a device's credential with the SensoryCloud service.
    ///
    /// @param queue The completion queue handling the event-loop processing.
    /// @param credential A credential string to authenticate that this device
    /// is allowed to renew its credentials.
    /// @param client_id The client ID associated with the device.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    /// @details
    /// The credential string authenticates that this device is allowed to
    /// renew. Depending on the server configuration the credential string
    /// may be one of multiple values:
    /// -   An empty string if no authentication is configured on the server,
    /// -   a shared secret (password), or
    /// -   a signed JWT.
    ///
    inline RenewCredentialAsyncCall* renew_device_credential(
        ::grpc::CompletionQueue* queue,
        const std::string& credential,
        const std::string& client_id
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new RenewCredentialAsyncCall);
        // Start the asynchronous RPC with the call's context and queue.
        call->request.set_deviceid(config.get_device_id());
        call->request.set_tenantid(config.get_tenant_id());
        call->request.set_credential(credential);
        call->request.set_clientid(client_id);
        call->rpc = device_stub->AsyncRenewDeviceCredential(&call->context, call->request, queue);
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

    /// @brief A type for encapsulating data for asynchronous `EnrollDevice`
    /// calls.
    typedef ::sensory::calldata::CallbackData<
        OAuthService,
        ::sensory::api::v1::management::RenewDeviceCredentialRequest,
        ::sensory::api::v1::management::DeviceResponse
    > RenewCredentialCallbackData;

    /// @brief Renew a device's credential with the SensoryCloud service.
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `RenewCredentialCallbackData*`.
    /// @param credential A credential string to authenticate that this device
    /// is allowed to renew its credentials.
    /// @param client_id The client ID associated with the device.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the asynchronous call spawned by this call.
    ///
    /// @details
    /// The credential string authenticates that this device is allowed to
    /// renew. Depending on the server configuration the credential string
    /// may be one of multiple values:
    /// -   An empty string if no authentication is configured on the server,
    /// -   a shared secret (password), or
    /// -   a signed JWT.
    ///
    template<typename Callback>
    inline std::shared_ptr<RenewCredentialCallbackData> renew_device_credential(
        const std::string& credential,
        const std::string& client_id,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<RenewCredentialCallbackData>
            call(new RenewCredentialCallbackData);
        call->request.set_deviceid(config.get_device_id());
        call->request.set_tenantid(config.get_tenant_id());
        call->request.set_credential(credential);
        call->request.set_clientid(client_id);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        device_stub->async()->RenewDeviceCredential(
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

    // ----- Get Token ---------------------------------------------------------

    /// @brief Request a new OAuth token from the server.
    ///
    /// @param response The token response to store the result of the RPC into.
    /// @param client_id The client ID to use for OAuth token generation.
    /// @param client_secret The client secret to use for OAuth token generation.
    /// @returns The status of the synchronous gRPC call.
    ///
    inline ::grpc::Status get_token(
        ::sensory::api::common::TokenResponse* response,
        const std::string& client_id,
        const std::string& client_secret
    ) {
        // Create a context for the client. Most requests require the existence
        // of a authorization Bearer token, but this request does not.
        ::grpc::ClientContext context;
        // Create the token request from the function parameters.
        ::sensory::api::oauth::TokenRequest request;
        request.set_clientid(client_id);
        request.set_secret(client_secret);
        // Execute the remote procedure call synchronously
        return oauth_stub->GetToken(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous `GetToken` calls
    /// based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncResponseReaderCall<
        OAuthService,
        ::sensory::api::oauth::TokenRequest,
        ::sensory::api::common::TokenResponse
    > GetTokenAsyncCall;

    /// @brief Request a new OAuth token from the server.
    ///
    /// @param queue The completion queue handling the event-loop processing.
    /// @param response The token response to store the result of the RPC into.
    /// @param client_id The client ID to use for OAuth token generation.
    /// @param client_secret The client secret to use for OAuth token generation.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    inline GetTokenAsyncCall* get_token(::grpc::CompletionQueue* queue,
        const std::string& client_id,
        const std::string& client_secret
    ) {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new GetTokenAsyncCall);
        // Start the asynchronous RPC with the call's context and queue.
        call->request.set_clientid(client_id);
        call->request.set_secret(client_secret);
        call->rpc = oauth_stub->AsyncGetToken(&call->context, call->request, queue);
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

    /// @brief A type for encapsulating data for asynchronous `GetToken` calls.
    typedef ::sensory::calldata::CallbackData<
        OAuthService,
        ::sensory::api::oauth::TokenRequest,
        ::sensory::api::common::TokenResponse
    > GetTokenCallbackData;

    /// @brief Register a new device with the SensoryCloud service.
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `GetTokenCallbackData*`.
    /// @param client_id The client ID to use for OAuth token generation.
    /// @param client_secret The client secret to use for OAuth token generation.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the asynchronous call spawned by this call.
    ///
    template<typename Callback>
    inline std::shared_ptr<GetTokenCallbackData> get_token(
        const std::string& client_id,
        const std::string& client_secret,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<GetTokenCallbackData>
            call(new GetTokenCallbackData);
        call->request.set_clientid(client_id);
        call->request.set_secret(client_secret);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        oauth_stub->async()->GetToken(
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

    // ----- Sign Token --------------------------------------------------------

    // TODO: Implement sign token endpoint.

    // ----- Get Public Key ----------------------------------------------------

    // TODO: Implement get public key endpoint.

    // ----- Who Am I (Oauth) --------------------------------------------------

    // TODO: What is the best way to support this functionality? Currently the
    // token manager relies on an instance of OAuth service on construction, and
    // thus imposes a circular dependency for GetWhoAmI that requires the token
    // manager to look up credentials. The token manager itself could be a good
    // place for these functions to live potentially since they are related to
    // information that should be in the token manager locally anyway.

    // /// @brief Request to obtain information based on the authorization token.
    // ///
    // /// @param response The device response to store the result of the RPC into.
    // /// @returns The status of the synchronous gRPC call.
    // ///
    // ::grpc::Status get_who_am_i(::sensory::api::oauth::WhoAmIResponse* response) {
    //     ::grpc::ClientContext context;
    //     config.setup_unary_client_context(context, token_manager);
    //     return oauth_stub->GetWhoAmI(&context, {}, response);
    // }

    // /// @brief A type for encapsulating data for asynchronous `GetToken` calls
    // /// based on CompletionQueue event loops.
    // typedef ::sensory::calldata::AsyncResponseReaderCall<
    //     OAuthService,
    //     ::sensory::api::oauth::WhoAmIRequest,
    //     ::sensory::api::oauth::WhoAmIResponse
    // > WhoAmIAsyncCall;

    // /// @brief Request to obtain information based on the authorization token.
    // ///
    // /// @param queue The completion queue handling the event-loop processing.
    // /// @param callback The callback to execute when the response arrives.
    // /// @returns A pointer to the call data associated with this asynchronous
    // /// call. This pointer can be used to identify the call in the event-loop
    // /// as the `tag` of the event. Ownership of the pointer passes to the
    // /// caller and the caller should `delete` the pointer after it appears in
    // /// a completion queue loop.
    // ///
    // inline WhoAmIAsyncCall* get_who_am_i(::grpc::CompletionQueue* queue) const {
    //     // Create a call data object to store the client context, the response,
    //     // the status of the call, and the response reader. The ownership of
    //     // this object is passed to the caller.
    //     auto call(new WhoAmIAsyncCall);
    //     // Set the client context for a unary call.
    //     config.setup_unary_client_context(call->context, token_manager);
    //     // Start the asynchronous RPC with the call's context and queue.
    //     call->rpc = oauth_stub->AsyncGetWhoAmI(&call->context, call->request, queue);
    //     // Finish the RPC to tell it where the response and status buffers are
    //     // located within the call object. Use the address of the call as the
    //     // tag for identifying the call in the event-loop.
    //     call->rpc->Finish(&call->response, &call->status, static_cast<void*>(call));
    //     // Return the pointer to the call. This both transfers the ownership
    //     // of the instance to the caller, and provides the caller with an
    //     // identifier for detecting the result of this call in the completion
    //     // queue.
    //     return call;
    // }

    // /// @brief A type for encapsulating data for asynchronous `GetWhoAmI`
    // /// calls.
    // typedef ::sensory::calldata::CallbackData<
    //     OAuthService,
    //     ::sensory::api::oauth::WhoAmIRequest,
    //     ::sensory::api::oauth::WhoAmIResponse
    // > WhoAmICallbackData;

    // /// @brief Request to obtain information based on the authorization token.
    // ///
    // /// @tparam Callback The type of the callback function. The callback should
    // /// accept a single pointer of type `WhoAmICallbackData*`.
    // /// @param name The friendly name of the device that is being registered.
    // /// @param credential A credential string to authenticate that this device
    // /// is allowed to register.
    // /// @param client_id The client ID to use for OAuth token generation.
    // /// @param client_secret The client secret to use for OAuth token generation.
    // /// @param callback The callback to execute when the response arrives.
    // /// @returns A pointer to the asynchronous call spawned by this call.
    // ///
    // template<typename Callback>
    // inline std::shared_ptr<WhoAmICallbackData> get_who_am_i(const Callback& callback) const {
    //     // Create a call to encapsulate data that needs to exist throughout the
    //     // scope of the call. This call is initiated as a shared pointer in
    //     // order to reference count between the parent and child context. This
    //     // also allows the caller to safely use `await()` without the
    //     // possibility of a race condition.
    //     std::shared_ptr<WhoAmICallbackData> call(new WhoAmICallbackData);
    //     config.setup_unary_client_context(call->context, token_manager);
    //     // Start the asynchronous call with the data from the request and
    //     // forward the input callback into the reactor callback.
    //     oauth_stub->async()->GetWhoAmI(
    //         &call->context,
    //         &call->request,
    //         &call->response,
    //         [call, callback](::grpc::Status status) {
    //             // Copy the status to the call.
    //             call->status = std::move(status);
    //             // Call the callback function with a raw pointer because
    //             // ownership is not being transferred.
    //             callback(call.get());
    //             // Mark the call as done for any awaiting process.
    //             call->setIsDone();
    //         });
    //     return call;
    // }

    // ----- Who Am I (Device) -------------------------------------------------

    // TODO: What is the best way to support this functionality? Currently the
    // token manager relies on an instance of OAuth service on construction, and
    // thus imposes a circular dependency for GetWhoAmI that requires the token
    // manager to look up credentials. The token manager itself could be a good
    // place for these functions to live potentially since they are related to
    // information that should be in the token manager locally anyway.

    // /// @brief Request to obtain information based on the authorization token.
    // ///
    // /// @param response The device response to store the result of the RPC into.
    // /// @returns The status of the synchronous gRPC call.
    // ///
    // ::grpc::Status get_who_am_i(::sensory::api::v1::management::WhoAmIResponse* response) {
    //     ::grpc::ClientContext context;
    //     config.setup_unary_client_context(context, token_manager);
    //     return device_stub->GetWhoAmI(&context, {}, response);
    // }

    // /// @brief A type for encapsulating data for asynchronous `GetToken` calls
    // /// based on CompletionQueue event loops.
    // typedef ::sensory::calldata::AsyncResponseReaderCall<
    //     OAuthService,
    //     ::sensory::api::v1::management::WhoAmIRequest,
    //     ::sensory::api::v1::management::WhoAmIResponse
    // > WhoAmIAsyncCall;

    // /// @brief Request to obtain information based on the authorization token.
    // ///
    // /// @param queue The completion queue handling the event-loop processing.
    // /// @param callback The callback to execute when the response arrives.
    // /// @returns A pointer to the call data associated with this asynchronous
    // /// call. This pointer can be used to identify the call in the event-loop
    // /// as the `tag` of the event. Ownership of the pointer passes to the
    // /// caller and the caller should `delete` the pointer after it appears in
    // /// a completion queue loop.
    // ///
    // inline WhoAmIAsyncCall* get_who_am_i(::grpc::CompletionQueue* queue) const {
    //     // Create a call data object to store the client context, the response,
    //     // the status of the call, and the response reader. The ownership of
    //     // this object is passed to the caller.
    //     auto call(new WhoAmIAsyncCall);
    //     // Set the client context for a unary call.
    //     config.setup_unary_client_context(call->context, token_manager);
    //     // Start the asynchronous RPC with the call's context and queue.
    //     call->rpc = device_stub->AsyncGetWhoAmI(&call->context, call->request, queue);
    //     // Finish the RPC to tell it where the response and status buffers are
    //     // located within the call object. Use the address of the call as the
    //     // tag for identifying the call in the event-loop.
    //     call->rpc->Finish(&call->response, &call->status, static_cast<void*>(call));
    //     // Return the pointer to the call. This both transfers the ownership
    //     // of the instance to the caller, and provides the caller with an
    //     // identifier for detecting the result of this call in the completion
    //     // queue.
    //     return call;
    // }

    // /// @brief A type for encapsulating data for asynchronous `GetWhoAmI`
    // /// calls.
    // typedef ::sensory::calldata::CallbackData<
    //     OAuthService,
    //     ::sensory::api::v1::management::WhoAmIRequest,
    //     ::sensory::api::v1::management::WhoAmIResponse
    // > WhoAmICallbackData;

    // /// @brief Request to obtain information based on the authorization token.
    // ///
    // /// @tparam Callback The type of the callback function. The callback should
    // /// accept a single pointer of type `WhoAmICallbackData*`.
    // /// @param name The friendly name of the device that is being registered.
    // /// @param credential A credential string to authenticate that this device
    // /// is allowed to register.
    // /// @param client_id The client ID to use for OAuth token generation.
    // /// @param client_secret The client secret to use for OAuth token generation.
    // /// @param callback The callback to execute when the response arrives.
    // /// @returns A pointer to the asynchronous call spawned by this call.
    // ///
    // template<typename Callback>
    // inline std::shared_ptr<WhoAmICallbackData> get_who_am_i(const Callback& callback) const {
    //     // Create a call to encapsulate data that needs to exist throughout the
    //     // scope of the call. This call is initiated as a shared pointer in
    //     // order to reference count between the parent and child context. This
    //     // also allows the caller to safely use `await()` without the
    //     // possibility of a race condition.
    //     std::shared_ptr<WhoAmICallbackData> call(new WhoAmICallbackData);
    //     config.setup_unary_client_context(call->context, token_manager);
    //     // Start the asynchronous call with the data from the request and
    //     // forward the input callback into the reactor callback.
    //     device_stub->async()->GetWhoAmI(
    //         &call->context,
    //         &call->request,
    //         &call->response,
    //         [call, callback](::grpc::Status status) {
    //             // Copy the status to the call.
    //             call->status = std::move(status);
    //             // Call the callback function with a raw pointer because
    //             // ownership is not being transferred.
    //             callback(call.get());
    //             // Mark the call as done for any awaiting process.
    //             call->setIsDone();
    //         });
    //     return call;
    // }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORYCLOUD_SERVICES_OAUTH_SERVICE_HPP_
