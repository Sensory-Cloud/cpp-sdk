// Test cases for the sensory::calldata::AwaitableBidiReactor structure.
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
#include "sensorycloud/calldata/awaitable_bidi_reactor.hpp"
#include "sensorycloud/generated/health/health.pb.h"
#include "sensorycloud/generated/health/health.grpc.pb.h"

/// @brief A dummy type acting as the encapsulating type of the reactor.
struct MockBidiReactorFriend { };
/// @brief The mock Bidi reactor to test based on arbitrary SDK messages.
typedef sensory::calldata::AwaitableBidiReactor<
    MockBidiReactorFriend,                        // Set the friend (parent) type to a dummy value
    ::sensory::api::health::HealthRequest,        // Use an arbitrary request from the SDK
    ::sensory::api::common::ServerHealthResponse  // Use an arbitrary response from the SDK
> MockAwaitableBidiReactor;

TEST_CASE("When the reactor is in its initial status, isDone should be false") {
    MockAwaitableBidiReactor reactor;
    WHEN("The reactor is in its initial state") {
        THEN("getIsDone evaluates to false") {
            REQUIRE_FALSE(reactor.getIsDone());
        }
        THEN("The status is ok") {
            REQUIRE(reactor.getStatus().ok());
        }
    }
}

SCENARIO("A user wants to wait for the OnDone callback to fire") {
    GIVEN("an arbitrary reactor") {
        MockAwaitableBidiReactor reactor;
        WHEN("The onDone callback is triggered synchronously") {
            reactor.OnDone({grpc::StatusCode::UNKNOWN, "foo"});
            THEN("getIsDone evaluates to true") {
                REQUIRE(reactor.getIsDone());
            }
            THEN("The reactor status is set") {
                REQUIRE_FALSE(reactor.getStatus().ok());
                REQUIRE(reactor.getStatus().error_code() == grpc::StatusCode::UNKNOWN);
                REQUIRE(reactor.getStatus().error_message() == "foo");
            }
            THEN("await() returns the status") {
                auto status = reactor.await();
                REQUIRE_FALSE(status.ok());
                REQUIRE(status.error_code() == grpc::StatusCode::UNKNOWN);
                REQUIRE(status.error_message() == "foo");
            }
        }
        WHEN("The onDone callback is triggered asynchronously") {
            // Trigger the callback in the background thread.
            std::thread thread([&reactor](){
                reactor.OnDone({grpc::StatusCode::UNKNOWN, "foo"});
            });
            // Wait for the OnDone callback to trigger
            auto status = reactor.await();
            // Join the background thread back in
            thread.join();
            // Check the outputs to the reactor and from await().
            THEN("getIsDone evaluates to true") {
                REQUIRE(reactor.getIsDone());
            }
            THEN("The tractor status is set") {
                REQUIRE_FALSE(reactor.getStatus().ok());
                REQUIRE(reactor.getStatus().error_code() == grpc::StatusCode::UNKNOWN);
                REQUIRE(reactor.getStatus().error_message() == "foo");
            }
            THEN("await() returns the status") {
                REQUIRE_FALSE(status.ok());
                REQUIRE(status.error_code() == grpc::StatusCode::UNKNOWN);
                REQUIRE(status.error_message() == "foo");
            }
        }
    }
}
