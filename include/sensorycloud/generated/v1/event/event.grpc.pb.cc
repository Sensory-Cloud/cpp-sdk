// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: v1/event/event.proto

#include "v1/event/event.pb.h"
#include "v1/event/event.grpc.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>
namespace sensory {
namespace api {
namespace v1 {
namespace event {

static const char* EventService_method_names[] = {
  "/sensory.api.v1.event.EventService/PublishUsageEvents",
  "/sensory.api.v1.event.EventService/GetUsageEventList",
  "/sensory.api.v1.event.EventService/GetUsageEventSummary",
  "/sensory.api.v1.event.EventService/GetGlobalUsageSummary",
};

std::unique_ptr< EventService::Stub> EventService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< EventService::Stub> stub(new EventService::Stub(channel, options));
  return stub;
}

EventService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_PublishUsageEvents_(EventService_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetUsageEventList_(EventService_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetUsageEventSummary_(EventService_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetGlobalUsageSummary_(EventService_method_names[3], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status EventService::Stub::PublishUsageEvents(::grpc::ClientContext* context, const ::sensory::api::v1::event::PublishUsageEventsRequest& request, ::sensory::api::v1::event::PublishUsageEventsResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::sensory::api::v1::event::PublishUsageEventsRequest, ::sensory::api::v1::event::PublishUsageEventsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_PublishUsageEvents_, context, request, response);
}

void EventService::Stub::async::PublishUsageEvents(::grpc::ClientContext* context, const ::sensory::api::v1::event::PublishUsageEventsRequest* request, ::sensory::api::v1::event::PublishUsageEventsResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::sensory::api::v1::event::PublishUsageEventsRequest, ::sensory::api::v1::event::PublishUsageEventsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_PublishUsageEvents_, context, request, response, std::move(f));
}

void EventService::Stub::async::PublishUsageEvents(::grpc::ClientContext* context, const ::sensory::api::v1::event::PublishUsageEventsRequest* request, ::sensory::api::v1::event::PublishUsageEventsResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_PublishUsageEvents_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::event::PublishUsageEventsResponse>* EventService::Stub::PrepareAsyncPublishUsageEventsRaw(::grpc::ClientContext* context, const ::sensory::api::v1::event::PublishUsageEventsRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::sensory::api::v1::event::PublishUsageEventsResponse, ::sensory::api::v1::event::PublishUsageEventsRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_PublishUsageEvents_, context, request);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::event::PublishUsageEventsResponse>* EventService::Stub::AsyncPublishUsageEventsRaw(::grpc::ClientContext* context, const ::sensory::api::v1::event::PublishUsageEventsRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncPublishUsageEventsRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status EventService::Stub::GetUsageEventList(::grpc::ClientContext* context, const ::sensory::api::v1::event::UsageEventListRequest& request, ::sensory::api::v1::event::UsageEventListResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::sensory::api::v1::event::UsageEventListRequest, ::sensory::api::v1::event::UsageEventListResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetUsageEventList_, context, request, response);
}

void EventService::Stub::async::GetUsageEventList(::grpc::ClientContext* context, const ::sensory::api::v1::event::UsageEventListRequest* request, ::sensory::api::v1::event::UsageEventListResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::sensory::api::v1::event::UsageEventListRequest, ::sensory::api::v1::event::UsageEventListResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetUsageEventList_, context, request, response, std::move(f));
}

void EventService::Stub::async::GetUsageEventList(::grpc::ClientContext* context, const ::sensory::api::v1::event::UsageEventListRequest* request, ::sensory::api::v1::event::UsageEventListResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetUsageEventList_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::event::UsageEventListResponse>* EventService::Stub::PrepareAsyncGetUsageEventListRaw(::grpc::ClientContext* context, const ::sensory::api::v1::event::UsageEventListRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::sensory::api::v1::event::UsageEventListResponse, ::sensory::api::v1::event::UsageEventListRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetUsageEventList_, context, request);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::event::UsageEventListResponse>* EventService::Stub::AsyncGetUsageEventListRaw(::grpc::ClientContext* context, const ::sensory::api::v1::event::UsageEventListRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetUsageEventListRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status EventService::Stub::GetUsageEventSummary(::grpc::ClientContext* context, const ::sensory::api::v1::event::UsageEventListRequest& request, ::sensory::api::v1::event::UsageEventSummary* response) {
  return ::grpc::internal::BlockingUnaryCall< ::sensory::api::v1::event::UsageEventListRequest, ::sensory::api::v1::event::UsageEventSummary, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetUsageEventSummary_, context, request, response);
}

void EventService::Stub::async::GetUsageEventSummary(::grpc::ClientContext* context, const ::sensory::api::v1::event::UsageEventListRequest* request, ::sensory::api::v1::event::UsageEventSummary* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::sensory::api::v1::event::UsageEventListRequest, ::sensory::api::v1::event::UsageEventSummary, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetUsageEventSummary_, context, request, response, std::move(f));
}

void EventService::Stub::async::GetUsageEventSummary(::grpc::ClientContext* context, const ::sensory::api::v1::event::UsageEventListRequest* request, ::sensory::api::v1::event::UsageEventSummary* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetUsageEventSummary_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::event::UsageEventSummary>* EventService::Stub::PrepareAsyncGetUsageEventSummaryRaw(::grpc::ClientContext* context, const ::sensory::api::v1::event::UsageEventListRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::sensory::api::v1::event::UsageEventSummary, ::sensory::api::v1::event::UsageEventListRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetUsageEventSummary_, context, request);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::event::UsageEventSummary>* EventService::Stub::AsyncGetUsageEventSummaryRaw(::grpc::ClientContext* context, const ::sensory::api::v1::event::UsageEventListRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetUsageEventSummaryRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status EventService::Stub::GetGlobalUsageSummary(::grpc::ClientContext* context, const ::sensory::api::v1::event::GlobalEventSummaryRequest& request, ::sensory::api::v1::event::UsageEventSummary* response) {
  return ::grpc::internal::BlockingUnaryCall< ::sensory::api::v1::event::GlobalEventSummaryRequest, ::sensory::api::v1::event::UsageEventSummary, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetGlobalUsageSummary_, context, request, response);
}

void EventService::Stub::async::GetGlobalUsageSummary(::grpc::ClientContext* context, const ::sensory::api::v1::event::GlobalEventSummaryRequest* request, ::sensory::api::v1::event::UsageEventSummary* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::sensory::api::v1::event::GlobalEventSummaryRequest, ::sensory::api::v1::event::UsageEventSummary, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetGlobalUsageSummary_, context, request, response, std::move(f));
}

void EventService::Stub::async::GetGlobalUsageSummary(::grpc::ClientContext* context, const ::sensory::api::v1::event::GlobalEventSummaryRequest* request, ::sensory::api::v1::event::UsageEventSummary* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetGlobalUsageSummary_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::event::UsageEventSummary>* EventService::Stub::PrepareAsyncGetGlobalUsageSummaryRaw(::grpc::ClientContext* context, const ::sensory::api::v1::event::GlobalEventSummaryRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::sensory::api::v1::event::UsageEventSummary, ::sensory::api::v1::event::GlobalEventSummaryRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetGlobalUsageSummary_, context, request);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::event::UsageEventSummary>* EventService::Stub::AsyncGetGlobalUsageSummaryRaw(::grpc::ClientContext* context, const ::sensory::api::v1::event::GlobalEventSummaryRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetGlobalUsageSummaryRaw(context, request, cq);
  result->StartCall();
  return result;
}

EventService::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      EventService_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< EventService::Service, ::sensory::api::v1::event::PublishUsageEventsRequest, ::sensory::api::v1::event::PublishUsageEventsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](EventService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::sensory::api::v1::event::PublishUsageEventsRequest* req,
             ::sensory::api::v1::event::PublishUsageEventsResponse* resp) {
               return service->PublishUsageEvents(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      EventService_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< EventService::Service, ::sensory::api::v1::event::UsageEventListRequest, ::sensory::api::v1::event::UsageEventListResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](EventService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::sensory::api::v1::event::UsageEventListRequest* req,
             ::sensory::api::v1::event::UsageEventListResponse* resp) {
               return service->GetUsageEventList(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      EventService_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< EventService::Service, ::sensory::api::v1::event::UsageEventListRequest, ::sensory::api::v1::event::UsageEventSummary, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](EventService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::sensory::api::v1::event::UsageEventListRequest* req,
             ::sensory::api::v1::event::UsageEventSummary* resp) {
               return service->GetUsageEventSummary(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      EventService_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< EventService::Service, ::sensory::api::v1::event::GlobalEventSummaryRequest, ::sensory::api::v1::event::UsageEventSummary, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](EventService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::sensory::api::v1::event::GlobalEventSummaryRequest* req,
             ::sensory::api::v1::event::UsageEventSummary* resp) {
               return service->GetGlobalUsageSummary(ctx, req, resp);
             }, this)));
}

EventService::Service::~Service() {
}

::grpc::Status EventService::Service::PublishUsageEvents(::grpc::ServerContext* context, const ::sensory::api::v1::event::PublishUsageEventsRequest* request, ::sensory::api::v1::event::PublishUsageEventsResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status EventService::Service::GetUsageEventList(::grpc::ServerContext* context, const ::sensory::api::v1::event::UsageEventListRequest* request, ::sensory::api::v1::event::UsageEventListResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status EventService::Service::GetUsageEventSummary(::grpc::ServerContext* context, const ::sensory::api::v1::event::UsageEventListRequest* request, ::sensory::api::v1::event::UsageEventSummary* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status EventService::Service::GetGlobalUsageSummary(::grpc::ServerContext* context, const ::sensory::api::v1::event::GlobalEventSummaryRequest* request, ::sensory::api::v1::event::UsageEventSummary* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace sensory
}  // namespace api
}  // namespace v1
}  // namespace event

