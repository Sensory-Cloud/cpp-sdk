// Configuration structures for the Sensory Cloud C++ SDK.
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

#ifndef SENSORY_CLOUD_CONFIG_HPP_
#define SENSORY_CLOUD_CONFIG_HPP_

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <memory>
#include <algorithm>
#include <cstdint>
#include <chrono>
#include <string>
#include <sstream>

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief A config error type thrown when configuration parameters are invalid.
struct ConfigError : public std::runtime_error {
 public:
    /// @brief Reasons for configuration errors to occur.
    enum class Code {
        /// The host name is not valid.
        InvalidHost = 0,
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
    static inline const std::string getMessage(const Code& code) {
        switch (code) {  // switch over the possible code type cases
        case Code::InvalidHost:
            return "The host name is not valid!";
        case Code::InvalidTenantID:
            return "The tenant ID is not valid!";
        case Code::InvalidDeviceID:
            return "The device ID is not valid!";
        default:
            return "Unrecognized Error Code";
        }
    }

    /// @brief Initialize a new configuration error.
    ///
    /// @param code The reason for the configuration error.
    ///
    explicit ConfigError(const Code& code) :
        std::runtime_error(getMessage(code)),
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

/// @brief A configuration endpoint for Sensory Cloud.
class Config {
 private:
    /// the name of the cloud host, i.e., DNS name
    const std::string host;
    /// the port that the cloud service is running on
    const uint16_t port;
    /// Tenant ID to use during device enrollment
    const std::string tenantID;
    /// Unique device identifier that model enrollments are associated to
    const std::string deviceID;
    /// whether the connection to the remote host should be secured by TLS/SSL
    const bool isSecure;
    /// the number of seconds to wait on a unary gRPC call before timing out
    uint32_t timeout = 10;

 public:
    /// @brief Initialize a new Sensory Cloud configuration object.
    ///
    /// @param host_ The host-name of the RPC service.
    /// @param port_ The port number of the RPC service.
    /// @param tenantID_ The unique ID of your tenant in Sensory Cloud.
    /// @param deviceID_ The unique ID of the device running the SDK.
    /// @param isSecure_ `true` to use SSL/TLS for message encryption, `false`
    /// to use an insecure connection.
    ///
    Config(
        const std::string& host_,
        const uint16_t& port_,
        const std::string& tenantID_,
        const std::string& deviceID_,
        const bool& isSecure_ = true
    ) :
        host(host_),
        port(port_),
        tenantID(tenantID_),
        deviceID(deviceID_),
        isSecure(isSecure_) {
        if (host.empty())  // the host name is not valid
            throw ConfigError(ConfigError::Code::InvalidHost);
        if (tenantID.empty())  // the tenant ID is not valid
            throw ConfigError(ConfigError::Code::InvalidTenantID);
        if (deviceID.empty())  // the device ID is not valid
            throw ConfigError(ConfigError::Code::InvalidDeviceID);
    }

    /// @brief Return the name of the remote host.
    ///
    /// @returns The name of the remote host to connect to.
    ///
    inline const std::string& getHost() const { return host; }

    /// @brief Return the port number of the remote host.
    ///
    /// @returns The port number of the remote host to connect to.
    ///
    inline const uint16_t& getPort() const { return port; }

    /// @brief Return the ID of the tenant.
    ///
    /// @returns The tenant ID for identifying the customer's account.
    ///
    inline const std::string& getTenantID() const { return tenantID; }

    /// @brief Return the ID of the device.
    ///
    /// @returns The unique ID for identifying a device in a customer network.
    ///
    inline const std::string& getDeviceID() const { return deviceID; }

    /// @brief Return the security policy of the remote host.
    ///
    /// @returns `true` if the connection is secured with TLS/SSL, `false`
    /// otherwise.
    ///
    inline const bool& getIsSecure() const { return isSecure; }

    /// @brief Set the timeout for gRPC unary calls to a new value.
    ///
    /// @param timeout The timeout for gRPC unary calls in seconds.
    ///
    inline void setTimeout(const uint32_t& timeout) { this->timeout = timeout; }

    /// @brief Return the timeout for gRPC unary calls.
    ///
    /// @returns The timeout for gRPC unary calls in seconds.
    ///
    inline const uint32_t getTimeout() const { return timeout; }

    /// @brief Create a new deadline based on the RPC timeout time.
    ///
    /// @returns The deadline for the next unary RPC call.
    ///
    inline std::chrono::system_clock::time_point getDeadline() const {
        return std::chrono::system_clock::now() + std::chrono::seconds(timeout);
    }

    /// @brief Return a formatted gRPC host-name and port combination.
    ///
    /// @returns A formatted string in `"{host}:{port}"` format.
    ///
    inline std::string getFullyQualifiedDomainName() const {
        return host + std::string(":") + std::to_string(port);
    }

    /// @brief Create a new gRPC channel.
    ///
    /// @returns A new gRPC channel to connect a service to.
    ///
    inline std::shared_ptr<::grpc::Channel> getChannel() const {
        // Create the credentials for the channel based on the security setting.
        // Use TLS (SSL) if `isSecure` is true, otherwise default to insecure
        // channel credentials.
        return ::grpc::CreateChannel(getFullyQualifiedDomainName(), isSecure ?
            ::grpc::SslCredentials(::grpc::SslCredentialsOptions()) :
            ::grpc::InsecureChannelCredentials()
        );
    }

    /// @brief Setup an existing client context for unary gRPC calls.
    ///
    /// @tparam TokenManager The type of the token manager.
    /// @param context The context to setup with a Bearer token and deadline.
    /// @param tokenManager The token manager for fetching tokens.
    ///
    template<typename TokenManager>
    inline void setupUnaryClientContext(
        ::grpc::ClientContext& context,
        TokenManager& tokenManager
    ) const {
        context.AddMetadata("authorization",
            std::string("Bearer ") + tokenManager.getAccessToken()
        );
        context.set_deadline(getDeadline());
    }

    /// @brief Setup an existing client context for bidirectional gRPC streams.
    ///
    /// @tparam TokenManager The type of the token manager.
    /// @param context The context to setup with a Bearer token and deadline.
    /// @param tokenManager The token manager for fetching tokens.
    ///
    template<typename TokenManager>
    inline void setupBidiClientContext(
        ::grpc::ClientContext& context,
        TokenManager& tokenManager
    ) const {
        context.AddMetadata("authorization",
            std::string("Bearer ") + tokenManager.getAccessToken()
        );
    }
};

}  // namespace sensory

#endif  // SENSORY_CLOUD_CONFIG_HPP_
