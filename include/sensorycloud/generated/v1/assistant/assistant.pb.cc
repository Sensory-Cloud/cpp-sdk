// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: v1/assistant/assistant.proto

#include "v1/assistant/assistant.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG
namespace sensory {
namespace api {
namespace v1 {
namespace assistant {
constexpr ChatMessage::ChatMessage(
  ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized)
  : content_(&::PROTOBUF_NAMESPACE_ID::internal::fixed_address_empty_string)
  , role_(0)
{}
struct ChatMessageDefaultTypeInternal {
  constexpr ChatMessageDefaultTypeInternal()
    : _instance(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{}) {}
  ~ChatMessageDefaultTypeInternal() {}
  union {
    ChatMessage _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT ChatMessageDefaultTypeInternal _ChatMessage_default_instance_;
constexpr TextChatRequest::TextChatRequest(
  ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized)
  : messages_()
  , modelname_(&::PROTOBUF_NAMESPACE_ID::internal::fixed_address_empty_string){}
struct TextChatRequestDefaultTypeInternal {
  constexpr TextChatRequestDefaultTypeInternal()
    : _instance(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{}) {}
  ~TextChatRequestDefaultTypeInternal() {}
  union {
    TextChatRequest _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT TextChatRequestDefaultTypeInternal _TextChatRequest_default_instance_;
constexpr TextChatResponse::TextChatResponse(
  ::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized)
  : message_(nullptr){}
struct TextChatResponseDefaultTypeInternal {
  constexpr TextChatResponseDefaultTypeInternal()
    : _instance(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized{}) {}
  ~TextChatResponseDefaultTypeInternal() {}
  union {
    TextChatResponse _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT TextChatResponseDefaultTypeInternal _TextChatResponse_default_instance_;
}  // namespace assistant
}  // namespace v1
}  // namespace api
}  // namespace sensory
static ::PROTOBUF_NAMESPACE_ID::Metadata file_level_metadata_v1_2fassistant_2fassistant_2eproto[3];
static const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* file_level_enum_descriptors_v1_2fassistant_2fassistant_2eproto[1];
static constexpr ::PROTOBUF_NAMESPACE_ID::ServiceDescriptor const** file_level_service_descriptors_v1_2fassistant_2fassistant_2eproto = nullptr;

const ::PROTOBUF_NAMESPACE_ID::uint32 TableStruct_v1_2fassistant_2fassistant_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::sensory::api::v1::assistant::ChatMessage, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::sensory::api::v1::assistant::ChatMessage, role_),
  PROTOBUF_FIELD_OFFSET(::sensory::api::v1::assistant::ChatMessage, content_),
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::sensory::api::v1::assistant::TextChatRequest, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::sensory::api::v1::assistant::TextChatRequest, modelname_),
  PROTOBUF_FIELD_OFFSET(::sensory::api::v1::assistant::TextChatRequest, messages_),
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::sensory::api::v1::assistant::TextChatResponse, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::sensory::api::v1::assistant::TextChatResponse, message_),
};
static const ::PROTOBUF_NAMESPACE_ID::internal::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, -1, sizeof(::sensory::api::v1::assistant::ChatMessage)},
  { 8, -1, -1, sizeof(::sensory::api::v1::assistant::TextChatRequest)},
  { 16, -1, -1, sizeof(::sensory::api::v1::assistant::TextChatResponse)},
};

static ::PROTOBUF_NAMESPACE_ID::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::sensory::api::v1::assistant::_ChatMessage_default_instance_),
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::sensory::api::v1::assistant::_TextChatRequest_default_instance_),
  reinterpret_cast<const ::PROTOBUF_NAMESPACE_ID::Message*>(&::sensory::api::v1::assistant::_TextChatResponse_default_instance_),
};

