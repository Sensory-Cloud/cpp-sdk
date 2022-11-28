// Test cases for the time functions in the sensory::util namespace.
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
#include "sensorycloud/util/time.hpp"

SCENARIO("A user wants to convert a time_point to a timestamp") {
    GIVEN("a valid time point (the epoch time)") {
        std::chrono::system_clock::time_point time_point;
        WHEN("the time point is converted to a timestamp") {
            const auto timestamp = sensory::util::timepoint_to_timestamp(time_point);
            THEN("the string follows the UTC ISO8601 standard") {
                REQUIRE_THAT(timestamp, Catch::Equals("1970-01-01T00:00:00Z"));
            }
        }
        WHEN("the time point is converted to a timestamp and back") {
            const auto timestamp = sensory::util::timepoint_to_timestamp(time_point);
            const auto decoded = sensory::util::timestamp_to_timepoint(timestamp);
            THEN("the time point does not change") {
                REQUIRE(time_point == decoded);
            }
        }
    }
    GIVEN("a valid arbitrary time point") {
        std::tm tm = {};
        tm.tm_year = 1994 - 1900;
        tm.tm_mon = 11 - 1;
        tm.tm_mday = 17;
        tm.tm_hour = 3;
        tm.tm_min = 23;
        tm.tm_sec = 37;
        std::time_t tt = timegm(&tm);
        const auto time_point = std::chrono::system_clock::from_time_t(tt);
        WHEN("the time point is converted to a timestamp") {
            const auto timestamp = sensory::util::timepoint_to_timestamp(time_point);
            THEN("the string follows the UTC ISO8601 standard") {
                REQUIRE_THAT(timestamp, Catch::Equals("1994-11-17T03:23:37Z"));
            }
        }
        WHEN("the time point is converted to a timestamp and back") {
            const auto timestamp = sensory::util::timepoint_to_timestamp(time_point);
            const auto decoded = sensory::util::timestamp_to_timepoint(timestamp);
            THEN("the time point does not change") {
                REQUIRE(time_point == decoded);
            }
        }
    }
}

SCENARIO("A user wants to convert a timestamp to a time_point") {
    GIVEN("a timestamp in UTC ISO8601 format (the epoch time)") {
        std::string timestamp = "1970-01-01T00:00:00Z";
        WHEN("the timestamp is converted to a timepoint") {
            const auto time_point = sensory::util::timestamp_to_timepoint(timestamp);
            THEN("the timepoint data matches the timestamp") {
                std::time_t tt = std::chrono::system_clock::to_time_t(time_point);
                std::tm utc_tm = *gmtime(&tt);
                REQUIRE(1970 == 1900 + utc_tm.tm_year);  // time starts at 1900
                REQUIRE(1 == 1 + utc_tm.tm_mon);  // months are indexed from 0
                REQUIRE(1 == utc_tm.tm_mday);  // days are indexed from 1
                REQUIRE(0 == utc_tm.tm_hour);  // days are indexed from 1
                REQUIRE(0 == utc_tm.tm_min);  // days are indexed from 1
                REQUIRE(0 == utc_tm.tm_sec);  // days are indexed from 1
            }
        }
        WHEN("the timestamp is converted to a time point and back") {
            const auto time_point = sensory::util::timestamp_to_timepoint(timestamp);
            const auto encoded = sensory::util::timepoint_to_timestamp(time_point);
            THEN("the time point does not change") {
                REQUIRE_THAT(timestamp, Catch::Equals(encoded));
            }
        }
    }
    GIVEN("an arbitrary timestamp in UTC ISO8601 format") {
        std::string timestamp = "1994-11-17T03:23:37Z";
        WHEN("the timestamp is converted to a timepoint") {
            const auto time_point = sensory::util::timestamp_to_timepoint(timestamp);
            THEN("the timepoint data matches the timestamp") {
                std::time_t tt = std::chrono::system_clock::to_time_t(time_point);
                std::tm utc_tm = *gmtime(&tt);
                REQUIRE(1994 == 1900 + utc_tm.tm_year);  // time starts at 1900
                REQUIRE(11 == 1 + utc_tm.tm_mon);  // months are indexed from 0
                REQUIRE(17 == utc_tm.tm_mday);  // days are indexed from 1
                REQUIRE(3 == utc_tm.tm_hour);  // days are indexed from 1
                REQUIRE(23 == utc_tm.tm_min);  // days are indexed from 1
                REQUIRE(37 == utc_tm.tm_sec);  // days are indexed from 1
            }
        }
        WHEN("the timestamp is converted to a time point and back") {
            const auto time_point = sensory::util::timestamp_to_timepoint(timestamp);
            const auto encoded = sensory::util::timepoint_to_timestamp(time_point);
            THEN("the time point does not change") {
                REQUIRE_THAT(timestamp, Catch::Equals(encoded));
            }
        }
    }
}
