// An OAuth Token manager for the Sensory Cloud C++ SDK.
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
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXTERNRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef SENSORY_CLOUD_TOKEN_MANAGER_TOKEN_MANAGER_HPP_
#define SENSORY_CLOUD_TOKEN_MANAGER_TOKEN_MANAGER_HPP_

#include "sensorycloud/token_manager/uuid.hpp"
#include "sensorycloud/token_manager/secure_random.hpp"
#include "sensorycloud/token_manager/time.hpp"
#include "sensorycloud/services/oauth_service.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Modules for generating and storing secure credentials.
namespace token_manager {

/// @brief A wrapper struct for OAuth token credentials.
struct AccessTokenCredentials {
    /// The OAuth client id
    std::string id;
    /// The OAuth client secret
    std::string secret;
};

// #define ClientID "clientID"
// #define ClientSecret "clientSecret"
// #define AccessToken "accessToken"
// #define Expiration "expiration"

/// @brief Keychain tags used to store OAuth credentials for Sensory Cloud.
static const struct {
    /// the ID of the client device (A RFC-4122v4 UUID)
    const std::string ClientID     = "clientID";
    /// the client secret (a cryptographically secure random number)
    const std::string ClientSecret = "clientSecret";
    /// the OAuth token from the server
    const std::string AccessToken  = "accessToken";
    /// the expiration time of the OAuth token
    const std::string Expiration   = "expiration";
} TAGS;

/// @brief A token manager class for generating OAuth tokens.
/// @tparam SecureCredentialStore a secure CRUD class for storing credentials.
template<typename SecureCredentialStore>
class TokenManager {
 private:
    /// the OAuth service to get secure tokens from the remote host
    ::sensory::service::OAuthService& service;
    /// the key-chain to interact with to store / query key-value pairs
    SecureCredentialStore& keychain;

 public:
    /// @brief Initialize a new token manager.
    ///
    /// @param service_ the OAuth service for requesting new tokens
    /// @param keychain_ the key-chain to query secure credentials from
    ///
    explicit TokenManager(
        ::sensory::service::OAuthService& service_,
        SecureCredentialStore& keychain_
    ) : service(service_), keychain(keychain_) { }

    /// @brief Generate and store a new set of OAuth credentials.
    ///
    /// @returns The generated OAuth credentials
    /// @throws An error if credentials cannot be securely generated or if the
    /// credentials cannot be stored in the Keychain.
    /// @details
    /// This function will overwrite any other credentials that have been
    /// generated using this function.
    ///
    inline AccessTokenCredentials generateCredentials() const {
        // Generate a new client ID and secure random secrete string
        const auto clientID = uuid_v4();
        const auto secret = secure_random<16>();
        // Insert the clientID and secret into the persistent credential store.
        // If any key-value pair already exists, overwrite it.
        keychain.emplace(TAGS.ClientID, clientID);
        keychain.emplace(TAGS.ClientSecret, secret);
        // Return a new access token with the credentials
        return AccessTokenCredentials{clientID, secret};
    }

    /// @brief Determine if a credentials pair is stored on the device.
    ///
    /// @returns `true` if a credential pair is found, `false` otherwise
    ///
    inline bool hasSavedCredentials() const {
        return keychain.contains(TAGS.ClientID) && keychain.contains(TAGS.ClientSecret);
    }

    /// @brief Determine if any token is stored on the device.
    ///
    /// @returns `true` if a token is found, `false` otherwise
    ///
    inline bool hasToken() const {
        return keychain.contains(TAGS.AccessToken) && keychain.contains(TAGS.Expiration);
    }

    /// @brief Delete any credentials stored for requesting access tokens, as
    /// well as any cached access tokens on device.
    ///
    inline void deleteCredentials() const {
        keychain.erase(TAGS.AccessToken);
        keychain.erase(TAGS.Expiration);
        keychain.erase(TAGS.ClientID);
        keychain.erase(TAGS.ClientSecret);
    }

    /// @brief Return a valid access token for Sensory Cloud gRPC calls.
    ///
    /// @throws An error if one occurs while retrieving the saved token, or if
    /// an error occurs while requesting a new one
    /// @returns A valid access token
    /// @details
    /// This function will immediately return if the cached access token is
    /// still valid. If a new token needs to be requested, this function will
    /// block on the current thread until a new token has been fetched from
    /// the server.
    ///
    std::string getAccessToken() const {
        // TODO: Prevent multiple access tokens from being requested at the same time

        if (!hasToken())  // no access token has been generated and stored
            return fetchNewAccessToken();
        // fetch existing access token and expiration date from the secure store
        const auto accessToken = keychain.at(TAGS.AccessToken);
        const auto expirationDate = keychain.at(TAGS.Expiration);
        // check for expiration of the token
        const auto now = std::chrono::system_clock::now();
        const auto expiration = timestamp_to_timepoint(expirationDate);
        if (now > expiration - std::chrono::minutes(5)) {  // token has expired
            return fetchNewAccessToken();
        }
        return accessToken;
    }

    /// @brief Fetch a new access token from a remote server.
    ///
    /// @returns the new token as a string
    ///
    std::string fetchNewAccessToken() const {
        // Check if credentials have been generated already, otherwise create
        // new credentials.
        if (!hasSavedCredentials()) generateCredentials();
        // Get the ID of the client and the secret from the secure store.
        const auto clientID = keychain.at(TAGS.ClientID);
        const auto secret = keychain.at(TAGS.ClientSecret);
        // Synchronously request a new token from the server.
        api::common::TokenResponse response;
        const auto status = service.getToken(&response, clientID, secret);
        // Insert the OAuth access token for the client in the secure store
        keychain.emplace(TAGS.AccessToken, response.accesstoken());
        // Determine when the token will expire and store this time
        const auto expirationDate =
            std::chrono::system_clock::now() + std::chrono::seconds(response.expiresin());
        const auto expiration = timepoint_to_timestamp(expirationDate);
        keychain.emplace(TAGS.Expiration, expiration);
        // Return the newly created OAuth token
        return response.accesstoken();
    }
};

}  // namespace token_manager

}  // namespace sensory

#endif  // SENSORY_CLOUD_TOKEN_MANAGER_TOKEN_MANAGER_HPP_
