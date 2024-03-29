// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: v1/audio/audio.proto

#include "v1/audio/audio.pb.h"
#include "v1/audio/audio.grpc.pb.h"

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
namespace audio {

static const char* AudioModels_method_names[] = {
  "/sensory.api.v1.audio.AudioModels/GetModels",
};

std::unique_ptr< AudioModels::Stub> AudioModels::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< AudioModels::Stub> stub(new AudioModels::Stub(channel, options));
  return stub;
}

AudioModels::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_GetModels_(AudioModels_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status AudioModels::Stub::GetModels(::grpc::ClientContext* context, const ::sensory::api::v1::audio::GetModelsRequest& request, ::sensory::api::v1::audio::GetModelsResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::sensory::api::v1::audio::GetModelsRequest, ::sensory::api::v1::audio::GetModelsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetModels_, context, request, response);
}

void AudioModels::Stub::async::GetModels(::grpc::ClientContext* context, const ::sensory::api::v1::audio::GetModelsRequest* request, ::sensory::api::v1::audio::GetModelsResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::sensory::api::v1::audio::GetModelsRequest, ::sensory::api::v1::audio::GetModelsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetModels_, context, request, response, std::move(f));
}

void AudioModels::Stub::async::GetModels(::grpc::ClientContext* context, const ::sensory::api::v1::audio::GetModelsRequest* request, ::sensory::api::v1::audio::GetModelsResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetModels_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::audio::GetModelsResponse>* AudioModels::Stub::PrepareAsyncGetModelsRaw(::grpc::ClientContext* context, const ::sensory::api::v1::audio::GetModelsRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::sensory::api::v1::audio::GetModelsResponse, ::sensory::api::v1::audio::GetModelsRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetModels_, context, request);
}

::grpc::ClientAsyncResponseReader< ::sensory::api::v1::audio::GetModelsResponse>* AudioModels::Stub::AsyncGetModelsRaw(::grpc::ClientContext* context, const ::sensory::api::v1::audio::GetModelsRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetModelsRaw(context, request, cq);
  result->StartCall();
  return result;
}

AudioModels::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      AudioModels_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< AudioModels::Service, ::sensory::api::v1::audio::GetModelsRequest, ::sensory::api::v1::audio::GetModelsResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](AudioModels::Service* service,
             ::grpc::ServerContext* ctx,
             const ::sensory::api::v1::audio::GetModelsRequest* req,
             ::sensory::api::v1::audio::GetModelsResponse* resp) {
               return service->GetModels(ctx, req, resp);
             }, this)));
}

AudioModels::Service::~Service() {
}

::grpc::Status AudioModels::Service::GetModels(::grpc::ServerContext* context, const ::sensory::api::v1::audio::GetModelsRequest* request, ::sensory::api::v1::audio::GetModelsResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


static const char* AudioBiometrics_method_names[] = {
  "/sensory.api.v1.audio.AudioBiometrics/CreateEnrollment",
  "/sensory.api.v1.audio.AudioBiometrics/Authenticate",
};

std::unique_ptr< AudioBiometrics::Stub> AudioBiometrics::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< AudioBiometrics::Stub> stub(new AudioBiometrics::Stub(channel, options));
  return stub;
}

AudioBiometrics::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_CreateEnrollment_(AudioBiometrics_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::BIDI_STREAMING, channel)
  , rpcmethod_Authenticate_(AudioBiometrics_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::BIDI_STREAMING, channel)
  {}

::grpc::ClientReaderWriter< ::sensory::api::v1::audio::CreateEnrollmentRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>* AudioBiometrics::Stub::CreateEnrollmentRaw(::grpc::ClientContext* context) {
  return ::grpc::internal::ClientReaderWriterFactory< ::sensory::api::v1::audio::CreateEnrollmentRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>::Create(channel_.get(), rpcmethod_CreateEnrollment_, context);
}

void AudioBiometrics::Stub::async::CreateEnrollment(::grpc::ClientContext* context, ::grpc::ClientBidiReactor< ::sensory::api::v1::audio::CreateEnrollmentRequest,::sensory::api::v1::audio::CreateEnrollmentResponse>* reactor) {
  ::grpc::internal::ClientCallbackReaderWriterFactory< ::sensory::api::v1::audio::CreateEnrollmentRequest,::sensory::api::v1::audio::CreateEnrollmentResponse>::Create(stub_->channel_.get(), stub_->rpcmethod_CreateEnrollment_, context, reactor);
}

