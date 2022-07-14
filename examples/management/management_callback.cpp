// The SensoryCloud C++ SDK Management service demo (callback interface).
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

#include <iostream>
#include <regex>
#include <google/protobuf/util/time_util.h>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/insecure_credential_store.hpp>
#include "dep/argparse.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::InsecureCredentialStore;
using sensory::service::ManagementService;

/// @brief Get the enrollments for the given user.
///
/// @param service The management service for getting enrollments.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int getEnrollments(
    ManagementService<InsecureCredentialStore>& service,
    const std::string& userID
) {
    int error_code = 0;
    service.getEnrollments(
        userID,
        [&error_code](ManagementService<InsecureCredentialStore>::GetEnrollmentsCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to get enrollments (" << call->getStatus().error_code() << "): " << call->getStatus().error_message() << std::endl;
            error_code = call->getStatus().error_code();
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
    return error_code;
}

/// @brief Delete the enrollment with the given ID.
///
/// @param service The management service for deleting the enrollment.
/// @param enrollmentID The UUID of the enrollment to delete.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int deleteEnrollment(
    ManagementService<InsecureCredentialStore>& service,
    const std::string& enrollmentID
) {
    int error_code = 0;
    service.deleteEnrollment(
        enrollmentID,
        [&error_code](ManagementService<InsecureCredentialStore>::DeleteEnrollmentCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to delete enrollment (" << call->getStatus().error_code() << "): " << call->getStatus().error_message() << std::endl;
            error_code = call->getStatus().error_code();
        }
    })->await();
    return error_code;
}

/// @brief Get the enrollment groups for the given user.
///
/// @param service The management service for getting enrollments.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int getEnrollmentGroups(
    ManagementService<InsecureCredentialStore>& service,
    const std::string& userID
) {
    int error_code = 0;
    service.getEnrollmentGroups(
        userID,
        [&error_code](ManagementService<InsecureCredentialStore>::GetEnrollmentGroupsCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to get enrollment groups (" << call->getStatus().error_code() << "): " << call->getStatus().error_message() << std::endl;
            error_code = call->getStatus().error_code();
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
    return error_code;
}

/// @brief Create a new enrollment group.
///
/// @param service The management service for getting enrollments.
/// @param userID The user ID of the user that owns the enrollment group.
/// @param groupdID The optional group ID to create the group with (an empty
/// string to generate the UUID on the server).
/// @param name The name of the group to create.
/// @param description A text description of the enrollment group.
/// @param model The name of the model associated with the enrollment group.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int createEnrollmentGroup(
    ManagementService<InsecureCredentialStore>& service,
    const std::string& userID,
    const std::string& groupID,
    const std::string& name,
    const std::string& description,
    const std::string& model,
    const std::vector<std::string>& enrollmentIDs
) {
    int error_code = 0;
    service.createEnrollmentGroup(userID, groupID,name, description, model, enrollmentIDs,
        [&error_code](ManagementService<InsecureCredentialStore>::CreateEnrollmentGroupCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to create enrollment group (" << call->getStatus().error_code() << "): " << call->getStatus().error_message() << std::endl;
            error_code = call->getStatus().error_code();
        }
    })->await();
    return error_code;
}

/// @brief Append enrollment IDs to an existing enrollment group.
///
/// @param service The management service for getting enrollments.
/// @param groupID the UUID of the group to append enrollments to
/// @param enrollments the list of enrollments to append to the group
/// @returns 0 if the call succeeds, 1 otherwise.
///
int appendEnrollmentGroup(
    ManagementService<InsecureCredentialStore>& service,
    const std::string& groupID,
    const std::vector<std::string>& enrollments
) {
    int error_code = 0;
    service.appendEnrollmentGroup(
        groupID,
        enrollments,
        [&error_code](ManagementService<InsecureCredentialStore>::AppendEnrollmentGroupCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to append enrollment group (" << call->getStatus().error_code() << "): " << call->getStatus().error_message() << std::endl;
            error_code = call->getStatus().error_code();
        }
    })->await();
    return error_code;
}

/// @brief Delete the enrollment group with the given ID.
///
/// @param service The management service for deleting the enrollment group.
/// @param groupID The UUID of the enrollment group to delete.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int deleteEnrollmentGroup(
    ManagementService<InsecureCredentialStore>& service,
    const std::string& groupID
) {
    int error_code = 0;
    service.deleteEnrollmentGroup(
        groupID,
        [&error_code](ManagementService<InsecureCredentialStore>::DeleteEnrollmentGroupCallData* call) {
        if (!call->getStatus().ok()) {  // The call failed.
            std::cout << "Failed to delete enrollment group (" << call->getStatus().error_code() << "): " << call->getStatus().error_message() << std::endl;
            error_code = call->getStatus().error_code();
        }
    })->await();
    return error_code;
}

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("authenticate")
        .description("A tool for authenticating with face biometrics using SensoryCloud.");
    parser.add_argument({ "path" })
        .help("PATH The path to an INI file containing server metadata.");
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
    const auto PATH = args.get<std::string>("path");
    const auto ENDPOINT = args.get<std::string>("endpoint");
    const auto USER_ID = args.get<std::string>("userid");
    const auto ENROLLMENT_ID = args.get<std::string>("enrollmentid");
    const auto NAME = args.get<std::string>("name");
    const auto DESCRIPTION = args.get<std::string>("description");
    const auto MODEL = args.get<std::string>("model");
    const auto ENROLLMENT_IDS = args.get<std::vector<std::string>>("enrollmentids+");
    const auto VERBOSE = args.get<bool>("verbose");

    // Create an insecure credential store for keeping OAuth credentials in.
    InsecureCredentialStore keychain(".", "com.sensory.cloud.examples");

    // Create the cloud services handle.
    SensoryCloud<InsecureCredentialStore> cloud(PATH, keychain);

    // Query the health of the remote service.
    sensory::api::common::ServerHealthResponse server_health;
    auto status = cloud.health.getHealth(&server_health);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }
    if (ENDPOINT == "getHealth") {
        std::cout << "Server status:" << std::endl;
        std::cout << "\tisHealthy: " << server_health.ishealthy() << std::endl;
        std::cout << "\tserverVersion: " << server_health.serverversion() << std::endl;
        std::cout << "\tid: " << server_health.id() << std::endl;
        return 0;
    }

    // Initialize the client.
    sensory::api::v1::management::DeviceResponse response;
    status = cloud.initialize(&response);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to initialize (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }

    if      (ENDPOINT == "getEnrollments")        return getEnrollments(cloud.management, USER_ID);
    else if (ENDPOINT == "deleteEnrollment")      return deleteEnrollment(cloud.management, ENROLLMENT_ID);
    else if (ENDPOINT == "getEnrollmentGroups")   return getEnrollmentGroups(cloud.management, USER_ID);
    else if (ENDPOINT == "createEnrollmentGroup") return createEnrollmentGroup(cloud.management, USER_ID, ENROLLMENT_ID, NAME, DESCRIPTION, MODEL, {});
    else if (ENDPOINT == "appendEnrollmentGroup") return appendEnrollmentGroup(cloud.management, ENROLLMENT_ID, ENROLLMENT_IDS);
    else if (ENDPOINT == "deleteEnrollmentGroup") return deleteEnrollmentGroup(cloud.management, ENROLLMENT_ID);
}
