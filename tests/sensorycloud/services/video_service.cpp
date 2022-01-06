// Test cases for the video service.
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
#include "sensorycloud/services/video_service.hpp"
#include "sensorycloud/services/oauth_service.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/token_manager/insecure_credential_store.hpp"

using sensory::Config;
using sensory::token_manager::InsecureCredentialStore;
using sensory::token_manager::TokenManager;
using sensory::service::OAuthService;
using sensory::service::VideoService;

TEST_CASE("Should create VideoService from Config and TokenManager") {
    // Create the configuration that provides information about the remote host.
    Config config("hostname.com", 443, "tenant ID", "device ID");
    // Create the OAuth service for requesting and managing OAuth tokens through
    // a token manager instance.
    OAuthService oauthService(config);
    // Create a credential store for keeping the clientID, clientSecret,
    // token, and expiration time.
    InsecureCredentialStore keychain(".", "com.sensory.cloud.examples");
    TokenManager<InsecureCredentialStore> tokenManager(oauthService, keychain);
    // Create the actual video service from the config and token manager.
    VideoService<InsecureCredentialStore> service(config, tokenManager);
}