::grpc::ClientAsyncReaderWriter< ::sensory::api::v1::audio::CreateEnrollmentRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>* AudioBiometrics::Stub::AsyncCreateEnrollmentRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::sensory::api::v1::audio::CreateEnrollmentRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>::Create(channel_.get(), cq, rpcmethod_CreateEnrollment_, context, true, tag);
}

::grpc::ClientAsyncReaderWriter< ::sensory::api::v1::audio::CreateEnrollmentRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>* AudioBiometrics::Stub::PrepareAsyncCreateEnrollmentRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::sensory::api::v1::audio::CreateEnrollmentRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>::Create(channel_.get(), cq, rpcmethod_CreateEnrollment_, context, false, nullptr);
}

::grpc::ClientReaderWriter< ::sensory::api::v1::audio::AuthenticateRequest, ::sensory::api::v1::audio::AuthenticateResponse>* AudioBiometrics::Stub::AuthenticateRaw(::grpc::ClientContext* context) {
  return ::grpc::internal::ClientReaderWriterFactory< ::sensory::api::v1::audio::AuthenticateRequest, ::sensory::api::v1::audio::AuthenticateResponse>::Create(channel_.get(), rpcmethod_Authenticate_, context);
}

void AudioBiometrics::Stub::async::Authenticate(::grpc::ClientContext* context, ::grpc::ClientBidiReactor< ::sensory::api::v1::audio::AuthenticateRequest,::sensory::api::v1::audio::AuthenticateResponse>* reactor) {
  ::grpc::internal::ClientCallbackReaderWriterFactory< ::sensory::api::v1::audio::AuthenticateRequest,::sensory::api::v1::audio::AuthenticateResponse>::Create(stub_->channel_.get(), stub_->rpcmethod_Authenticate_, context, reactor);
}

::grpc::ClientAsyncReaderWriter< ::sensory::api::v1::audio::AuthenticateRequest, ::sensory::api::v1::audio::AuthenticateResponse>* AudioBiometrics::Stub::AsyncAuthenticateRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::sensory::api::v1::audio::AuthenticateRequest, ::sensory::api::v1::audio::AuthenticateResponse>::Create(channel_.get(), cq, rpcmethod_Authenticate_, context, true, tag);
}

::grpc::ClientAsyncReaderWriter< ::sensory::api::v1::audio::AuthenticateRequest, ::sensory::api::v1::audio::AuthenticateResponse>* AudioBiometrics::Stub::PrepareAsyncAuthenticateRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::sensory::api::v1::audio::AuthenticateRequest, ::sensory::api::v1::audio::AuthenticateResponse>::Create(channel_.get(), cq, rpcmethod_Authenticate_, context, false, nullptr);
}

AudioBiometrics::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      AudioBiometrics_method_names[0],
      ::grpc::internal::RpcMethod::BIDI_STREAMING,
      new ::grpc::internal::BidiStreamingHandler< AudioBiometrics::Service, ::sensory::api::v1::audio::CreateEnrollmentRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>(
          [](AudioBiometrics::Service* service,
             ::grpc::ServerContext* ctx,
             ::grpc::ServerReaderWriter<::sensory::api::v1::audio::CreateEnrollmentResponse,
             ::sensory::api::v1::audio::CreateEnrollmentRequest>* stream) {
               return service->CreateEnrollment(ctx, stream);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      AudioBiometrics_method_names[1],
      ::grpc::internal::RpcMethod::BIDI_STREAMING,
      new ::grpc::internal::BidiStreamingHandler< AudioBiometrics::Service, ::sensory::api::v1::audio::AuthenticateRequest, ::sensory::api::v1::audio::AuthenticateResponse>(
          [](AudioBiometrics::Service* service,
             ::grpc::ServerContext* ctx,
             ::grpc::ServerReaderWriter<::sensory::api::v1::audio::AuthenticateResponse,
             ::sensory::api::v1::audio::AuthenticateRequest>* stream) {
               return service->Authenticate(ctx, stream);
             }, this)));
}

AudioBiometrics::Service::~Service() {
}