const char descriptor_table_protodef_v1_2fassistant_2fassistant_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\034v1/assistant/assistant.proto\022\030sensory."
  "api.v1.assistant\032\023common/common.proto\"P\n"
  "\013ChatMessage\0220\n\004role\030\001 \001(\0162\".sensory.api"
  ".v1.assistant.ChatRole\022\017\n\007content\030\002 \001(\t\""
  "]\n\017TextChatRequest\022\021\n\tmodelName\030\001 \001(\t\0227\n"
  "\010messages\030\002 \003(\0132%.sensory.api.v1.assista"
  "nt.ChatMessage\"J\n\020TextChatResponse\0226\n\007me"
  "ssage\030\001 \001(\0132%.sensory.api.v1.assistant.C"
  "hatMessage*/\n\010ChatRole\022\n\n\006SYSTEM\020\000\022\010\n\004US"
  "ER\020\001\022\r\n\tASSISTANT\020\0022w\n\020AssistantService\022"
  "c\n\010TextChat\022).sensory.api.v1.assistant.T"
  "extChatRequest\032*.sensory.api.v1.assistan"
  "t.TextChatResponse\"\000B\207\001\n ai.sensorycloud"
  ".api.v1.assistantB!SensoryApiV1Managemen"
  "tServerProtoP\001Z>gitlab.com/sensory-cloud"
  "/server/titan.git/pkg/api/v1/assistantb\006"
  "proto3"
  ;
static const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable*const descriptor_table_v1_2fassistant_2fassistant_2eproto_deps[1] = {
  &::descriptor_table_common_2fcommon_2eproto,
};
static ::PROTOBUF_NAMESPACE_ID::internal::once_flag descriptor_table_v1_2fassistant_2fassistant_2eproto_once;
const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_v1_2fassistant_2fassistant_2eproto = {
  false, false, 646, descriptor_table_protodef_v1_2fassistant_2fassistant_2eproto, "v1/assistant/assistant.proto", 
  &descriptor_table_v1_2fassistant_2fassistant_2eproto_once, descriptor_table_v1_2fassistant_2fassistant_2eproto_deps, 1, 3,
  schemas, file_default_instances, TableStruct_v1_2fassistant_2fassistant_2eproto::offsets,
  file_level_metadata_v1_2fassistant_2fassistant_2eproto, file_level_enum_descriptors_v1_2fassistant_2fassistant_2eproto, file_level_service_descriptors_v1_2fassistant_2fassistant_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable* descriptor_table_v1_2fassistant_2fassistant_2eproto_getter() {
  return &descriptor_table_v1_2fassistant_2fassistant_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY static ::PROTOBUF_NAMESPACE_ID::internal::AddDescriptorsRunner dynamic_init_dummy_v1_2fassistant_2fassistant_2eproto(&descriptor_table_v1_2fassistant_2fassistant_2eproto);
namespace sensory {
namespace api {
namespace v1 {
namespace assistant {
const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* ChatRole_descriptor() {
  ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&descriptor_table_v1_2fassistant_2fassistant_2eproto);
  return file_level_enum_descriptors_v1_2fassistant_2fassistant_2eproto[0];
}
bool ChatRole_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
    case 2:
      return true;
    default:
      return false;
  }
}


// ===================================================================

class ChatMessage::_Internal {
 public:
};

ChatMessage::ChatMessage(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor();
  if (!is_message_owned) {
    RegisterArenaDtor(arena);
  }
  // @@protoc_insertion_point(arena_constructor:sensory.api.v1.assistant.ChatMessage)
}
ChatMessage::ChatMessage(const ChatMessage& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  content_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_content().empty()) {
    content_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_content(), 
      GetArenaForAllocation());
  }
  role_ = from.role_;
  // @@protoc_insertion_point(copy_constructor:sensory.api.v1.assistant.ChatMessage)
}

void ChatMessage::SharedCtor() {
content_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
role_ = 0;
}

ChatMessage::~ChatMessage() {
  // @@protoc_insertion_point(destructor:sensory.api.v1.assistant.ChatMessage)
  if (GetArenaForAllocation() != nullptr) return;
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

inline void ChatMessage::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  content_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

void ChatMessage::ArenaDtor(void* object) {
  ChatMessage* _this = reinterpret_cast< ChatMessage* >(object);
  (void)_this;
}
void ChatMessage::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void ChatMessage::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}

void ChatMessage::Clear() {
// @@protoc_insertion_point(message_clear_start:sensory.api.v1.assistant.ChatMessage)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  content_.ClearToEmpty();
  role_ = 0;
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* ChatMessage::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // .sensory.api.v1.assistant.ChatRole role = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 8)) {
          ::PROTOBUF_NAMESPACE_ID::uint64 val = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
          _internal_set_role(static_cast<::sensory::api::v1::assistant::ChatRole>(val));
        } else
          goto handle_unusual;
        continue;
      // string content = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 18)) {
          auto str = _internal_mutable_content();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "sensory.api.v1.assistant.ChatMessage.content"));
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* ChatMessage::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:sensory.api.v1.assistant.ChatMessage)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // .sensory.api.v1.assistant.ChatRole role = 1;
  if (this->_internal_role() != 0) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::WriteEnumToArray(
      1, this->_internal_role(), target);
  }

  // string content = 2;
  if (!this->_internal_content().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_content().data(), static_cast<int>(this->_internal_content().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "sensory.api.v1.assistant.ChatMessage.content");
    target = stream->WriteStringMaybeAliased(
        2, this->_internal_content(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:sensory.api.v1.assistant.ChatMessage)
  return target;
}

size_t ChatMessage::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:sensory.api.v1.assistant.ChatMessage)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // string content = 2;
  if (!this->_internal_content().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_content());
  }

  // .sensory.api.v1.assistant.ChatRole role = 1;
  if (this->_internal_role() != 0) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::EnumSize(this->_internal_role());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData ChatMessage::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSizeCheck,
    ChatMessage::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*ChatMessage::GetClassData() const { return &_class_data_; }

