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
#include <google/protobuf/util/time_util.h>
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/health_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>
#include <sensorycloud/services/management_service.hpp>
#include <sensorycloud/services/audio_service.hpp>
#include <sensorycloud/services/video_service.hpp>
#include <sensorycloud/token_manager/secure_credential_store.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>

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
    sensory::service::HealthService healthService(config);
    sensory::api::common::ServerHealthResponse serverHealth;
    auto status = healthService.getHealth(&serverHealth);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health with\n\t" <<
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

    // Create an OAuth service
    sensory::token_manager::SecureCredentialStore keychain("com.sensory.cloud");
    sensory::service::OAuthService oauthService(config);
    sensory::token_manager::TokenManager<sensory::token_manager::SecureCredentialStore> tokenManager(oauthService, keychain);

    if (!tokenManager.hasSavedCredentials()) {  // the device is not registered
        // Generate a new clientID and clientSecret for this device
        const auto credentials = tokenManager.generateCredentials();

        // Query the friendly device name
        std::string name = "";
        std::cout << "Device Name: ";
        std::cin >> name;

        // Query the shared pass-phrase
        std::string password = "";
        std::cout << "password: ";
        std::cin >> password;

        // Register this device with the remote host
        sensory::api::v1::management::DeviceResponse registerResponse;
        auto status = oauthService.registerDevice(&registerResponse,
            name,
            password,
            credentials.id,
            credentials.secret
        );
        if (!status.ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to register device with\n\t" <<
                status.error_code() << ": " << status.error_message() << std::endl;
            return 1;
        }
    }

    // Query the available video models
    std::cout << "Available video models:" << std::endl;
    sensory::service::VideoService<sensory::token_manager::SecureCredentialStore> videoService(config, tokenManager);
    sensory::api::v1::video::GetModelsResponse videoModelsResponse;
    status = videoService.getModels(&videoModelsResponse);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get video models with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    for (auto& model : videoModelsResponse.models())
        std::cout << "\t" << model.name() << std::endl;

    // Query the available audio models
    std::cout << "Available audio models:" << std::endl;
    sensory::service::AudioService<sensory::token_manager::SecureCredentialStore> audioService(config, tokenManager);
    sensory::api::v1::audio::GetModelsResponse audioModelsResponse;
    status = audioService.getModels(&audioModelsResponse);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get audio models with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    for (auto& model : audioModelsResponse.models())
        std::cout << "\t" << model.name() << std::endl;

    // Query this user's active enrollments
    std::cout << "Active enrollments:" << std::endl;
    sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore> mgmtService(config, tokenManager);
    sensory::api::v1::management::GetEnrollmentsResponse enrollmentResponse;
    status = mgmtService.getEnrollments(&enrollmentResponse, userID);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get enrollments with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    for (auto& enrollment : enrollmentResponse.enrollments()) {
        std::cout << "\tDescription:     " << enrollment.description()  << std::endl;
        std::cout << "\t\tModel Name:    " << enrollment.modelname()    << std::endl;
        std::cout << "\t\tModel Type:    " << enrollment.modeltype()    << std::endl;
        std::cout << "\t\tModel Version: " << enrollment.modelversion() << std::endl;
        std::cout << "\t\tUser ID:       " << enrollment.userid()       << std::endl;
        std::cout << "\t\tDevice ID:     " << enrollment.deviceid()     << std::endl;
        std::cout << "\t\tCreated:       "
            << google::protobuf::util::TimeUtil::ToString(enrollment.createdat())
            << std::endl;
        std::cout << "\t\tUpdated:       "
            << google::protobuf::util::TimeUtil::ToString(enrollment.updatedat())
            << std::endl;
        std::cout << "\t\tID:            " << enrollment.id()    << std::endl;
    }

    // Query this user's enrollment groups
    std::cout << "Active enrollment groups:" << std::endl;
    sensory::api::v1::management::GetEnrollmentGroupsResponse enrollmentGroupResponse;
    status = mgmtService.getEnrollmentGroups(&enrollmentGroupResponse, userID);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get enrollment groups with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    for (auto& enrollment : enrollmentGroupResponse.enrollmentgroups()) {
        std::cout << "\tDescription:     " << enrollment.description()  << std::endl;
        std::cout << "\t\tModel Name:    " << enrollment.modelname()    << std::endl;
        std::cout << "\t\tModel Type:    " << enrollment.modeltype()    << std::endl;
        std::cout << "\t\tModel Version: " << enrollment.modelversion() << std::endl;
        std::cout << "\t\tUser ID:       " << enrollment.userid()       << std::endl;
        std::cout << "\t\tCreated:       "
            << google::protobuf::util::TimeUtil::ToString(enrollment.createdat())
            << std::endl;
        std::cout << "\t\tUpdated:       "
            << google::protobuf::util::TimeUtil::ToString(enrollment.updatedat())
            << std::endl;
        std::cout << "\t\tID:            " << enrollment.id()    << std::endl;
    }
}
