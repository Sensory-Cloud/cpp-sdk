// The Sensory Cloud C++ SDK Management service demo (event-loop interface).
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
#include <sensorycloud/token_manager/insecure_credential_store.hpp>
#include <sensorycloud/token_manager/token_manager.hpp>
#include "dep/argparse.hpp"

using sensory::token_manager::InsecureCredentialStore;
using sensory::token_manager::TokenManager;
using sensory::service::HealthService;
using sensory::service::OAuthService;
using sensory::service::ManagementService;

/// @brief Login to the OAuth service on the remote server.
///
/// @param oauthService The OAuth service for requesting tokens from the server
/// @param tokenManager The token manager for storing and accessing credentials
/// @returns 0 if the call succeeds, 1 otherwise.
///
int registerDevice(
    sensory::service::OAuthService& oauthService,
    sensory::token_manager::TokenManager<sensory::token_manager::InsecureCredentialStore>& tokenManager
) {
    int errCode = 0;
    if (!tokenManager.hasToken()) {  // the device is not registered
        // Generate a new clientID and clientSecret for this device
        const auto credentials = tokenManager.generateCredentials();

        std::cout << "Registering device with server..." << std::endl;

        // Query the friendly device name
        std::string name = "";
        std::cout << "Device Name: ";
        std::cin >> name;

        // Query the shared pass-phrase
        std::string password = "";
        std::cout << "Password: ";
        std::cin >> password;

        ::grpc::CompletionQueue queue;
        auto call = oauthService.registerDevice(&queue,
            name, password, credentials.id, credentials.secret);
        void* tag(nullptr);
        bool ok(false);
        queue.Next(&tag, &ok);
        if (ok && tag == call) {
            if (!call->getStatus().ok()) {  // The call failed, print a descriptive message.
                std::cout << "Failed to register device with\n\t"
                    << call->getStatus().error_code() << ": "
                    << call->getStatus().error_message() << std::endl;
                errCode = call->getStatus().error_code();
            }
            delete call;
        }
    }
    return errCode;
}

/// @brief Get the enrollments for the given user.
///
/// @param mgmtService The management service for getting enrollments.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int getEnrollments(
    sensory::service::ManagementService<sensory::token_manager::InsecureCredentialStore>& mgmtService,
    const std::string& userID
) {
    ::grpc::CompletionQueue queue;
    auto call = mgmtService.getEnrollments(&queue, userID);
    void* tag(nullptr);
    bool ok(false);
    queue.Next(&tag, &ok);
    int errCode = 0;
    if (ok && tag == call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to get enrollments with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
            errCode = call->getStatus().error_code();
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
        delete call;
    }
    return errCode;
}

/// @brief Delete the enrollment with the given ID.
///
/// @param mgmtService The management service for deleting the enrollment.
/// @param enrollmentID The UUID of the enrollment to delete.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int deleteEnrollment(
    sensory::service::ManagementService<sensory::token_manager::InsecureCredentialStore>& mgmtService,
    const std::string& enrollmentID
) {
    ::grpc::CompletionQueue queue;
    auto call = mgmtService.deleteEnrollment(&queue, enrollmentID);
    void* tag(nullptr);
    bool ok(false);
    queue.Next(&tag, &ok);
    int errCode = 0;
    if (ok && tag == call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to delete enrollment with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
            errCode = call->getStatus().error_code();
        }
        delete call;
    }
    return errCode;
}

/// @brief Get the enrollment groups for the given user.
///
/// @param mgmtService The management service for getting enrollments.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int getEnrollmentGroups(
    sensory::service::ManagementService<sensory::token_manager::InsecureCredentialStore>& mgmtService,
    const std::string& userID
) {
    ::grpc::CompletionQueue queue;
    auto call = mgmtService.getEnrollmentGroups(&queue, userID);
    void* tag(nullptr);
    bool ok(false);
    queue.Next(&tag, &ok);
    int errCode = 0;
    if (ok && tag == call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to get enrollment groups with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
            errCode = call->getStatus().error_code();
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
        delete call;
    }
    return errCode;
}

