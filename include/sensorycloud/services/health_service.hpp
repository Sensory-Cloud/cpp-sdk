// The health service for the Sensory Cloud SDK.
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

#ifndef SENSORY_CLOUD_SERVICES_HEALTH_SERVICE_HPP_
#define SENSORY_CLOUD_SERVICES_HEALTH_SERVICE_HPP_

#include <memory>
#include <string>
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include "sensorycloud/generated/health/health.pb.h"
#include "sensorycloud/generated/health/health.grpc.pb.h"
#include "sensorycloud/config.hpp"
#include "sensorycloud/services/network_error.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud Services.
namespace service {

/// @brief A service for querying the health of the remote server.
class HealthService {
 private:
    /// the global configuration for the remote connection
    const ::sensory::Config& config;
    /// The gRPC stub for the health service
    std::unique_ptr<::sensory::api::health::HealthService::Stub> stub;

    /// @brief Create a copy of this object.
    ///
    /// @param other the other instance to copy data from
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object
    HealthService(const HealthService& other);

 public:
    /// @brief Initialize a new health service.
    ///
    /// @param config_ the global configuration for the remote connection
    ///
    explicit HealthService(const ::sensory::Config& config_) : config(config_),
        stub(::sensory::api::health::HealthService::NewStub(config.getChannel())) { }

    /// @brief Get the health status of the remote server.
    ///
    /// @param response the response object to store the result of the call in
    /// @returns a gRPC status object indicating whether the call succeeded
    ///
    inline ::grpc::Status getHealth(
      ::sensory::api::common::ServerHealthResponse* response
   ) const {
        // Create a client context to query the health service. This request
        // does not require the "authorization" : "Bearer <token>" for auth.
        ::grpc::ClientContext context;
        // Create the parameter-less request to execute on the remote server
        return stub->GetHealth(&context, {}, response);
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORY_CLOUD_SERVICES_HEALTH_SERVICE_HPP_