::grpc::Status AudioBiometrics::Service::CreateEnrollment(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::sensory::api::v1::audio::CreateEnrollmentResponse, ::sensory::api::v1::audio::CreateEnrollmentRequest>* stream) {
  (void) context;
  (void) stream;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status AudioBiometrics::Service::Authenticate(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::sensory::api::v1::audio::AuthenticateResponse, ::sensory::api::v1::audio::AuthenticateRequest>* stream) {
  (void) context;
  (void) stream;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


static const char* AudioEvents_method_names[] = {
  "/sensory.api.v1.audio.AudioEvents/ValidateEvent",
  "/sensory.api.v1.audio.AudioEvents/CreateEnrolledEvent",
  "/sensory.api.v1.audio.AudioEvents/ValidateEnrolledEvent",
};

std::unique_ptr< AudioEvents::Stub> AudioEvents::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< AudioEvents::Stub> stub(new AudioEvents::Stub(channel, options));
  return stub;
}

AudioEvents::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_ValidateEvent_(AudioEvents_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::BIDI_STREAMING, channel)
  , rpcmethod_CreateEnrolledEvent_(AudioEvents_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::BIDI_STREAMING, channel)
  , rpcmethod_ValidateEnrolledEvent_(AudioEvents_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::BIDI_STREAMING, channel)
  {}

::grpc::ClientReaderWriter< ::sensory::api::v1::audio::ValidateEventRequest, ::sensory::api::v1::audio::ValidateEventResponse>* AudioEvents::Stub::ValidateEventRaw(::grpc::ClientContext* context) {
  return ::grpc::internal::ClientReaderWriterFactory< ::sensory::api::v1::audio::ValidateEventRequest, ::sensory::api::v1::audio::ValidateEventResponse>::Create(channel_.get(), rpcmethod_ValidateEvent_, context);
}

void AudioEvents::Stub::async::ValidateEvent(::grpc::ClientContext* context, ::grpc::ClientBidiReactor< ::sensory::api::v1::audio::ValidateEventRequest,::sensory::api::v1::audio::ValidateEventResponse>* reactor) {
  ::grpc::internal::ClientCallbackReaderWriterFactory< ::sensory::api::v1::audio::ValidateEventRequest,::sensory::api::v1::audio::ValidateEventResponse>::Create(stub_->channel_.get(), stub_->rpcmethod_ValidateEvent_, context, reactor);
}

::grpc::ClientAsyncReaderWriter< ::sensory::api::v1::audio::ValidateEventRequest, ::sensory::api::v1::audio::ValidateEventResponse>* AudioEvents::Stub::AsyncValidateEventRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::sensory::api::v1::audio::ValidateEventRequest, ::sensory::api::v1::audio::ValidateEventResponse>::Create(channel_.get(), cq, rpcmethod_ValidateEvent_, context, true, tag);
}

::grpc::ClientAsyncReaderWriter< ::sensory::api::v1::audio::ValidateEventRequest, ::sensory::api::v1::audio::ValidateEventResponse>* AudioEvents::Stub::PrepareAsyncValidateEventRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::sensory::api::v1::audio::ValidateEventRequest, ::sensory::api::v1::audio::ValidateEventResponse>::Create(channel_.get(), cq, rpcmethod_ValidateEvent_, context, false, nullptr);
}

::grpc::ClientReaderWriter< ::sensory::api::v1::audio::CreateEnrolledEventRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>* AudioEvents::Stub::CreateEnrolledEventRaw(::grpc::ClientContext* context) {
  return ::grpc::internal::ClientReaderWriterFactory< ::sensory::api::v1::audio::CreateEnrolledEventRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>::Create(channel_.get(), rpcmethod_CreateEnrolledEvent_, context);
}

void AudioEvents::Stub::async::CreateEnrolledEvent(::grpc::ClientContext* context, ::grpc::ClientBidiReactor< ::sensory::api::v1::audio::CreateEnrolledEventRequest,::sensory::api::v1::audio::CreateEnrollmentResponse>* reactor) {
  ::grpc::internal::ClientCallbackReaderWriterFactory< ::sensory::api::v1::audio::CreateEnrolledEventRequest,::sensory::api::v1::audio::CreateEnrollmentResponse>::Create(stub_->channel_.get(), stub_->rpcmethod_CreateEnrolledEvent_, context, reactor);
}

