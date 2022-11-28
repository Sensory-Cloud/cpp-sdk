// Management service for the SensoryCloud SDK.
//
// Copyright (c) 2022 Sensory, Inc.
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

#ifndef SENSORYCLOUD_SERVICES_MANAGEMENT_SERVICE_HPP_
#define SENSORYCLOUD_SERVICES_MANAGEMENT_SERVICE_HPP_

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "sensorycloud/generated/v1/management/enrollment.pb.h"
#include "sensorycloud/generated/v1/management/enrollment.grpc.pb.h"
#include "sensorycloud/config.hpp"
#include "sensorycloud/token_manager/token_manager.hpp"
#include "sensorycloud/calldata.hpp"

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief SensoryCloud services.
namespace service {

/// @brief A service for managing enrollments and enrollment groups.
/// @tparam CredentialStore A key-value store for storing and fetching
/// credentials and tokens.
template<typename CredentialStore>
class ManagementService {
 private:
    /// the global configuration for the remote connection
    const ::sensory::Config& config;
    /// the token manager for securing gRPC requests to the server
    ::sensory::token_manager::TokenManager<CredentialStore>& token_manager;
    /// The gRPC stub for the enrollment service
    std::unique_ptr<::sensory::api::v1::management::EnrollmentService::StubInterface> stub;

    /// @brief Create a copy of this object.
    ///
    /// @param other the other instance to copy data from
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object
    ManagementService(const ManagementService& other) = delete;

    /// @brief Assign to this object using the `=` operator.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This assignment operator is private to prevent copying of this object.
    ///
    void operator=(const ManagementService& other) = delete;

 public:
    /// @brief Initialize a new management service.
    ///
    /// @param config_ The global configuration for the remote connection.
    /// @param token_manager_ The token manager for requesting Bearer tokens.
    ///
    ManagementService(
        const ::sensory::Config& config_,
        ::sensory::token_manager::TokenManager<CredentialStore>& token_manager_
    ) : config(config_),
        token_manager(token_manager_),
        stub(::sensory::api::v1::management::EnrollmentService::NewStub(config.get_channel())) { }

    /// @brief Initialize a new management service.
    ///
    /// @param config_ The global configuration for the remote connection.
    /// @param token_manager_ The token manager for requesting Bearer tokens.
    /// @param stub_ The enrollment stub to initialize the service with.
    ///
    ManagementService(
        const ::sensory::Config& config_,
        ::sensory::token_manager::TokenManager<CredentialStore>& token_manager_,
        ::sensory::api::v1::management::EnrollmentService::StubInterface* stub_
    ) : config(config_), token_manager(token_manager_), stub(stub_) { }

    /// @brief Return the cloud configuration associated with this service.
    ///
    /// @returns the configuration used by this service.
    ///
    inline const ::sensory::Config& get_config() const { return config; }

    // ----- Get Enrollments ---------------------------------------------------

