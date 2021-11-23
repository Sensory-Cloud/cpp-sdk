// Functions for generating UUIDs for the Sensory Cloud C++ SDK.
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

#ifndef SENSORY_CLOUD_UUID_HPP_
#define SENSORY_CLOUD_UUID_HPP_

#include <random>
#include <chrono>

/// @brief The Sensory Cloud SDK.
namespace sensory {

// TODO: benchmark this function.
// TODO: refactor to minimize random number generation.

/// @brief Generate a pseudo-random UUID compliant with RFC-4122 Version 4.
///
/// @returns a 36-character UUID string based on Mersinne
///
/// @details
///
/// > 4.4.  Algorithms for Creating a UUID from Truly Random or
///         Pseudo-Random Numbers
/// The version 4 UUID is meant for generating UUIDs from truly-random or
/// pseudo-random numbers. The algorithm is as follows:
/// 1.  Set the two most significant bits (bits 6 and 7) of the
///     clock_seq_hi_and_reserved to zero and one, respectively.
/// 2.  Set the four most significant bits (bits 12 through 15) of the
///     time_hi_and_version field to the 4-bit version number from
///     Section 4.1.3.
/// 3.  Set all the other bits to randomly (or pseudo-randomly) chosen
///     values.
///
/// The resulting code is in the following format where the character at
/// position (1) is statically 'A' and the character at position (2) is randomly
/// selected from {'8', '9', 'A', 'B'}. The remaining characters are randomly
/// selected without condition.
///
/// ```
/// AA97B177-9383-4934-8543-0F91A7A02836
///               ^    ^
///               1    2
/// ```
///
/// Reference: https://datatracker.ietf.org/doc/html/rfc4122#section-4.4
///
std::string uuid_v4() {
    // Create a random number generator conditioned on the current time
    std::mt19937_64 generator(std::random_device("/dev/random")());
    // Create the UUID from randomly generated digits
    std::string uuid(36, '-');
    for (unsigned i = 0; i < uuid.size(); i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23)  // hyphenated position
            continue;
        else if (i == 14)  // character 14 is always A
            uuid[i] = '4';
        else if (i == 19)  // character 19 is always in {8, 9, A, B}
            uuid[i] = "89AB"[generator() & 0x3];
        else  // arbitrary characters have no constraints
            uuid[i] = "0123456789ABCDEF"[generator() & 0xF];
    }
    return uuid;
}

}  // namespace sensory

#endif  // SENSORY_CLOUD_UUID_HPP_
