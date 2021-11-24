// The Sensory Cloud C++ SDK Hello, World!
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

#include <iostream>
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/management_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>
#include <sensorycloud/services/audio_service.hpp>
#include <sensorycloud/services/video_service.hpp>
#include <sensorycloud/token_manager/uuid.hpp>
#include <sensorycloud/token_manager/secure_random.hpp>
#include <sensorycloud/token_manager/keychain.hpp>
#include <sensorycloud/token_manager/time.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>

int main() {
    // Initialize the configuration to the host for given address and port
    sensory::Config config("io.stage.cloud.sensory.com", 443);
    std::cout << config.getFullyQualifiedDomainName() << std::endl;
    // Set the Tenant ID for the default tenant
    config.tenantID = "cabb7700-206f-4cc7-8e79-cd7f288aa78d";
    // a dummy device ID for enrolling in the cloud
    config.deviceID = "D895F447-91E8-486F-A783-6E3A33E4C7C5";

    sensory::token_manager::Keychain keychain("com.sensory.cloud");
    //
    // keychain.remove("clientID");
    // keychain.remove("clientSecret");
    //
    // keychain.insert("clientID", sensory::token_manager::uuid_v4());
    // std::cout << keychain.get("clientID") << std::endl;
    //
    // keychain.insert("clientSecret", sensory::token_manager::secure_random<16>());
    // std::cout << keychain.get("clientSecret") << std::endl;

    auto oauthService = sensory::service::OAuthService(config);

    std::string userID = "";
    std::cout << "user ID: ";
    std::cin >> userID;

    std::string password = "";
    std::cout << "password: ";
    std::cin >> password;

    const auto clientID = keychain.get("clientID");
    const auto clientSecret = keychain.get("clientSecret");

    std::cout << "Hello, Sensory Cloud C++ SDK!" << std::endl;

    if (false) {
        const auto rsp = oauthService.enrollDevice(userID, password, clientID, clientSecret);
        std::cout << "Your user name is \"" << rsp.name() << "\"" << std::endl;
        std::cout << "Your device ID is \"" << rsp.deviceid() << "\"" << std::endl;
    }

    const auto rsp = oauthService.getToken(clientID, clientSecret);
    std::cout << "Your current token is " << rsp.accesstoken() << std::endl;

    // sensory::token_manager::TokenManager<sensory::token_manager::Keychain> token_manager(oauthService, keychain);
    // const auto access_token = token_manager.getAccessToken();
}
