// The SensoryCloud C++ SDK Management service demo (synchronous interface).
//
// Copyright (c) 2023 Sensory, Inc.
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
#include <google/protobuf/util/json_util.h>
#include <sensorycloud/sensorycloud.hpp>
#include <sensorycloud/token_manager/file_system_credential_store.hpp>
#include "dep/argparse.hpp"

using sensory::SensoryCloud;
using sensory::token_manager::FileSystemCredentialStore;
using sensory::service::ManagementService;

/// @brief Get the enrollments for the given user.
///
/// @param service The management service to interact with.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int get_enrollments(
    ManagementService<FileSystemCredentialStore>& service,
    const std::string& userID
) {
    sensory::api::v1::management::GetEnrollmentsResponse rsp;
    auto status = service.get_enrollments(&rsp, userID);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to get enrollments (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }
    if (rsp.enrollments().size() == 0) {
        std::cout << "No enrollments" << std::endl;
        return 0;
    }
    for (auto& enrollment : rsp.enrollments()) {
        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true;
        options.always_print_primitive_fields = true;
        options.always_print_enums_as_ints = false;
        options.preserve_proto_field_names = true;
        std::string enrollment_json;
        google::protobuf::util::MessageToJsonString(enrollment, &enrollment_json, options);
        std::cout << enrollment_json;
    }
    return 0;
}

/// @brief Delete the enrollment with the given ID.
///
/// @param service The management service for deleting the enrollment.
/// @param enrollmentID The UUID of the enrollment to delete.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int delete_enrollment(
    ManagementService<FileSystemCredentialStore>& service,
    const std::string& enrollmentID
) {
    sensory::api::v1::management::EnrollmentResponse rsp;
    auto status = service.delete_enrollment(&rsp, enrollmentID);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to delete enrollment (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }
    return 0;
}

/// @brief Get the enrollment groups for the given user.
///
/// @param service The management service to interact with.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int get_enrollment_groups(
    ManagementService<FileSystemCredentialStore>& service,
    const std::string& userID
) {
    sensory::api::v1::management::GetEnrollmentGroupsResponse rsp;
    auto status = service.get_enrollment_groups(&rsp, userID);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to get enrollment groups (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }
    if (rsp.enrollmentgroups().size() == 0) {
        std::cout << "No enrollment groups" << std::endl;
        return 0;
    }
    for (auto& enrollment : rsp.enrollmentgroups()) {
        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true;
        options.always_print_primitive_fields = true;
        options.always_print_enums_as_ints = false;
        options.preserve_proto_field_names = true;
        std::string enrollment_json;
        google::protobuf::util::MessageToJsonString(enrollment, &enrollment_json, options);
        std::cout << enrollment_json;
    }
    return 0;
}

/// @brief Create a new enrollment group.
///
/// @param service The management service to interact with.
/// @param userID The user ID of the user that owns the enrollment group.
/// @param groupdID The optional group ID to create the group with (an empty
/// string to generate the UUID on the server).
/// @param name The name of the group to create.
/// @param description A text description of the enrollment group.
/// @param model The name of the model associated with the enrollment group.
/// @param enrollmentIDs The IDs to create the enrollment group with. This
/// value may be an empty list to create an empty enrollment group.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int create_enrollment_group(
    ManagementService<FileSystemCredentialStore>& service,
    const std::string& userID,
    const std::string& groupID,
    const std::string& name,
    const std::string& description,
    const std::string& model,
    const std::vector<std::string>& enrollmentIDs
) {
    sensory::api::v1::management::EnrollmentGroupResponse rsp;
    auto status = service.create_enrollment_group(
        &rsp, userID, groupID, name, description, model, enrollmentIDs);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to create enrollment group (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }
    std::cout << "Created group with ID " << rsp.id() << std::endl;
    return 0;
}

/// @brief Update an existing enrollment group.
///
/// @param service The management service to interact with.
/// @param groupdID The ID of the enrollment group to update
/// @param groupName The new name for the group.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int update_enrollment_group(
    ManagementService<FileSystemCredentialStore>& service,
    const std::string& groupID,
    const std::string& groupName
) {
    sensory::api::v1::management::EnrollmentGroupResponse rsp;
    auto status = service.update_enrollment_group(&rsp, groupID, groupName);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to update enrollment group (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }
    std::cout << "Updated group with ID " << groupID << " to have name " << groupName << std::endl;
    return 0;
}

/// @brief Append enrollment IDs to an existing enrollment group.
///
/// @param service The management service to interact with.
/// @param groupID the UUID of the group to append enrollments to
/// @param enrollments the list of enrollments to append to the group
/// @returns 0 if the call succeeds, 1 otherwise.
///
int append_enrollment_group(
    ManagementService<FileSystemCredentialStore>& service,
    const std::string& groupID,
    const std::vector<std::string>& enrollments
) {
    sensory::api::v1::management::EnrollmentGroupResponse rsp;
    auto status = service.append_enrollment_group(&rsp, groupID, enrollments);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to append enrollment group (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }
    return 0;
}

/// @brief Remove enrollments from an existing enrollment group.
///
/// @param service The management service to interact with.
/// @param groupID the UUID of the group to remove enrollments from
/// @param enrollments the list of enrollments to remove from the group
/// @returns 0 if the call succeeds, 1 otherwise.
///
int remove_enrollments_from_group(
    ManagementService<FileSystemCredentialStore>& service,
    const std::string& groupID,
    const std::vector<std::string>& enrollments
) {
    sensory::api::v1::management::EnrollmentGroupResponse rsp;
    auto status = service.remove_enrollments_from_group(&rsp, groupID, enrollments);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to remove enrollments from group (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }
    return 0;
}