::grpc::ClientAsyncReaderWriter< ::sensory::api::v1::audio::CreateEnrolledEventRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>* AudioEvents::Stub::AsyncCreateEnrolledEventRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::sensory::api::v1::audio::CreateEnrolledEventRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>::Create(channel_.get(), cq, rpcmethod_CreateEnrolledEvent_, context, true, tag);
}

::grpc::ClientAsyncReaderWriter< ::sensory::api::v1::audio::CreateEnrolledEventRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>* AudioEvents::Stub::PrepareAsyncCreateEnrolledEventRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::sensory::api::v1::audio::CreateEnrolledEventRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>::Create(channel_.get(), cq, rpcmethod_CreateEnrolledEvent_, context, false, nullptr);
}

::grpc::ClientReaderWriter< ::sensory::api::v1::audio::ValidateEnrolledEventRequest, ::sensory::api::v1::audio::ValidateEnrolledEventResponse>* AudioEvents::Stub::ValidateEnrolledEventRaw(::grpc::ClientContext* context) {
  return ::grpc::internal::ClientReaderWriterFactory< ::sensory::api::v1::audio::ValidateEnrolledEventRequest, ::sensory::api::v1::audio::ValidateEnrolledEventResponse>::Create(channel_.get(), rpcmethod_ValidateEnrolledEvent_, context);
}

void AudioEvents::Stub::async::ValidateEnrolledEvent(::grpc::ClientContext* context, ::grpc::ClientBidiReactor< ::sensory::api::v1::audio::ValidateEnrolledEventRequest,::sensory::api::v1::audio::ValidateEnrolledEventResponse>* reactor) {
  ::grpc::internal::ClientCallbackReaderWriterFactory< ::sensory::api::v1::audio::ValidateEnrolledEventRequest,::sensory::api::v1::audio::ValidateEnrolledEventResponse>::Create(stub_->channel_.get(), stub_->rpcmethod_ValidateEnrolledEvent_, context, reactor);
}

::grpc::ClientAsyncReaderWriter< ::sensory::api::v1::audio::ValidateEnrolledEventRequest, ::sensory::api::v1::audio::ValidateEnrolledEventResponse>* AudioEvents::Stub::AsyncValidateEnrolledEventRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::sensory::api::v1::audio::ValidateEnrolledEventRequest, ::sensory::api::v1::audio::ValidateEnrolledEventResponse>::Create(channel_.get(), cq, rpcmethod_ValidateEnrolledEvent_, context, true, tag);
}

::grpc::ClientAsyncReaderWriter< ::sensory::api::v1::audio::ValidateEnrolledEventRequest, ::sensory::api::v1::audio::ValidateEnrolledEventResponse>* AudioEvents::Stub::PrepareAsyncValidateEnrolledEventRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::sensory::api::v1::audio::ValidateEnrolledEventRequest, ::sensory::api::v1::audio::ValidateEnrolledEventResponse>::Create(channel_.get(), cq, rpcmethod_ValidateEnrolledEvent_, context, false, nullptr);
}

