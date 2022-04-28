#ifndef DEJSC_CACHE_H
#define DEJSC_CACHE_H

#include <string>
#include <vector>
#include <memory>

#include <v8/src/snapshot/code-serializer.h>
#include <v8/include/v8.h>

namespace dejsc {
namespace Cache {

class CachedCode : public v8::internal::SerializedData {

public:
    CachedCode(const std::string &cache_filename);
    CachedCode(v8::internal::ScriptData* data);
    CachedCode(v8::ScriptCompiler::CachedData* data);

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
    v8::internal::Vector<const v8::internal::byte> Payload() const;

    // Return ScriptData object and the ownership unchanged.
    std::unique_ptr<v8::internal::ScriptData> GetScriptData();
    // Return CachedData object and the ownership unchanged.
    std::unique_ptr<v8::ScriptCompiler::CachedData> GetCachedData();

    void PrintHeaderInfo();

private:
    std::vector<uint8_t> m_raw;
};


} // namespace Cache
} // namespace dejsc


#endif // DEJSC_CACHE_H