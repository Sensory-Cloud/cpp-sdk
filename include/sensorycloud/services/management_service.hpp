// Management service for the Sensory Cloud SDK.
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

#ifndef SENSORY_CLOUD_SERVICES_MANAGEMENT_SERVICE_HPP_
#define SENSORY_CLOUD_SERVICES_MANAGEMENT_SERVICE_HPP_

#include <memory>
#include <string>
#include <vector>
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include "sensorycloud/generated/v1/management/enrollment.pb.h"
#include "sensorycloud/generated/v1/management/enrollment.grpc.pb.h"
#include "sensorycloud/config.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud Services.
namespace service {

/// @brief A service for managing enrollments.
class ManagementService {
 private:
    /// the global configuration for the remote connection
    const Config& config;
    /// The gRPC stub for the enrollment service
    std::unique_ptr<api::v1::management::EnrollmentService::Stub> stub;

 public:
    /// @brief Initialize a new management service.
    ///
    /// @param config the global configuration for the remote connection
    ///
    explicit ManagementService(const Config& config_) : config(config_),
        stub(api::v1::management::EnrollmentService::NewStub(config.getChannel())) { }

    /// @brief Fetch a list of the current enrollments for the given userID
    ///
    /// @param userID userID to fetch enrollments for
    /// @returns A future to be fulfilled with either a list of enrollments,
    /// or the network error that occurred
    ///
    api::v1::management::GetEnrollmentsResponse getEnrollments(const std::string& userID) {
        // std::cout << "Requesting current enrollments from server with userID: " << userID << std::endl;
        // Create a context for the client.
        grpc::ClientContext context;
        // Create the request
        api::v1::management::GetEnrollmentsRequest request;
        request.set_userid(userID);
        // Execute the remote procedure call synchronously and return the result
        api::v1::management::GetEnrollmentsResponse response;
        grpc::Status status = stub->GetEnrollments(&context, request, &response);
        if (!status.ok()) {  // an error occurred in the RPC
            // std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            throw "GetEnrollments failure";
        }
        return response;
    }

    /// @brief Request the deletion of an enrollment.
    ///
    /// @param enrollmentID enrollmentID for the enrollment to delete
    /// @return A future to be fulfilled with either the deleted enrollment,
    /// or the network error that occurred
    ///
    /// @details
    /// The server will prevent users from deleting their last enrollment
    ///
    api::v1::management::EnrollmentResponse deleteEnrollment(const std::string& enrollmentID) {
        // std::cout << "Requesting to delete enrollment: " << enrollmentID << std::endl;
        // Create a context for the client.
        grpc::ClientContext context;
        // Create the request
        api::v1::management::DeleteEnrollmentRequest request;
        request.set_id(enrollmentID);
        // Execute the remote procedure call synchronously and return the result
        api::v1::management::EnrollmentResponse response;
        grpc::Status status = stub->DeleteEnrollment(&context, request, &response);
        if (!status.ok()) {  // an error occurred in the RPC
            // std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            throw "DeleteEnrollment failure";
        }
        return response;
    }

    /// @brief Request the deletion of multiple enrollments.
    ///
    /// @param ids: List of enrollment ids to delete from the server
    /// @return A future that will either contain a list of all server
    /// responses, or the first error to occur.
    ///
    /// @details
    /// If an error occurs during the deletion process, already completed
    /// deletions will not be rolled back. The server will prevent users from
    /// deleting their last enrollment.
    ///
    // std::vector<api::v1::management::EnrollmentResponse> deleteEnrollments(const std::vector<std::string>& ids) {
    //     // std::cout << "Deleting " << ids.size() << " enrollments" << std::endl;
    //     std::vector<api::v1::management::EnrollmentResponse> responses;
    //     for (const auto& id : ids) responses.emplace_back(deleteEnrollment(id));
    //     return responses;
    // }

    /// @brief Fetch a list of the current enrollment groups owned by a given
    /// userID.
    ///
    /// @param userID userID to fetch enrollment groups for
    /// @returns A future to be fulfilled with either a list of enrollment
    /// groups, or the network error that occurred
    ///
    api::v1::management::GetEnrollmentGroupsResponse getEnrollmentGroups(const std::string& userID) {
        // std::cout << "Requesting current enrollment groups from server with userID: " << userID << std::endl;
        // Create a context for the client.
        grpc::ClientContext context;
        // Create the request
        api::v1::management::GetEnrollmentsRequest request;
        request.set_userid(userID);
        // Execute the remote procedure call synchronously and return the result
        api::v1::management::GetEnrollmentGroupsResponse response;
        grpc::Status status = stub->GetEnrollmentGroups(&context, request, &response);
        if (!status.ok()) {  // an error occurred in the RPC
            // std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            throw "GetEnrollmentGroups failure";
        }
        return response;
    }