AudioEvents::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      AudioEvents_method_names[0],
      ::grpc::internal::RpcMethod::BIDI_STREAMING,
      new ::grpc::internal::BidiStreamingHandler< AudioEvents::Service, ::sensory::api::v1::audio::ValidateEventRequest, ::sensory::api::v1::audio::ValidateEventResponse>(
          [](AudioEvents::Service* service,
             ::grpc::ServerContext* ctx,
             ::grpc::ServerReaderWriter<::sensory::api::v1::audio::ValidateEventResponse,
             ::sensory::api::v1::audio::ValidateEventRequest>* stream) {
               return service->ValidateEvent(ctx, stream);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      AudioEvents_method_names[1],
      ::grpc::internal::RpcMethod::BIDI_STREAMING,
      new ::grpc::internal::BidiStreamingHandler< AudioEvents::Service, ::sensory::api::v1::audio::CreateEnrolledEventRequest, ::sensory::api::v1::audio::CreateEnrollmentResponse>(
          [](AudioEvents::Service* service,
             ::grpc::ServerContext* ctx,
             ::grpc::ServerReaderWriter<::sensory::api::v1::audio::CreateEnrollmentResponse,
             ::sensory::api::v1::audio::CreateEnrolledEventRequest>* stream) {
               return service->CreateEnrolledEvent(ctx, stream);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      AudioEvents_method_names[2],
      ::grpc::internal::RpcMethod::BIDI_STREAMING,
      new ::grpc::internal::BidiStreamingHandler< AudioEvents::Service, ::sensory::api::v1::audio::ValidateEnrolledEventRequest, ::sensory::api::v1::audio::ValidateEnrolledEventResponse>(
          [](AudioEvents::Service* service,
             ::grpc::ServerContext* ctx,
             ::grpc::ServerReaderWriter<::sensory::api::v1::audio::ValidateEnrolledEventResponse,
             ::sensory::api::v1::audio::ValidateEnrolledEventRequest>* stream) {
               return service->ValidateEnrolledEvent(ctx, stream);
             }, this)));
}

AudioEvents::Service::~Service() {
}

::grpc::Status AudioEvents::Service::ValidateEvent(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::sensory::api::v1::audio::ValidateEventResponse, ::sensory::api::v1::audio::ValidateEventRequest>* stream) {
  (void) context;
  (void) stream;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status AudioEvents::Service::CreateEnrolledEvent(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::sensory::api::v1::audio::CreateEnrollmentResponse, ::sensory::api::v1::audio::CreateEnrolledEventRequest>* stream) {
  (void) context;
  (void) stream;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status AudioEvents::Service::ValidateEnrolledEvent(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::sensory::api::v1::audio::ValidateEnrolledEventResponse, ::sensory::api::v1::audio::ValidateEnrolledEventRequest>* stream) {
  (void) context;
  (void) stream;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


static const char* AudioTranscriptions_method_names[] = {
  "/sensory.api.v1.audio.AudioTranscriptions/Transcribe",
};

std::unique_ptr< AudioTranscriptions::Stub> AudioTranscriptions::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< AudioTranscriptions::Stub> stub(new AudioTranscriptions::Stub(channel, options));
  return stub;
}

AudioTranscriptions::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_Transcribe_(AudioTranscriptions_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::BIDI_STREAMING, channel)
  {}

::grpc::ClientReaderWriter< ::sensory::api::v1::audio::TranscribeRequest, ::sensory::api::v1::audio::TranscribeResponse>* AudioTranscriptions::Stub::TranscribeRaw(::grpc::ClientContext* context) {
  return ::grpc::internal::ClientReaderWriterFactory< ::sensory::api::v1::audio::TranscribeRequest, ::sensory::api::v1::audio::TranscribeResponse>::Create(channel_.get(), rpcmethod_Transcribe_, context);
}

void AudioTranscriptions::Stub::async::Transcribe(::grpc::ClientContext* context, ::grpc::ClientBidiReactor< ::sensory::api::v1::audio::TranscribeRequest,::sensory::api::v1::audio::TranscribeResponse>* reactor) {
  ::grpc::internal::ClientCallbackReaderWriterFactory< ::sensory::api::v1::audio::TranscribeRequest,::sensory::api::v1::audio::TranscribeResponse>::Create(stub_->channel_.get(), stub_->rpcmethod_Transcribe_, context, reactor);
}

::grpc::ClientAsyncReaderWriter< ::sensory::api::v1::audio::TranscribeRequest, ::sensory::api::v1::audio::TranscribeResponse>* AudioTranscriptions::Stub::AsyncTranscribeRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::sensory::api::v1::audio::TranscribeRequest, ::sensory::api::v1::audio::TranscribeResponse>::Create(channel_.get(), cq, rpcmethod_Transcribe_, context, true, tag);
}

::grpc::ClientAsyncReaderWriter< ::sensory::api::v1::audio::TranscribeRequest, ::sensory::api::v1::audio::TranscribeResponse>* AudioTranscriptions::Stub::PrepareAsyncTranscribeRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncReaderWriterFactory< ::sensory::api::v1::audio::TranscribeRequest, ::sensory::api::v1::audio::TranscribeResponse>::Create(channel_.get(), cq, rpcmethod_Transcribe_, context, false, nullptr);
}

AudioTranscriptions::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      AudioTranscriptions_method_names[0],
      ::grpc::internal::RpcMethod::BIDI_STREAMING,
      new ::grpc::internal::BidiStreamingHandler< AudioTranscriptions::Service, ::sensory::api::v1::audio::TranscribeRequest, ::sensory::api::v1::audio::TranscribeResponse>(
          [](AudioTranscriptions::Service* service,
             ::grpc::ServerContext* ctx,
             ::grpc::ServerReaderWriter<::sensory::api::v1::audio::TranscribeResponse,
             ::sensory::api::v1::audio::TranscribeRequest>* stream) {
               return service->Transcribe(ctx, stream);
             }, this)));
}

