// Test cases for the secure RNG functions in the sensory::token_manager namespace.
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
#include "sensorycloud/token_manager/secure_random.hpp"

// the number of repetitions for sampling UUIDs during testing
#define REPETITIONS 100

TEST_CASE("secure_random strings should have 8 characters") {
    for (int i = 0; i < REPETITIONS; ++i) {
        SECTION(std::string("Sample ") + std::to_string(i)) {
            const auto uuid = sensory::token_manager::secure_random<8>();
            REQUIRE(8 == uuid.length());
        }
    }
}

TEST_CASE("secure_random strings should have 16 characters") {
    for (int i = 0; i < REPETITIONS; ++i) {
        SECTION(std::string("Sample ") + std::to_string(i)) {
            const auto uuid = sensory::token_manager::secure_random<16>();
            REQUIRE(16 == uuid.length());
        }
    }
}

TEST_CASE("secure_random strings should have 32 characters") {
    for (int i = 0; i < REPETITIONS; ++i) {
        SECTION(std::string("Sample ") + std::to_string(i)) {
            const auto uuid = sensory::token_manager::secure_random<32>();
            REQUIRE(32 == uuid.length());
        }
    }
}

SCENARIO("A user wants to generate sequential random numbers") {
    GIVEN("a length of 8") {
        static constexpr std::size_t LENGTH = 8;
        WHEN("two random strings are generated") {
            const auto uuid1 = sensory::token_manager::secure_random<LENGTH>();
            const auto uuid2 = sensory::token_manager::secure_random<LENGTH>();
            THEN("the strings are not the same") {
                REQUIRE(uuid1 != uuid2);
            }
        }
    }
    GIVEN("a length of 16") {
        static constexpr std::size_t LENGTH = 16;
        WHEN("two random strings are generated") {
            const auto uuid1 = sensory::token_manager::secure_random<LENGTH>();
            const auto uuid2 = sensory::token_manager::secure_random<LENGTH>();
            THEN("the strings are not the same") {
                REQUIRE(uuid1 != uuid2);
            }
        }
    }
    GIVEN("a length of 32") {
        static constexpr std::size_t LENGTH = 32;
        WHEN("two random strings are generated") {
            const auto uuid1 = sensory::token_manager::secure_random<LENGTH>();
            const auto uuid2 = sensory::token_manager::secure_random<LENGTH>();
            THEN("the strings are not the same") {
                REQUIRE(uuid1 != uuid2);
            }
        }
    }
}
