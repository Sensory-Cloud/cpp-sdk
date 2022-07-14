// An OAuth Token manager for the SensoryCloud C++ SDK.
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

#ifndef SENSORYCLOUD_TOKEN_MANAGER_TOKEN_MANAGER_HPP_
#define SENSORYCLOUD_TOKEN_MANAGER_TOKEN_MANAGER_HPP_

#include <mutex>
#include "sensorycloud/token_manager/uuid.hpp"
#include "sensorycloud/token_manager/secure_random.hpp"
#include "sensorycloud/token_manager/time.hpp"
#include "sensorycloud/services/oauth_service.hpp"

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Modules for generating and storing secure credentials.
namespace token_manager {

/// @brief A wrapper struct for OAuth token credentials.
struct AccessTokenCredentials {
    /// The OAuth client id.
    const std::string id;
    /// The OAuth client secret.
    const std::string secret;
};

/// @brief Key-value tags used to store OAuth credentials for SensoryCloud.
static const struct {
    /// The ID of the client device (A RFC-4122v4 UUID).
    const std::string ClientID     = "clientID";
    /// The client secret (a cryptographically secure random number).
    const std::string ClientSecret = "clientSecret";
    /// The OAuth token from the server.
    const std::string AccessToken  = "accessToken";
    /// The expiration time of the OAuth token.
    const std::string Expiration   = "expiration";
} TAGS;

/// @brief A token manager class for generating OAuth tokens.
/// @tparam SecureCredentialStore A secure key-value store for storing and
/// fetching credentials and tokens.
template<typename SecureCredentialStore>
class TokenManager {
 private:
    /// the OAuth service to get secure tokens from the remote host
    ::sensory::service::OAuthService& service;
    /// the key-chain to interact with to store / query key-value pairs
    SecureCredentialStore& keychain;

    /// @brief Create a copy of this object.
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object.
    ///
    TokenManager(const TokenManager& other) = delete;

    /// @brief Assign to this object using the `=` operator.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This assignment operator is private to prevent copying of this object.
    ///
    void operator=(const TokenManager& other) = delete;

 public:
    /// @brief Initialize a new token manager.
    ///
    /// @param service_ The OAuth service for requesting new tokens.
    /// @param keychain_ The keychain to query secure credentials from.
    ///
    TokenManager(
        ::sensory::service::OAuthService& service_,
        SecureCredentialStore& keychain_
    ) : service(service_), keychain(keychain_) { }

    /// @brief Generate a new set of OAuth credentials and store them in the
    /// keychain.
    ///
    /// @returns The generated OAuth credentials.
    /// @throws An error if credentials cannot be securely generated or if the
    /// credentials cannot be stored in the keychain.
    ///
    /// @details
    /// This function will overwrite any other credentials that have been
    /// generated using this function, i.e., the `clientID` and `clientSecret`
    /// in the keychain.
    ///
    inline AccessTokenCredentials generate_credentials() const {
        // Generate a new client ID and secure random secret string.
        const auto clientID = uuid_v4();  // v4 UUIDs don't identify the host.
        const auto secret = secure_random<24>();  // Use a 24 character secret.
        // Insert the clientID and secret into the persistent credential store.
        // If any key-value pair already exists, overwrite it.
        keychain.emplace(TAGS.ClientID, clientID);
        keychain.emplace(TAGS.ClientSecret, secret);
        // Return a new access token with the credentials
        return {clientID, secret};
    }

    /// @brief Return the stored credentials.
    ///
    /// @returns The `clientID` and `clientSecret` from the keychain in an
    /// `AccessTokenCredentials` instance.
    ///
    inline AccessTokenCredentials get_saved_credentials() const {
        return {keychain.at(TAGS.ClientID), keychain.at(TAGS.ClientSecret)};
    }

    /// @brief Determine if client ID and client secret are stored on device.
    ///
    /// @returns `true` if a credential pair is found, `false` otherwise.
    ///
    /// @details
    /// This function checks for the existence of the `clientID` and
    /// `clientSecret` keys in the keychain.
    ///
    inline bool has_saved_credentials() const {
        return keychain.contains(TAGS.ClientID) &&
            keychain.contains(TAGS.ClientSecret);
    }

