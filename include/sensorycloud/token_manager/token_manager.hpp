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
#include "sensorycloud/token_manager/keychain.hpp"
#include "sensorycloud/services/oauth_service.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Modules for generating and storing secure credentials.
namespace token_manager {

/// @brief A wrapper struct for OAuth token credentials.
struct AccessTokenCredentials {
    /// The OAuth client id
    std::string clientID;
    /// The OAuth client secret
    std::string secret;
};

/// @brief A token manager class for generating OAuth tokens.
/// @tparam SecureCredentialStore a secure CRUD class for storing credentials.
template<typename SecureCredentialStore>
class TokenManager {
 private:
    /// the OAuth service to get secure tokens from the remote host
    service::OAuthService& service;
    /// the key-chain to interact with to store / query key-value pairs
    SecureCredentialStore& keychain;

 public:
    /// @brief String keys for accessing secure values in the keychain
    enum class KeychainTag {
        /// the ID of the client device (A RFC-4122v4 UUID)
        ClientID     = "clientID",
        /// the client secret (a cryptographically secure random number)
        ClientSecret = "clientSecret",
        /// the OAuth token from the server
        AccessToken  = "accessToken",
        /// the expiration time of the OAuth token
        Expiration   = "expiration"
    };

    /// @brief Initialize a new token manager.
    ///
    /// @param service_ the OAuth service for requesting new tokens
    /// @param keychain_ the key-chain to query secure credentials from
    ///
    explicit TokenManager(
        service::OAuthService& service_,
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
        keychain.insert(KeychainTag::ClientID, clientID);
        keychain.insert(KeychainTag::ClientSecret, secret);
        // Return a new access token with the credentials
        return AccessTokenCredentials{clientID, secret};
    }

    /// @brief Determine if any credentials are stored on device.
    ///
    /// @returns `true` if any credentials are found, `false` otherwise
    ///
    inline bool hasSavedCredentials() const {
        return keychain.has(KeychainTag::ClientID) && keychain.has(KeychainTag::ClientSecret);
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
        static constexpr std::chrono::seconds tokenExpirationBuffer = std::chrono::seconds(300);

        // TODO: Prevent multiple access tokens from being requested at the same time

        if (
            !keychain.has(KeychainTag::AccessToken) ||
            !keychain.has(KeychainTag::Expiration)
        )  // no access token has been stored in the credential store
            return fetchNewAccessToken();
        // fetch existing access token and expiration date from the secure store
        const auto accessToken = keychain.get(KeychainTag::AccessToken);
        const auto expirationDate = keychain.get(KeychainTag::Expiration);
        // check for expiration of the token
        const auto now = std::chrono::system_clock::now();
        const auto expiration = timestamp_to_timepoint(expirationDate);
        if (now > expiration - tokenExpirationBuffer) {  // token has expired
            // std::cout << "Cached access token has expired, requesting new token" << std::endl;
            return fetchNewAccessToken();
        }
        // std::cout << "Returning cached access token" << std::endl;
        return accessToken;
    }

    /// @brief Delete any credentials stored for requesting access tokens, as
    /// well as any cached access tokens on device.
    ///
    inline void deleteCredentials() const {
        keychain.remove(KeychainTag::AccessToken);
        keychain.remove(KeychainTag::Expiration);
        keychain.remove(KeychainTag::ClientID);
        keychain.remove(KeychainTag::ClientSecret);
    }

    /// @brief Fetch a new access token from a remote server.
    ///
    /// @returns the new token as a string
    ///
    std::string fetchNewAccessToken() const {
        if (!keychain.has(KeychainTag::ClientID)) {
            // TODO: raise exception
        }
        if (!keychain.has(KeychainTag::ClientSecret)) {
            // TODO: raise exception
        }
        // Get the ID of the client and the secret from the secure store.
        const auto clientID = keychain.get(KeychainTag::ClientID);
        const auto secret = keychain.get(KeychainTag::ClientSecret);
        // Synchronously request a new token from the server.
        const auto result = service.getToken(clientID, secret);
        // Insert the OAuth access token for the client in the secure store
        keychain.insert(KeychainTag::AccessToken, result.accessToken);
        // Determine when the token will expire and store this time
        const auto expirationDate =
            std::chrono::system_clock::now() + std::chrono::seconds(result.expiresIn);
        const auto expiration = timepoint_to_timestamp(expirationDate);
        keychain.insert(KeychainTag::Expiration, expiration);
        // Return the newly created OAuth token
        return result.accessToken;
    }
};

}  // namespace token_manager

}  // namespace sensory

#endif  // SENSORY_CLOUD_TOKEN_MANAGER_TOKEN_MANAGER_HPP_
