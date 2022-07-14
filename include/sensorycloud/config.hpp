// Configuration structures for the SensoryCloud C++ SDK.
//
// Copyright (c) 2022 Sensory, Inc.
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

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief A config error type thrown when configuration parameters are invalid.
struct ConfigError : public std::runtime_error {
 public:
    /// @brief Reasons for configuration errors to occur.
    enum class Code {
        /// The fully qualified domain name is not valid.
        InvalidFQDN = 0,
        /// The host name is not valid.
        InvalidHost,
        /// The port number is not valid.
        InvalidPort,
        /// The tenant ID is not valid.
        InvalidTenantID,
        /// The device ID is not valid.
        InvalidDeviceID,
    };

    /// @brief Return a message for the given error code.
    ///
    /// @param code The code to get the error message for.
    /// @returns A text error message associated with the given error code.
    ///
    static inline const std::string get_message(const Code& code) {
        switch (code) {  // switch over the possible code type cases
        case Code::InvalidFQDN:
            return "The fully qualified domain name is not valid";
        case Code::InvalidHost:
            return "The host name is not valid";
        case Code::InvalidPort:
            return "The port number is not valid";
        case Code::InvalidTenantID:
            return "The tenant ID is not valid";
        case Code::InvalidDeviceID:
            return "The device ID is not valid";
        default:
            return "Unrecognized error code";
        }
    }

    /// @brief Initialize a new configuration error.
    ///
    /// @param code The reason for the configuration error.
    ///
    explicit ConfigError(const Code& code) :
        std::runtime_error(get_message(code)),
        err_code(code) { }

    /// @brief Initialize a new configuration error.
    ///
    /// @param code The reason for the configuration error.
    /// @param message The message to provide through the `what()` call.
    ///
    explicit ConfigError(const Code& code, const std::string& message) :
        std::runtime_error(message),
        err_code(code) { }

    /// Destroy an instance of a configuration error.
    ~ConfigError() throw() {}

    /// @brief Return the reason the exception occurred.
    ///
    /// @returns The reason for the configuration error.
    ///
    inline const Code& code() const throw() { return err_code; }

 private:
    /// the reason the configuration error occurred
    Code err_code;
};

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
        // Check that the FQDN is properly formatted in `host:port` format.
        const auto idx = fqdn.find(':');
        if (fqdn.empty() || idx == 0 || idx >= fqdn.length() - 1)
            throw ConfigError(ConfigError::Code::InvalidFQDN);
        // Parse the port as a 32-bit signed integer and ensure that the value
        // is a valid 16-bit unsigned integer.
        const auto port = std::stoi(fqdn.substr(idx + 1));
        if (port < std::numeric_limits<uint16_t>::min() ||
            port > std::numeric_limits<uint16_t>::max())
            throw ConfigError(ConfigError::Code::InvalidPort);
        // Ensure the tenant ID and device ID are not empty.
        if (tenant_id.empty())  // the tenant ID is not valid
            throw ConfigError(ConfigError::Code::InvalidTenantID);
        if (device_id.empty())  // the device ID is not valid
            throw ConfigError(ConfigError::Code::InvalidDeviceID);
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
    inline std::shared_ptr<::grpc::Channel> get_channel() const {
        return channel;
    }

    /// @brief Return the fully qualified domain name of the server.
    ///
    /// @returns The fully qualified domain name in `host:port` format.
    ///
    inline const std::string& get_fully_qualified_domain_name() const {
        return fqdn;
    }

    /// @brief Return the host name of the server.
    ///
    /// @returns The host name.
    ///
    inline std::string get_host() const {
        // Note: Typically one would need to check that `find` returned a value
        // other than std::string::npos; however, the fully qualified domain
        // name is immutable and the existence of the ':' character is
        // guaranteed past initialization time.
        return fqdn.substr(0, fqdn.find(':'));
    }

    /// @brief Return the port number of the service.
    ///
    /// @returns The port number of the service.
    ///
    inline uint16_t get_port() const {
        // Note: Typically one would need to check that `find` returned a value
        // other than std::string::npos; however, the fully qualified domain
        // name is immutable and the existence of the ':' character is
        // guaranteed past initialization time.
        return std::stoi(fqdn.substr(fqdn.find(':') + 1));
    }

    /// @brief Return the UUID of the tenant.
    ///
    /// @returns The UUID for identifying a tenant.
    ///
    inline const std::string& get_tenant_id() const {
        return tenant_id;
    }

    /// @brief Return the UUID of the device.
    ///
    /// @returns The UUID for identifying a registered device.
    ///
    inline const std::string& get_device_id() const {
        return device_id;
    }

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
    inline const uint32_t get_timeout() const { return timeout; }

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
