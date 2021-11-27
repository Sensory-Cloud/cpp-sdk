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
#include "sensorycloud/token_manager/token_manager.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud Services.
namespace service {

/// @brief A service for managing enrollments.
/// @tparam SecureCredentialStore a secure CRUD class for storing credentials.
template<typename SecureCredentialStore>
class ManagementService {
 private:
    /// the global configuration for the remote connection
    const Config& config;
    /// the token manager for securing gRPC requests to the server
    token_manager::TokenManager<SecureCredentialStore>& tokenManager;
    /// The gRPC stub for the enrollment service
    std::unique_ptr<api::v1::management::EnrollmentService::Stub> stub;

 public:
    /// @brief Initialize a new management service.
    ///
    /// @param config_ the global configuration for the remote connection
    /// @param tokenManager_ the token manager for requesting Bearer tokens
    ///
    explicit ManagementService(
        const Config& config_,
        token_manager::TokenManager<SecureCredentialStore>& tokenManager_
    ) : config(config_),
        tokenManager(tokenManager_),
        stub(api::v1::management::EnrollmentService::NewStub(config.getChannel())) { }

    /// @brief Fetch a list of the current enrollments for the given userID
    ///
    /// @param response the response to store the result of the RPC into
    /// @param userID userID to fetch enrollments for
    /// @returns A future to be fulfilled with either a list of enrollments,
    /// or the network error that occurred
    ///
    grpc::Status getEnrollments(
        api::v1::management::GetEnrollmentsResponse* response,
        const std::string& userID
    ) {
        // Create a context for the client.
        grpc::ClientContext context;
        config.setupClientContext(context, tokenManager, true);
        // Create the request
        api::v1::management::GetEnrollmentsRequest request;
        request.set_userid(userID);
        // Execute the remote procedure call synchronously and return the status
        return stub->GetEnrollments(&context, request, response);
    }

    /// @brief Request the deletion of an enrollment.
    ///
    /// @param response the response to store the result of the RPC into
    /// @param enrollmentID enrollmentID for the enrollment to delete
    /// @return A future to be fulfilled with either the deleted enrollment,
    /// or the network error that occurred
    ///
    /// @details
    /// The server will prevent users from deleting their last enrollment
    ///
    grpc::Status deleteEnrollment(
        api::v1::management::EnrollmentResponse* response,
        const std::string& enrollmentID
    ) {
        // Create a context for the client.
        grpc::ClientContext context;
        config.setupClientContext(context, tokenManager, true);
        // Create the request
        api::v1::management::DeleteEnrollmentRequest request;
        request.set_id(enrollmentID);
        // Execute the remote procedure call synchronously and return the result
        return stub->DeleteEnrollment(&context, request, response);
    }

    /// @brief Fetch a list of the current enrollment groups owned by a given
    /// userID.
    ///
    /// @param response the response to store the result of the RPC into
    /// @param userID userID to fetch enrollment groups for
    /// @returns A future to be fulfilled with either a list of enrollment
    /// groups, or the network error that occurred
    ///
    grpc::Status getEnrollmentGroups(
        api::v1::management::GetEnrollmentGroupsResponse* response,
        const std::string& userID
    ) {
        // Create a context for the client.
        grpc::ClientContext context;
        config.setupClientContext(context, tokenManager, true);
        // Create the request
        api::v1::management::GetEnrollmentsRequest request;
        request.set_userid(userID);
        // Execute the remote procedure call synchronously and return the result
        return stub->GetEnrollmentGroups(&context, request, response);
    }

    /// @brief Create a new group of enrollments that can be used for group
    /// authentication.
    ///
    /// @param response the response to store the result of the RPC into
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
    grpc::Status createEnrollmentGroup(
        api::v1::management::EnrollmentGroupResponse* response,
        const std::string& userID,
        const std::string& groupID,
        const std::string& groupName,
        const std::string& description,
        const std::string& modelName
    ) {
        // Create a context for the client.
        grpc::ClientContext context;
        config.setupClientContext(context, tokenManager, true);
        // Create the request
        api::v1::management::CreateEnrollmentGroupRequest request;
        request.set_id(groupID);
        request.set_name(groupName);
        request.set_description(description);
        request.set_modelname(modelName);
        request.set_userid(userID);
        // Execute the remote procedure call synchronously and return the result
        return stub->CreateEnrollmentGroup(&context, request, response);
    }

    /// @brief Append enrollments to an existing enrollment group.
    ///
    /// @param response the response to store the result of the RPC into
    /// @param groupID GroupID of the enrollment group to append enrollments to
    /// @param enrollments A list of enrollment ids to append to the enrollment
    /// group
    /// @returns A future to be fulfilled with either the updated enrollment
    /// group, or the network error that occurred
    ///
    grpc::Status appendEnrollmentGroup(
        api::v1::management::EnrollmentGroupResponse* response,
        const std::string& groupID,
        const std::vector<std::string>& enrollments
    ) {
        // Create a context for the client.
        grpc::ClientContext context;
        config.setupClientContext(context, tokenManager, true);
        // Create the request
        api::v1::management::AppendEnrollmentGroupRequest request;
        request.set_groupid(groupID);
        for (auto& enrollment: enrollments)
            request.add_enrollmentids(enrollment);
        // Execute the remote procedure call synchronously and return the result
        return stub->AppendEnrollmentGroup(&context, request, response);
    }

    /// @brief Request the deletion of enrollment groups.
    ///
    /// @param response the response to store the result of the RPC into
    /// @param id: group ID to delete
    /// @return A future to be fulfilled with either the deleted enrollment
    /// group, or the network error that occurred
    ///
    grpc::Status deleteEnrollmentGroup(
        api::v1::management::EnrollmentGroupResponse* response,
        const std::string& id
    ) {
        // Create a context for the client.
        grpc::ClientContext context;
        config.setupClientContext(context, tokenManager, true);
        // Create the request
        api::v1::management::DeleteEnrollmentGroupRequest request;
        request.set_id(id);
        // Execute the remote procedure call synchronously and return the result
        return stub->DeleteEnrollmentGroup(&context, request, response);
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORY_CLOUD_SERVICES_MANAGEMENT_SERVICE_HPP_
