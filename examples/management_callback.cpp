// The Sensory Cloud C++ SDK Management service demo.
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
#include <regex>
#include <google/protobuf/util/time_util.h>
#include <sensorycloud/config.hpp>
#include <sensorycloud/services/health_service.hpp>
#include <sensorycloud/services/oauth_service.hpp>
#include <sensorycloud/services/management_service.hpp>
#include <sensorycloud/token_manager/secure_credential_store.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>

/// @brief Print help about the application to the console.
void print_help() {
    std::cout << "Sensory Cloud Management Shell" << std::endl;
    std::cout << "health                      Display information about the server's health" << std::endl;
    std::cout << "getEnrollments <username>   List the enrollments for the given user" << std::endl;
    std::cout << "deleteEnrollment <ID>       Delete the enrollment with the given ID" << std::endl;
    std::cout << "getGroups <username>        List the enrollment groups for the given user" << std::endl;
    std::cout << "deleteEnrollmentGroup <ID>  Delete the enrollment group with the given ID" << std::endl;
    std::cout << "quit                        Exit the shell" << std::endl;
}

/// @brief Check the health of the remote server.
///
/// @param config The global configuration for the cloud host.
///
void check_health(const sensory::service::HealthService& healthService) {
    // Query the health of the remote service.
    healthService.getHealth([](sensory::service::HealthService::GetHealthCallData* call) {
        if (!call->getStatus().ok()) {  // the call failed, print a descriptive message
            std::cout << "Failed to get server health with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
        }
        // Report the health of the remote service
        std::cout << "Server status" << std::endl;
        std::cout << "\tIs Healthy:     " << call->getResponse().ishealthy()     << std::endl;
        std::cout << "\tServer Version: " << call->getResponse().serverversion() << std::endl;
        std::cout << "\tID:             " << call->getResponse().id()            << std::endl;
    })->await();
}

/// @brief Login to the OAuth service on the remote server.
///
/// @param oauthService The OAuth service for requesting tokens from the server
/// @param tokenManager The token manager for storing and accessing credentials
/// @returns 0 if the call succeeds, 1 otherwise.
///
int login(
    sensory::service::OAuthService& oauthService,
    sensory::token_manager::TokenManager<sensory::token_manager::SecureCredentialStore>& tokenManager
) {
    if (!tokenManager.hasSavedCredentials()) {  // the device is not registered
        // Generate a new clientID and clientSecret for this device
        const auto credentials = tokenManager.generateCredentials();

        // Query the friendly device name
        std::string name = "";
        std::cout << "Device Name: ";
        std::cin >> name;

        // Query the shared pass-phrase
        std::string password = "";
        std::cout << "Password: ";
        std::cin >> password;

        // Flush anything that may still be in the input buffer.
        std::cin.ignore();

        // Register this device with the remote host
        sensory::api::v1::management::DeviceResponse rsp;
        auto status = oauthService.registerDevice(&rsp,
            name,
            password,
            credentials.id,
            credentials.secret
        );
        if (!status.ok()) {  // The call failed, print a descriptive message.
            std::cout << "Failed to register device with\n\t" <<
                status.error_code() << ": " << status.error_message() << std::endl;
            return 1;
        }
    }
    return 0;
}

/// @brief Get the enrollments for the given user.
///
/// @param mgmtService The management service for getting enrollments.
/// @returns 0 if the call succeeds, 1 otherwise.
///
void get_enrollments(
    sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>& mgmtService,
    const std::string& userID
) {
    mgmtService.getEnrollments(
        userID,
        [](sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>::GetEnrollmentsCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to get enrollments with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
        }
        if (call->getResponse().enrollments().size() == 0) {
            std::cout << "No enrollments" << std::endl;
        }
        for (auto& enrollment : call->getResponse().enrollments()) {
            std::cout << "Description: "     << enrollment.description()  << std::endl;
            std::cout << "\tModel Name:    " << enrollment.modelname()    << std::endl;
            std::cout << "\tModel Type:    " << enrollment.modeltype()    << std::endl;
            std::cout << "\tModel Version: " << enrollment.modelversion() << std::endl;
            std::cout << "\tUser ID:       " << enrollment.userid()       << std::endl;
            std::cout << "\tDevice ID:     " << enrollment.deviceid()     << std::endl;
            std::cout << "\tCreated:       "
                << google::protobuf::util::TimeUtil::ToString(enrollment.createdat())
                << std::endl;
            std::cout << "\tUpdated:       "
                << google::protobuf::util::TimeUtil::ToString(enrollment.updatedat())
                << std::endl;
            std::cout << "\tID:            " << enrollment.id()    << std::endl;
        }
    })->await();
}

