// The assistant service for the SensoryCloud SDK.
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

#ifndef SENSORYCLOUD_SERVICES_ASSISTANT_SERVICE_HPP_
#define SENSORYCLOUD_SERVICES_ASSISTANT_SERVICE_HPP_

#include <memory>
#include <string>
#include <utility>
#include "sensorycloud/generated/v1/assistant/assistant.pb.h"
#include "sensorycloud/generated/v1/assistant/assistant.grpc.pb.h"
#include "sensorycloud/config.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/calldata.hpp"

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief SensoryCloud services.
namespace service {

/// @brief A service for assistance.
/// @tparam CredentialStore A key-value store for storing and fetching
/// credentials and tokens.
template<typename CredentialStore>
class AssistantService {
 private:
    /// the global configuration for the remote connection
    const ::sensory::Config& config;
    /// the token manager for securing gRPC requests to the server
    ::sensory::token_manager::TokenManager<CredentialStore>& token_manager;
    /// The gRPC stub for the assistant service
    std::unique_ptr<::sensory::api::v1::assistant::AssistantService::StubInterface> stub;

    /// @brief Create a copy of this object.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object.
    ///
    AssistantService(const AssistantService& other) = delete;

    /// @brief Assign to this object using the `=` operator.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This assignment operator is private to prevent copying of this object.
    ///
    void operator=(const AssistantService& other) = delete;

 public:
    /// @brief Initialize a new assistant service.
    ///
    /// @param config_ The global configuration for the remote connection.
    /// @param token_manager_ The token manager for requesting Bearer tokens.
    ///
    explicit AssistantService(
        const ::sensory::Config& config_,
        ::sensory::token_manager::TokenManager<CredentialStore>& token_manager_
    ) : config(config_),
        token_manager(token_manager_),
        stub(::sensory::api::v1::assistant::AssistantService::NewStub(config.get_channel())) { }

    /// @brief Initialize a new assistant service.
    ///
    /// @param config_ The global configuration for the remote connection.
    /// @param token_manager_ The token manager for requesting Bearer tokens.
    /// @param stub_ The assistant service stub to initialize the service with.
    ///
    explicit AssistantService(
        const ::sensory::Config& config_,
        ::sensory::token_manager::TokenManager<CredentialStore>& token_manager_,
        ::sensory::api::v1::assistant::AssistantService::StubInterface* stub_
    ) : config(config_), token_manager(token_manager_), stub(stub_) { }

    /// @brief Return the cloud configuration associated with this service.
    ///
    /// @returns the configuration used by this service.
    ///
    inline const ::sensory::Config& get_config() const { return config; }

    /// @brief Get the health status of the remote server.
    ///
    /// @param response The response object to store the result of the RPC into.
    /// @returns A gRPC status object indicating whether the call succeeded.
    ///
    inline ::grpc::Status text_chat(
        const ::sensory::api::v1::assistant::TextChatRequest& request,
        ::sensory::api::v1::assistant::TextChatResponse* response
    ) const {
        ::grpc::ClientContext context;
        token_manager.setup_unary_client_context(context);
        return stub->TextChat(&context, request, response);
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORYCLOUD_SERVICES_ASSISTANT_SERVICE_HPP_
