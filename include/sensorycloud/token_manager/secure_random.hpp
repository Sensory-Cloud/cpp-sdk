// Functions for cryptographically secure RNG for the SensoryCloud C++ SDK.
//
// Copyright (c) 2021 Sensory, Inc.
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

#ifndef SENSORYCLOUD_TOKEN_MANAGER_SECURE_RANDOM_HPP_
#define SENSORYCLOUD_TOKEN_MANAGER_SECURE_RANDOM_HPP_

#include <iomanip>
#include <sstream>
#include <string>
#include "arc4random.hpp"

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Modules for generating and storing secure credentials.
namespace token_manager {

/// @brief Generate a cryptographic-ally secure random number.
///
/// @tparam length The length of the alpha-numeric string to generate.
/// @returns A cryptographic-ally secure random alpha-numeric string.
///
template<std::size_t length>
std::string secure_random() {
    // Initialize an empty string of the specified length.
    std::string uuid(length, ' ');
    // Iterate over the characters in the string to generate random characters.
    for (std::size_t i = 0; i < length; i++)
        uuid[i] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"[arc4_getbyte() % (10 + 26 + 26)];
    // Move the output string to the caller's container.
    return std::move(uuid);
}

}  // namespace token_manager

}  // namespace sensory

#endif  // SENSORYCLOUD_TOKEN_MANAGER_SECURE_RANDOM_HPP_
