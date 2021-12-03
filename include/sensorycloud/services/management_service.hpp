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
#include <utility>
#include <vector>
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include "sensorycloud/generated/v1/management/enrollment.pb.h"
#include "sensorycloud/generated/v1/management/enrollment.grpc.pb.h"
#include "sensorycloud/config.hpp"
#include "sensorycloud/call_data.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief Sensory Cloud Services.
namespace service {

/// @brief A service for managing enrollments.
/// @tparam SecureCredentialStore A secure key-value store for storing and
/// fetching credentials and tokens.
template<typename SecureCredentialStore>
class ManagementService {
 private:
    /// the global configuration for the remote connection
    const ::sensory::Config& config;
    /// the token manager for securing gRPC requests to the server
    ::sensory::token_manager::TokenManager<SecureCredentialStore>& tokenManager;
    /// The gRPC stub for the enrollment service
    std::unique_ptr<::sensory::api::v1::management::EnrollmentService::Stub> stub;

    /// @brief Create a copy of this object.
    ///
    /// @param other the other instance to copy data from
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object
    ManagementService(const ManagementService& other);

 public:
    /// @brief Initialize a new management service.
    ///
    /// @param config_ the global configuration for the remote connection
    /// @param tokenManager_ the token manager for requesting Bearer tokens
    ///
    ManagementService(
        const ::sensory::Config& config_,
        ::sensory::token_manager::TokenManager<SecureCredentialStore>& tokenManager_
    ) : config(config_),
        tokenManager(tokenManager_),
        stub(::sensory::api::v1::management::EnrollmentService::NewStub(config.getChannel())) { }

    // ----- Get Enrollments ---------------------------------------------------

