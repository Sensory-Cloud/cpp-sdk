// Functions for cryptographically secure RNG for the Sensory Cloud C++ SDK.
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

#ifndef SENSORY_CLOUD_TOKEN_MANAGER_SECURE_RANDOM_HPP_
#define SENSORY_CLOUD_TOKEN_MANAGER_SECURE_RANDOM_HPP_

#include <iomanip>
#include <sstream>
#include <string>
#include "arc4random.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Modules for generating and storing secure credentials.
namespace token_manager {

/// @brief Generate a cryptographic-ally secure random number.
///
/// @tparam length The length of the hex string to generate.
/// @returns A cryptographic-ally secure random hex string.
///
template<std::size_t length>
std::string secure_random() {
    // This function operates on nibbles and data must be byte aligned. This
    // means the requested length must be divisible by 2. Perform this assertion
    // statically since the value will be known at compile time.
    static_assert(!(length % 2), "length must be a multiple of 2");
    // Generate a buffer of bytes half the size of the requested string.
    uint8_t data[length / 2];
    // The old implementation of this function utilized the <openssl/rand.h>
    // header to use the RAND_bytes function defined in that header.
    // RAND_bytes((unsigned char*) data, sizeof(data));
    // The current portable implementation incorporates arc4random from BSD.
    arc4random_buf((unsigned char*) data, sizeof(data));
    // Create a stream to write the string data to
    std::stringstream stream;
    stream << std::hex;
    // Iterate over the random numbers and convert them to hex characters
    for (int i = 0; i < length / 2; i++)
        stream << std::setw(2) << std::setfill('0') << data[i];
    // Flush the stream to a std::string object
    return stream.str();
}

}  // namespace token_manager

}  // namespace sensory

#endif  // SENSORY_CLOUD_TOKEN_MANAGER_SECURE_RANDOM_HPP_