/// @brief Delete the enrollment with the given ID.
///
/// @param mgmtService The management service for deleting the enrollment.
/// @param enrollmentID The UUID of the enrollment to delete.
/// @returns 0 if the call succeeds, 1 otherwise.
///
void delete_enrollment(
    sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>& mgmtService,
    const std::string& enrollmentID
) {
    mgmtService.deleteEnrollment(
        enrollmentID,
        [](sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>::DeleteEnrollmentCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to delete enrollment with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
        }
    })->await();
}

/// @brief Get the enrollment groups for the given user.
///
/// @param mgmtService The management service for getting enrollments.
/// @returns 0 if the call succeeds, 1 otherwise.
///
void get_enrollment_groups(
    sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>& mgmtService,
    const std::string& userID
) {
    mgmtService.getEnrollmentGroups(
        userID,
        [](sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>::GetEnrollmentGroupsCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to get enrollment groups with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
        }
        if (call->getResponse().enrollmentgroups().size() == 0) {
            std::cout << "No enrollment groups" << std::endl;
        }
        for (auto& enrollment : call->getResponse().enrollmentgroups()) {
            std::cout << "Description: "     << enrollment.description()  << std::endl;
            std::cout << "\tModel Name:    " << enrollment.modelname()    << std::endl;
            std::cout << "\tModel Type:    " << enrollment.modeltype()    << std::endl;
            std::cout << "\tModel Version: " << enrollment.modelversion() << std::endl;
            std::cout << "\tUser ID:       " << enrollment.userid()       << std::endl;
            std::cout << "\tCreated:       "
                << google::protobuf::util::TimeUtil::ToString(enrollment.createdat())
                << std::endl;
            std::cout << "\tUpdated:       "
                << google::protobuf::util::TimeUtil::ToString(enrollment.updatedat())
                << std::endl;
            std::cout << "\tID:            " << enrollment.id()    << std::endl;
        }
    })->await();
}

/// @brief Create a new enrollment group.
///
/// @param mgmtService The management service for getting enrollments.
/// @param userID The user ID of the user that owns the enrollment group.
/// @returns 0 if the call succeeds, 1 otherwise.
///
void create_enrollment_group(
    sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>& mgmtService,
    const std::string& userID
) {
    // Get the name of the group from the command line.
    std::cout << "Group Name: ";
    std::string groupName = "";
    std::getline(std::cin, groupName);
    // Get the description of the group from the command line.
    std::cout << "Group Description: ";
    std::string description = "";
    std::getline(std::cin, description);
    // Get the name of the model for the group from the command line.
    std::cout << "Model Name: ";
    std::string modelName = "";
    std::getline(std::cin, modelName);
    // Execute the RPC to create the enrollment group.
    mgmtService.createEnrollmentGroup(
        userID,
        "",
        groupName,
        description,
        modelName,
        [](sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>::CreateEnrollmentGroupCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to create enrollment group with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
        }
    })->await();
}

/// @brief Append enrollment IDs to an existing enrollment group.
///
/// @param mgmtService The management service for getting enrollments.
/// @param groupID the UUID of the group to append enrollments to
/// @param enrollments the list of enrollments to append to the group
/// @returns 0 if the call succeeds, 1 otherwise.
///
void append_enrollment_group(
    sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>& mgmtService,
    const std::string& groupID,
    const std::vector<std::string>& enrollments
) {
    mgmtService.appendEnrollmentGroup(
        groupID,
        enrollments,
        [](sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>::AppendEnrollmentGroupCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to append enrollment group with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
        }
    })->await();
}

