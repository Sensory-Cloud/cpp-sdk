// A type for encapsulating data for asynchronous unary read calls.
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

#ifndef SENSORYCLOUD_CALLDATA_ASYNC_RESPONSE_READER_CALL_HPP_
#define SENSORYCLOUD_CALLDATA_ASYNC_RESPONSE_READER_CALL_HPP_

#include <grpc/grpc.h>
#include <grpcpp/client_context.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <utility>

/// @brief The SensoryCloud SDK.
namespace sensory {

/// @brief Abstractions of asynchronous call data.
namespace calldata {

/// @brief A type for encapsulating data for asynchronous unary read calls.
/// @tparam Factory The factory class that will manage the scope of the call.
/// @tparam Request The type of the request message.
/// @tparam Response The type of the response message.
///
/// @details
/// The `Factory` is marked as a friend in order to provide mutable access to
/// private attributes of the structure. This allows instances of `Factory` to
/// mutate to the structure while all external scopes are limited to the
/// immutable interface exposed by the public accessor functions. Instances of
/// `AsyncResponseReaderCall` are mutable within the scope of `Factory`, but
/// immutable outside of the scope of `Factory`.
///
template<typename Factory, typename Request, typename Response>
struct AsyncResponseReaderCall {
 public:
    /// A type for encapsulating the RPC stream.
    typedef ::grpc::ClientAsyncResponseReaderInterface<Response> RPC;

 private:
    /// The gPRC context that the call is initiated with.
    ::grpc::ClientContext context;
    /// The status of the RPC after the response is processed.
    ::grpc::Status status;
    /// The request to execute in the unary call.
    Request request;
    /// The response to process after the RPC completes.
    Response response;
    /// The reader RPC executing the call.
    std::unique_ptr<RPC> rpc;

    /// @brief Create a copy of this object.
    ///
    /// @param other the other instance to copy data from
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object
    AsyncResponseReaderCall(const AsyncResponseReaderCall& other) = delete;

    /// @brief Assign to this object using the `=` operator.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This assignment operator is private to prevent copying of this object.
    ///
    void operator=(const AsyncResponseReaderCall& other) = delete;

    // Mark the Factory type as a friend to allow it to have write access to
    // the internal types. This allows the parent scope to have mutability, but
    // all other scopes must access data through the immutable `get` interface.
    friend Factory;

 public:
    /// @brief Initialize a new call.
    AsyncResponseReaderCall() { }

    /// @brief Return the context that the call was created with.
    ///
    /// @returns The gRPC context associated with the call.
    ///
    inline const ::grpc::ClientContext& getContext() const { return context; }

    /// @brief Return the status of the call.
    ///
    /// @returns The gRPC status code and message from the call.
    ///
    inline const ::grpc::Status& getStatus() const { return status; }

    /// @brief Return the request of the call.
    ///
    /// @returns The request message buffer for the call.
    ///
    inline const Request& getRequest() const { return request; }

    /// @brief Return the response of the call.
    ///
    /// @returns The response message buffer for the call.
    ///
    inline const Response& getResponse() const { return response; }

    /// @brief Return the gRPC client interface associated with this call.
    ///
    /// @returns the gRPC stream that is represented by this call object.
    ///
    inline RPC* getCall() const { return rpc.get(); }
};

}  // namespace calldata

}  // namespace sensory

#endif  // SENSORYCLOUD_CALLDATA_ASYNC_RESPONSE_READER_CALL_HPP_
