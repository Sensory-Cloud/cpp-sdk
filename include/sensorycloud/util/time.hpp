// Functions for interacting with timestamps.
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

#ifndef SENSORYCLOUD_UTIL_TIME_HPP_
#define SENSORYCLOUD_UTIL_TIME_HPP_

#include <chrono>
#include <string>

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Utility functions.
namespace util {

/// @brief Convert a time point to a UTC ISO8601 timestamp.
///
/// @param time The time point to convert to a UTC ISO8601 timestamp.
/// @returns The UTC ISO8601 timestamp representation of the time point.
///
std::string timepoint_to_timestamp(const std::chrono::system_clock::time_point& time_point);

/// @brief Convert a UTC ISO8601 timestamp to a time point.
///
/// @param time The UTC ISO8601 timestamp to convert to a time point.
/// @returns The input timestamp converted to a native time point.
///
std::chrono::system_clock::time_point timestamp_to_timepoint(const std::string& timestamp);

}  // namespace util

}  // namespace sensory

#endif  // SENSORYCLOUD_UTIL_TIME_HPP_
