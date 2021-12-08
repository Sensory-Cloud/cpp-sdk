// An abstraction of asynchronous call data.
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

#ifndef SENSORY_CLOUD_CALL_DATA_HPP_
#define SENSORY_CLOUD_CALL_DATA_HPP_

#include <grpc/grpc.h>
#include <grpcpp/client_context.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <utility>

/// @brief The Sensory Cloud SDK.
namespace sensory {

/// @brief A type for encapsulating data for asynchronous calls.
/// @tparam Factory The factory class that will manage the scope of the call.
/// @tparam Request The type of the request message.
/// @tparam Response The type of the response message.
///
/// @details
/// The `Factory` is marked as a friend in order to provide mutable access to
/// private attributes of the structure. This allows instances of `Factory` to
/// mutate to the structure while all external scopes are limited to the
/// immutable interface exposed by the public accessor functions. Instances of
/// `CallData` are mutable within the scope of `Factory`, but immutable outside
/// of the scope of `Factory`.
///
template<typename Factory, typename Request, typename Response>
struct CallData {
 private:
    /// The gPRC context that the call is initiated with.
    ::grpc::ClientContext context;
    /// The status of the RPC after the response is processed.
    ::grpc::Status status;
    /// The request to execute in the unary call.
    Request request;
    /// The response to process after the RPC completes.
    Response response;
    /// A flag determining whether the asynchronous has terminated.
    std::atomic<bool> isDone;

    /// @brief Initialize a new call.
    CallData() : isDone(false) { }

    /// @brief Create a copy of this object.
    ///
    /// @param other the other instance to copy data from
    ///
    /// @details
    /// This copy constructor is private to prevent the copying of this object
    CallData(const CallData& other) = delete;

    /// @brief Assign to this object using the `=` operator.
    ///
    /// @param other The other instance to copy data from.
    ///
    /// @details
    /// This assignment operator is private to prevent copying of this object.
    ///
    void operator=(const CallData& other) = delete;

    // Mark the Factory type as a friend to allow it to have write access to
    // the internal types. This allows the parent scope to have mutability, but
    // all other scopes must access data through the immutable `get` interface.
    friend Factory;

 public:
    /// @brief Wait for the asynchronous call to complete.
    ///
    /// @details
    /// This will block the calling thread until the asynchronous call returns
    /// with a response.
    ///
    inline void await() { while (!isDone) continue; }

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

    /// @brief Return a flag determining if the call has concluded.
    ///
    /// @returns `true` if the stream has resolved, `false` otherwise.
    ///
    inline const bool& getIsDone() const { return isDone; }
};

/// @brief A type for encapsulating data for asynchronous streaming calls.
/// @tparam Factory The factory class that will manage the scope of the stream.
/// @tparam Request The type of the request message.
/// @tparam Response The type of the response message.
///
/// @details
/// The `Factory` is marked as a friend in order to provide mutable access to
/// private attributes of the structure. This allows instances of `Factory` to
/// mutate to the structure while all external scopes are limited to the
/// immutable interface exposed by the public accessor functions. Instances of
/// `AwaitableBidiReactor` are mutable within the scope of `Factory`, but
/// immutable outside of the scope of `Factory`.
///
template<typename Factory, typename Request, typename Response>
class AwaitableBidiReactor : public ::grpc::ClientBidiReactor<Request, Response> {
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
    /// The response buffer
    Response response;

    /// @brief Create a new bidirectional reactor.
    AwaitableBidiReactor() : isDone(false) { }

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
};

}  // namespace sensory

#endif  // SENSORY_CLOUD_CALL_DATA_HPP_
