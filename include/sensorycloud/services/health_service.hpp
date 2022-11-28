// The health service for the SensoryCloud SDK.
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

#ifndef SENSORYCLOUD_SERVICES_HEALTH_SERVICE_HPP_
#define SENSORYCLOUD_SERVICES_HEALTH_SERVICE_HPP_

#include <memory>
#include <string>
#include <utility>
#include "sensorycloud/generated/health/health.pb.h"
#include "sensorycloud/generated/health/health.grpc.pb.h"
#include "sensorycloud/config.hpp"
#include "sensorycloud/calldata.hpp"

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief SensoryCloud services.
namespace service {

/// @brief A service for querying the health of the remote server.
class HealthService {
 private:
    /// the global configuration for the remote connection
    const ::sensory::Config& config;
    /// The gRPC stub for the health service
    std::unique_ptr<::sensory::api::health::HealthService::StubInterface> stub;

    /// @brief Create a copy of this object.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object.
    ///
    HealthService(const HealthService& other) = delete;

    /// @brief Assign to this object using the `=` operator.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This assignment operator is private to prevent copying of this object.
    ///
    void operator=(const HealthService& other) = delete;

 public:
    /// @brief Initialize a new health service.
    ///
    /// @param config_ The global configuration for the remote connection.
    ///
    explicit HealthService(const ::sensory::Config& config_) : config(config_),
        stub(::sensory::api::health::HealthService::NewStub(config.get_channel())) { }

    /// @brief Initialize a new health service.
    ///
    /// @param config_ The global configuration for the remote connection.
    /// @param stub_ The health service stub to initialize the service with.
    ///
    explicit HealthService(
        const ::sensory::Config& config_,
        ::sensory::api::health::HealthService::StubInterface* stub_
    ) : config(config_), stub(stub_) { }

    /// @brief Return the cloud configuration associated with this service.
    ///
    /// @returns the configuration used by this service.
    ///
    inline const ::sensory::Config& get_config() const { return config; }

    /// @brief Get the health status of the remote server.
    ///
    /// @param response The response object to store the result of the RPC into.
    /// @returns A gRPC status object indicating whether the call succeeded.
    ///
    inline ::grpc::Status get_health(
        ::sensory::api::common::ServerHealthResponse* response
    ) const {
        // Create a client context to query the health service. This request
        // does not require the "authorization" : "Bearer <token>" for auth.
        ::grpc::ClientContext context;
        // Create the parameter-less request to execute on the remote server
        return stub->GetHealth(&context, {}, response);
    }

    /// @brief A type for encapsulating data for asynchronous `GetModels` calls
    /// based on CompletionQueue event loops.
    typedef ::sensory::calldata::AsyncResponseReaderCall<
        HealthService,
        ::sensory::api::health::HealthRequest,
        ::sensory::api::common::ServerHealthResponse
    > GetHealthAsyncCall;

    /// @brief Get the health status of the remote server.
    ///
    /// @param queue The completion queue handling the event-loop processing.
    /// @returns A pointer to the call data associated with this asynchronous
    /// call. This pointer can be used to identify the call in the event-loop
    /// as the `tag` of the event. Ownership of the pointer passes to the
    /// caller and the caller should `delete` the pointer after it appears in
    /// a completion queue loop.
    ///
    inline GetHealthAsyncCall* get_health(::grpc::CompletionQueue* queue) const {
        // Create a call data object to store the client context, the response,
        // the status of the call, and the response reader. The ownership of
        // this object is passed to the caller.
        auto call(new GetHealthAsyncCall);
        // Start the asynchronous RPC with the call's context and queue.
        call->rpc = stub->AsyncGetHealth(&call->context, call->request, queue);
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

    /// @brief A type for encapsulating data for asynchronous `GetHealth` calls.
    typedef ::sensory::calldata::CallbackData<
        HealthService,
        ::sensory::api::health::HealthRequest,
        ::sensory::api::common::ServerHealthResponse
    > GetHealthCallbackData;

    /// @brief Get the health status of the remote server.
    ///
    /// @tparam Callback The type of the callback function. The callback should
    /// accept a single pointer of type `GetHealthCallbackData*`.
    /// @param callback The callback to execute when the response arrives.
    /// @returns A pointer to the asynchronous call spawned by this call.
    ///
    template<typename Callback>
    inline std::shared_ptr<GetHealthCallbackData> get_health(
        const Callback& callback
    ) const {
        // Create a call to encapsulate data that needs to exist throughout the
        // scope of the call. This call is initiated as a shared pointer in
        // order to reference count between the parent and child context. This
        // also allows the caller to safely use `await()` without the
        // possibility of a race condition.
        std::shared_ptr<GetHealthCallbackData> call(new GetHealthCallbackData);
        // Start the asynchronous call with the data from the request and
        // forward the input callback into the reactor callback.
        stub->async()->GetHealth(
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

#endif  // SENSORYCLOUD_SERVICES_HEALTH_SERVICE_HPP_
