// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: v1/event/event.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_v1_2fevent_2fevent_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_v1_2fevent_2fevent_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3017000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3017003 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
#include <google/protobuf/timestamp.pb.h>
#include "sensorycloud/protoc/validate/validate.pb.h"
#include "sensorycloud/protoc/common/common.pb.h"
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_v1_2fevent_2fevent_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_v1_2fevent_2fevent_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxiliaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[3]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_v1_2fevent_2fevent_2eproto;
namespace sensory {
namespace api {
namespace v1 {
namespace event {
class PublishUsageEventsRequest;
struct PublishUsageEventsRequestDefaultTypeInternal;
extern PublishUsageEventsRequestDefaultTypeInternal _PublishUsageEventsRequest_default_instance_;
class PublishUsageEventsResponse;
struct PublishUsageEventsResponseDefaultTypeInternal;
extern PublishUsageEventsResponseDefaultTypeInternal _PublishUsageEventsResponse_default_instance_;
class UsageEvent;
struct UsageEventDefaultTypeInternal;
extern UsageEventDefaultTypeInternal _UsageEvent_default_instance_;
}  // namespace event
}  // namespace v1
}  // namespace api
}  // namespace sensory
PROTOBUF_NAMESPACE_OPEN
template<> ::sensory::api::v1::event::PublishUsageEventsRequest* Arena::CreateMaybeMessage<::sensory::api::v1::event::PublishUsageEventsRequest>(Arena*);
template<> ::sensory::api::v1::event::PublishUsageEventsResponse* Arena::CreateMaybeMessage<::sensory::api::v1::event::PublishUsageEventsResponse>(Arena*);
template<> ::sensory::api::v1::event::UsageEvent* Arena::CreateMaybeMessage<::sensory::api::v1::event::UsageEvent>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace sensory {
namespace api {
namespace v1 {
namespace event {

// ===================================================================

class PublishUsageEventsRequest final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:sensory.api.v1.event.PublishUsageEventsRequest) */ {
 public:
  inline PublishUsageEventsRequest() : PublishUsageEventsRequest(nullptr) {}
  ~PublishUsageEventsRequest() override;
  explicit constexpr PublishUsageEventsRequest(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  PublishUsageEventsRequest(const PublishUsageEventsRequest& from);
  PublishUsageEventsRequest(PublishUsageEventsRequest&& from) noexcept
    : PublishUsageEventsRequest() {
    *this = ::std::move(from);
  }

  inline PublishUsageEventsRequest& operator=(const PublishUsageEventsRequest& from) {
    CopyFrom(from);
    return *this;
  }
  inline PublishUsageEventsRequest& operator=(PublishUsageEventsRequest&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const PublishUsageEventsRequest& default_instance() {
    return *internal_default_instance();
  }
  static inline const PublishUsageEventsRequest* internal_default_instance() {
    return reinterpret_cast<const PublishUsageEventsRequest*>(
               &_PublishUsageEventsRequest_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(PublishUsageEventsRequest& a, PublishUsageEventsRequest& b) {
    a.Swap(&b);
  }
  inline void Swap(PublishUsageEventsRequest* other) {
    if (other == this) return;
    if (GetOwningArena() == other->GetOwningArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(PublishUsageEventsRequest* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline PublishUsageEventsRequest* New() const final {
    return new PublishUsageEventsRequest();
  }

  PublishUsageEventsRequest* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<PublishUsageEventsRequest>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const PublishUsageEventsRequest& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom(const PublishUsageEventsRequest& from);
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message*to, const ::PROTOBUF_NAMESPACE_ID::Message&from);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(PublishUsageEventsRequest* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "sensory.api.v1.event.PublishUsageEventsRequest";
  }
  protected:
  explicit PublishUsageEventsRequest(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kEventsFieldNumber = 1,
  };
  // repeated .sensory.api.v1.event.UsageEvent events = 1;
  int events_size() const;
  private:
  int _internal_events_size() const;
  public:
  void clear_events();
  ::sensory::api::v1::event::UsageEvent* mutable_events(int index);
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::sensory::api::v1::event::UsageEvent >*
      mutable_events();
  private:
  const ::sensory::api::v1::event::UsageEvent& _internal_events(int index) const;
  ::sensory::api::v1::event::UsageEvent* _internal_add_events();
  public:
  const ::sensory::api::v1::event::UsageEvent& events(int index) const;
  ::sensory::api::v1::event::UsageEvent* add_events();
  const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::sensory::api::v1::event::UsageEvent >&
      events() const;

  // @@protoc_insertion_point(class_scope:sensory.api.v1.event.PublishUsageEventsRequest)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::sensory::api::v1::event::UsageEvent > events_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_v1_2fevent_2fevent_2eproto;
};
// -------------------------------------------------------------------

class UsageEvent final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:sensory.api.v1.event.UsageEvent) */ {
 public:
  inline UsageEvent() : UsageEvent(nullptr) {}
  ~UsageEvent() override;
  explicit constexpr UsageEvent(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  UsageEvent(const UsageEvent& from);
  UsageEvent(UsageEvent&& from) noexcept
    : UsageEvent() {
    *this = ::std::move(from);
  }

  inline UsageEvent& operator=(const UsageEvent& from) {
    CopyFrom(from);
    return *this;
  }
  inline UsageEvent& operator=(UsageEvent&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const UsageEvent& default_instance() {
    return *internal_default_instance();
  }
  static inline const UsageEvent* internal_default_instance() {
    return reinterpret_cast<const UsageEvent*>(
               &_UsageEvent_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(UsageEvent& a, UsageEvent& b) {
    a.Swap(&b);
  }
  inline void Swap(UsageEvent* other) {
    if (other == this) return;
    if (GetOwningArena() == other->GetOwningArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(UsageEvent* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline UsageEvent* New() const final {
    return new UsageEvent();
  }

  UsageEvent* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<UsageEvent>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const UsageEvent& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom(const UsageEvent& from);
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message*to, const ::PROTOBUF_NAMESPACE_ID::Message&from);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(UsageEvent* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "sensory.api.v1.event.UsageEvent";
  }
  protected:
  explicit UsageEvent(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kIdFieldNumber = 3,
    kClientIdFieldNumber = 4,
    kRouteFieldNumber = 6,
    kTimestampFieldNumber = 1,
    kDurationFieldNumber = 2,
    kTypeFieldNumber = 5,
  };
  // string id = 3 [(.validate.rules) = {
  void clear_id();
  const std::string& id() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_id(ArgT0&& arg0, ArgT... args);
  std::string* mutable_id();
  PROTOBUF_MUST_USE_RESULT std::string* release_id();
  void set_allocated_id(std::string* id);
  private:
  const std::string& _internal_id() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_id(const std::string& value);
  std::string* _internal_mutable_id();
  public:

  // string clientId = 4 [(.validate.rules) = {
  void clear_clientid();
  const std::string& clientid() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_clientid(ArgT0&& arg0, ArgT... args);
  std::string* mutable_clientid();
  PROTOBUF_MUST_USE_RESULT std::string* release_clientid();
  void set_allocated_clientid(std::string* clientid);
  private:
  const std::string& _internal_clientid() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_clientid(const std::string& value);
  std::string* _internal_mutable_clientid();
  public:

  // string route = 6 [(.validate.rules) = {
  void clear_route();
  const std::string& route() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_route(ArgT0&& arg0, ArgT... args);
  std::string* mutable_route();
  PROTOBUF_MUST_USE_RESULT std::string* release_route();
  void set_allocated_route(std::string* route);
  private:
  const std::string& _internal_route() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_route(const std::string& value);
  std::string* _internal_mutable_route();
  public:

  // .google.protobuf.Timestamp timestamp = 1 [(.validate.rules) = {
  bool has_timestamp() const;
  private:
  bool _internal_has_timestamp() const;
  public:
  void clear_timestamp();
  const PROTOBUF_NAMESPACE_ID::Timestamp& timestamp() const;
  PROTOBUF_MUST_USE_RESULT PROTOBUF_NAMESPACE_ID::Timestamp* release_timestamp();
  PROTOBUF_NAMESPACE_ID::Timestamp* mutable_timestamp();
  void set_allocated_timestamp(PROTOBUF_NAMESPACE_ID::Timestamp* timestamp);
  private:
  const PROTOBUF_NAMESPACE_ID::Timestamp& _internal_timestamp() const;
  PROTOBUF_NAMESPACE_ID::Timestamp* _internal_mutable_timestamp();
  public:
  void unsafe_arena_set_allocated_timestamp(
      PROTOBUF_NAMESPACE_ID::Timestamp* timestamp);
  PROTOBUF_NAMESPACE_ID::Timestamp* unsafe_arena_release_timestamp();

  // int64 duration = 2 [(.validate.rules) = {
  void clear_duration();
  ::PROTOBUF_NAMESPACE_ID::int64 duration() const;
  void set_duration(::PROTOBUF_NAMESPACE_ID::int64 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int64 _internal_duration() const;
  void _internal_set_duration(::PROTOBUF_NAMESPACE_ID::int64 value);
  public:

  // .sensory.api.common.UsageEventType type = 5 [(.validate.rules) = {
  void clear_type();
  ::sensory::api::common::UsageEventType type() const;
  void set_type(::sensory::api::common::UsageEventType value);
  private:
  ::sensory::api::common::UsageEventType _internal_type() const;
  void _internal_set_type(::sensory::api::common::UsageEventType value);
  public:

  // @@protoc_insertion_point(class_scope:sensory.api.v1.event.UsageEvent)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr id_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr clientid_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr route_;
  PROTOBUF_NAMESPACE_ID::Timestamp* timestamp_;
  ::PROTOBUF_NAMESPACE_ID::int64 duration_;
  int type_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_v1_2fevent_2fevent_2eproto;
};
// -------------------------------------------------------------------

class PublishUsageEventsResponse final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:sensory.api.v1.event.PublishUsageEventsResponse) */ {
 public:
  inline PublishUsageEventsResponse() : PublishUsageEventsResponse(nullptr) {}
  ~PublishUsageEventsResponse() override;
  explicit constexpr PublishUsageEventsResponse(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  PublishUsageEventsResponse(const PublishUsageEventsResponse& from);
  PublishUsageEventsResponse(PublishUsageEventsResponse&& from) noexcept
    : PublishUsageEventsResponse() {
    *this = ::std::move(from);
  }

  inline PublishUsageEventsResponse& operator=(const PublishUsageEventsResponse& from) {
    CopyFrom(from);
    return *this;
  }
  inline PublishUsageEventsResponse& operator=(PublishUsageEventsResponse&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const PublishUsageEventsResponse& default_instance() {
    return *internal_default_instance();
  }
  static inline const PublishUsageEventsResponse* internal_default_instance() {
    return reinterpret_cast<const PublishUsageEventsResponse*>(
               &_PublishUsageEventsResponse_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    2;

  friend void swap(PublishUsageEventsResponse& a, PublishUsageEventsResponse& b) {
    a.Swap(&b);
  }
  inline void Swap(PublishUsageEventsResponse* other) {
    if (other == this) return;
    if (GetOwningArena() == other->GetOwningArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(PublishUsageEventsResponse* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline PublishUsageEventsResponse* New() const final {
    return new PublishUsageEventsResponse();
  }

  PublishUsageEventsResponse* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<PublishUsageEventsResponse>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const PublishUsageEventsResponse& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom(const PublishUsageEventsResponse& from);
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message*to, const ::PROTOBUF_NAMESPACE_ID::Message&from);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(PublishUsageEventsResponse* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "sensory.api.v1.event.PublishUsageEventsResponse";
  }
  protected:
  explicit PublishUsageEventsResponse(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:sensory.api.v1.event.PublishUsageEventsResponse)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_v1_2fevent_2fevent_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// PublishUsageEventsRequest

// repeated .sensory.api.v1.event.UsageEvent events = 1;
inline int PublishUsageEventsRequest::_internal_events_size() const {
  return events_.size();
}
inline int PublishUsageEventsRequest::events_size() const {
  return _internal_events_size();
}
inline void PublishUsageEventsRequest::clear_events() {
  events_.Clear();
}
inline ::sensory::api::v1::event::UsageEvent* PublishUsageEventsRequest::mutable_events(int index) {
  // @@protoc_insertion_point(field_mutable:sensory.api.v1.event.PublishUsageEventsRequest.events)
  return events_.Mutable(index);
}
inline ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::sensory::api::v1::event::UsageEvent >*
PublishUsageEventsRequest::mutable_events() {
  // @@protoc_insertion_point(field_mutable_list:sensory.api.v1.event.PublishUsageEventsRequest.events)
  return &events_;
}
inline const ::sensory::api::v1::event::UsageEvent& PublishUsageEventsRequest::_internal_events(int index) const {
  return events_.Get(index);
}
inline const ::sensory::api::v1::event::UsageEvent& PublishUsageEventsRequest::events(int index) const {
  // @@protoc_insertion_point(field_get:sensory.api.v1.event.PublishUsageEventsRequest.events)
  return _internal_events(index);
}
inline ::sensory::api::v1::event::UsageEvent* PublishUsageEventsRequest::_internal_add_events() {
  return events_.Add();
}
inline ::sensory::api::v1::event::UsageEvent* PublishUsageEventsRequest::add_events() {
  ::sensory::api::v1::event::UsageEvent* _add = _internal_add_events();
  // @@protoc_insertion_point(field_add:sensory.api.v1.event.PublishUsageEventsRequest.events)
  return _add;
}
inline const ::PROTOBUF_NAMESPACE_ID::RepeatedPtrField< ::sensory::api::v1::event::UsageEvent >&
PublishUsageEventsRequest::events() const {
  // @@protoc_insertion_point(field_list:sensory.api.v1.event.PublishUsageEventsRequest.events)
  return events_;
}

// -------------------------------------------------------------------

// UsageEvent

// .google.protobuf.Timestamp timestamp = 1 [(.validate.rules) = {
inline bool UsageEvent::_internal_has_timestamp() const {
  return this != internal_default_instance() && timestamp_ != nullptr;
}
inline bool UsageEvent::has_timestamp() const {
  return _internal_has_timestamp();
}
inline const PROTOBUF_NAMESPACE_ID::Timestamp& UsageEvent::_internal_timestamp() const {
  const PROTOBUF_NAMESPACE_ID::Timestamp* p = timestamp_;
  return p != nullptr ? *p : reinterpret_cast<const PROTOBUF_NAMESPACE_ID::Timestamp&>(
      PROTOBUF_NAMESPACE_ID::_Timestamp_default_instance_);
}
inline const PROTOBUF_NAMESPACE_ID::Timestamp& UsageEvent::timestamp() const {
  // @@protoc_insertion_point(field_get:sensory.api.v1.event.UsageEvent.timestamp)
  return _internal_timestamp();
}
inline void UsageEvent::unsafe_arena_set_allocated_timestamp(
    PROTOBUF_NAMESPACE_ID::Timestamp* timestamp) {
  if (GetArenaForAllocation() == nullptr) {
    delete reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(timestamp_);
  }
  timestamp_ = timestamp;
  if (timestamp) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:sensory.api.v1.event.UsageEvent.timestamp)
}
inline PROTOBUF_NAMESPACE_ID::Timestamp* UsageEvent::release_timestamp() {
  
  PROTOBUF_NAMESPACE_ID::Timestamp* temp = timestamp_;
  timestamp_ = nullptr;
#ifdef PROTOBUF_FORCE_COPY_IN_RELEASE
  auto* old =  reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(temp);
  temp = ::PROTOBUF_NAMESPACE_ID::internal::DuplicateIfNonNull(temp);
  if (GetArenaForAllocation() == nullptr) { delete old; }
#else  // PROTOBUF_FORCE_COPY_IN_RELEASE
  if (GetArenaForAllocation() != nullptr) {
    temp = ::PROTOBUF_NAMESPACE_ID::internal::DuplicateIfNonNull(temp);
  }
#endif  // !PROTOBUF_FORCE_COPY_IN_RELEASE
  return temp;
}
inline PROTOBUF_NAMESPACE_ID::Timestamp* UsageEvent::unsafe_arena_release_timestamp() {
  // @@protoc_insertion_point(field_release:sensory.api.v1.event.UsageEvent.timestamp)
  
  PROTOBUF_NAMESPACE_ID::Timestamp* temp = timestamp_;
  timestamp_ = nullptr;
  return temp;
}
inline PROTOBUF_NAMESPACE_ID::Timestamp* UsageEvent::_internal_mutable_timestamp() {
  
  if (timestamp_ == nullptr) {
    auto* p = CreateMaybeMessage<PROTOBUF_NAMESPACE_ID::Timestamp>(GetArenaForAllocation());
    timestamp_ = p;
  }
  return timestamp_;
}
inline PROTOBUF_NAMESPACE_ID::Timestamp* UsageEvent::mutable_timestamp() {
  PROTOBUF_NAMESPACE_ID::Timestamp* _msg = _internal_mutable_timestamp();
  // @@protoc_insertion_point(field_mutable:sensory.api.v1.event.UsageEvent.timestamp)
  return _msg;
}
inline void UsageEvent::set_allocated_timestamp(PROTOBUF_NAMESPACE_ID::Timestamp* timestamp) {
  ::PROTOBUF_NAMESPACE_ID::Arena* message_arena = GetArenaForAllocation();
  if (message_arena == nullptr) {
    delete reinterpret_cast< ::PROTOBUF_NAMESPACE_ID::MessageLite*>(timestamp_);
  }
  if (timestamp) {
    ::PROTOBUF_NAMESPACE_ID::Arena* submessage_arena =
        ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper<
            ::PROTOBUF_NAMESPACE_ID::MessageLite>::GetOwningArena(
                reinterpret_cast<::PROTOBUF_NAMESPACE_ID::MessageLite*>(timestamp));
    if (message_arena != submessage_arena) {
      timestamp = ::PROTOBUF_NAMESPACE_ID::internal::GetOwnedMessage(
          message_arena, timestamp, submessage_arena);
    }
    
  } else {
    
  }
  timestamp_ = timestamp;
  // @@protoc_insertion_point(field_set_allocated:sensory.api.v1.event.UsageEvent.timestamp)
}

// int64 duration = 2 [(.validate.rules) = {
inline void UsageEvent::clear_duration() {
  duration_ = int64_t{0};
}
inline ::PROTOBUF_NAMESPACE_ID::int64 UsageEvent::_internal_duration() const {
  return duration_;
}
inline ::PROTOBUF_NAMESPACE_ID::int64 UsageEvent::duration() const {
  // @@protoc_insertion_point(field_get:sensory.api.v1.event.UsageEvent.duration)
  return _internal_duration();
}
inline void UsageEvent::_internal_set_duration(::PROTOBUF_NAMESPACE_ID::int64 value) {
  
  duration_ = value;
}
inline void UsageEvent::set_duration(::PROTOBUF_NAMESPACE_ID::int64 value) {
  _internal_set_duration(value);
  // @@protoc_insertion_point(field_set:sensory.api.v1.event.UsageEvent.duration)
}

// string id = 3 [(.validate.rules) = {
inline void UsageEvent::clear_id() {
  id_.ClearToEmpty();
}
inline const std::string& UsageEvent::id() const {
  // @@protoc_insertion_point(field_get:sensory.api.v1.event.UsageEvent.id)
  return _internal_id();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void UsageEvent::set_id(ArgT0&& arg0, ArgT... args) {
 
 id_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:sensory.api.v1.event.UsageEvent.id)
}
inline std::string* UsageEvent::mutable_id() {
  std::string* _s = _internal_mutable_id();
  // @@protoc_insertion_point(field_mutable:sensory.api.v1.event.UsageEvent.id)
  return _s;
}
inline const std::string& UsageEvent::_internal_id() const {
  return id_.Get();
}
inline void UsageEvent::_internal_set_id(const std::string& value) {
  
  id_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* UsageEvent::_internal_mutable_id() {
  
  return id_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* UsageEvent::release_id() {
  // @@protoc_insertion_point(field_release:sensory.api.v1.event.UsageEvent.id)
  return id_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void UsageEvent::set_allocated_id(std::string* id) {
  if (id != nullptr) {
    
  } else {
    
  }
  id_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), id,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:sensory.api.v1.event.UsageEvent.id)
}

// string clientId = 4 [(.validate.rules) = {
inline void UsageEvent::clear_clientid() {
  clientid_.ClearToEmpty();
}
inline const std::string& UsageEvent::clientid() const {
  // @@protoc_insertion_point(field_get:sensory.api.v1.event.UsageEvent.clientId)
  return _internal_clientid();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void UsageEvent::set_clientid(ArgT0&& arg0, ArgT... args) {
 
 clientid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:sensory.api.v1.event.UsageEvent.clientId)
}
inline std::string* UsageEvent::mutable_clientid() {
  std::string* _s = _internal_mutable_clientid();
  // @@protoc_insertion_point(field_mutable:sensory.api.v1.event.UsageEvent.clientId)
  return _s;
}
inline const std::string& UsageEvent::_internal_clientid() const {
  return clientid_.Get();
}
inline void UsageEvent::_internal_set_clientid(const std::string& value) {
  
  clientid_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* UsageEvent::_internal_mutable_clientid() {
  
  return clientid_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* UsageEvent::release_clientid() {
  // @@protoc_insertion_point(field_release:sensory.api.v1.event.UsageEvent.clientId)
  return clientid_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void UsageEvent::set_allocated_clientid(std::string* clientid) {
  if (clientid != nullptr) {
    
  } else {
    
  }
  clientid_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), clientid,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:sensory.api.v1.event.UsageEvent.clientId)
}

// .sensory.api.common.UsageEventType type = 5 [(.validate.rules) = {
inline void UsageEvent::clear_type() {
  type_ = 0;
}
inline ::sensory::api::common::UsageEventType UsageEvent::_internal_type() const {
  return static_cast< ::sensory::api::common::UsageEventType >(type_);
}
inline ::sensory::api::common::UsageEventType UsageEvent::type() const {
  // @@protoc_insertion_point(field_get:sensory.api.v1.event.UsageEvent.type)
  return _internal_type();
}
inline void UsageEvent::_internal_set_type(::sensory::api::common::UsageEventType value) {
  
  type_ = value;
}
inline void UsageEvent::set_type(::sensory::api::common::UsageEventType value) {
  _internal_set_type(value);
  // @@protoc_insertion_point(field_set:sensory.api.v1.event.UsageEvent.type)
}

// string route = 6 [(.validate.rules) = {
inline void UsageEvent::clear_route() {
  route_.ClearToEmpty();
}
inline const std::string& UsageEvent::route() const {
  // @@protoc_insertion_point(field_get:sensory.api.v1.event.UsageEvent.route)
  return _internal_route();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void UsageEvent::set_route(ArgT0&& arg0, ArgT... args) {
 
 route_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:sensory.api.v1.event.UsageEvent.route)
}
inline std::string* UsageEvent::mutable_route() {
  std::string* _s = _internal_mutable_route();
  // @@protoc_insertion_point(field_mutable:sensory.api.v1.event.UsageEvent.route)
  return _s;
}
inline const std::string& UsageEvent::_internal_route() const {
  return route_.Get();
}
inline void UsageEvent::_internal_set_route(const std::string& value) {
  
  route_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* UsageEvent::_internal_mutable_route() {
  
  return route_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* UsageEvent::release_route() {
  // @@protoc_insertion_point(field_release:sensory.api.v1.event.UsageEvent.route)
  return route_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
}
inline void UsageEvent::set_allocated_route(std::string* route) {
  if (route != nullptr) {
    
  } else {
    
  }
  route_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), route,
      GetArenaForAllocation());
  // @@protoc_insertion_point(field_set_allocated:sensory.api.v1.event.UsageEvent.route)
}

// -------------------------------------------------------------------

// PublishUsageEventsResponse

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace event
}  // namespace v1
}  // namespace api
}  // namespace sensory

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_v1_2fevent_2fevent_2eproto
