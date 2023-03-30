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

#ifndef SENSORYCLOUD_IO_PATH_HPP_
#define SENSORYCLOUD_IO_PATH_HPP_

#include <string>

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief File IO components.
namespace io {

/// @brief Path manipulation tools.
namespace path {

/// @brief Normalize URIs to a strict `host(:port)?` format.
///
/// @param uri The URI to normalize.
/// @returns A URI that conforms strictly to `host(:port)?` format.
///
std::string normalize_uri(const std::string& uri);

/// @brief Return a flag determining whether the given path references a file.
/// @param path The path on the OS to verify the file-ness of.
/// @returns True if the path points to a file, false otherwise.
bool is_file(const char* path);

}  // namespace path

}  // namespace io

}  // namespace sensory

#endif  // SENSORYCLOUD_IO_PATH_HPP_