void ChatMessage::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message* to,
                      const ::PROTOBUF_NAMESPACE_ID::Message& from) {
  static_cast<ChatMessage *>(to)->MergeFrom(
      static_cast<const ChatMessage &>(from));
}


void ChatMessage::MergeFrom(const ChatMessage& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:sensory.api.v1.assistant.ChatMessage)
  GOOGLE_DCHECK_NE(&from, this);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_content().empty()) {
    _internal_set_content(from._internal_content());
  }
  if (from._internal_role() != 0) {
    _internal_set_role(from._internal_role());
  }
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void ChatMessage::CopyFrom(const ChatMessage& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:sensory.api.v1.assistant.ChatMessage)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool ChatMessage::IsInitialized() const {
  return true;
}

void ChatMessage::InternalSwap(ChatMessage* other) {
  using std::swap;
  auto* lhs_arena = GetArenaForAllocation();
  auto* rhs_arena = other->GetArenaForAllocation();
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      &content_, lhs_arena,
      &other->content_, rhs_arena
  );
  swap(role_, other->role_);
}

::PROTOBUF_NAMESPACE_ID::Metadata ChatMessage::GetMetadata() const {
  return ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(
      &descriptor_table_v1_2fassistant_2fassistant_2eproto_getter, &descriptor_table_v1_2fassistant_2fassistant_2eproto_once,
      file_level_metadata_v1_2fassistant_2fassistant_2eproto[0]);
}

// ===================================================================

class TextChatRequest::_Internal {
 public:
};

TextChatRequest::TextChatRequest(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned),
  messages_(arena) {
  SharedCtor();
  if (!is_message_owned) {
    RegisterArenaDtor(arena);
  }
  // @@protoc_insertion_point(arena_constructor:sensory.api.v1.assistant.TextChatRequest)
}
TextChatRequest::TextChatRequest(const TextChatRequest& from)
  : ::PROTOBUF_NAMESPACE_ID::Message(),
      messages_(from.messages_) {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  modelname_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
  if (!from._internal_modelname().empty()) {
    modelname_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, from._internal_modelname(), 
      GetArenaForAllocation());
  }
  // @@protoc_insertion_point(copy_constructor:sensory.api.v1.assistant.TextChatRequest)
}

void TextChatRequest::SharedCtor() {
modelname_.UnsafeSetDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

TextChatRequest::~TextChatRequest() {
  // @@protoc_insertion_point(destructor:sensory.api.v1.assistant.TextChatRequest)
  if (GetArenaForAllocation() != nullptr) return;
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

inline void TextChatRequest::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  modelname_.DestroyNoArena(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited());
}

void TextChatRequest::ArenaDtor(void* object) {
  TextChatRequest* _this = reinterpret_cast< TextChatRequest* >(object);
  (void)_this;
}
void TextChatRequest::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void TextChatRequest::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}