    /// @brief Fetch a list of the current enrollments for the given userID
    ///
    /// @param response The response to store the result of the RPC into.
    /// @param userID The ID of the user to fetch enrollments for.
    /// @returns A gRPC status object indicating whether the call succeeded.
    ///
    inline ::grpc::Status get_enrollments(
        ::sensory::api::v1::management::GetEnrollmentsResponse* response,
        const std::string& userID
    ) const {
        // Create a context for the client.
        ::grpc::ClientContext context;
        token_manager.setup_unary_client_context(context);
        // Create the request
        ::sensory::api::v1::management::GetEnrollmentsRequest request;
        request.set_userid(userID);
        // Execute the remote procedure call synchronously and return the status
        return stub->GetEnrollments(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous `GetEnrollments`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncResponseReaderCall<
        ManagementService<CredentialStore>,
        ::sensory::api::v1::management::GetEnrollmentsRequest,
        ::sensory::api::v1::management::GetEnrollmentsResponse
    > GetEnrollmentsAsyncCall;

    /// @brief Fetch a list of the current enrollments for the given userID
    ///
    /// @param queue The completion queue handling the event-loop processing.
    /// @param userID The ID of the user to fetch enrollments for.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    inline GetEnrollmentsAsyncCall* get_enrollments(
        ::grpc::CompletionQueue* queue,
        const std::string& userID
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new GetEnrollmentsAsyncCall);
        token_manager.setup_unary_client_context(call->context);
        // Start the asynchronous RPC with the call's context and queue.
        call->request.set_userid(userID);
        call->rpc = stub->AsyncGetEnrollments(&call->context, call->request, queue);
        // Finish the RPC to tell it where the response and status buffers are
        // located within the call object. Use the address of the call as the
        // tag for identifying the call in the event-loop.
        call->rpc->Finish(&call->response, &call->status, static_cast<void*>(call));
        // Return the pointer to the call. This both transfers the ownership
        // of the instance to the caller, and provides the caller with an
        // identifier for detecting the result of this call in the completion
        // queue.
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous `GetEnrollments`
    /// calls.
    typedef ::sensory::calldata::CallbackData<
        ManagementService<CredentialStore>,
        ::sensory::api::v1::management::GetEnrollmentsRequest,
        ::sensory::api::v1::management::GetEnrollmentsResponse
    > GetEnrollmentsCallbackData;

    /// @brief Fetch a list of the current enrollments for the given userID
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `GetEnrollmentsCallbackData*`.
    /// @param userID The ID of the user to fetch enrollments for.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the asynchronous call spawned by this call.
    ///
    template<typename Callback>
    inline std::shared_ptr<GetEnrollmentsCallbackData> get_enrollments(
        const std::string& userID,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<GetEnrollmentsCallbackData>
            call(new GetEnrollmentsCallbackData);
        token_manager.setup_unary_client_context(call->context);
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
                call->setIsDone();
            });
        return call;
    }

    // ----- Delete Enrollment -------------------------------------------------

