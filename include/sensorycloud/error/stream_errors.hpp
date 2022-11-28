// Streaming errors.
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

#ifndef SENSORYCLOUD_ERROR_STREAM_ERRORS_HPP_
#define SENSORYCLOUD_ERROR_STREAM_ERRORS_HPP_

#include <exception>
#include <string>

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Errors.
namespace error {

/// @brief An error type thrown when a stream fails to initialize.
struct NullStreamError : public std::runtime_error {
 public:
    /// @brief Initialize a new null stream error.
    ///
    /// @param message The message to provide through the `what()` call.
    ///
    explicit NullStreamError(const std::string& message) : std::runtime_error(message) { }

    /// Destroy an instance of a null stream error.
    ~NullStreamError() throw() {}
};

/// @brief An error type thrown when a write call on a stream fails.
struct WriteStreamError : public std::runtime_error {
 public:
    /// @brief Initialize a new stream write error.
    ///
    /// @param message The message to provide through the `what()` call.
    ///
    explicit WriteStreamError(const std::string& message) : std::runtime_error(message) { }

    /// Destroy an instance of a stream write error.
    ~WriteStreamError() throw() {}
};

/// @brief An error type thrown when a read call on a stream fails.
struct ReadStreamError : public std::runtime_error {
 public:
    /// @brief Initialize a new stream read error.
    ///
    /// @param message The message to provide through the `what()` call.
    ///
    explicit ReadStreamError(const std::string& message) : std::runtime_error(message) { }

    /// Destroy an instance of a stream read error.
    ~ReadStreamError() throw() {}
};

}  // namespace error

}  // namespace sensory

#endif  // SENSORYCLOUD_ERROR_STREAM_ERRORS_HPP_
