// Extensions to the C++11 <string> header.
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

#include "sensorycloud/util/string_extensions.hpp"

namespace sensory {

namespace util {

std::string lstrip(const std::string &str) {
    int index;
    for (index = 0; index < str.length(); ++index)
        if (str[index] > 0x20 && str[index] < 0x7F) break;
    return str.substr(index, str.length() - index);
}

std::string rstrip(const std::string &str) {
    int index;
    for (index = str.length(); index > 0; --index)
        if (str[index - 1] > 0x20 && str[index - 1] < 0x7F) break;
    return str.substr(0, index);
}

}  // namespace util

}  // namespace sensory
