#include "cache.h"

namespace dejsc {
namespace Cache {

using namespace v8::internal;

CachedCode::CachedCode(ScriptData* data)
    : SerializedData(const_cast<byte*>(data->data()), data->length()) {}


uint32_t CachedCode::GetVersionHash()
{
    return GetHeaderValue(kVersionHashOffset);
}


uint32_t CachedCode::GetSourceHash()
{
    return GetHeaderValue(kSourceHashOffset);
}


uint32_t CachedCode::GetFlagsHash()
{
    return GetHeaderValue(kFlagHashOffset);
}


uint32_t CachedCode::GetPayloadLength()
{
    return GetHeaderValue(kPayloadLengthOffset);
}


uint32_t CachedCode::GetChecksum()
{
    return GetHeaderValue(kChecksumOffset);
}


void CachedCode::PrintHeaderInfo()
{
    std::cout << "MagicNumber    : " << GetMagicNumber() << std::endl;
    std::cout << "VersionHash    : " << GetVersionHash() << std::endl;
    std::cout << "SourceHash     : " << GetSourceHash() << std::endl;
    std::cout << "FlagsHash      : " << GetFlagsHash() << std::endl;
    std::cout << "PayloadLength  : " << GetPayloadLength() << std::endl;
    std::cout << "Checksum       : " << GetChecksum() << std::endl;
}

} // namespace Cache
} // namespace dejsc