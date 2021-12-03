// Exception types for handling network errors.
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

#ifndef SENSORY_CLOUD_SERVICES_NETWORK_ERROR_HPP_
#define SENSORY_CLOUD_SERVICES_NETWORK_ERROR_HPP_

#include <string>
#include <exception>

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud services.
namespace service {

/// @brief A network error type thrown when connection issues arise.
struct NetworkError : public std::runtime_error {
 public:
    /// Reasons for network errors to occur
    enum class Code {
        /// the connection was not properly initialized
        NotInitialized = 0,
    };

    /// @brief Return a message for the given error code.
    ///
    /// @param code the code to get the error message for
    /// @returns a text error message associated with the given error code
    ///
    static inline const std::string getMessage(const Code& code) {
        switch (code) {  // switch over the possible code type cases
        case Code::NotInitialized:
            return "the cloud host has not been initialized!";
        }
    }

    /// @brief Initialize a new network error.
    ///
    /// @param code the reason for the network error
    ///
    explicit NetworkError(const Code& code) :
        std::runtime_error(getMessage(code)),
        err_code(code) { }

    /// @brief Initialize a new network error.
    ///
    /// @param code the reason for the network error
    /// @param message the message to provide through the `what()` call.
    ///
    explicit NetworkError(const Code& code, const std::string& message) :
        std::runtime_error(message),
        err_code(code) { }

    /// Destroy an instance of a network error.
    ~NetworkError() throw() {}

    /// @brief Return the reason the exception occurred.
    ///
    /// @returns the reason for the network error
    ///
    inline const Code& code() const throw() { return err_code; }

 private:
    /// the reason the network error occurred
    Code err_code;
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORY_CLOUD_SERVICES_NETWORK_ERROR_HPP_