    /// @brief Request the deletion of an enrollment.
    ///
    /// @param response The response to store the result of the RPC into.
    /// @param enrollmentID The ID of the enrollment to delete.
    /// @returns A gRPC status object indicating whether the call succeeded.
    ///
    /// @details
    /// The server will prevent users from deleting their last enrollment.
    ///
    inline ::grpc::Status delete_enrollment(
        ::sensory::api::v1::management::EnrollmentResponse* response,
        const std::string& enrollmentID
    ) const {
        // Create a context for the client.
        grpc::ClientContext context;
        token_manager.setup_unary_client_context(context);
        // Create the request
        ::sensory::api::v1::management::DeleteEnrollmentRequest request;
        request.set_id(enrollmentID);
        // Execute the remote procedure call synchronously and return the result
        return stub->DeleteEnrollment(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous `DeleteEnrollment`
    /// calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncResponseReaderCall<
        ManagementService<CredentialStore>,
        ::sensory::api::v1::management::DeleteEnrollmentRequest,
        ::sensory::api::v1::management::EnrollmentResponse
    > DeleteEnrollmentAsyncCall;

    /// @brief Request the deletion of an enrollment.
    ///
    /// @param queue The completion queue handling the event-loop processing.
    /// @param enrollmentID The ID of the enrollment to delete.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    inline DeleteEnrollmentAsyncCall* delete_enrollment(
        ::grpc::CompletionQueue* queue,
        const std::string& enrollmentID
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new DeleteEnrollmentAsyncCall);
        token_manager.setup_unary_client_context(call->context);
        // Start the asynchronous RPC with the call's context and queue.
        call->request.set_id(enrollmentID);
        call->rpc = stub->AsyncDeleteEnrollment(&call->context, call->request, queue);
        // Finish the RPC to tell it where the response and status buffers are
        // located within the call object. Use the address of the call as the
        // tag for identifying the call in the event-loop.
        call->rpc->Finish(&call->response, &call->status, static_cast<void*>(call));
        // Return the pointer to the call. This both transfers the ownership
        // of the instance to the caller, and provides the caller with an
        // identifier for detecting the result of this call in the completion
        // queue.
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `DeleteEnrollment` calls.
    typedef ::sensory::calldata::CallbackData<
        ManagementService<CredentialStore>,
        ::sensory::api::v1::management::DeleteEnrollmentRequest,
        ::sensory::api::v1::management::EnrollmentResponse
    > DeleteEnrollmentCallbackData;

    /// @brief Request the deletion of an enrollment.
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `DeleteEnrollmentCallbackData*`.
    /// @param enrollmentID The ID of the enrollment to delete.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the asynchronous call spawned by this call.
    ///
    /// @details
    /// The server will prevent users from deleting their last enrollment.
    ///
    template<typename Callback>
    inline std::shared_ptr<DeleteEnrollmentCallbackData> delete_enrollment(
        const std::string& enrollmentID,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<DeleteEnrollmentCallbackData>
            call(new DeleteEnrollmentCallbackData);
        token_manager.setup_unary_client_context(call->context);
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
                call->setIsDone();
            });
        return call;
    }

    // ----- Get Enrollment Groups ---------------------------------------------

    /// @brief Fetch a list of the current enrollment groups owned by a given
    /// userID.
    ///
    /// @param response The response to store the result of the RPC into.
    /// @param userID The ID of the user to fetch enrollment groups for.
    /// @returns A gRPC status object indicating whether the call succeeded.
    ///
    inline ::grpc::Status get_enrollment_groups(
        ::sensory::api::v1::management::GetEnrollmentGroupsResponse* response,
        const std::string& userID
    ) const {
        // Create a context for the client.
        ::grpc::ClientContext context;
        token_manager.setup_unary_client_context(context);
        // Create the request
        ::sensory::api::v1::management::GetEnrollmentsRequest request;
        request.set_userid(userID);
        // Execute the remote procedure call synchronously and return the result
        return stub->GetEnrollmentGroups(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `GetEnrollmentGroups` calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncResponseReaderCall<
        ManagementService<CredentialStore>,
        ::sensory::api::v1::management::GetEnrollmentsRequest,
        ::sensory::api::v1::management::GetEnrollmentGroupsResponse
    > GetEnrollmentGroupsAsyncCall;

    /// @brief Fetch a list of the current enrollment groups owned by a given
    /// userID.
    ///
    /// @param queue The completion queue handling the event-loop processing.
    /// @param userID The ID of the user to fetch enrollment groups for.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    inline GetEnrollmentGroupsAsyncCall* get_enrollment_groups(
        ::grpc::CompletionQueue* queue,
        const std::string& userID
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new GetEnrollmentGroupsAsyncCall);
        token_manager.setup_unary_client_context(call->context);
        // Start the asynchronous RPC with the call's context and queue.
        call->request.set_userid(userID);
        call->rpc = stub->AsyncGetEnrollmentGroups(&call->context, call->request, queue);
        // Finish the RPC to tell it where the response and status buffers are
        // located within the call object. Use the address of the call as the
        // tag for identifying the call in the event-loop.
        call->rpc->Finish(&call->response, &call->status, static_cast<void*>(call));
        // Return the pointer to the call. This both transfers the ownership
        // of the instance to the caller, and provides the caller with an
        // identifier for detecting the result of this call in the completion
        // queue.
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `GetEnrollmentGroups` calls.
    typedef ::sensory::calldata::CallbackData<
        ManagementService<CredentialStore>,
        ::sensory::api::v1::management::GetEnrollmentsRequest,
        ::sensory::api::v1::management::GetEnrollmentGroupsResponse
    > GetEnrollmentGroupsCallbackData;

    /// @brief Fetch a list of the current enrollment groups owned by a given
    /// userID.
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `GetEnrollmentGroupsCallbackData*`.
    /// @param userID The ID of the user to fetch enrollments for.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the asynchronous call spawned by this call.
    ///
    template<typename Callback>
    inline std::shared_ptr<GetEnrollmentGroupsCallbackData> get_enrollment_groups(
        const std::string& userID,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<GetEnrollmentGroupsCallbackData>
            call(new GetEnrollmentGroupsCallbackData);
        token_manager.setup_unary_client_context(call->context);
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
                call->setIsDone();
            });
        return call;
    }

    // ----- Create Enrollment Group -------------------------------------------

    /// @brief Create a new group of enrollments that can be used for group
    /// authentication.
    ///
    /// @param response The response to store the result of the RPC into.
    /// @param userID The ID of the user that owns the enrollment group.
    /// @param groupID A unique group identifier for the enrollment group. If
    /// empty, an ID will be automatically generated.
    /// @param groupName A friendly display name to use for the enrollment
    /// group.
    /// @param description A description of the enrollment group.
    /// @param modelName The name of the model that all enrollments in this
    /// group will use.
    /// @param enrollments The vector of enrollments to create the group with.
    /// @returns A gRPC status object indicating whether the call succeeded.
    ///
    /// @details
    /// Enrollment groups are initially created without any associated
    /// enrollments `append_enrollment_group()` may be used to add enrollments
    /// to an enrollment group.
    ///
    inline ::grpc::Status create_enrollment_group(
        ::sensory::api::v1::management::EnrollmentGroupResponse* response,
        const std::string& userID,
        const std::string& groupID,
        const std::string& groupName,
        const std::string& description,
        const std::string& modelName,
        const std::vector<std::string>& enrollments
    ) const {
        // Create a context for the client.
        ::grpc::ClientContext context;
        token_manager.setup_unary_client_context(context);
        // Create the request
        ::sensory::api::v1::management::CreateEnrollmentGroupRequest request;
        request.set_userid(userID);
        request.set_id(
            groupID.empty() ? ::sensory::util::uuid_v4() : groupID
        );
        request.set_name(groupName);
        request.set_description(description);
        request.set_modelname(modelName);
        for (auto& enrollment: enrollments)
            request.add_enrollmentids(enrollment);
        // Execute the remote procedure call synchronously and return the result
        return stub->CreateEnrollmentGroup(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `CreateEnrollmentGroup` calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncResponseReaderCall<
        ManagementService<CredentialStore>,
        ::sensory::api::v1::management::CreateEnrollmentGroupRequest,
        ::sensory::api::v1::management::EnrollmentGroupResponse
    > CreateEnrollmentGroupAsyncCall;

    /// @brief Create a new group of enrollments that can be used for group
    /// authentication.
    ///
    /// @param queue The completion queue handling the event-loop processing.
    /// @param userID The ID of the user that owns the enrollment group.
    /// @param groupID A unique group identifier for the enrollment group. If
    /// empty, an ID will be automatically generated.
    /// @param groupName A friendly display name to use for the enrollment
    /// group.
    /// @param description A description of the enrollment group.
    /// @param modelName The name of the model that all enrollments in this
    /// group will use.
    /// @param enrollments The vector of enrollments to create the group with.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    inline CreateEnrollmentGroupAsyncCall* create_enrollment_group(
        ::grpc::CompletionQueue* queue,
        const std::string& userID,
        const std::string& groupID,
        const std::string& groupName,
        const std::string& description,
        const std::string& modelName,
        const std::vector<std::string>& enrollments
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new CreateEnrollmentGroupAsyncCall);
        token_manager.setup_unary_client_context(call->context);
        // Start the asynchronous RPC with the call's context and queue.
        call->request.set_userid(userID);
        call->request.set_id(
            groupID.empty() ? ::sensory::util::uuid_v4() : groupID
        );
        call->request.set_name(groupName);
        call->request.set_description(description);
        call->request.set_modelname(modelName);
        for (auto& enrollment: enrollments)
            call->request.add_enrollmentids(enrollment);
        call->rpc = stub->AsyncCreateEnrollmentGroup(&call->context, call->request, queue);
        // Finish the RPC to tell it where the response and status buffers are
        // located within the call object. Use the address of the call as the
        // tag for identifying the call in the event-loop.
        call->rpc->Finish(&call->response, &call->status, static_cast<void*>(call));
        // Return the pointer to the call. This both transfers the ownership
        // of the instance to the caller, and provides the caller with an
        // identifier for detecting the result of this call in the completion
        // queue.
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `CreateEnrollmentGroup` calls.
    typedef ::sensory::calldata::CallbackData<
        ManagementService<CredentialStore>,
        ::sensory::api::v1::management::CreateEnrollmentGroupRequest,
        ::sensory::api::v1::management::EnrollmentGroupResponse
    > CreateEnrollmentGroupCallbackData;

    /// @brief Create a new group of enrollments that can be used for group
    /// authentication.
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `CreateEnrollmentGroupCallbackData*`.
    /// @param userID The ID of the user that owns the enrollment group.
    /// @param groupID A unique group identifier for the enrollment group. If
    /// empty, an ID will be automatically generated.
    /// @param groupName A friendly display name to use for the enrollment
    /// group.
    /// @param description A description of the enrollment group.
    /// @param modelName The name of the model that all enrollments in this
    /// group will use.
    /// @param enrollments The vector of enrollments to create the group with.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the asynchronous call spawned by this call.
    ///
    template<typename Callback>
    inline std::shared_ptr<CreateEnrollmentGroupCallbackData> create_enrollment_group(
        const std::string& userID,
        const std::string& groupID,
        const std::string& groupName,
        const std::string& description,
        const std::string& modelName,
        const std::vector<std::string>& enrollments,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<CreateEnrollmentGroupCallbackData>
            call(new CreateEnrollmentGroupCallbackData);
        token_manager.setup_unary_client_context(call->context);
        call->request.set_userid(userID);
        call->request.set_id(
            groupID.empty() ? ::sensory::util::uuid_v4() : groupID
        );
        call->request.set_name(groupName);
        call->request.set_description(description);
        call->request.set_modelname(modelName);
        for (auto& enrollment: enrollments)
            call->request.add_enrollmentids(enrollment);
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
                call->setIsDone();
            });
        return call;
    }

    // ----- Append Enrollment Group -------------------------------------------

    /// @brief Append enrollments to an existing enrollment group.
    ///
    /// @param response The response to store the result of the RPC into.
    /// @param groupID The ID of the enrollment group to append enrollments to.
    /// @param enrollments A list of enrollment IDs to append to the enrollment
    /// group.
    /// @returns A gRPC status object indicating whether the call succeeded.
    ///
    inline ::grpc::Status append_enrollment_group(
        ::sensory::api::v1::management::EnrollmentGroupResponse* response,
        const std::string& groupID,
        const std::vector<std::string>& enrollments
    ) const {
        // Create a context for the client.
        ::grpc::ClientContext context;
        token_manager.setup_unary_client_context(context);
        // Create the request
        ::sensory::api::v1::management::AppendEnrollmentGroupRequest request;
        request.set_groupid(groupID);
        for (auto& enrollment: enrollments)
            request.add_enrollmentids(enrollment);
        // Execute the remote procedure call synchronously and return the result
        return stub->AppendEnrollmentGroup(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `AppendEnrollmentGroup` calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncResponseReaderCall<
        ManagementService<CredentialStore>,
        ::sensory::api::v1::management::AppendEnrollmentGroupRequest,
        ::sensory::api::v1::management::EnrollmentGroupResponse
    > AppendEnrollmentGroupAsyncCall;

    /// @brief Append enrollments to an existing enrollment group.
    ///
    /// @param queue The completion queue handling the event-loop processing.
    /// @param groupID The ID of the enrollment group to append enrollments to.
    /// @param enrollments A list of enrollment IDs to append to the enrollment
    /// group.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    inline AppendEnrollmentGroupAsyncCall* append_enrollment_group(
        ::grpc::CompletionQueue* queue,
        const std::string& groupID,
        const std::vector<std::string>& enrollments
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new AppendEnrollmentGroupAsyncCall);
        token_manager.setup_unary_client_context(call->context);
        // Start the asynchronous RPC with the call's context and queue.
        call->request.set_groupid(groupID);
        for (auto& enrollment: enrollments)
            call->request.add_enrollmentids(enrollment);
        call->rpc = stub->AsyncAppendEnrollmentGroup(&call->context, call->request, queue);
        // Finish the RPC to tell it where the response and status buffers are
        // located within the call object. Use the address of the call as the
        // tag for identifying the call in the event-loop.
        call->rpc->Finish(&call->response, &call->status, static_cast<void*>(call));
        // Return the pointer to the call. This both transfers the ownership
        // of the instance to the caller, and provides the caller with an
        // identifier for detecting the result of this call in the completion
        // queue.
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `AppendEnrollmentGroup` calls.
    typedef ::sensory::calldata::CallbackData<
        ManagementService<CredentialStore>,
        ::sensory::api::v1::management::AppendEnrollmentGroupRequest,
        ::sensory::api::v1::management::EnrollmentGroupResponse
    > AppendEnrollmentGroupCallbackData;

    /// @brief Append enrollments to an existing enrollment group.
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `AppendEnrollmentGroupCallbackData*`.
    /// @param groupID The ID of the enrollment group to append enrollments to.
    /// @param enrollments A list of enrollment IDs to append to the enrollment
    /// group.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the asynchronous call spawned by this call.
    ///
    template<typename Callback>
    inline std::shared_ptr<AppendEnrollmentGroupCallbackData> append_enrollment_group(
        const std::string& groupID,
        const std::vector<std::string>& enrollments,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<AppendEnrollmentGroupCallbackData>
            call(new AppendEnrollmentGroupCallbackData);
        token_manager.setup_unary_client_context(call->context);
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
                call->setIsDone();
            });
        return call;
    }

    // ----- Delete Enrollment Group -------------------------------------------

    /// @brief Request the deletion of enrollment groups.
    ///
    /// @param response The response to store the result of the RPC into.
    /// @param groupID The ID of the group to delete.
    /// @returns A gRPC status object indicating whether the call succeeded.
    ///
    inline ::grpc::Status delete_enrollment_group(
        ::sensory::api::v1::management::EnrollmentGroupResponse* response,
        const std::string& groupID
    ) const {
        // Create a context for the client.
        ::grpc::ClientContext context;
        token_manager.setup_unary_client_context(context);
        // Create the request
        ::sensory::api::v1::management::DeleteEnrollmentGroupRequest request;
        request.set_id(groupID);
        // Execute the remote procedure call synchronously and return the result
        return stub->DeleteEnrollmentGroup(&context, request, response);
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `DeleteEnrollmentGroup` calls based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncResponseReaderCall<
        ManagementService<CredentialStore>,
        ::sensory::api::v1::management::DeleteEnrollmentGroupRequest,
        ::sensory::api::v1::management::EnrollmentGroupResponse
    > DeleteEnrollmentGroupAsyncCall;

    /// @brief Request the deletion of enrollment groups.
    ///
    /// @param queue The completion queue handling the event-loop processing.
    /// @param groupID The ID of the group to delete.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    inline DeleteEnrollmentGroupAsyncCall* delete_enrollment_group(
        ::grpc::CompletionQueue* queue,
        const std::string& groupID
    ) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new DeleteEnrollmentGroupAsyncCall);
        token_manager.setup_unary_client_context(call->context);
        // Start the asynchronous RPC with the call's context and queue.
        call->request.set_id(groupID);
        call->rpc = stub->AsyncDeleteEnrollmentGroup(&call->context, call->request, queue);
        // Finish the RPC to tell it where the response and status buffers are
        // located within the call object. Use the address of the call as the
        // tag for identifying the call in the event-loop.
        call->rpc->Finish(&call->response, &call->status, static_cast<void*>(call));
        // Return the pointer to the call. This both transfers the ownership
        // of the instance to the caller, and provides the caller with an
        // identifier for detecting the result of this call in the completion
        // queue.
        return call;
    }

    /// @brief A type for encapsulating data for asynchronous
    /// `DeleteEnrollmentGroupRequest` calls.
    typedef ::sensory::calldata::CallbackData<
        ManagementService<CredentialStore>,
        ::sensory::api::v1::management::DeleteEnrollmentGroupRequest,
        ::sensory::api::v1::management::EnrollmentGroupResponse
    > DeleteEnrollmentGroupCallbackData;

    /// @brief Request the deletion of enrollment groups.
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `DeleteEnrollmentGroupCallbackData*`.
    /// @param groupID The ID of the group to delete.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the asynchronous call spawned by this call.
    ///
    template<typename Callback>
    inline std::shared_ptr<DeleteEnrollmentGroupCallbackData> delete_enrollment_group(
        const std::string& groupID,
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<DeleteEnrollmentGroupCallbackData>
            call(new DeleteEnrollmentGroupCallbackData);
        token_manager.setup_unary_client_context(call->context);
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
                call->setIsDone();
            });
        return call;
    }
};

}  // namespace service

}  // namespace sensory

#endif  // SENSORYCLOUD_SERVICES_MANAGEMENT_SERVICE_HPP_
