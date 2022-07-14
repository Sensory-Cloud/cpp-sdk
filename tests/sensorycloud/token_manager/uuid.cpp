// Test cases for the UUID functions in the sensory::token_manager namespace.
//
// Copyright (c) 2021 Sensory, Inc.
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
#include <unordered_set>
#include <catch2/catch.hpp>
#include "sensorycloud/token_manager/uuid.hpp"

// the number of repetitions for sampling UUIDs during testing
#define REPETITIONS 30

TEST_CASE("UUIDv4 strings should have 36 characters") {
    for (int i = 0; i < REPETITIONS; ++i) {
        SECTION(std::string("Sample ") + std::to_string(i)) {
            const auto uuid = sensory::token_manager::uuid_v4();
            REQUIRE(36 == uuid.length());
        }
    }
}

TEST_CASE("UUIDv4 strings should have 4 hyphens") {
    for (int i = 0; i < REPETITIONS; ++i) {
        SECTION(std::string("Sample ") + std::to_string(i)) {
            // The UUID should be formatted like this:
            // AA97B177-9383-4934-8543-0F91A7A02836
            //         ^    ^    ^    ^
            //         8    13   18   23
            const auto uuid = sensory::token_manager::uuid_v4();
            REQUIRE('-' == uuid[8]);
            REQUIRE('-' == uuid[13]);
            REQUIRE('-' == uuid[18]);
            REQUIRE('-' == uuid[23]);
        }
    }
}

TEST_CASE("UUIDv4 strings should have a static '4' at index 14") {
    for (int i = 0; i < REPETITIONS; ++i) {
        SECTION(std::string("Sample ") + std::to_string(i)) {
            // The UUID should be formatted like this:
            // AA97B177-9383-4934-8543-0F91A7A02836
            //               ^
            //               14 (should always be '4')
            const auto uuid = sensory::token_manager::uuid_v4();
            REQUIRE('4' == uuid[14]);
        }
    }
}

TEST_CASE("UUIDv4 strings should have one of '8', '9', 'A', 'B' at index 19") {
    const std::unordered_set<char> EXPECTED = {'8', '9', 'A', 'B'};
    for (int i = 0; i < REPETITIONS; ++i) {
        SECTION(std::string("Sample ") + std::to_string(i)) {
            // The UUID should be formatted like this:
            // AA97B177-9383-4934-8543-0F91A7A02836
            //                    ^
            //                    19 (should always be on of '8', '9', 'A', 'B')
            const auto uuid = sensory::token_manager::uuid_v4();
            REQUIRE(EXPECTED.find(uuid[19]) != EXPECTED.end());
        }
    }
}