/// @brief Delete the enrollment group with the given ID.
///
/// @param mgmtService The management service for deleting the enrollment group.
/// @param groupID The UUID of the enrollment group to delete.
/// @returns 0 if the call succeeds, 1 otherwise.
///
void delete_enrollment_group(
    sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>& mgmtService,
    const std::string& groupID
) {
    mgmtService.deleteEnrollmentGroup(
        groupID,
        [](sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore>::DeleteEnrollmentGroupCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to delete enrollment group with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
        }
    })->await();
}

int main() {
    // Initialize the configuration to the host for given address and port
    sensory::Config config(
        "io.stage.cloud.sensory.com",
        443,
        "cabb7700-206f-4cc7-8e79-cd7f288aa78d",
        "D895F447-91E8-486F-A783-6E3A33E4C7C5"
    );
    std::cout << "Connecting to remote host: " << config.getFullyQualifiedDomainName() << std::endl;

    // Create the OAuth service for requesting tokens from the server.
    sensory::token_manager::SecureCredentialStore keychain("com.sensory.cloud");
    sensory::service::OAuthService oauthService(config);
    sensory::token_manager::TokenManager<sensory::token_manager::SecureCredentialStore> tokenManager(oauthService, keychain);
    login(oauthService, tokenManager);
    // Create the health service for query health information from the server.
    sensory::service::HealthService healthService(config);
    // Create the management service for fetching and updating enrollments and
    // enrollment groups.
    sensory::service::ManagementService<sensory::token_manager::SecureCredentialStore> mgmtService(config, tokenManager);

    std::string cmd = "";
    while (cmd != "quit") {  // Run the CLI until the `quit` command is input.
        // Fetch a new command from the command line
        std::cout << "> ";
        std::getline(std::cin, cmd);

        std::smatch matches;

        static const std::regex GET_ENROLLMENTS("getEnrollments ([a-zA-Z0-9]+)");
        static const std::regex DELETE_ENROLLMENT("deleteEnrollment ([a-zA-Z0-9\\-]+)");
        static const std::regex GET_GROUPS("getGroups ([a-zA-Z0-9]+)");
        static const std::regex CREATE_GROUP("createGroup ([a-zA-Z0-9]+)");
        static const std::regex APPEND_GROUP("appendGroup ([a-zA-Z0-9]+)");
        static const std::regex DELETE_GROUP("deleteGroup ([a-zA-Z0-9\\-]+)");

        if (cmd == "health") {
            check_health(healthService);
        } else if (std::regex_search(cmd, matches, GET_ENROLLMENTS)) {
            const auto userID = matches[1].str();
            get_enrollments(mgmtService, userID);
        } else if (std::regex_search(cmd, matches, DELETE_ENROLLMENT)) {
            const auto enrollmentID = matches[1].str();
            delete_enrollment(mgmtService, enrollmentID);
        } else if (std::regex_search(cmd, matches, GET_GROUPS)) {
            const auto userID = matches[1].str();
            get_enrollment_groups(mgmtService, userID);
        } else if (std::regex_search(cmd, matches, CREATE_GROUP)) {
            const auto userID = matches[1].str();
            create_enrollment_group(mgmtService, userID);
        } else if (std::regex_search(cmd, matches, APPEND_GROUP)) {
            std::cout << "Appending enrollment group: " << std::endl;
            // append_enrollment_group(mgmtService);
        } else if (std::regex_search(cmd, matches, DELETE_GROUP)) {
            const auto groupID = matches[1].str();
            delete_enrollment_group(mgmtService, groupID);
        } else if (cmd != "quit") {
            if (cmd != "help") {
                std::cout << "command not recognized" << std::endl;
            }
            print_help();
        }
    }
}