void TextChatRequest::Clear() {
// @@protoc_insertion_point(message_clear_start:sensory.api.v1.assistant.TextChatRequest)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  messages_.Clear();
  modelname_.ClearToEmpty();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* TextChatRequest::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // string modelName = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 10)) {
          auto str = _internal_mutable_modelname();
          ptr = ::PROTOBUF_NAMESPACE_ID::internal::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(::PROTOBUF_NAMESPACE_ID::internal::VerifyUTF8(str, "sensory.api.v1.assistant.TextChatRequest.modelName"));
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // repeated .sensory.api.v1.assistant.ChatMessage messages = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 18)) {
          ptr -= 1;
          do {
            ptr += 1;
            ptr = ctx->ParseMessage(_internal_add_messages(), ptr);
            CHK_(ptr);
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::ExpectTag<18>(ptr));
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* TextChatRequest::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:sensory.api.v1.assistant.TextChatRequest)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // string modelName = 1;
  if (!this->_internal_modelname().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_modelname().data(), static_cast<int>(this->_internal_modelname().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "sensory.api.v1.assistant.TextChatRequest.modelName");
    target = stream->WriteStringMaybeAliased(
        1, this->_internal_modelname(), target);
  }

  // repeated .sensory.api.v1.assistant.ChatMessage messages = 2;
  for (unsigned int i = 0,
      n = static_cast<unsigned int>(this->_internal_messages_size()); i < n; i++) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
      InternalWriteMessage(2, this->_internal_messages(i), target, stream);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:sensory.api.v1.assistant.TextChatRequest)
  return target;
}

size_t TextChatRequest::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:sensory.api.v1.assistant.TextChatRequest)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated .sensory.api.v1.assistant.ChatMessage messages = 2;
  total_size += 1UL * this->_internal_messages_size();
  for (const auto& msg : this->messages_) {
    total_size +=
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(msg);
  }

  // string modelName = 1;
  if (!this->_internal_modelname().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_modelname());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData TextChatRequest::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSizeCheck,
    TextChatRequest::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*TextChatRequest::GetClassData() const { return &_class_data_; }

void TextChatRequest::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message* to,
                      const ::PROTOBUF_NAMESPACE_ID::Message& from) {
  static_cast<TextChatRequest *>(to)->MergeFrom(
      static_cast<const TextChatRequest &>(from));
}


void TextChatRequest::MergeFrom(const TextChatRequest& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:sensory.api.v1.assistant.TextChatRequest)
  GOOGLE_DCHECK_NE(&from, this);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  messages_.MergeFrom(from.messages_);
  if (!from._internal_modelname().empty()) {
    _internal_set_modelname(from._internal_modelname());
  }
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void TextChatRequest::CopyFrom(const TextChatRequest& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:sensory.api.v1.assistant.TextChatRequest)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool TextChatRequest::IsInitialized() const {
  return true;
}

void TextChatRequest::InternalSwap(TextChatRequest* other) {
  using std::swap;
  auto* lhs_arena = GetArenaForAllocation();
  auto* rhs_arena = other->GetArenaForAllocation();
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  messages_.InternalSwap(&other->messages_);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      &modelname_, lhs_arena,
      &other->modelname_, rhs_arena
  );
}

::PROTOBUF_NAMESPACE_ID::Metadata TextChatRequest::GetMetadata() const {
  return ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(
      &descriptor_table_v1_2fassistant_2fassistant_2eproto_getter, &descriptor_table_v1_2fassistant_2fassistant_2eproto_once,
      file_level_metadata_v1_2fassistant_2fassistant_2eproto[1]);
}

// ===================================================================

class TextChatResponse::_Internal {
 public:
  static const ::sensory::api::v1::assistant::ChatMessage& message(const TextChatResponse* msg);
};

const ::sensory::api::v1::assistant::ChatMessage&
TextChatResponse::_Internal::message(const TextChatResponse* msg) {
  return *msg->message_;
}
TextChatResponse::TextChatResponse(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor();
  if (!is_message_owned) {
    RegisterArenaDtor(arena);
  }
  // @@protoc_insertion_point(arena_constructor:sensory.api.v1.assistant.TextChatResponse)
}
TextChatResponse::TextChatResponse(const TextChatResponse& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  if (from._internal_has_message()) {
    message_ = new ::sensory::api::v1::assistant::ChatMessage(*from.message_);
  } else {
    message_ = nullptr;
  }
  // @@protoc_insertion_point(copy_constructor:sensory.api.v1.assistant.TextChatResponse)
}

void TextChatResponse::SharedCtor() {
message_ = nullptr;
}

TextChatResponse::~TextChatResponse() {
  // @@protoc_insertion_point(destructor:sensory.api.v1.assistant.TextChatResponse)
  if (GetArenaForAllocation() != nullptr) return;
  SharedDtor();
  _internal_metadata_.Delete<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

inline void TextChatResponse::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  if (this != internal_default_instance()) delete message_;
}

