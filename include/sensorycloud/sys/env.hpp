// Functions for manipulating file-system and web paths/URIs.
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

#ifndef SENSORYCLOUD_SYS_ENV_HPP_
#define SENSORYCLOUD_SYS_ENV_HPP_

#include <string>

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief File IO components.
namespace sys {

/// @brief Return the environment variable with given key.
/// @param key The key of the environment variable to lookup.
/// @param default_value A default value to return if the key is not found.
/// @returns The value of the environment variable with given key if found,
/// otherwise the default value.
inline std::string get_env_var(
    const std::string& key,
    const std::string& default_value = ""
) {
    auto value = getenv(key.c_str());
    return value == nullptr ? default_value : std::string(value);
}

}  // namespace sys

}  // namespace sensory

#endif  // SENSORYCLOUD_SYS_ENV_HPP_