    /// @brief Determine if any token is stored on the device.
    ///
    /// @returns `true` if a token is found, `false` otherwise
    ///
    /// @details
    /// This function checks for the existence of the `accessToken` and
    /// `expiration` keys in the keychain.
    ///
    inline bool has_token() const {
        return keychain.contains(TAGS.AccessToken) &&
            keychain.contains(TAGS.Expiration);
    }

    /// @brief Delete any credentials stored for requesting access tokens, as
    /// well as any cached access tokens on device.
    ///
    /// @details
    /// This will erase the `clientID`, `clientSecret`, `accessToken`, and
    /// `expiration` keys-value pairs from the secure credential store.
    ///
    inline void delete_credentials() const {
        keychain.erase(TAGS.AccessToken);
        keychain.erase(TAGS.Expiration);
        keychain.erase(TAGS.ClientID);
        keychain.erase(TAGS.ClientSecret);
    }

    /// @brief Return a valid access token for SensoryCloud gRPC calls.
    ///
    /// @returns A valid access token
    /// @throws An error if one occurs while retrieving the saved token, or if
    /// an error occurs while requesting a new one.
    /// @details
    /// This function will immediately return if the cached access token is
    /// still valid. If a new token needs to be requested, this function will
    /// block on the current thread until a new token has been fetched from
    /// the server.
    ///
    std::string get_access_token() const {
        // Prevent multiple access tokens from being requested at the same time.
        static std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);

        if (!has_token())  // no access token has been generated and stored
            return fetch_new_access_token();
        // fetch existing access token and expiration date from the secure store
        const auto access_token = keychain.at(TAGS.AccessToken);
        const auto expiration_date = keychain.at(TAGS.Expiration);
        // check for expiration of the token
        const auto now = std::chrono::system_clock::now();
        const auto expiration = timestamp_to_timepoint(expiration_date);
        if (now > expiration - std::chrono::minutes(5))  // token has expired
            return fetch_new_access_token();
        return access_token;
    }

    /// @brief Fetch a new access token from a remote server.
    ///
    /// @returns The new token as a string.
    ///
    std::string fetch_new_access_token() const {
        // Check if credentials have been generated already, otherwise create
        // new credentials.
        if (!has_saved_credentials()) generate_credentials();
        // Get the ID of the client and the secret from the secure store.
        const auto client_id = keychain.at(TAGS.ClientID);
        const auto secret = keychain.at(TAGS.ClientSecret);
        // Synchronously request a new token from the server.
        api::common::TokenResponse response;
        const auto status = service.getToken(&response, client_id, secret);
        // Insert the OAuth access token for the client in the secure store
        keychain.emplace(TAGS.AccessToken, response.accesstoken());
        // Determine when the token will expire and store this time
        const auto expiration_date =
            std::chrono::system_clock::now() + std::chrono::seconds(response.expiresin());
        keychain.emplace(TAGS.Expiration, timepoint_to_timestamp(expiration_date));
        // Return the newly created OAuth token
        return response.accesstoken();
    }

    /// @brief Setup an existing client context for unary gRPC calls.
    ///
    /// @param context The context to setup with a Bearer token and deadline.
    /// @param token_manager The token manager for fetching OAuth tokens.
    ///
    inline void setup_unary_client_context(::grpc::ClientContext& context) const {
        context.AddMetadata("authorization",
            std::string("Bearer ") + get_access_token()
        );
        context.set_deadline(service.get_config().get_deadline());
    }

    /// @brief Setup an existing client context for bidirectional gRPC streams.
    ///
    /// @param context The context to setup with a Bearer token and deadline.
    /// @param token_manager The token manager for fetching OAuth tokens.
    ///
    inline void setup_bidi_client_context(::grpc::ClientContext& context) const {
        context.AddMetadata("authorization",
            std::string("Bearer ") + get_access_token()
        );
    }
};

}  // namespace token_manager

}  // namespace sensory

#endif  // SENSORYCLOUD_TOKEN_MANAGER_TOKEN_MANAGER_HPP_
