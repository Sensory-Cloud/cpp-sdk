// Test cases for C++11 <string> header extensions.
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
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "sensorycloud/util/string_extensions.hpp"

using sensory::util::lstrip;
using sensory::util::rstrip;
using sensory::util::strip;

SCENARIO("White space needs stripped from strings") {
    GIVEN("a string with no leading or trailing white-space") {
        const std::string str = "foo";
        WHEN("the string is left stripped") {
            THEN("the output string has no leading white-space") {
                REQUIRE_THAT(lstrip(str), Catch::Equals("foo"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing white-space") {
                REQUIRE_THAT(rstrip(str), Catch::Equals("foo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing white-space") {
                REQUIRE_THAT(strip(str), Catch::Equals("foo"));
            }
        }
    }
    GIVEN("a string with leading white-space") {
        const std::string str = "  foo";
        WHEN("the string is left stripped") {
            THEN("the output string has no leading white-space") {
                REQUIRE_THAT(lstrip(str), Catch::Equals("foo"));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing white-space") {
                REQUIRE_THAT(rstrip(str), Catch::Equals("  foo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing white-space") {
                REQUIRE_THAT(strip(str), Catch::Equals("foo"));
            }
        }
    }
    GIVEN("a string with trailing white-space") {
        const std::string str = "foo  ";
        WHEN("the string is left stripped") {
            THEN("the output string has no leading white-space") {
                REQUIRE_THAT(lstrip(str), Catch::Equals("foo  "));
            }
        }
        WHEN("the string is right stripped") {
            THEN("the output string has no trailing white-space") {
                REQUIRE_THAT(rstrip(str), Catch::Equals("foo"));
            }
        }
        WHEN("the string is left+right stripped") {
            THEN("the output string has no leading or trailing white-space") {
                REQUIRE_THAT(strip(str), Catch::Equals("foo"));
            }
        }
    }
}
