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

/// @brief A structure for providing information about a cloud host.
class CloudHost {
 private:
    /// Cloud DNS Host
    std::string host;
    /// Cloud port
    uint16_t port;
    /// Says if the cloud host is setup for secure communication
    bool isSecure;
    /// Number of seconds to wait on a unary gRPC call before timing out
    uint32_t timeout = 10;

 public:
    /// @brief Initialize a new cloud host.
    ///
    /// @param host_ the host-name of the RPC service
    /// @param port_ the port number of the RPC service
    /// @param isSecure_ whether to use SSL/TLS for message encryption
    ///
    explicit CloudHost(
        const std::string& host_,
        const uint16_t& port_,
        const bool& isSecure_
    ) : host(host_), port(port_), isSecure(isSecure_) { }

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
        std::stringstream stream;
        // RPC addresses are formatted as "host:port"
        stream << host << ":" << port;
        return stream.str();
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

    /// @brief Create a new gRPC client context for gRPC calls.
    ///
    /// @tparam CredentialProvider the type of the credential provider
    /// @param credentialProvider the credential provider for retrieving tokens
    /// @param isUnary whether the connection is unary
    /// @returns a new client context for gRPC calls
    ///
    template<typename CredentialProvider>
    std::unique_ptr<grpc::ClientContext> getClientContext(
        const CredentialProvider& credentialProvider,
        const bool& isUnary=false
    ) {
        // Create a new client context.
        std::unique_ptr<grpc::ClientContext> context(new grpc::ClientContext);
        // Get the OAuth token and write it to the metadata header
        std::stringstream stream;
        stream << "Bearer " << credentialProvider.getAccessToken();
        context->AddMetadata("authorization", stream.str());
        if (isUnary)  // Set the deadline for the RPC call
            context->set_deadline(getDeadline());

        return context;
    }
};

/// @brief A configuration endpoint for Sensory Cloud.
class Config {
 private:
    /// the cloud host to interact with
    CloudHost* cloudHost = nullptr;
    /// JPEG Compression factor used, a value between 0 and 1 where 0 is most
    /// compressed, and 1 is highest quality
    float jpegCompression = 0.5;

 public:
    /// Tenant ID to use during device enrollment
    std::string tenantID = "";
    /// Unique device identifier that model enrollments are associated to
    std::string deviceID = "";

    /// User's preferred language/region code (ex: en-US, used for audio
    /// enrollments. Defaults to the system Locale
    std::string languageCode = "en-US";

    /// Sample rate to record audio at, defaults to 16kHz
    float audioSampleRate = 16000.f;

    /// Photo pixel height, defaults to 720 pixels
    uint32_t photoHeight = 720;
    /// Photo pixel width, defaults to 480 pixels
    uint32_t photoWidth = 480;

    /// @brief Return a flag indicating whether a cloud host has been specified.
    ///
    /// @returns true if the cloud host exists, false otherwise
    ///
    inline const bool hasCloudHost() const { return cloudHost != nullptr; }

    /// @brief Set a host for transacting with Sensory cloud.
    ///
    /// @param host cloud host to use
    /// @param port optional port (50051 is used by default)
    /// @param isSecure whether to use a secure connection with SSL
    ///
    inline void setCloudHost(
        const std::string& host,
        const uint16_t& port = 50051,
        const bool& isSecure = true
    ) {
        if (cloudHost != nullptr) delete cloudHost;
        cloudHost = new CloudHost(host, port, isSecure);
    }

    /// @brief Returns the currently configured cloud host.
    ///
    /// @returns the cloud host or `nullptr` if a host has not been configured
    ///
    inline const CloudHost* getCloudHost() const { return cloudHost; }

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

    /// @brief Return true if the configuration represents a valid connection.
    ///
    /// @returns true if the tenant ID and device ID are specified
    ///
    inline bool isValid() const {
        return !tenantID.empty() && !deviceID.empty();
    }
};

}  // namespace sensory

#endif  // SENSORY_CLOUD_CONFIG_HPP_
