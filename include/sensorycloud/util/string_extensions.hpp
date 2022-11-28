// Extensions to the C++11 <string> header.
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

#ifndef SENSORYCLOUD_UTIL_STRING_EXTENSIONS
#define SENSORYCLOUD_UTIL_STRING_EXTENSIONS

#include <string>

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Utility functions.
namespace util {

/// @brief Strip leading white spaces from a string.
///
/// @param s The string to strip the leading white spaces from
/// @returns The input string with leading white spaces removed.
///
inline std::string lstrip(const std::string &s) {
    size_t start = s.find_first_not_of(" ");
    return (start == std::string::npos) ? "" : s.substr(start);
}

/// @brief Strip trailing white spaces from a string.
///
/// @param s The string to strip the trailing white spaces from.
/// @returns The input string with trailing white spaces removed.
///
inline std::string rstrip(const std::string &s) {
    size_t end = s.find_last_not_of(" ");
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

/// @brief Strip leading & trailing white spaces from a string.
///
/// @param s The string to strip the leading & trailing white spaces from.
/// @returns The input string with leading & trailing white spaces removed.
///
inline std::string strip(const std::string &s) { return rstrip(lstrip(s)); }

}  // namespace util

}  // namespace sensory

#endif  // SENSORYCLOUD_UTIL_STRING_EXTENSIONS
