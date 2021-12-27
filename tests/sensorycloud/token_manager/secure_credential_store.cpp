// Test cases for the sensory::token_manager::SecureCredentialStore.
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
#include "sensorycloud/token_manager/secure_credential_store.hpp"

SCENARIO("a user wants to securely store credentials on disk") {
    GIVEN("an application ID") {
        const std::string package("com.sensory.test");
        sensory::token_manager::SecureCredentialStore store(package);
        WHEN("contains is called with a non-existent key") {
            THEN("false is returned") {
                REQUIRE_FALSE(store.contains("foo"));
            }
        }
        WHEN("erase is called with a non-existent key") {
            THEN("no error occurs") {
                store.erase("non-existent-key");
            }
        }
        WHEN("at is called with a non-existent key") {
            THEN("an empty string is returned") {
                REQUIRE("" == store.at("non-existent-key"));
            }
        }
        WHEN("a key-value pair is inserted into the secure store") {
            store.emplace("foo", "bar");
            THEN("contains returns true") {
                REQUIRE(store.contains("foo"));
            }
            THEN("at returns the value") {
                REQUIRE("bar" == store.at("foo"));
            }
            THEN("emplace overwrites the key-value pair") {
                store.emplace("foo", "zar");
                REQUIRE("zar" == store.at("foo"));
            }
            THEN("erase removes the key from the store") {
                store.erase("foo");
                REQUIRE_FALSE(store.contains("foo"));
            }
        }
        WHEN("a key store is initialized from a persistent state") {
            const std::string package("com.sensory.test");
            {  // Write the key-value pair within a constrained scope.
                sensory::token_manager::SecureCredentialStore store(package);
                store.emplace("foo", "bar");
            }
            THEN("previously created keys are detected") {
                sensory::token_manager::SecureCredentialStore store(package);
                REQUIRE(store.contains("foo"));
                REQUIRE("bar" == store.at("foo"));
                store.erase("foo");
            }
        }
    }
}
