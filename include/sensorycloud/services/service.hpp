// Abstract base service for the Sensory Cloud SDK.
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

#ifndef SENSORY_CLOUD_SERVICES_SERVICE_HPP_
#define SENSORY_CLOUD_SERVICES_SERVICE_HPP_

#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include "sensorycloud/config.hpp"
#include "sensorycloud/services/network_error.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud Services.
namespace service {

/// @brief The base service for interacting with the Sensory Cloud API.
class Service {
 private:
    /// The configuration for cloud host access
    std::shared_ptr<Config> config = nullptr;

 public:
    /// @brief Initialize a new service with given cloud host configuration.
    ///
    /// @param config_ the configuration object with information about the host
    ///
    explicit Service(std::shared_ptr<Config> config_) : config(config_) { }

    /// @brief Create a new gRPC channel.
    ///
    /// @returns a new gRPC channel to connect a service to
    /// @raises NetworkError.notInitialized if a cloud host has not been set,
    /// or any error encountered while creating the grpc channel
    ///
    std::shared_ptr<grpc::Channel> getGRPCChannel() {
        if (config == nullptr || !config->hasCloudHost())
            throw NetworkError(NetworkError::Code::NotInitialized);
        return config->getCloudHost()->getGRPCChannel();
    }

    /// @brief Create a new gRPC client context for gRPC calls.
    ///
    /// @param isUnary whether the connection is unary
    /// @returns a new client context for gRPC calls
    ///
    std::unique_ptr<grpc::ClientContext> getClientContext(
        const bool& isUnary=false
    ) {
        // Create a new client context.
        std::unique_ptr<grpc::ClientContext> context(new grpc::ClientContext);
        // let token = try credentialProvider.getAccessToken()
        // let headers: HPACKHeaders = ["authorization": "Bearer \(token)"]
        if (isUnary)  // Set the deadline for the RPC call
            context->set_deadline(config->getDeadline());

        return context;
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORY_CLOUD_SERVICES_SERVICE_HPP_
