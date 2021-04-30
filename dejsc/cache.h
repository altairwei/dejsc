#ifndef DEJSC_CACHE_H
#define DEJSC_CACHE_H

#include "src/snapshot/code-serializer.h"

namespace dejsc {
namespace Cache {

class CachedCode : public v8::internal::SerializedData {

public:
    CachedCode(v8::internal::ScriptData* data);

    // The data header consists of uint32_t-sized entries:
    // [0] magic number and (internally provided) external reference count
    // [1] version hash
    // [2] source hash
    // [3] flag hash
    // [4] payload length
    // [5] payload checksum
    // ...  serialized payload
    static const uint32_t kVersionHashOffset = kMagicNumberOffset + v8::internal::kUInt32Size;
    static const uint32_t kSourceHashOffset = kVersionHashOffset + v8::internal::kUInt32Size;
    static const uint32_t kFlagHashOffset = kSourceHashOffset + v8::internal::kUInt32Size;
    static const uint32_t kPayloadLengthOffset = kFlagHashOffset + v8::internal::kUInt32Size;
    static const uint32_t kChecksumOffset = kPayloadLengthOffset + v8::internal::kUInt32Size;
    static const uint32_t kUnalignedHeaderSize = kChecksumOffset + v8::internal::kUInt32Size;
    static const uint32_t kHeaderSize = POINTER_SIZE_ALIGN(kUnalignedHeaderSize);

    uint32_t GetVersionHash();
    uint32_t GetSourceHash();
    uint32_t GetFlagsHash();
    uint32_t GetPayloadLength();
    uint32_t GetChecksum();

    void PrintHeaderInfo();
};


} // namespace Cache
} // namespace dejsc


#endif // DEJSC_CACHE_H