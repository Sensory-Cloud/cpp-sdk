// Test cases for the sensory::calldata::AsyncResponseReaderCall structure.
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
#include "sensorycloud/calldata/async_response_reader_call.hpp"
#include "sensorycloud/generated/health/health.pb.h"
#include "sensorycloud/generated/health/health.grpc.pb.h"

/// @brief A dummy type acting as the encapsulating type of the async stream.
struct MockAsyncResponseReaderCallFriend { };

/// @brief The mock call data to test based on arbitrary SDK messages.
typedef sensory::calldata::AsyncResponseReaderCall<
    MockAsyncResponseReaderCallFriend,            // Set the friend (parent) type to a dummy value
    ::sensory::api::health::HealthRequest,        // Use an arbitrary request from the SDK
    ::sensory::api::common::ServerHealthResponse  // Use an arbitrary response from the SDK
> MockAsyncResponseReaderCall;

TEST_CASE("When MockAsyncResponseReaderCall is in its initial state, the call pointer is null") {
    MockAsyncResponseReaderCall stream;
    REQUIRE(nullptr == stream.getCall());
}
