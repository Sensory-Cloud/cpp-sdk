// Test cases for the sensory::calldata::CallbackData structure.
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
#include "sensorycloud/calldata/callback_data.hpp"
#include "sensorycloud/generated/health/health.pb.h"
#include "sensorycloud/generated/health/health.grpc.pb.h"

/// @brief A dummy type acting as the encapsulating type of the reactor.
struct MockCallbackDataFriend {
    /// @brief Set the is done flag of the call data instance.
    ///
    /// @tparam T The MockCallbackData type (templated instead of forward declared).
    /// @param t The MockCallbackData instance.
    ///
    /// @details
    /// This function is intended to be used with MockCallbackDataFriend as the
    /// friend parent of the MockCallbackData. This allows the scope of the
    /// MockCallbackDataFriend to extend into the private scope of CallbackData in
    /// order to call private functions and mutate types.
    ///
    template <typename T>
    inline static void setIsDone(T& t) { t.setIsDone(); }
};
/// @brief The mock call data to test based on arbitrary SDK messages.
typedef sensory::calldata::CallbackData<
    MockCallbackDataFriend,                       // Set the friend (parent) type to a dummy value
    ::sensory::api::health::HealthRequest,        // Use an arbitrary request from the SDK
    ::sensory::api::common::ServerHealthResponse  // Use an arbitrary response from the SDK
> MockCallbackData;

TEST_CASE("When the call data is in its initial status, isDone should be false") {
    MockCallbackData callData;
    WHEN("The call data is in its initial state") {
        THEN("getIsDone evaluates to false") {
            REQUIRE_FALSE(callData.getIsDone());
        }
        THEN("The status is ok") {
            REQUIRE(callData.getStatus().ok());
        }
    }
}

SCENARIO("A user wants to wait for the callback to fire from a CallbackData") {
    GIVEN("an arbitrary call data structure") {
        MockCallbackData callData;
        WHEN("The onDone callback is triggered synchronously") {
            MockCallbackDataFriend::setIsDone(callData);
            THEN("getIsDone evaluates to true") {
                REQUIRE(callData.getIsDone());
            }
        }
        WHEN("The onDone callback is triggered asynchronously") {
            // Trigger the callback in the background thread.
            std::thread thread([&callData](){
                MockCallbackDataFriend::setIsDone(callData);
            });
            // Wait for the OnDone callback to trigger
            callData.await();
            // Join the background thread back in
            thread.join();
            // Check the outputs to the call data and from await().
            THEN("getIsDone evaluates to true") {
                REQUIRE(callData.getIsDone());
            }
        }
    }
}