/// @brief Create a new enrollment group.
///
/// @param mgmtService The management service for getting enrollments.
/// @param userID The user ID of the user that owns the enrollment group.
/// @param groupdID The optional group ID to create the group with (an empty
/// string to generate the UUID on the server).
/// @param name The name of the group to create.
/// @param description A text description of the enrollment group.
/// @param model The name of the model associated with the enrollment group.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int createEnrollmentGroup(
    sensory::service::ManagementService<sensory::token_manager::InsecureCredentialStore>& mgmtService,
    const std::string& userID,
    const std::string& groupID,
    const std::string& name,
    const std::string& description,
    const std::string& model
) {
    ::grpc::CompletionQueue queue;
    auto call = mgmtService.createEnrollmentGroup(
        &queue, userID, groupID, name, description, model);
    void* tag(nullptr);
    bool ok(false);
    queue.Next(&tag, &ok);
    int errCode = 0;
    if (ok && tag == call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to create enrollment group with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
            errCode = call->getStatus().error_code();
        }
        delete call;
    }
    return errCode;
}

/// @brief Append enrollment IDs to an existing enrollment group.
///
/// @param mgmtService The management service for getting enrollments.
/// @param groupID the UUID of the group to append enrollments to
/// @param enrollments the list of enrollments to append to the group
/// @returns 0 if the call succeeds, 1 otherwise.
///
int appendEnrollmentGroup(
    sensory::service::ManagementService<sensory::token_manager::InsecureCredentialStore>& mgmtService,
    const std::string& groupID,
    const std::vector<std::string>& enrollments
) {
    ::grpc::CompletionQueue queue;
    auto call = mgmtService.appendEnrollmentGroup(&queue, groupID, enrollments);
    void* tag(nullptr);
    bool ok(false);
    queue.Next(&tag, &ok);
    int errCode = 0;
    if (ok && tag == call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to append enrollment group with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
            errCode = call->getStatus().error_code();
        }
        delete call;
    }
    return errCode;
}