    /// @brief Create a new group of enrollments that can be used for group
    /// authentication.
    ///
    /// @param userID userID of the user that owns the enrollment group
    /// @param groupID Unique group identifier for the enrollment group, if
    /// empty an id will be automatically generated
    /// @param groupName Friendly display name to use for the enrollment group
    /// @param description Description of the enrollment group
    /// @param modelName The name of the model that all enrollments in this
    /// group will use
    /// @returns A future to be fulfilled with either the newly created
    /// enrollment group, or the network error that occurred
    ///
    /// @details
    /// Enrollment groups are initially created without any associated
    /// enrollments `appendEnrollmentGroup()` may be used to add enrollments
    /// to an enrollment group.
    ///
    api::v1::management::EnrollmentGroupResponse createEnrollmentGroup(
        const std::string& userID,
        const std::string& groupID,
        const std::string& groupName,
        const std::string& description,
        const std::string& modelName
    ) {
        // std::cout << "Requesting enrollment group creation with name: " << groupName << std::endl;
        // Create a context for the client.
        grpc::ClientContext context;
        // Create the request
        api::v1::management::CreateEnrollmentGroupRequest request;
        request.set_id(groupID);
        request.set_name(groupName);
        request.set_description(description);
        request.set_modelname(modelName);
        request.set_userid(userID);
        // Execute the remote procedure call synchronously and return the result
        api::v1::management::EnrollmentGroupResponse response;
        grpc::Status status = stub->CreateEnrollmentGroup(&context, request, &response);
        if (!status.ok()) {  // an error occurred in the RPC
            // std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            throw "CreateEnrollmentGroup failure";
        }
        return response;
    }

    /// @brief Append enrollments to an existing enrollment group.
    ///
    /// @param groupID GroupID of the enrollment group to append enrollments to
    /// @param enrollments A list of enrollment ids to append to the enrollment
    /// group
    /// @returns A future to be fulfilled with either the updated enrollment
    /// group, or the network error that occurred
    ///
    api::v1::management::EnrollmentGroupResponse appendEnrollmentGroup(
        const std::string& groupID,
        const std::vector<std::string>& enrollments
    ) {
        // std::cout << "Requesting to append enrollments to enrollment group: " << groupId << std::endl;
        // Create a context for the client.
        grpc::ClientContext context;
        // Create the request
        api::v1::management::AppendEnrollmentGroupRequest request;
        request.set_groupid(groupID);
        for (auto& enrollment: enrollments)
            request.add_enrollmentids(enrollment);
        // Execute the remote procedure call synchronously and return the result
        api::v1::management::EnrollmentGroupResponse response;
        grpc::Status status = stub->AppendEnrollmentGroup(&context, request, &response);
        if (!status.ok()) {  // an error occurred in the RPC
            // std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            throw "AppendEnrollmentGroup failure";
        }
        return response;
    }

    /// @brief Request the deletion of enrollment groups.
    ///
    /// @param id: group ID to delete
    /// @return A future to be fulfilled with either the deleted enrollment
    /// group, or the network error that occurred
    ///
    api::v1::management::EnrollmentGroupResponse deleteEnrollmentGroup(
        const std::string& id
    ) {
        // std::cout << "Requesting to delete enrollment group: " << id << std::endl;
        // Create a context for the client.
        grpc::ClientContext context;
        // Create the request
        api::v1::management::DeleteEnrollmentGroupRequest request;
        request.set_id(id);
        // Execute the remote procedure call synchronously and return the result
        api::v1::management::EnrollmentGroupResponse response;
        grpc::Status status = stub->DeleteEnrollmentGroup(&context, request, &response);
        if (!status.ok()) {  // an error occurred in the RPC
            // std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            throw "DeleteEnrollmentGroup failure";
        }
        return response;
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORY_CLOUD_SERVICES_MANAGEMENT_SERVICE_HPP_