void TextChatResponse::ArenaDtor(void* object) {
  TextChatResponse* _this = reinterpret_cast< TextChatResponse* >(object);
  (void)_this;
}
void TextChatResponse::RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena*) {
}
void TextChatResponse::SetCachedSize(int size) const {
  _cached_size_.Set(size);
}

void TextChatResponse::Clear() {
// @@protoc_insertion_point(message_clear_start:sensory.api.v1.assistant.TextChatResponse)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  if (GetArenaForAllocation() == nullptr && message_ != nullptr) {
    delete message_;
  }
  message_ = nullptr;
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* TextChatResponse::_InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    ::PROTOBUF_NAMESPACE_ID::uint32 tag;
    ptr = ::PROTOBUF_NAMESPACE_ID::internal::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // .sensory.api.v1.assistant.ChatMessage message = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<::PROTOBUF_NAMESPACE_ID::uint8>(tag) == 10)) {
          ptr = ctx->ParseMessage(_internal_mutable_message(), ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

::PROTOBUF_NAMESPACE_ID::uint8* TextChatResponse::_InternalSerialize(
    ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:sensory.api.v1.assistant.TextChatResponse)
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  // .sensory.api.v1.assistant.ChatMessage message = 1;
  if (this->_internal_has_message()) {
    target = stream->EnsureSpace(target);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
      InternalWriteMessage(
        1, _Internal::message(this), target, stream);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:sensory.api.v1.assistant.TextChatResponse)
  return target;
}

size_t TextChatResponse::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:sensory.api.v1.assistant.TextChatResponse)
  size_t total_size = 0;

  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // .sensory.api.v1.assistant.ChatMessage message = 1;
  if (this->_internal_has_message()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(
        *message_);
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData TextChatResponse::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSizeCheck,
    TextChatResponse::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*TextChatResponse::GetClassData() const { return &_class_data_; }

void TextChatResponse::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message* to,
                      const ::PROTOBUF_NAMESPACE_ID::Message& from) {
  static_cast<TextChatResponse *>(to)->MergeFrom(
      static_cast<const TextChatResponse &>(from));
}


void TextChatResponse::MergeFrom(const TextChatResponse& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:sensory.api.v1.assistant.TextChatResponse)
  GOOGLE_DCHECK_NE(&from, this);
  ::PROTOBUF_NAMESPACE_ID::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (from._internal_has_message()) {
    _internal_mutable_message()->::sensory::api::v1::assistant::ChatMessage::MergeFrom(from._internal_message());
  }
  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void TextChatResponse::CopyFrom(const TextChatResponse& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:sensory.api.v1.assistant.TextChatResponse)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool TextChatResponse::IsInitialized() const {
  return true;
}

void TextChatResponse::InternalSwap(TextChatResponse* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  swap(message_, other->message_);
}

::PROTOBUF_NAMESPACE_ID::Metadata TextChatResponse::GetMetadata() const {
  return ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(
      &descriptor_table_v1_2fassistant_2fassistant_2eproto_getter, &descriptor_table_v1_2fassistant_2fassistant_2eproto_once,
      file_level_metadata_v1_2fassistant_2fassistant_2eproto[2]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace assistant
}  // namespace v1
}  // namespace api
}  // namespace sensory
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::sensory::api::v1::assistant::ChatMessage* Arena::CreateMaybeMessage< ::sensory::api::v1::assistant::ChatMessage >(Arena* arena) {
  return Arena::CreateMessageInternal< ::sensory::api::v1::assistant::ChatMessage >(arena);
}
template<> PROTOBUF_NOINLINE ::sensory::api::v1::assistant::TextChatRequest* Arena::CreateMaybeMessage< ::sensory::api::v1::assistant::TextChatRequest >(Arena* arena) {
  return Arena::CreateMessageInternal< ::sensory::api::v1::assistant::TextChatRequest >(arena);
}
template<> PROTOBUF_NOINLINE ::sensory::api::v1::assistant::TextChatResponse* Arena::CreateMaybeMessage< ::sensory::api::v1::assistant::TextChatResponse >(Arena* arena) {
  return Arena::CreateMessageInternal< ::sensory::api::v1::assistant::TextChatResponse >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
