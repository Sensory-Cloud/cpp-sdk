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

#include <ctime>
#include <utility>
#include "sensorycloud/util/time.hpp"

namespace sensory {

namespace util {

std::string timepoint_to_timestamp(const std::chrono::system_clock::time_point& time_point) {
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

std::chrono::system_clock::time_point timestamp_to_timepoint(const std::string& timestamp) {
    // Stream the contents of the timestamp string into the tm structure
    std::tm tm = {};
    strptime(timestamp.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm);
    // Convert tm -> time_t in GMT format -> time_point
    return std::chrono::system_clock::from_time_t(timegm(&tm));
}

}  // namespace util

}  // namespace sensory