    /// @brief Fetch a list of the current enrollments for the given userID
    ///
    /// @param response the response to store the result of the RPC into
    /// @param userID userID to fetch enrollments for
    /// @returns A future to be fulfilled with either a list of enrollments,
    /// or the network error that occurred
    ///
    inline ::grpc::Status getEnrollments(
        ::sensory::api::v1::management::GetEnrollmentsResponse* response,
        const std::string& userID
    ) const {
        // Create a context for the client.
        ::grpc::ClientContext context;
        config.setupUnaryClientContext(context, tokenManager);
        // Create the request
        ::sensory::api::v1::management::GetEnrollmentsRequest request;
        request.set_userid(userID);
        // Execute the remote procedure call synchronously and return the status
        return stub->GetEnrollments(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous `GetEnrollments`
    /// calls.
    typedef ::sensory::CallData<
        ManagementService<SecureCredentialStore>,
        ::sensory::api::v1::management::GetEnrollmentsRequest,
        ::sensory::api::v1::management::GetEnrollmentsResponse
    > GetEnrollmentsCallData;

    /// @brief Fetch a list of the current enrollments for the given userID
    ///
    /// @tparam Callback the type of the callback function. The callback should
    /// accept a single pointer of type `GetEnrollmentsCallData*`.
    /// @param userID userID to fetch enrollments for
    /// @param callback The callback to execute when the response arrives
    /// @returns A pointer to the asynchronous call spawned by this call
    ///
    template<typename Callback>
    inline std::shared_ptr<GetEnrollmentsCallData> asyncGetEnrollments(
        const std::string& userID,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<GetEnrollmentsCallData>
            call(new GetEnrollmentsCallData);
        config.setupUnaryClientContext(call->context, tokenManager);
        call->request.set_userid(userID);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        stub->async()->GetEnrollments(
            &call->context,
            &call->request,
            &call->response,
            [call, callback](::grpc::Status status) {
                // Copy the status to the call.
                call->status = std::move(status);
                // Call the callback function with a raw pointer because
                // ownership is not being transferred.
                callback(call.get());
                // Mark the call as done for any awaiting process.
                call->isDone = true;
            });
        return call;
    }

    // ----- Delete Enrollment -------------------------------------------------

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
    inline ::grpc::Status deleteEnrollment(
        ::sensory::api::v1::management::EnrollmentResponse* response,
        const std::string& enrollmentID
    ) const {
        // Create a context for the client.
        grpc::ClientContext context;
        config.setupUnaryClientContext(context, tokenManager);
        // Create the request
        ::sensory::api::v1::management::DeleteEnrollmentRequest request;
        request.set_id(enrollmentID);
        // Execute the remote procedure call synchronously and return the result
        return stub->DeleteEnrollment(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `DeleteEnrollment` calls.
    typedef ::sensory::CallData<
        ManagementService<SecureCredentialStore>,
        ::sensory::api::v1::management::DeleteEnrollmentRequest,
        ::sensory::api::v1::management::EnrollmentResponse
    > DeleteEnrollmentCallData;

    /// @brief Request the deletion of an enrollment.
    ///
    /// @tparam Callback the type of the callback function. The callback should
    /// accept a single pointer of type `DeleteEnrollmentCallData*`.
    /// @param enrollmentID enrollmentID for the enrollment to delete
    /// @param callback The callback to execute when the response arrives
    /// @returns A pointer to the asynchronous call spawned by this call
    ///
    template<typename Callback>
    inline std::shared_ptr<DeleteEnrollmentCallData> asyncDeleteEnrollment(
        const std::string& enrollmentID,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<DeleteEnrollmentCallData>
            call(new DeleteEnrollmentCallData);
        config.setupUnaryClientContext(call->context, tokenManager);
        call->request.set_id(enrollmentID);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        stub->async()->DeleteEnrollment(
            &call->context,
            &call->request,
            &call->response,
            [call, callback](::grpc::Status status) {
                // Copy the status to the call.
                call->status = std::move(status);
                // Call the callback function with a raw pointer because
                // ownership is not being transferred.
                callback(call.get());
                // Mark the call as done for any awaiting process.
                call->isDone = true;
            });
        return call;
    }

    // ----- Get Enrollment Groups ---------------------------------------------

    /// @brief Fetch a list of the current enrollment groups owned by a given
    /// userID.
    ///
    /// @param response the response to store the result of the RPC into
    /// @param userID userID to fetch enrollment groups for
    /// @returns A future to be fulfilled with either a list of enrollment
    /// groups, or the network error that occurred
    ///
    inline ::grpc::Status getEnrollmentGroups(
        ::sensory::api::v1::management::GetEnrollmentGroupsResponse* response,
        const std::string& userID
    ) const {
        // Create a context for the client.
        ::grpc::ClientContext context;
        config.setupUnaryClientContext(context, tokenManager);
        // Create the request
        ::sensory::api::v1::management::GetEnrollmentsRequest request;
        request.set_userid(userID);
        // Execute the remote procedure call synchronously and return the result
        return stub->GetEnrollmentGroups(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `GetEnrollmentGroups` calls.
    typedef ::sensory::CallData<
        ManagementService<SecureCredentialStore>,
        ::sensory::api::v1::management::GetEnrollmentsRequest,
        ::sensory::api::v1::management::GetEnrollmentGroupsResponse
    > GetEnrollmentGroupsCallData;

    /// @brief Fetch a list of the current enrollment groups owned by a given
    /// userID.
    ///
    /// @tparam Callback the type of the callback function. The callback should
    /// accept a single pointer of type `GetEnrollmentGroupsCallData*`.
    /// @param userID userID to fetch enrollments for
    /// @param callback The callback to execute when the response arrives
    /// @returns A pointer to the asynchronous call spawned by this call
    ///
    template<typename Callback>
    inline std::shared_ptr<GetEnrollmentGroupsCallData> asyncGetEnrollmentGroups(
        const std::string& userID,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<GetEnrollmentGroupsCallData>
            call(new GetEnrollmentGroupsCallData);
        config.setupUnaryClientContext(call->context, tokenManager);
        call->request.set_userid(userID);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        stub->async()->GetEnrollmentGroups(
            &call->context,
            &call->request,
            &call->response,
            [call, callback](::grpc::Status status) {
                // Copy the status to the call.
                call->status = std::move(status);
                // Call the callback function with a raw pointer because
                // ownership is not being transferred.
                callback(call.get());
                // Mark the call as done for any awaiting process.
                call->isDone = true;
            });
        return call;
    }

    // ----- Create Enrollment Group -------------------------------------------

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
    inline ::grpc::Status createEnrollmentGroup(
        ::sensory::api::v1::management::EnrollmentGroupResponse* response,
        const std::string& userID,
        const std::string& groupID,
        const std::string& groupName,
        const std::string& description,
        const std::string& modelName
    ) const {
        // Create a context for the client.
        ::grpc::ClientContext context;
        config.setupUnaryClientContext(context, tokenManager);
        // Create the request
        ::sensory::api::v1::management::CreateEnrollmentGroupRequest request;
        request.set_userid(userID);
        request.set_id(
            groupID.empty() ? ::sensory::token_manager::uuid_v4() : groupID
        );
        request.set_name(groupName);
        request.set_description(description);
        request.set_modelname(modelName);
        // Execute the remote procedure call synchronously and return the result
        return stub->CreateEnrollmentGroup(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `CreateEnrollmentGroup` calls.
    typedef ::sensory::CallData<
        ManagementService<SecureCredentialStore>,
        ::sensory::api::v1::management::CreateEnrollmentGroupRequest,
        ::sensory::api::v1::management::EnrollmentGroupResponse
    > CreateEnrollmentGroupCallData;

    /// @brief Create a new group of enrollments that can be used for group
    /// authentication.
    ///
    /// @tparam Callback the type of the callback function. The callback should
    /// accept a single pointer of type `CreateEnrollmentGroupCallData*`.
    /// @param userID userID of the user that owns the enrollment group
    /// @param groupID Unique group identifier for the enrollment group, if
    /// empty an id will be automatically generated
    /// @param groupName Friendly display name to use for the enrollment group
    /// @param description Description of the enrollment group
    /// @param modelName The name of the model that all enrollments in this
    /// group will use
    /// @param callback The callback to execute when the response arrives
    /// @returns A pointer to the asynchronous call spawned by this call
    ///
    template<typename Callback>
    inline std::shared_ptr<CreateEnrollmentGroupCallData> asyncCreateEnrollmentGroup(
        const std::string& userID,
        const std::string& groupID,
        const std::string& groupName,
        const std::string& description,
        const std::string& modelName,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<CreateEnrollmentGroupCallData>
            call(new CreateEnrollmentGroupCallData);
        config.setupUnaryClientContext(call->context, tokenManager);
        call->request.set_userid(userID);
        call->request.set_id(
            groupID.empty() ? ::sensory::token_manager::uuid_v4() : groupID
        );
        call->request.set_name(groupName);
        call->request.set_description(description);
        call->request.set_modelname(modelName);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        stub->async()->CreateEnrollmentGroup(
            &call->context,
            &call->request,
            &call->response,
            [call, callback](::grpc::Status status) {
                // Copy the status to the call.
                call->status = std::move(status);
                // Call the callback function with a raw pointer because
                // ownership is not being transferred.
                callback(call.get());
                // Mark the call as done for any awaiting process.
                call->isDone = true;
            });
        return call;
    }

    // ----- Append Enrollment Group -------------------------------------------

    /// @brief Append enrollments to an existing enrollment group.
    ///
    /// @param response the response to store the result of the RPC into
    /// @param groupID The ID of the enrollment group to append enrollments to
    /// @param enrollments A list of enrollment ids to append to the enrollment
    /// group
    /// @returns A future to be fulfilled with either the updated enrollment
    /// group, or the network error that occurred
    ///
    inline ::grpc::Status appendEnrollmentGroup(
        ::sensory::api::v1::management::EnrollmentGroupResponse* response,
        const std::string& groupID,
        const std::vector<std::string>& enrollments
    ) const {
        // Create a context for the client.
        ::grpc::ClientContext context;
        config.setupUnaryClientContext(context, tokenManager);
        // Create the request
        ::sensory::api::v1::management::AppendEnrollmentGroupRequest request;
        request.set_groupid(groupID);
        for (auto& enrollment: enrollments)
            request.add_enrollmentids(enrollment);
        // Execute the remote procedure call synchronously and return the result
        return stub->AppendEnrollmentGroup(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `AppendEnrollmentGroup` calls.
    typedef ::sensory::CallData<
        ManagementService<SecureCredentialStore>,
        ::sensory::api::v1::management::AppendEnrollmentGroupRequest,
        ::sensory::api::v1::management::EnrollmentGroupResponse
    > AppendEnrollmentGroupCallData;

    /// @brief Append enrollments to an existing enrollment group.
    ///
    /// @tparam Callback the type of the callback function. The callback should
    /// accept a single pointer of type `AppendEnrollmentGroupCallData*`.
    /// @param groupID The ID of the enrollment group to append enrollments to
    /// @param enrollments A list of enrollment ids to append to the enrollment
    /// group
    /// @param callback The callback to execute when the response arrives
    /// @returns A pointer to the asynchronous call spawned by this call
    ///
    template<typename Callback>
    inline std::shared_ptr<AppendEnrollmentGroupCallData> asyncAppendEnrollmentGroup(
        const std::string& groupID,
        const std::vector<std::string>& enrollments,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<AppendEnrollmentGroupCallData>
            call(new AppendEnrollmentGroupCallData);
        config.setupUnaryClientContext(call->context, tokenManager);
        call->request.set_groupid(groupID);
        for (auto& enrollment: enrollments)
            call->request.add_enrollmentids(enrollment);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        stub->async()->AppendEnrollmentGroup(
            &call->context,
            &call->request,
            &call->response,
            [call, callback](::grpc::Status status) {
                // Copy the status to the call.
                call->status = std::move(status);
                // Call the callback function with a raw pointer because
                // ownership is not being transferred.
                callback(call.get());
                // Mark the call as done for any awaiting process.
                call->isDone = true;
            });
        return call;
    }

    // ----- Delete Enrollment Group -------------------------------------------

    /// @brief Request the deletion of enrollment groups.
    ///
    /// @param response the response to store the result of the RPC into
    /// @param groupID The group ID to delete
    /// @return A future to be fulfilled with either the deleted enrollment
    /// group, or the network error that occurred
    ///
    inline ::grpc::Status deleteEnrollmentGroup(
        ::sensory::api::v1::management::EnrollmentGroupResponse* response,
        const std::string& groupID
    ) const {
        // Create a context for the client.
        ::grpc::ClientContext context;
        config.setupUnaryClientContext(context, tokenManager);
        // Create the request
        ::sensory::api::v1::management::DeleteEnrollmentGroupRequest request;
        request.set_id(groupID);
        // Execute the remote procedure call synchronously and return the result
        return stub->DeleteEnrollmentGroup(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `DeleteEnrollmentGroupRequest` calls.
    typedef ::sensory::CallData<
        ManagementService<SecureCredentialStore>,
        ::sensory::api::v1::management::DeleteEnrollmentGroupRequest,
        ::sensory::api::v1::management::EnrollmentGroupResponse
    > DeleteEnrollmentGroupCallData;

    /// @brief Request the deletion of enrollment groups.
    ///
    /// @tparam Callback the type of the callback function. The callback should
    /// accept a single pointer of type `DeleteEnrollmentGroupCallData*`.
    /// @param groupID The group ID to delete
    /// @param callback The callback to execute when the response arrives
    /// @returns A pointer to the asynchronous call spawned by this call
    ///
    template<typename Callback>
    inline std::shared_ptr<DeleteEnrollmentGroupCallData> asyncDeleteEnrollmentGroup(
        const std::string& groupID,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<DeleteEnrollmentGroupCallData>
            call(new DeleteEnrollmentGroupCallData);
        config.setupUnaryClientContext(call->context, tokenManager);
        call->request.set_id(groupID);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        stub->async()->DeleteEnrollmentGroup(
            &call->context,
            &call->request,
            &call->response,
            [call, callback](::grpc::Status status) {
                // Copy the status to the call.
                call->status = std::move(status);
                // Call the callback function with a raw pointer because
                // ownership is not being transferred.
                callback(call.get());
                // Mark the call as done for any awaiting process.
                call->isDone = true;
            });
        return call;
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORY_CLOUD_SERVICES_MANAGEMENT_SERVICE_HPP_
