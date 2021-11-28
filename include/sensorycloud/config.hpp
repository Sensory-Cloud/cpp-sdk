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

#include <memory>
#include <algorithm>
#include <cstdint>
#include <chrono>
#include <string>
#include <sstream>
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief A config error type thrown when configuration parameters are invalid.
struct ConfigError : public std::runtime_error {
 public:
    /// Reasons for configuration errors to occur
    enum class Code {
        /// the host name is not valid
        InvalidHost = 0,
        /// the tenant ID is not valid
        InvalidTenantID,
        /// the device ID is not valid
        InvalidDeviceID,
    };

    /// @brief Return a message for the given error code.
    ///
    /// @param code the code to get the error message for
    /// @returns a text error message associated with the given error code
    ///
    static inline const std::string getMessage(const Code& code) {
        switch (code) {  // switch over the possible code type cases
        case Code::InvalidHost:
            return "the host name is not valid!";
        case Code::InvalidTenantID:
            return "the tenant ID is not valid!";
        case Code::InvalidDeviceID:
            return "the device ID is not valid!";
        }
    }

    /// @brief Initialize a new configuration error.
    ///
    /// @param code the reason for the configuration error
    ///
    explicit ConfigError(const Code& code) :
        std::runtime_error(getMessage(code)),
        err_code(code) { }

    /// @brief Initialize a new configuration error.
    ///
    /// @param code the reason for the configuration error
    /// @param message the message to provide through the `what()` call.
    ///
    explicit ConfigError(const Code& code, const std::string& message) :
        std::runtime_error(message),
        err_code(code) { }

    /// Destroy an instance of a configuration error.
    ~ConfigError() throw() {}

    /// @brief Return the reason the exception occurred.
    ///
    /// @returns the reason for the configuration error
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
    /// JPEG Compression factor used, a value between 0 and 1 where 0 is most
    /// compressed, and 1 is highest quality
    float jpegCompression = 0.5;

 public:
    /// Sample rate to record audio at, defaults to 16kHz
    float audioSampleRate = 16000.f;

    /// Photo pixel height, defaults to 720 pixels
    uint32_t photoHeight = 720;
    /// Photo pixel width, defaults to 480 pixels
    uint32_t photoWidth = 480;

    /// @brief Initialize a new Sensory Cloud configuration object.
    ///
    /// @param host_ the host-name of the RPC service
    /// @param port_ the port number of the RPC service
    /// @param isSecure_ whether to use SSL/TLS for message encryption
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
    /// @returns the name of the remote host to connect to
    ///
    inline const std::string& getHost() const { return host; }

    /// @brief Return the port number of the remote host.
    ///
    /// @returns the port number of the remote host to connect to
    ///
    inline const uint16_t& getPort() const { return port; }

    /// @brief Return the ID of the tenant.
    ///
    /// @returns the tenant ID for identifying the customer's account
    ///
    inline const std::string& getTenantID() const { return tenantID; }

    /// @brief Return the ID of the device.
    ///
    /// @returns the unique ID for identifying a device in a customer network
    ///
    inline const std::string& getDeviceID() const { return deviceID; }

    /// @brief Return the security policy of the remote host.
    ///
    /// @returns true if the connection is secured with TLS, false otherwise
    ///
    inline const bool& getIsSecure() const { return isSecure; }

    /// @brief Set the timeout for gRPC unary calls to a new value.
    ///
    /// @param timeout the timeout for gRPC unary calls in seconds
    ///
    inline void setTimeout(const uint32_t& timeout) { this->timeout = timeout; }

    /// @brief Return the timeout for gRPC unary calls.
    ///
    /// @returns the timeout for gRPC unary calls in seconds
    ///
    inline const uint32_t getTimeout() const { return timeout; }

    /// @brief Create a new deadline based on the RPC timeout time.
    ///
    /// @returns the deadline for the next unary RPC call.
    ///
    inline std::chrono::system_clock::time_point getDeadline() const {
        return std::chrono::system_clock::now() + std::chrono::seconds(timeout);
    }

    /// @brief Return a formatted gRPC host-name and port combination.
    ///
    /// @returns a formatted string in `"{host}:{port}"` format
    ///
    inline std::string getFullyQualifiedDomainName() const {
        return host + std::string(":") + std::to_string(port);
    }

    /// @brief Create a new gRPC channel.
    ///
    /// @returns a new gRPC channel to connect a service to
    ///
    inline std::shared_ptr<grpc::Channel> getChannel() const {
        // Create the credentials for the channel based on the security setting.
        // Use TLS (SSL) if `isSecure` is true, otherwise default to insecure
        // channel credentials.
        return grpc::CreateChannel(getFullyQualifiedDomainName(), isSecure ?
            grpc::SslCredentials(grpc::SslCredentialsOptions()) :
            grpc::InsecureChannelCredentials()
        );
    }

    /// @brief Setup an existing gRPC client context for gRPC calls.
    ///
    /// @tparam TokenManager the type of the token manager
    /// @param context the context to setup with a Bearer token and deadline
    /// @param tokenManager the token manager for retrieving tokens
    /// @param isUnary whether the connection is unary
    /// @returns a new client context for gRPC calls
    ///
    template<typename TokenManager>
    inline void setupClientContext(
        grpc::ClientContext& context,
        TokenManager& tokenManager,
        const bool& isUnary=false
    ) const {
        // Get the OAuth token and write it to the metadata header
        context.AddMetadata("authorization",
            std::string("Bearer ") + tokenManager.getAccessToken()
        );
        if (isUnary)  // Set the deadline for the RPC call
            context.set_deadline(getDeadline());
    }

    /// @brief Set the JPEG compression level to a new value.
    ///
    /// @param jpegCompression the compression factor to use. A value
    /// between 0 and 1 where 0 is most compressed, and 1 is highest quality.
    ///
    inline void setJpegCompression(const float& jpegCompression = 0.5f) {
        this->jpegCompression = std::min(std::max(jpegCompression, 0.0f), 1.0f);
    }

    /// @brief Return the compression factor for JPEG compression.
    ///
    /// @returns A value between 0 and 1 where 0 is most compressed, and 1 is
    /// the highest quality.
    ///
    inline const float& getJpegCompression() const { return jpegCompression; }
};

}  // namespace sensory

#endif  // SENSORY_CLOUD_CONFIG_HPP_