/// @brief Delete the enrollment group with the given ID.
///
/// @param mgmtService The management service for deleting the enrollment group.
/// @param groupID The UUID of the enrollment group to delete.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int deleteEnrollmentGroup(
    sensory::service::ManagementService<sensory::token_manager::InsecureCredentialStore>& mgmtService,
    const std::string& groupID
) {
    ::grpc::CompletionQueue queue;
    auto call = mgmtService.deleteEnrollmentGroup(&queue, groupID);
    void* tag(nullptr);
    bool ok(false);
    queue.Next(&tag, &ok);
    int errCode = 0;
    if (ok && tag == call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to delete enrollment group with\n\t" <<
                call->getStatus().error_code() << ": " <<
                call->getStatus().error_message() << std::endl;
            errCode = call->getStatus().error_code();
        }
        delete call;
    }
    return errCode;
}

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("authenticate")
        .description("A tool for authenticating with face biometrics using Sensory Cloud.");
    parser.add_argument({ "-H", "--host" })
        .required(true)
        .help("HOST The hostname of a Sensory Cloud inference server.");
    parser.add_argument({ "-P", "--port" })
        .required(true)
        .help("PORT The port number that the Sensory Cloud inference server is running at.");
    parser.add_argument({ "-T", "--tenant" })
        .required(true)
        .help("TENANT The ID of your tenant on a Sensory Cloud inference server.");
    parser.add_argument({ "-I", "--insecure" })
        .action("store_true")
        .help("INSECURE Disable TLS.");
    parser.add_argument("endpoint")
        .choices({
            "getHealth",
            "getEnrollments",
            "deleteEnrollment",
            "getEnrollmentGroups",
            "createEnrollmentGroup",
            "appendEnrollmentGroup",
            "deleteEnrollmentGroup"
        }).help("ENDPOINT The management endpoint to use.");
    parser.add_argument({ "-u", "--userid" })
        .help("USERID The ID of the user initiating the request.");
    parser.add_argument({ "-e", "--enrollmentid" })
        .help("ENROLLMENTID The ID of the enrollment / enrollment group.");
    parser.add_argument({ "-n", "--name" })
        .help("NAME The name of the enrollment group to create.");
    parser.add_argument({ "-d", "--description" })
        .help("DESCRIPTION A description of the enrollment group to create.");
    parser.add_argument({ "-m", "--model" })
        .help("MODEL The model to create an enrollment group with.");
    parser.add_argument({ "-E+", "--enrollmentids+" })
        .action("store")
        .nargs("+")
        .help("ENROLLMENTIDS A collection of enrollment IDs to append to a group.");
    parser.add_argument({ "-v", "--verbose" })
        .action("store_true")
        .help("VERBOSE Produce verbose output during authentication.");
    // Parse the arguments from the command line.
    const auto args = parser.parse_args();
    const auto HOSTNAME = args.get<std::string>("host");
    const auto PORT = args.get<uint16_t>("port");
    const auto TENANT = args.get<std::string>("tenant");
    const auto IS_SECURE = !args.get<bool>("insecure");
    const auto ENDPOINT = args.get<std::string>("endpoint");
    const auto USER_ID = args.get<std::string>("userid");
    const auto ENROLLMENT_ID = args.get<std::string>("enrollmentid");
    const auto NAME = args.get<std::string>("name");
    const auto DESCRIPTION = args.get<std::string>("description");
    const auto MODEL = args.get<std::string>("model");
    const auto ENROLLMENT_IDS = args.get<std::vector<std::string>>("enrollmentids+");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create an insecure credential store for keeping OAuth credentials in.
    sensory::token_manager::InsecureCredentialStore keychain(".", "com.sensory.cloud.examples");
    if (!keychain.contains("deviceID"))
        keychain.emplace("deviceID", sensory::token_manager::uuid_v4());
    const auto DEVICE_ID(keychain.at("deviceID"));

    // Initialize the configuration to the host for given address and port
    sensory::Config config(HOSTNAME, PORT, TENANT, DEVICE_ID, IS_SECURE);

    // Query the health of the remote service.
    sensory::service::HealthService healthService(config);
    sensory::api::common::ServerHealthResponse serverHealth;
    auto status = healthService.getHealth(&serverHealth);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health with\n\t" <<
            status.error_code() << ": " << status.error_message() << std::endl;
        return 1;
    } else if (ENDPOINT == "getHealth") {
        // Report the health of the remote service
        std::cout << "Server status:" << std::endl;
        std::cout << "\tisHealthy: " << serverHealth.ishealthy() << std::endl;
        std::cout << "\tserverVersion: " << serverHealth.serverversion() << std::endl;
        std::cout << "\tid: " << serverHealth.id() << std::endl;
        return 0;
    }

    // Create an OAuth service and register this device with the server
    sensory::service::OAuthService oauthService(config);
    sensory::token_manager::TokenManager<sensory::token_manager::InsecureCredentialStore> tokenManager(oauthService, keychain);
    if (registerDevice(oauthService, tokenManager)) return 1;

    // Create the management service and execute the request.
    sensory::service::ManagementService<sensory::token_manager::InsecureCredentialStore> mgmtService(config, tokenManager);
    if      (ENDPOINT == "getEnrollments")        return getEnrollments(mgmtService, USER_ID);
    else if (ENDPOINT == "deleteEnrollment")      return deleteEnrollment(mgmtService, ENROLLMENT_ID);
    else if (ENDPOINT == "getEnrollmentGroups")   return getEnrollmentGroups(mgmtService, USER_ID);
    else if (ENDPOINT == "createEnrollmentGroup") return createEnrollmentGroup(mgmtService, USER_ID, ENROLLMENT_ID, NAME, DESCRIPTION, MODEL);
    else if (ENDPOINT == "appendEnrollmentGroup") return appendEnrollmentGroup(mgmtService, ENROLLMENT_ID, ENROLLMENT_IDS);
    else if (ENDPOINT == "deleteEnrollmentGroup") return deleteEnrollmentGroup(mgmtService, ENROLLMENT_ID);
}
