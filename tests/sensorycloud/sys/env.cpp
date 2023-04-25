// Test cases for SDK system functions.
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
#include "sensorycloud/sys/env.hpp"

SCENARIO("Environment variables need to be looked up with default values") {
    GIVEN("A key for an environment variable that does not exist") {
        const std::string key = "FOO_BAR_VARIABLE";
        WHEN("The environment variable value is requested with the provided default value") {
            const std::string value = sensory::sys::get_env_var(key);
            THEN("The provided default value is returned") {
                REQUIRE_THAT("", Catch::Equals(value));
            }
        }
        WHEN("The environment variable value is requested with a custom default value") {
            const std::string default_value = "foo";
            const std::string value = sensory::sys::get_env_var(key, default_value);
            THEN("The custom default value is returned") {
                REQUIRE_THAT(default_value, Catch::Equals(value));
            }
        }
    }
    GIVEN("A key for an environment variable that does exist") {
        const std::string key = "SENSORYCLOUD_SYSTEM_ENV_TEST_GET_ENV_VAR";
        const std::string expected_value = "bar";
        setenv(key.c_str(), expected_value.c_str(), 0);
        WHEN("The environment variable value is requested") {
            const std::string value = sensory::sys::get_env_var(key);
            THEN("The value of the variable is returned") {
                REQUIRE_THAT(expected_value, Catch::Equals(value));
            }
        }
    }
}