// /// @brief Remove enrollment IDs from an existing enrollment group.
// ///
// /// @param service The management service to interact with.
// /// @param groupID the UUID of the group to remove enrollments from
// /// @param enrollments the list of enrollments to remove from the group
// /// @returns 0 if the call succeeds, 1 otherwise.
// ///
// int remove_enrollments_from_group(
//     ManagementService<FileSystemCredentialStore>& service,
//     const std::string& groupID,
//     const std::vector<std::string>& enrollments
// ) {
//     sensory::api::v1::management::EnrollmentGroupResponse rsp;
//     auto status = service.remove_enrollments_from_group(&rsp, groupID, enrollments);
//     if (!status.ok()) {  // The call failed, print a descriptive message.
//         std::cout << "Failed to remove enrollments from group (" << status.error_code() << "): " << status.error_message() << std::endl;
//         return 1;
//     }
//     return 0;
// }

/// @brief Delete the enrollment group with the given ID.
///
/// @param service The management service for deleting the enrollment group.
/// @param enrollmentID The UUID of the enrollment group to delete.
/// @returns 0 if the call succeeds, 1 otherwise.
///
int delete_enrollment_group(
    ManagementService<FileSystemCredentialStore>& service,
    const std::string& groupID
) {
    sensory::api::v1::management::EnrollmentGroupResponse rsp;
    auto status = service.delete_enrollment_group(&rsp, groupID);
    if (!status.ok()) {  // The call failed, print a descriptive message.
        std::cout << "Failed to delete enrollment group (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }
    return 0;
}

int main(int argc, const char** argv) {
    // Create an argument parser to parse inputs from the command line.
    auto parser = argparse::ArgumentParser(argc, argv)
        .prog("authenticate")
        .description("A tool for authenticating with face biometrics using SensoryCloud.");
    parser.add_argument({ "path" })
        .help("The path to an INI file containing server metadata.");
    parser.add_argument("endpoint")
        .choices({
            "get_health",
            "get_enrollments",
            "delete_enrollment",
            "get_enrollment_groups",
            "create_enrollment_group",
            "append_enrollment_group",
            "update_enrollment_group",
            "remove_enrollments_from_group",
            "delete_enrollment_group"
        }).help("The management endpoint to use.");
    parser.add_argument({ "-u", "--userid" })
        .help("The ID of the user initiating the request.");
    parser.add_argument({ "-e", "--enrollmentid" })
        .help("The ID of the enrollment / enrollment group.");
    parser.add_argument({ "-n", "--name" })
        .help("The name of the enrollment group to create.");
    parser.add_argument({ "-d", "--description" })
        .help("A description of the enrollment group to create.");
    parser.add_argument({ "-m", "--model" })
        .help("The model to create an enrollment group with.");
    parser.add_argument({ "-E+", "--enrollmentids+" })
        .action("store")
        .nargs("+")
        .help("A collection of enrollment IDs to append to a group.");
    parser.add_argument({ "-v", "--verbose" })
        .action("store_true")
        .help("Produce verbose output during authentication.");
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

    // Create a credential store for keeping OAuth credentials in.
    FileSystemCredentialStore keychain(".", "com.sensory.cloud.examples");

    // Create the cloud services handle.
    SensoryCloud<FileSystemCredentialStore> cloud(PATH, keychain);

    // Query the health of the remote service.
    sensory::api::common::ServerHealthResponse server_health;
    auto status = cloud.health.get_health(&server_health);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to get server health (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }
    if (ENDPOINT == "get_health") {
        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true;
        options.always_print_primitive_fields = true;
        options.always_print_enums_as_ints = false;
        options.preserve_proto_field_names = true;
        std::string server_health_json;
        google::protobuf::util::MessageToJsonString(server_health, &server_health_json, options);
        std::cout << server_health_json;
        return 0;
    }

    // Initialize the client.
    sensory::api::v1::management::DeviceResponse response;
    status = cloud.initialize(&response);
    if (!status.ok()) {  // the call failed, print a descriptive message
        std::cout << "Failed to initialize (" << status.error_code() << "): " << status.error_message() << std::endl;
        return 1;
    }

    if      (ENDPOINT == "get_enrollments")
        return get_enrollments(cloud.management, USER_ID);
    else if (ENDPOINT == "delete_enrollment")
        return delete_enrollment(cloud.management, ENROLLMENT_ID);
    else if (ENDPOINT == "get_enrollment_groups")
        return get_enrollment_groups(cloud.management, USER_ID);
    else if (ENDPOINT == "create_enrollment_group")
        return create_enrollment_group(cloud.management, USER_ID, ENROLLMENT_ID, NAME, DESCRIPTION, MODEL, ENROLLMENT_IDS);
    else if (ENDPOINT == "append_enrollment_group")
        return append_enrollment_group(cloud.management, ENROLLMENT_ID, ENROLLMENT_IDS);
    else if (ENDPOINT == "update_enrollment_group")
        return update_enrollment_group(cloud.management, ENROLLMENT_ID, NAME);
    else if (ENDPOINT == "remove_enrollments_from_group")
        return remove_enrollments_from_group(cloud.management, ENROLLMENT_ID, ENROLLMENT_IDS);
    else if (ENDPOINT == "delete_enrollment_group")
        return delete_enrollment_group(cloud.management, ENROLLMENT_ID);
}
