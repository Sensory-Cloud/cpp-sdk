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
#include <sensorycloud/token_manager/keychain.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>

/// @brief Print help about the application to the console.
void print_help() {
    std::cout << "Sensory Cloud Management Shell" << std::endl;
    std::cout << "getEnrollments <username>\tList the enrollments for the given user" << std::endl;
    std::cout << "deleteEnrollment <ID>\tDelete the enrollment with the given ID" << std::endl;
    std::cout << "getGroups <username>\tList the enrollment groups for the given user" << std::endl;
    std::cout << "deleteEnrollmentGroup <ID>\tDelete the enrollment group with the given ID" << std::endl;
    std::cout << "quit\tExit the shell" << std::endl;
}

/// @brief Check the health of the remote server.
///
/// @param config The global configuration for the cloud host.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int check_health(const sensory::Config& config) {
    // Query the health of the remote service.
    sensory::service::HealthService healthService(config);
    sensory::api::common::ServerHealthResponse serverHealth;
    auto status = healthService.getHealth(&serverHealth);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "GetHealth failed with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    // Report the health of the remote service
    std::cout << "Server status:" << std::endl;
    std::cout << "\tisHealthy: " << serverHealth.ishealthy() << std::endl;
    std::cout << "\tserverVersion: " << serverHealth.serverversion() << std::endl;
    std::cout << "\tid: " << serverHealth.id() << std::endl;
    return 0;
}

/// @brief Login to the OAuth service on the remote server.
///
/// @param oauthService The OAuth service for requesting tokens from the server
/// @param tokenManager The token manager for storing and accessing credentials
/// @returns 0 if the call succeeds, 1 otherwise.
///
int login(
    sensory::service::OAuthService& oauthService,
    sensory::token_manager::TokenManager<sensory::token_manager::Keychain>& tokenManager
) {
    if (!tokenManager.hasSavedCredentials()) {  // the device is not registered
        // Generate a new clientID and clientSecret for this device
        const auto credentials = tokenManager.generateCredentials();

        // Query the user ID
        std::string userID = "";
        std::cout << "user ID: ";
        std::cin >> userID;

        // Query the shared pass-phrase
        std::string password = "";
        std::cout << "password: ";
        std::cin >> password;

        // Register this device with the remote host
        sensory::api::v1::management::DeviceResponse rsp;
        auto status = oauthService.registerDevice(&rsp,
            userID,
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
int get_enrollments(
    sensory::service::ManagementService<sensory::token_manager::Keychain>& mgmtService,
    const std::string& userID
) {
    sensory::api::v1::management::GetEnrollmentsResponse rsp;
    auto status = mgmtService.getEnrollments(&rsp, userID);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to get enrollments with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    if (rsp.enrollments().size() == 0) {
        std::cout << "No enrollments" << std::endl;
        return 0;
    }
    for (auto& enrollment : rsp.enrollments()) {
        std::cout << "Description: "     << enrollment.description()  << std::endl;
        std::cout << "\tModel Name: "    << enrollment.modelname()    << std::endl;
        std::cout << "\tModel Type: "    << enrollment.modeltype()    << std::endl;
        std::cout << "\tModel Version: " << enrollment.modelversion() << std::endl;
        std::cout << "\tUser ID: "       << enrollment.userid()       << std::endl;
        std::cout << "\tDevice ID: "     << enrollment.deviceid()     << std::endl;
        std::cout << "\tCreated: "
            << google::protobuf::util::TimeUtil::ToString(enrollment.createdat())
            << std::endl;
        std::cout << "\tUpdated: "
            << google::protobuf::util::TimeUtil::ToString(enrollment.updatedat())
            << std::endl;
        std::cout << "\tID: "            << enrollment.id()    << std::endl;
    }
    return 0;
}

/// @brief Delete the enrollment with the given ID.
///
/// @param mgmtService The management service for deleting the enrollment.
/// @param enrollmentID The UUID of the enrollment to delete.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int delete_enrollment(
    sensory::service::ManagementService<sensory::token_manager::Keychain>& mgmtService,
    const std::string& enrollmentID
) {
    sensory::api::v1::management::EnrollmentResponse rsp;
    auto status = mgmtService.deleteEnrollment(&rsp, enrollmentID);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to delete enrollment with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    return 0;
}

/// @brief Get the enrollment groups for the given user.
///
/// @param mgmtService The management service for getting enrollments.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int get_enrollment_groups(
    sensory::service::ManagementService<sensory::token_manager::Keychain>& mgmtService,
    const std::string& userID
) {
    sensory::api::v1::management::GetEnrollmentGroupsResponse rsp;
    auto status = mgmtService.getEnrollmentGroups(&rsp, userID);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to get enrollment groups with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    if (rsp.enrollmentgroups().size() == 0) {
        std::cout << "No enrollment groups" << std::endl;
        return 0;
    }
    for (auto& enrollment : rsp.enrollmentgroups()) {
        std::cout << "Description: "     << enrollment.description()  << std::endl;
        std::cout << "\tModel Name: "    << enrollment.modelname()    << std::endl;
        std::cout << "\tModel Type: "    << enrollment.modeltype()    << std::endl;
        std::cout << "\tModel Version: " << enrollment.modelversion() << std::endl;
        std::cout << "\tUser ID: "       << enrollment.userid()       << std::endl;
        std::cout << "\tCreated: "
            << google::protobuf::util::TimeUtil::ToString(enrollment.createdat())
            << std::endl;
        std::cout << "\tUpdated: "
            << google::protobuf::util::TimeUtil::ToString(enrollment.updatedat())
            << std::endl;
        std::cout << "\tID: "            << enrollment.id()    << std::endl;
    }
    return 0;
}

/// @brief Create a new enrollment group.
///
/// @param mgmtService The management service for getting enrollments.
/// @param userID The user ID of the user that owns the enrollment group.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int create_enrollment_group(
    sensory::service::ManagementService<sensory::token_manager::Keychain>& mgmtService,
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
    sensory::api::v1::management::EnrollmentGroupResponse rsp;
    auto status = mgmtService.createEnrollmentGroup(&rsp, userID, "", groupName, description, modelName);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to create enrollment group with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    return 0;
}

