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

#include <random>
#include <chrono>
#include "sensorycloud/util/uuid.hpp"

namespace sensory {

namespace util {

std::string uuid_v4() {
    // Create a random number generator conditioned on the system RNG
    std::mt19937_64 generator(std::random_device("/dev/random")());
    // Create the UUID from randomly generated digits
    std::string uuid(36, '-');
    for (unsigned i = 0; i < 36; i++) {
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

}  // namespace util

}  // namespace sensory
