// Test cases for the TokenManager in the sensory::token_manager namespace.
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
#include <unordered_map>
#include <catch2/catch.hpp>
#include "sensorycloud/config.hpp"
#include "sensorycloud/services/oauth_service.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"

using sensory::Config;
using sensory::service::OAuthService;
using sensory::token_manager::TokenManager;
using sensory::token_manager::TAGS;

/// @brief A mock secure credential store for testing the TokenManager.
struct SecureCredentialStore : public std::unordered_map<std::string, std::string> {
#if (__cplusplus <= 202002L)  // C++ 2020 and later defines contains for STL maps
    /// @brief Return true if the key exists in the key-value store.
    ///
    /// @returns `true` if the key exists, `false` otherwise.
    ///
    inline bool contains(const std::string& key) const {
        return find(key) != end();
    }
#endif

    /// @brief Emplace or replace a key/value pair in the key-chain.
    ///
    /// @param key the plain-text key of the value to store
    /// @param value the secure value to store
    ///
    inline void emplace(const std::string& key, const std::string& value) {
        if (contains(key)) erase(key);
        std::unordered_map<std::string, std::string>::emplace(key, value);
    }
};

SCENARIO("a user wants to create a TokenManager based on an STL key-store") {
    GIVEN("an initialized STL key-value store object and an OAuthService") {
        Config config(
            "localhost",
            50051,
            "tenantID",
            "deviceID"
        );
        config.connect();

        OAuthService oauth_service(config);
        SecureCredentialStore keychain;
        WHEN("a TokenManager is initialized with an empty key-value store") {
            TokenManager<SecureCredentialStore> tokenManager(oauth_service, keychain);
            THEN("the token manager has no credentials stored") {
                REQUIRE_FALSE(tokenManager.hasSavedCredentials());
            }
            THEN("the token manager has no token stored") {
                REQUIRE_FALSE(tokenManager.hasToken());
            }
        }
        WHEN("a TokenManager is initialized with credentials in the key-value store") {
            keychain.emplace(TAGS.ClientID, "foo");
            keychain.emplace(TAGS.ClientSecret, "bar");
            TokenManager<SecureCredentialStore> tokenManager(oauth_service, keychain);
            THEN("the token manager has no credentials stored") {
                REQUIRE(tokenManager.hasSavedCredentials());
            }
            THEN("the token manager has no token stored") {
                REQUIRE_FALSE(tokenManager.hasToken());
            }
        }
        WHEN("a TokenManager is initialized with a token in the key-value store") {
            keychain.emplace(TAGS.AccessToken, "foo");
            keychain.emplace(TAGS.Expiration, "bar");
            TokenManager<SecureCredentialStore> tokenManager(oauth_service, keychain);
            THEN("the token manager has no credentials stored") {
                REQUIRE_FALSE(tokenManager.hasSavedCredentials());
            }
            THEN("the token manager has no token stored") {
                REQUIRE(tokenManager.hasToken());
            }
        }
    }
}

SCENARIO("a user wants to generate credentials in an empty secure store") {
    GIVEN("an initialized TokenManager") {
        Config config(
            "localhost",
            50051,
            "tenantID",
            "deviceID"
        );
        config.connect();

        OAuthService oauth_service(config);
        SecureCredentialStore keychain;
        TokenManager<SecureCredentialStore> tokenManager(oauth_service, keychain);
        WHEN("credentials are generated") {
            const auto credentials = tokenManager.generateCredentials();
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
        Config config(
            "localhost",
            50051,
            "tenantID",
            "deviceID"
        );
        config.connect();

        OAuthService oauth_service(config);
        SecureCredentialStore keychain;
        keychain.emplace(TAGS.ClientID, "foo");
        keychain.emplace(TAGS.ClientSecret, "bar");
        TokenManager<SecureCredentialStore> tokenManager(oauth_service, keychain);
        WHEN("new credentials are generated") {
            const auto credentials = tokenManager.generateCredentials();
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
        Config config(
            "localhost",
            50051,
            "tenantID",
            "deviceID"
        );
        config.connect();

        OAuthService oauth_service(config);
        SecureCredentialStore keychain;
        keychain.emplace(TAGS.ClientID, "foo");
        keychain.emplace(TAGS.ClientSecret, "bar");
        keychain.emplace(TAGS.AccessToken, "baz");
        keychain.emplace(TAGS.Expiration, "fee");
        const std::string ARB_KEY = "arb";
        const std::string ARB_VALUE = "asdf";
        keychain.emplace(ARB_KEY, ARB_VALUE);
        TokenManager<SecureCredentialStore> tokenManager(oauth_service, keychain);
        WHEN("credentials are erased") {
            tokenManager.deleteCredentials();
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
