// The Sensory Cloud C++ SDK
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

#ifndef SENSORY_CLOUD_HPP_
#define SENSORY_CLOUD_HPP_

#include "./config.hpp"
#include "./services/health_service.hpp"
#include "./services/oauth_service.hpp"
#include "./services/management_service.hpp"
#include "./services/audio_service.hpp"
#include "./services/video_service.hpp"
#include "./token_manager/token_manager.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief The Sensory Cloud service.
/// @tparam CredentialStore The type for the secure credential store.
template<typename CredentialStore>
struct SensoryCloud {
    /// The configuration for the remote service.
    const ::sensory::Config& config;
    /// The OAuth service.
    ::sensory::service::OAuthService oauthService;
    /// The token manager.
    ::sensory::token_manager::TokenManager<CredentialStore> tokenManager;
    /// The health service.
    ::sensory::service::HealthService healthService;
    /// The management service.
    ::sensory::service::ManagementService<CredentialStore> mgmtService;
    /// The video service.
    ::sensory::service::VideoService<CredentialStore> videoService;
    /// The audio service.
    ::sensory::service::AudioService<CredentialStore> audioService;

    /// @brief Initialize the Sensory Cloud service.
    ///
    /// @param config_ The config for the remote service.
    /// @param keychain The secure credential store.
    ///
    SensoryCloud(const ::sensory::Config& config_, const CredentialStore& keychain) :
        config(config_),
        oauthService(config),
        healthService(config),
        tokenManager(oauthService, keychain),
        mgmtService(config, tokenManager),
        videoService(config, tokenManager),
        audioService(config, tokenManager) { }
};

}  // namespace sensory

#endif  // SENSORY_CLOUD_HPP_