AudioTranscriptions::Service::~Service() {
}

::grpc::Status AudioTranscriptions::Service::Transcribe(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::sensory::api::v1::audio::TranscribeResponse, ::sensory::api::v1::audio::TranscribeRequest>* stream) {
  (void) context;
  (void) stream;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


static const char* AudioSynthesis_method_names[] = {
  "/sensory.api.v1.audio.AudioSynthesis/SynthesizeSpeech",
};

std::unique_ptr< AudioSynthesis::Stub> AudioSynthesis::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< AudioSynthesis::Stub> stub(new AudioSynthesis::Stub(channel, options));
  return stub;
}

AudioSynthesis::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_SynthesizeSpeech_(AudioSynthesis_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::SERVER_STREAMING, channel)
  {}

::grpc::ClientReader< ::sensory::api::v1::audio::SynthesizeSpeechResponse>* AudioSynthesis::Stub::SynthesizeSpeechRaw(::grpc::ClientContext* context, const ::sensory::api::v1::audio::SynthesizeSpeechRequest& request) {
  return ::grpc::internal::ClientReaderFactory< ::sensory::api::v1::audio::SynthesizeSpeechResponse>::Create(channel_.get(), rpcmethod_SynthesizeSpeech_, context, request);
}

void AudioSynthesis::Stub::async::SynthesizeSpeech(::grpc::ClientContext* context, const ::sensory::api::v1::audio::SynthesizeSpeechRequest* request, ::grpc::ClientReadReactor< ::sensory::api::v1::audio::SynthesizeSpeechResponse>* reactor) {
  ::grpc::internal::ClientCallbackReaderFactory< ::sensory::api::v1::audio::SynthesizeSpeechResponse>::Create(stub_->channel_.get(), stub_->rpcmethod_SynthesizeSpeech_, context, request, reactor);
}

::grpc::ClientAsyncReader< ::sensory::api::v1::audio::SynthesizeSpeechResponse>* AudioSynthesis::Stub::AsyncSynthesizeSpeechRaw(::grpc::ClientContext* context, const ::sensory::api::v1::audio::SynthesizeSpeechRequest& request, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncReaderFactory< ::sensory::api::v1::audio::SynthesizeSpeechResponse>::Create(channel_.get(), cq, rpcmethod_SynthesizeSpeech_, context, request, true, tag);
}

::grpc::ClientAsyncReader< ::sensory::api::v1::audio::SynthesizeSpeechResponse>* AudioSynthesis::Stub::PrepareAsyncSynthesizeSpeechRaw(::grpc::ClientContext* context, const ::sensory::api::v1::audio::SynthesizeSpeechRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncReaderFactory< ::sensory::api::v1::audio::SynthesizeSpeechResponse>::Create(channel_.get(), cq, rpcmethod_SynthesizeSpeech_, context, request, false, nullptr);
}

AudioSynthesis::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      AudioSynthesis_method_names[0],
      ::grpc::internal::RpcMethod::SERVER_STREAMING,
      new ::grpc::internal::ServerStreamingHandler< AudioSynthesis::Service, ::sensory::api::v1::audio::SynthesizeSpeechRequest, ::sensory::api::v1::audio::SynthesizeSpeechResponse>(
          [](AudioSynthesis::Service* service,
             ::grpc::ServerContext* ctx,
             const ::sensory::api::v1::audio::SynthesizeSpeechRequest* req,
             ::grpc::ServerWriter<::sensory::api::v1::audio::SynthesizeSpeechResponse>* writer) {
               return service->SynthesizeSpeech(ctx, req, writer);
             }, this)));
}

AudioSynthesis::Service::~Service() {
}

::grpc::Status AudioSynthesis::Service::SynthesizeSpeech(::grpc::ServerContext* context, const ::sensory::api::v1::audio::SynthesizeSpeechRequest* request, ::grpc::ServerWriter< ::sensory::api::v1::audio::SynthesizeSpeechResponse>* writer) {
  (void) context;
  (void) request;
  (void) writer;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace sensory
}  // namespace api
}  // namespace v1
}  // namespace audio

