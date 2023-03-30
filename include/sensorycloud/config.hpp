// Configuration structures for the SensoryCloud C++ SDK.
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

#ifndef SENSORYCLOUD_CONFIG_HPP_
#define SENSORYCLOUD_CONFIG_HPP_

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <memory>
#include <algorithm>
#include <cstdint>
#include <chrono>
#include <limits>
#include <string>
#include <sstream>
#include "sensorycloud/error/config_error.hpp"

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Configuration for a cloud endpoint.
class Config {
 private:
    /// The fully qualified domain name of the server in host:port format.
    const std::string fqdn;
    /// Tenant ID to use during device enrollment
    const std::string tenant_id;
    /// Unique device identifier that model enrollments are associated to
    const std::string device_id;
    /// whether the connection to the remote host should be secured by TLS/SSL
    const bool is_secure;
    /// the number of milliseconds to wait on a unary gRPC call before timeout.
    uint32_t timeout = 10 * 1000;
    /// the gRPC channel associated with this config.
    std::shared_ptr<::grpc::Channel> channel = nullptr;

 public:
    /// @brief Initialize a new configuration object.
    ///
    /// @param host_ The fully qualified domain name (FQDN) of the server in
    ///     `host:port` format, e.g., `localhost:50051`.
    /// @param tenant_id_ The UUID for your tenant.
    /// @param device_id_ The UUID of the device running the SDK.
    /// @param is_secure_ `true` to use SSL/TLS for message encryption, `false`
    /// to use an insecure connection.
    ///
    /// @exception ConfigError If the FQDN is improperly formatted.
    /// @exception ConfigError If the tenant ID is improperly formatted.
    /// @exception ConfigError If the device ID is improperly formatted.
    ///
    Config(
        const std::string& fqdn_,
        const std::string& tenant_id_,
        const std::string& device_id_,
        const bool& is_secure_ = true
    ) :
        fqdn(fqdn_),
        tenant_id(tenant_id_),
        device_id(device_id_),
        is_secure(is_secure_) {
        if (fqdn.empty())  // The FQDN is not valid
            throw ::sensory::error::ConfigError(::sensory::error::ConfigError::Code::InvalidFQDN);
        if (tenant_id.empty())  // the tenant ID is not valid
            throw ::sensory::error::ConfigError(::sensory::error::ConfigError::Code::InvalidTenantID);
        if (device_id.empty())  // the device ID is not valid
            throw ::sensory::error::ConfigError(::sensory::error::ConfigError::Code::InvalidDeviceID);
        // Create the credentials for the channel based on the security setting.
        // Use TLS (SSL) if `is_secure` is true, otherwise default to insecure
        // channel credentials.
        channel = ::grpc::CreateChannel(fqdn, is_secure ?
            ::grpc::SslCredentials(::grpc::SslCredentialsOptions()) :
            ::grpc::InsecureChannelCredentials()
        );
    }

    /// @brief Initialize a new configuration object.
    ///
    /// @param host_ The DNS name or IP address of the server.
    /// @param port_ The port number for the service.
    /// @param tenant_id_ The UUID for your tenant.
    /// @param device_id_ The UUID of the device running the SDK.
    /// @param is_secure_ `true` to use SSL/TLS for message encryption, `false`
    /// to use an insecure connection.
    ///
    /// @exception ConfigError If the host is improperly formatted.
    /// @exception ConfigError If the tenant ID is improperly formatted.
    /// @exception ConfigError If the device ID is improperly formatted.
    ///
    Config(
        const std::string& host_,
        const uint16_t& port_,
        const std::string& tenant_id_,
        const std::string& device_id_,
        const bool& is_secure_ = true
    ) : Config(host_ + ":" + std::to_string(port_), tenant_id_, device_id_, is_secure_) { }

    /// @brief Return the gRPC channel.
    ///
    /// @returns The gRPC channel to use for connecting services.
    ///
    inline std::shared_ptr<::grpc::Channel> get_channel() const { return channel; }

    /// @brief Return the fully qualified domain name of the server.
    ///
    /// @returns The fully qualified domain name in `host:port` format.
    ///
    inline const std::string& get_fully_qualified_domain_name() const { return fqdn; }

    /// @brief Return the UUID of the tenant.
    ///
    /// @returns The UUID for identifying a tenant.
    ///
    inline const std::string& get_tenant_id() const { return tenant_id; }

    /// @brief Return the UUID of the device.
    ///
    /// @returns The UUID for identifying a registered device.
    ///
    inline const std::string& get_device_id() const { return device_id; }

    /// @brief Return the security policy of the remote host.
    ///
    /// @returns `true` if the connection is secured with TLS/SSL, `false`
    /// otherwise.
    ///
    inline const bool& get_is_secure() const { return is_secure; }

    /// @brief Set the timeout for unary gRPC calls to a new value.
    ///
    /// @param timeout The timeout for unary gRPC calls in milliseconds.
    ///
    inline void set_timeout(const uint32_t& timeout) { this->timeout = timeout; }

    /// @brief Return the timeout for unary gRPC calls.
    ///
    /// @returns The timeout for unary gRPC calls in milliseconds.
    ///
    inline const uint32_t& get_timeout() const { return timeout; }

    /// @brief Create a new deadline from the current time and RPC timeout.
    ///
    /// @returns A new deadline for an RPC call.
    ///
    inline std::chrono::system_clock::time_point get_deadline() const {
        return std::chrono::system_clock::now() + std::chrono::milliseconds(timeout);
    }
};

}  // namespace sensory

#endif  // SENSORYCLOUD_CONFIG_HPP_
