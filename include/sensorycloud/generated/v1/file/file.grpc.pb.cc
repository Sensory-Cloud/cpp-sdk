// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: v1/file/file.proto

#include "v1/file/file.pb.h"
#include "v1/file/file.grpc.pb.h"

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
namespace file {

static const char* File_method_names[] = {
  "/sensory.api.v1.file.File/GetInfo",
  "/sensory.api.v1.file.File/GetCatalog",
  "/sensory.api.v1.file.File/GetCompleteCatalog",
  "/sensory.api.v1.file.File/Download",
};

std::unique_ptr< File::Stub> File::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< File::Stub> stub(new File::Stub(channel, options));
  return stub;
}

File::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_GetInfo_(File_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetCatalog_(File_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetCompleteCatalog_(File_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Download_(File_method_names[3], options.suffix_for_stats(),::grpc::internal::RpcMethod::SERVER_STREAMING, channel)
  {}

::grpc::Status File::Stub::GetInfo(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileRequest& request, ::sensory::api::v1::file::FileInfo* response) {
  return ::grpc::internal::BlockingUnaryCall< ::sensory::api::v1::file::FileRequest, ::sensory::api::v1::file::FileInfo, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetInfo_, context, request, response);
}

void File::Stub::async::GetInfo(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileRequest* request, ::sensory::api::v1::file::FileInfo* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::sensory::api::v1::file::FileRequest, ::sensory::api::v1::file::FileInfo, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetInfo_, context, request, response, std::move(f));
}

void File::Stub::async::GetInfo(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileRequest* request, ::sensory::api::v1::file::FileInfo* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetInfo_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::file::FileInfo>* File::Stub::PrepareAsyncGetInfoRaw(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::sensory::api::v1::file::FileInfo, ::sensory::api::v1::file::FileRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetInfo_, context, request);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::file::FileInfo>* File::Stub::AsyncGetInfoRaw(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetInfoRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status File::Stub::GetCatalog(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileCatalogRequest& request, ::sensory::api::v1::file::FileCatalogResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::sensory::api::v1::file::FileCatalogRequest, ::sensory::api::v1::file::FileCatalogResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetCatalog_, context, request, response);
}

void File::Stub::async::GetCatalog(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileCatalogRequest* request, ::sensory::api::v1::file::FileCatalogResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::sensory::api::v1::file::FileCatalogRequest, ::sensory::api::v1::file::FileCatalogResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetCatalog_, context, request, response, std::move(f));
}

void File::Stub::async::GetCatalog(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileCatalogRequest* request, ::sensory::api::v1::file::FileCatalogResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetCatalog_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::file::FileCatalogResponse>* File::Stub::PrepareAsyncGetCatalogRaw(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileCatalogRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::sensory::api::v1::file::FileCatalogResponse, ::sensory::api::v1::file::FileCatalogRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetCatalog_, context, request);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::file::FileCatalogResponse>* File::Stub::AsyncGetCatalogRaw(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileCatalogRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetCatalogRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status File::Stub::GetCompleteCatalog(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileCompleteCatalogRequest& request, ::sensory::api::v1::file::FileCatalogResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::sensory::api::v1::file::FileCompleteCatalogRequest, ::sensory::api::v1::file::FileCatalogResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetCompleteCatalog_, context, request, response);
}

void File::Stub::async::GetCompleteCatalog(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileCompleteCatalogRequest* request, ::sensory::api::v1::file::FileCatalogResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::sensory::api::v1::file::FileCompleteCatalogRequest, ::sensory::api::v1::file::FileCatalogResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetCompleteCatalog_, context, request, response, std::move(f));
}

void File::Stub::async::GetCompleteCatalog(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileCompleteCatalogRequest* request, ::sensory::api::v1::file::FileCatalogResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetCompleteCatalog_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::file::FileCatalogResponse>* File::Stub::PrepareAsyncGetCompleteCatalogRaw(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileCompleteCatalogRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::sensory::api::v1::file::FileCatalogResponse, ::sensory::api::v1::file::FileCompleteCatalogRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetCompleteCatalog_, context, request);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::file::FileCatalogResponse>* File::Stub::AsyncGetCompleteCatalogRaw(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileCompleteCatalogRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetCompleteCatalogRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::ClientReader< ::sensory::api::v1::file::FileResponse>* File::Stub::DownloadRaw(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileRequest& request) {
  return ::grpc::internal::ClientReaderFactory< ::sensory::api::v1::file::FileResponse>::Create(channel_.get(), rpcmethod_Download_, context, request);
}

void File::Stub::async::Download(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileRequest* request, ::grpc::ClientReadReactor< ::sensory::api::v1::file::FileResponse>* reactor) {
  ::grpc::internal::ClientCallbackReaderFactory< ::sensory::api::v1::file::FileResponse>::Create(stub_->channel_.get(), stub_->rpcmethod_Download_, context, request, reactor);
}

::grpc::ClientAsyncReader< ::sensory::api::v1::file::FileResponse>* File::Stub::AsyncDownloadRaw(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileRequest& request, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncReaderFactory< ::sensory::api::v1::file::FileResponse>::Create(channel_.get(), cq, rpcmethod_Download_, context, request, true, tag);
}

::grpc::ClientAsyncReader< ::sensory::api::v1::file::FileResponse>* File::Stub::PrepareAsyncDownloadRaw(::grpc::ClientContext* context, const ::sensory::api::v1::file::FileRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncReaderFactory< ::sensory::api::v1::file::FileResponse>::Create(channel_.get(), cq, rpcmethod_Download_, context, request, false, nullptr);
}

File::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      File_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< File::Service, ::sensory::api::v1::file::FileRequest, ::sensory::api::v1::file::FileInfo, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](File::Service* service,
             ::grpc::ServerContext* ctx,
             const ::sensory::api::v1::file::FileRequest* req,
             ::sensory::api::v1::file::FileInfo* resp) {
               return service->GetInfo(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      File_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< File::Service, ::sensory::api::v1::file::FileCatalogRequest, ::sensory::api::v1::file::FileCatalogResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](File::Service* service,
             ::grpc::ServerContext* ctx,
             const ::sensory::api::v1::file::FileCatalogRequest* req,
             ::sensory::api::v1::file::FileCatalogResponse* resp) {
               return service->GetCatalog(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      File_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< File::Service, ::sensory::api::v1::file::FileCompleteCatalogRequest, ::sensory::api::v1::file::FileCatalogResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](File::Service* service,
             ::grpc::ServerContext* ctx,
             const ::sensory::api::v1::file::FileCompleteCatalogRequest* req,
             ::sensory::api::v1::file::FileCatalogResponse* resp) {
               return service->GetCompleteCatalog(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      File_method_names[3],
      ::grpc::internal::RpcMethod::SERVER_STREAMING,
      new ::grpc::internal::ServerStreamingHandler< File::Service, ::sensory::api::v1::file::FileRequest, ::sensory::api::v1::file::FileResponse>(
          [](File::Service* service,
             ::grpc::ServerContext* ctx,
             const ::sensory::api::v1::file::FileRequest* req,
             ::grpc::ServerWriter<::sensory::api::v1::file::FileResponse>* writer) {
               return service->Download(ctx, req, writer);
             }, this)));
}

File::Service::~Service() {
}

::grpc::Status File::Service::GetInfo(::grpc::ServerContext* context, const ::sensory::api::v1::file::FileRequest* request, ::sensory::api::v1::file::FileInfo* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status File::Service::GetCatalog(::grpc::ServerContext* context, const ::sensory::api::v1::file::FileCatalogRequest* request, ::sensory::api::v1::file::FileCatalogResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status File::Service::GetCompleteCatalog(::grpc::ServerContext* context, const ::sensory::api::v1::file::FileCompleteCatalogRequest* request, ::sensory::api::v1::file::FileCatalogResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status File::Service::Download(::grpc::ServerContext* context, const ::sensory::api::v1::file::FileRequest* request, ::grpc::ServerWriter< ::sensory::api::v1::file::FileResponse>* writer) {
  (void) context;
  (void) request;
  (void) writer;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace sensory
}  // namespace api
}  // namespace v1
}  // namespace file

