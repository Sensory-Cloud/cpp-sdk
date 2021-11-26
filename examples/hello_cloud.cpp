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
#include <sensorycloud/services/health_service.hpp>

int main() {
    std::cout << "Hello, Sensory Cloud C++ SDK!" << std::endl;

    // Initialize the configuration to the host for given address and port
    sensory::Config config(
        "io.stage.cloud.sensory.com",
        443,
        "cabb7700-206f-4cc7-8e79-cd7f288aa78d",
        "D895F447-91E8-486F-A783-6E3A33E4C7C5"
    );
    std::cout << "Connecting to remote host: " << config.getFullyQualifiedDomainName() << std::endl;

    // Query the health of the remote service.
    auto healthService = sensory::service::HealthService(config);
    sensory::api::common::ServerHealthResponse serverHealth;
    auto status = healthService.getHealth(&serverHealth);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "GetHealth failed with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    // Report the health of the remote service
    std::cout << "Server status:" << std::endl;
    std::cout << "\tisHealthy: " << serverHealth.ishealthy() << std::endl;
    std::cout << "\tserverVersion: " << serverHealth.serverversion() << std::endl;
    std::cout << "\tid: " << serverHealth.id() << std::endl;

    // Query the user ID
    std::string userID = "";
    std::cout << "user ID: ";
    std::cin >> userID;
    // Query the shared pass-phrase
    std::string password = "";
    std::cout << "password: ";
    std::cin >> password;

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
    //
    // Get the client ID and client secret from the secure credential store
    const auto clientID = keychain.get("clientID");
    const auto clientSecret = keychain.get("clientSecret");

    // Create an OAuth service
    auto oauthService = sensory::service::OAuthService(config);
    if (false) {  // the device is not enrolled yet, enroll it
        sensory::api::v1::management::DeviceResponse enrollResponse;
        auto status = oauthService.enrollDevice(&enrollResponse, userID, password, clientID, clientSecret);
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "EnrollDevice failed with\n\t" <<
                status.error_code() << ": " << status.error_message() << std::endl;
            return 1;
        }
        std::cout << "Your user name is \"" << enrollResponse.name() << "\"" << std::endl;
        std::cout << "Your device ID is \"" << enrollResponse.deviceid() << "\"" << std::endl;
    }

    // Fetch a new OAuth token from the remote service
    sensory::api::common::TokenResponse tokenResponse;
    status = oauthService.getToken(&tokenResponse, clientID, clientSecret);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "GetToken failed with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    std::cout << "Your current token is " << tokenResponse.accesstoken() << std::endl;

    sensory::token_manager::TokenManager<sensory::token_manager::Keychain> token_manager(oauthService, keychain);
    // const auto access_token = token_manager.getAccessToken();

    // Query the available video models
    std::cout << "Available video models are" << std::endl;
    sensory::service::VideoService<sensory::token_manager::Keychain> videoService(config, token_manager);
    sensory::api::v1::video::GetModelsResponse videoModelsResponse;
    status = videoService.getModels(&videoModelsResponse);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "GetVideoModels failed with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    for (auto& model : videoModelsResponse.models())
        std::cout << "\t" << model.name() << std::endl;

    // Query the available audio models
    std::cout << "Available audio models are" << std::endl;
    sensory::service::AudioService<sensory::token_manager::Keychain> audioService(config, token_manager);
    sensory::api::v1::audio::GetModelsResponse audioModelsResponse;
    status = audioService.getModels(&audioModelsResponse);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "GetAudioModels failed with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    for (auto& model : audioModelsResponse.models())
        std::cout << "\t" << model.name() << std::endl;

    // Query this user's active enrollments
    std::cout << "Active enrollments are" << std::endl;
    sensory::service::ManagementService<sensory::token_manager::Keychain> mgmtService(config, token_manager);
}
