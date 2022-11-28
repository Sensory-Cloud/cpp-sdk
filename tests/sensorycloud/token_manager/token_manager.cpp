// Test cases for the sensory::token_manager::TokenManager class.
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
#include <unordered_map>
#include <catch2/catch.hpp>
#include "sensorycloud/config.hpp"
#include "sensorycloud/services/oauth_service.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/token_manager/in_memory_credential_store.hpp"

using sensory::Config;
using sensory::service::OAuthService;
using sensory::token_manager::TokenManager;
using sensory::token_manager::TAGS;
using sensory::token_manager::InMemoryCredentialStore;

SCENARIO("a user wants to create a TokenManager based on an STL key-store") {
    GIVEN("an initialized STL key-value store object and an OAuthService") {
        Config config("localhost", 50051, "tenantID", "deviceID");
        OAuthService oauth_service(config);
        InMemoryCredentialStore keychain;
        WHEN("a TokenManager is initialized with an empty key-value store") {
            TokenManager<InMemoryCredentialStore> token_manager(oauth_service, keychain);
            THEN("the token manager has no credentials stored") {
                REQUIRE_FALSE(token_manager.has_saved_credentials());
            }
            THEN("get_saved_credentials throws an error") {
                REQUIRE_THROWS(token_manager.get_saved_credentials());
            }
            THEN("the token manager has no token stored") {
                REQUIRE_FALSE(token_manager.has_token());
            }
        }
        WHEN("a TokenManager is initialized with credentials in the key-value store") {
            keychain.emplace(TAGS.ClientID, "foo");
            keychain.emplace(TAGS.ClientSecret, "bar");
            TokenManager<InMemoryCredentialStore> token_manager(oauth_service, keychain);
            THEN("the token manager has credentials stored") {
                REQUIRE(token_manager.has_saved_credentials());
            }
            THEN("get_saved_credentials returns the credentials") {
                auto credentials = token_manager.get_saved_credentials();
                REQUIRE_THAT(credentials.id, Catch::Equals(keychain.at(TAGS.ClientID)));
                REQUIRE_THAT(credentials.secret, Catch::Equals(keychain.at(TAGS.ClientSecret)));
            }
            THEN("the token manager has no token stored") {
                REQUIRE_FALSE(token_manager.has_token());
            }
        }
        WHEN("a TokenManager is initialized with a token in the key-value store") {
            keychain.emplace(TAGS.AccessToken, "foo");
            keychain.emplace(TAGS.Expiration, "bar");
            TokenManager<InMemoryCredentialStore> token_manager(oauth_service, keychain);
            THEN("the token manager has no credentials stored") {
                REQUIRE_FALSE(token_manager.has_saved_credentials());
            }
            THEN("get_saved_credentials throws an error") {
                REQUIRE_THROWS(token_manager.get_saved_credentials());
            }
            THEN("the token manager has a token stored") {
                REQUIRE(token_manager.has_token());
            }
        }
    }
}

SCENARIO("a user wants to generate credentials in an empty secure store") {
    GIVEN("an initialized TokenManager") {
        Config config("localhost", 50051, "tenantID", "deviceID");
        OAuthService oauth_service(config);
        InMemoryCredentialStore keychain;
        TokenManager<InMemoryCredentialStore> token_manager(oauth_service, keychain);
        WHEN("credentials are generated") {
            const auto credentials = token_manager.generate_credentials();
            THEN("The returned credentials should be in the key-value store") {
                REQUIRE_THAT(credentials.id, Catch::Equals(keychain.at(TAGS.ClientID)));
                REQUIRE_THAT(credentials.secret, Catch::Equals(keychain.at(TAGS.ClientSecret)));
            }
            THEN("The returned client ID should be a length 36 UUIDv4") {
                REQUIRE(36 == credentials.id.length());
            }
            THEN("The returned client secret should be a length 24 secure random") {
                REQUIRE(24 == credentials.secret.length());
            }
        }
    }
}

SCENARIO("a user wants to overwrite credentials in a secure store") {
    GIVEN("an initialized TokenManager with existing client ID and secret") {
        Config config("localhost", 50051, "tenantID", "deviceID");
        OAuthService oauth_service(config);
        InMemoryCredentialStore keychain;
        keychain.emplace(TAGS.ClientID, "foo");
        keychain.emplace(TAGS.ClientSecret, "bar");
        TokenManager<InMemoryCredentialStore> token_manager(oauth_service, keychain);
        WHEN("new credentials are generated") {
            const auto credentials = token_manager.generate_credentials();
            THEN("The returned credentials should be in the key-value store") {
                REQUIRE_THAT(credentials.id, Catch::Equals(keychain.at(TAGS.ClientID)));
                REQUIRE_THAT(credentials.secret, Catch::Equals(keychain.at(TAGS.ClientSecret)));
            }
            THEN("The returned client ID should be a length 36 UUIDv4") {
                REQUIRE(36 == credentials.id.length());
            }
            THEN("The returned client secret should be a length 24 secure random") {
                REQUIRE(24 == credentials.secret.length());
            }
        }
    }
}

SCENARIO("a user wants to erase credentials in a secure store") {
    GIVEN("an initialized TokenManager with existing all keys") {
        Config config("localhost", 50051, "tenantID", "deviceID");
        OAuthService oauth_service(config);
        InMemoryCredentialStore keychain;
        keychain.emplace(TAGS.ClientID, "foo");
        keychain.emplace(TAGS.ClientSecret, "bar");
        keychain.emplace(TAGS.AccessToken, "baz");
        keychain.emplace(TAGS.Expiration, "fee");
        const std::string ARB_KEY = "arb";
        const std::string ARB_VALUE = "asdf";
        keychain.emplace(ARB_KEY, ARB_VALUE);
        TokenManager<InMemoryCredentialStore> token_manager(oauth_service, keychain);
        WHEN("credentials are erased") {
            token_manager.delete_credentials();
            THEN("The keychain is cleared of the values") {
                REQUIRE_FALSE(keychain.contains(TAGS.ClientID));
                REQUIRE_FALSE(keychain.contains(TAGS.ClientSecret));
                REQUIRE_FALSE(keychain.contains(TAGS.AccessToken));
                REQUIRE_FALSE(keychain.contains(TAGS.Expiration));
            }
            THEN("Other values in the keychain are unchanged") {
                REQUIRE(keychain.contains(ARB_KEY));
                REQUIRE_THAT(ARB_VALUE, Catch::Equals(keychain.at(ARB_KEY)));
            }
        }
    }
}
