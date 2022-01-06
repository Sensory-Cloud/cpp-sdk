// Functions for interacting with timestamps for the Sensory Cloud C++ SDK.
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

#ifndef SENSORY_CLOUD_TOKEN_MANAGER_TIME_HPP_
#define SENSORY_CLOUD_TOKEN_MANAGER_TIME_HPP_

#include <chrono>
#include <ctime>
#include <utility>
#include <string>

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Modules for generating and storing secure credentials.
namespace token_manager {

/// @brief Convert a time point to a UTC ISO8601 timestamp.
///
/// @param time The time point to convert to a UTC ISO8601 timestamp.
/// @returns The UTC ISO8601 timestamp representation of the time point.
///
inline std::string timepoint_to_timestamp(const std::chrono::system_clock::time_point& time_point) {
    // Convert the time point to a time_t object
    const auto tt = std::chrono::system_clock::to_time_t(time_point);
    // Format the time point into a character buffer (as a string).
    std::string output("0000-00-00T00:00:00Z");
    // use a local `std::tm` buffer and the accompanying `gmtime_r` function
    // instead of the static memory used by `gmtime`. This is for thread safety.
    std::tm tm;
    strftime(&output[0], output.length(), "%Y-%m-%dT%H:%M:%SZ", gmtime_r(&tt, &tm));
    return std::move(output);
}

/// @brief Convert a UTC ISO8601 timestamp to a time point.
///
/// @param time The UTC ISO8601 timestamp to convert to a time point.
/// @returns The input timestamp converted to a native time point.
///
inline std::chrono::system_clock::time_point timestamp_to_timepoint(const std::string& timestamp) {
    // Stream the contents of the timestamp string into the tm structure
    std::tm tm = {};
    strptime(timestamp.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm);
    // Convert tm -> time_t in GMT format -> time_point
    return std::chrono::system_clock::from_time_t(timegm(&tm));
}

}  // namespace token_manager

}  // namespace sensory

#endif  // SENSORY_CLOUD_TOKEN_MANAGER_TIME_HPP_
