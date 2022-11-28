// Functions for cryptographically secure RNG for the SensoryCloud C++ SDK.
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

#ifndef SENSORYCLOUD_UTIL_SECURE_RANDOM_HPP_
#define SENSORYCLOUD_UTIL_SECURE_RANDOM_HPP_

#include <string>
#include <openssl/rand.h>

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Utility functions.
namespace util {

/// @brief Generate a cryptographically secure random number.
///
/// @tparam length The length of the alpha-numeric string to generate.
/// @returns A cryptographically secure random alpha-numeric string.
///
template<std::size_t length>
std::string secure_random() {
    // Initialize an empty string of the specified length.
    std::string uuid(length, ' ');
    // Randomly initialize the bytes of the string using OpenSSL rand bytes.
    // Here we assume that std::string is backed by a contiguous buffer, which
    // is a specification of C++00 and is true of all active std::string
    // implementations that are currently known predating the C++00 standard.
    // The reinterpret cast is necessary to coerce the char* to uint8_t* that
    // RAND_bytes expects.
    RAND_bytes(reinterpret_cast<uint8_t*>(&uuid[0]), uuid.size());
    // Iterate over the bytes in the string to generate random characters.
    // This is necessary because we want a specific subset of characters that
    // are not contiguously spaced in the ASCII codec.
    for (std::size_t i = 0; i < length; i++)
        uuid[i] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"[static_cast<uint8_t>(uuid[i]) % (10 + 26 + 26)];
    // Move the output string to the caller's container.
    return std::move(uuid);
}

}  // namespace util

}  // namespace sensory

#endif  // SENSORYCLOUD_UTIL_SECURE_RANDOM_HPP_
