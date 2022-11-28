// An abstract reactor for asynchronous unary write streams.
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

#ifndef SENSORYCLOUD_CALLDATA_AWAITABLE_WRITE_REACTOR_HPP_
#define SENSORYCLOUD_CALLDATA_AWAITABLE_WRITE_REACTOR_HPP_

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

/// @brief An abstract reactor for asynchronous unary write streams.
/// @tparam Factory The factory class that will manage the scope of the stream.
/// @tparam Request The type of the request message.
///
/// @details
/// The `Factory` is marked as a friend in order to provide mutable access to
/// private attributes of the structure. This allows instances of `Factory` to
/// mutate to the structure while all external scopes are limited to the
/// immutable interface exposed by the public accessor functions. Instances of
/// `AwaitableBidiReactor` are mutable within the scope of `Factory`, but
/// immutable outside of the scope of `Factory`.
///
template<typename Factory, typename Request>
class AwaitableWriteReactor : public ::grpc::ClientWriteReactor<Request> {
 private:
    /// The gPRC context that the call is initiated with.
    ::grpc::ClientContext context;
    /// The status of the RPC after the response is processed.
    ::grpc::Status status;
    /// A flag determining whether the asynchronous has terminated.
    bool isDone;
    /// A mutex for guarding access to the `isDone` and `status` variables.
    std::mutex mutex;
    /// A condition variable for signalling to an awaiting process.
    std::condition_variable conditionVariable;

    // Mark the Factory type as a friend to allow it to have write access to
    // the internal types. This allows the parent scope to have mutability, but
    // all other scopes must access data through the immutable `get` interface.
    friend Factory;

 public:
    /// The request buffer
    Request request;

    /// @brief Create a new bidirectional reactor.
    AwaitableWriteReactor() : isDone(false) { }

    /// @brief Respond to the completion of the stream.
    ///
    /// @param status_ The completion status of the stream.
    ///
    inline virtual void OnDone(const grpc::Status& status_) override {
        // Lock the critical section for updating the `isDone` flag and the
        // gRPC `status` variable.
        std::lock_guard<std::mutex> lock(mutex);
        status = status_;
        isDone = true;
        // Notify the awaiting thread that the condition variable has changed.
        conditionVariable.notify_one();
    }

    /// @brief Return the status of the stream.
    ///
    /// @returns The gRPC status of the stream after completion.
    ///
    inline ::grpc::Status getStatus() {
        // Lock the critical section for querying the `status`.
        std::lock_guard<std::mutex> lock(mutex);
        return status;
    }

    /// @brief Return a flag determining if the stream has concluded.
    ///
    /// @returns `true` if the stream has resolved, `false` otherwise.
    ///
    inline bool getIsDone() {
        // Lock the critical section for querying the `isDone` flag.
        std::lock_guard<std::mutex> lock(mutex);
        return isDone;
    }

    /// @brief Block until the `onDone` callback is triggered in the background.
    ///
    /// @returns The final gRPC status of the stream.
    ///
    inline ::grpc::Status await() {
        // Lock the critical section for updating the `isDone` flag and the
        // gRPC `status` variable.
        std::unique_lock<std::mutex> lock(mutex);
        // Wait for the signal that the `isDone` flag has changed.
        conditionVariable.wait(lock, [this] { return isDone; });
        return status;
    }
};

}  // namespace calldata

}  // namespace sensory

#endif  // SENSORYCLOUD_CALLDATA_AWAITABLE_WRITE_REACTOR_HPP_
