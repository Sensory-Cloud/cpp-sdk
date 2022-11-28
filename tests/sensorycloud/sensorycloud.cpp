// Test cases for the sensorycloud module.
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
#include "sensorycloud/sensorycloud.hpp"

using sensory::parse_enrollment_type;
using sensory::EnrollmentType;

// ---------------------------------------------------------------------------
// MARK: parse_enrollment_type
// ---------------------------------------------------------------------------

SCENARIO("a user wants to parse an EnrollmentType from a string") {
    GIVEN("the string \"none\"") {
        const auto input = "none";
        WHEN("The string is parsed") {
            const auto enrollment_type = parse_enrollment_type(input);
            THEN("EnrollmentType::None is returned") {
                REQUIRE(EnrollmentType::None == enrollment_type);
            }
        }
    }
    GIVEN("the string \"sharedSecret\"") {
        const auto input = "sharedSecret";
        WHEN("The string is parsed") {
            const auto enrollment_type = parse_enrollment_type(input);
            THEN("EnrollmentType::SharedSecret is returned") {
                REQUIRE(EnrollmentType::SharedSecret == enrollment_type);
            }
        }
    }
    GIVEN("the string \"jwt\"") {
        const auto input = "jwt";
        WHEN("The string is parsed") {
            const auto enrollment_type = parse_enrollment_type(input);
            THEN("EnrollmentType::JWT is returned") {
                REQUIRE(EnrollmentType::JWT == enrollment_type);
            }
        }
    }
    GIVEN("the string \"foo\"") {
        const auto input = "foo";
        WHEN("The string is parsed") {
            THEN("an std::runtime_error is thrown") {
                REQUIRE_THROWS(parse_enrollment_type(input));
            }
        }
    }
}
