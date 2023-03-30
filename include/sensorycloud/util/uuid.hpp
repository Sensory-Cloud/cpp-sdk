// Functions for generating UUIDs.
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

#ifndef SENSORYCLOUD_UTIL_UUID_HPP_
#define SENSORYCLOUD_UTIL_UUID_HPP_

#include <string>

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Utility functions.
namespace util {

/// @brief Generate a pseudo-random UUID compliant with
/// [RFC-4122 v4](https://datatracker.ietf.org/doc/html/rfc4122#section-4.4).
///
/// @returns A 36-character UUID string based on Mersinne Twister random number
/// generation.
///
/// @details
///
/// RFC-4122 states, with regard to version 4 of the standard:
///
/// > 4.4.  Algorithms for Creating a UUID from Truly Random or
/// >       Pseudo-Random Numbers
/// > The version 4 UUID is meant for generating UUIDs from truly-random or
/// > pseudo-random numbers. The algorithm is as follows:
/// > 1.  Set the two most significant bits (bits 6 and 7) of the
/// >     clock_seq_hi_and_reserved to zero and one, respectively.
/// > 2.  Set the four most significant bits (bits 12 through 15) of the
/// >     time_hi_and_version field to the 4-bit version number from
/// >     Section 4.1.3.
/// > 3.  Set all the other bits to randomly (or pseudo-randomly) chosen
/// >     values.
///
/// The resulting code is in the following format where the character at
/// position (1) is statically `4` and the character at position (2) is randomly
/// selected from {`8`, `9`, `A`, `B`}. The remaining characters are randomly
/// selected without condition (see below for example UUID).
///
/// ```
/// AA97B177-9383-4934-8543-0F91A7A02836
///               ^    ^
///               1    2
/// ```
///
std::string uuid_v4();

}  // namespace util

}  // namespace sensory

#endif  // SENSORYCLOUD_UTIL_UUID_HPP_
