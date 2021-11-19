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

#include <algorithm>
#include <cstdint>
#include <string>

/// @brief The Sensory Cloud API
namespace sensory {

/// @brief Configuration structures for setting up the Sensory Cloud SDK.
namespace config {

/// @brief Struct for providing info on a cloud host
struct CloudHost {
    /// Cloud DNS Host
    std::string host;
    /// Cloud port
    uint32_t port;
    /// Says if the cloud host is setup for secure communication
    bool isSecure;
};

/// @brief Configuration endpoints for Sensory Cloud.
class Config {
 private:
    /// the cloud host to interact with
    CloudHost* cloudHost = nullptr;

    /// Jpeg Compression factor used, a value between 0 and 1 where 0 is most
    /// compressed, and 1 is highest quality
    double jpegCompression = 0.5;

 public:
    /// Tenant ID to use during device enrollment
    std::string tenantID = "";
    /// Unique device identifier that model enrollments are associated to
    std::string deviceID = "";

    /// Sample rate to record audio at, defaults to 16kHz
    float audioSampleRate = 16000.f;

    /// Photo pixel height, defaults to 720 pixels
    uint32_t photoHeight = 720;
    /// Photo pixel width, defaults to 480 pixels
    uint32_t photoWidth = 480;

    /// User's preferred language/region code (ex: en-US, used for audio
    /// enrollments. Defaults to the system Locale
    // std::string languageCode = "\(Locale.current.languageCode ?? "en")-\(Locale.current.regionCode ?? "US")";
    std::string languageCode = "";

    /// Number of seconds to wait on a unary gRPC call before timing out,
    /// defaults to 10 seconds.
    uint32_t grpcTimeout = 10;

    /// Sets a secure host for transacting with Sensory cloud.
    ///
    /// @param host cloud host to use
    /// @param port optional port (443 is used by default)
    ///
    inline void setCloudHost(
        const std::string& host,
        const uint32_t& port = 443
    ) {
        if (cloudHost != nullptr) delete cloudHost;
        cloudHost = new CloudHost{host, port, true};
    }

    /// Sets an insecure host for transacting with Sensory Cloud.
    ///
    /// @param host cloud host to use
    /// @param port optional port (443 is used by default)
    ///
    /// @details
    /// This should only be used for testing against a test cloud instance and
    /// not used in production.
    ///
    inline void setInsecureCloudHost(
        const std::string& host,
        const uint32_t& port = 443
    ) {
        if (cloudHost != nullptr) delete cloudHost;
        cloudHost = new CloudHost{host, port, false};
    }

    /// @brief Returns the currently configured cloud host.
    ///
    /// @returns the cloud host or `nullptr` if a host has not been configured
    ///
    inline CloudHost* getCloudHost() const { return cloudHost; }

    /// @brief Set the JPEG compression level to a new value.
    ///
    /// @param jpegCompression the compression factor to use. A value
    /// between 0 and 1 where 0 is most compressed, and 1 is highest quality.
    ///
    inline void setJpegCompression(const double& jpegCompression = 0.5) {
        this->jpegCompression = std::min(std::max(jpegCompression, 0.0), 1.0);
    }

    /// @brief Return the compression factor for JPEG compression.
    ///
    /// @returns A value between 0 and 1 where 0 is most compressed, and 1 is
    /// the highest quality.
    ///
    inline const double& getJpegCompression() const { return jpegCompression; }
};

}  // namespace config

}  // namespace sensory

#endif  // SENSORY_CLOUD_CONFIG_HPP_