/// @brief Append enrollment IDs to an existing enrollment group.
///
/// @param mgmtService The management service for getting enrollments.
/// @param groupID the UUID of the group to append enrollments to
/// @param enrollments the list of enrollments to append to the group
/// @returns 0 if the call succeeds, 1 otherwise.
///
int append_enrollment_group(
    sensory::service::ManagementService<sensory::token_manager::Keychain>& mgmtService,
    const std::string& groupID,
    const std::vector<std::string>& enrollments
) {
    sensory::api::v1::management::EnrollmentGroupResponse rsp;
    auto status = mgmtService.appendEnrollmentGroup(&rsp, groupID, enrollments);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to append enrollment group with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    return 0;
}

/// @brief Delete the enrollment group with the given ID.
///
/// @param mgmtService The management service for deleting the enrollment group.
/// @param enrollmentID The UUID of the enrollment group to delete.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int delete_enrollment_group(
    sensory::service::ManagementService<sensory::token_manager::Keychain>& mgmtService,
    const std::string& groupID
) {
    sensory::api::v1::management::EnrollmentGroupResponse rsp;
    auto status = mgmtService.deleteEnrollmentGroup(&rsp, groupID);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to delete enrollment group with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    }
    return 0;
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
    sensory::token_manager::Keychain keychain("com.sensory.cloud");
    sensory::service::OAuthService oauthService(config);
    sensory::token_manager::TokenManager<sensory::token_manager::Keychain> tokenManager(oauthService, keychain);
    login(oauthService, tokenManager);
    // Create the management service for fetching and updating enrollments and
    // enrollment groups.
    sensory::service::ManagementService<sensory::token_manager::Keychain> mgmtService(config, tokenManager);

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
            check_health(config);
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
