#include "cache.h"

#include "utils.h"

namespace dejsc {
namespace Cache {

using namespace v8::internal;

CachedCode::CachedCode(const std::string &cache_filename)
{
    m_raw = dejsc::IO::read_binary(cache_filename);
    data_ = m_raw.data();
    size_ = m_raw.size();
    owns_data_ = false;
}


CachedCode::CachedCode(ScriptData* data)
    : SerializedData(const_cast<byte*>(data->data()), data->length()) {}


CachedCode::CachedCode(v8::ScriptCompiler::CachedData* data)
    : SerializedData(const_cast<byte*>(data->data), data->length) {}


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


Vector<const byte> CachedCode::Payload() const
{
  const byte* payload = data_ + kHeaderSize;
  DCHECK(IsAligned(reinterpret_cast<intptr_t>(payload), kPointerAlignment));
  int length = GetHeaderValue(kPayloadLengthOffset);
  DCHECK_EQ(data_ + size_, payload + length);
  return Vector<const byte>(payload, length);
}


std::unique_ptr<ScriptData> CachedCode::GetScriptData()
{
    DCHECK(owns_data_);
    return std::make_unique<ScriptData>(data_, size_);
}


std::unique_ptr<v8::ScriptCompiler::CachedData> CachedCode::GetCachedData()
{
    DCHECK(owns_data_);
    return std::make_unique<v8::ScriptCompiler::CachedData>(
        data_, size_, v8::ScriptCompiler::CachedData::BufferNotOwned);
}


void CachedCode::PrintHeaderInfo()
{
    std::cout << "MagicNumber    : " << std::hex << GetMagicNumber() << std::dec << std::endl;
    std::cout << "VersionHash    : " << std::hex << GetVersionHash() << std::dec << std::endl;
    std::cout << "SourceHash     : " << GetSourceHash() << std::endl;
    std::cout << "FlagsHash      : " << GetFlagsHash() << std::endl;
    std::cout << "PayloadLength  : " << GetPayloadLength() << std::endl;
    std::cout << "Checksum       : " << GetChecksum() << std::endl;

    Vector<const byte> payload = Payload();
    std::cout << "PayloadChecked : " << payload.length() << std::endl;
}

} // namespace Cache
} // namespace dejsc