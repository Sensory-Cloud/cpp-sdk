// Configuration errors.
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

#ifndef SENSORYCLOUD_ERROR_CONFIG_ERROR_HPP_
#define SENSORYCLOUD_ERROR_CONFIG_ERROR_HPP_

#include <exception>
#include <string>

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Errors.
namespace error {

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

}  // namespace error

}  // namespace sensory

#endif  // SENSORYCLOUD_ERROR_CONFIG_ERROR_HPP_
