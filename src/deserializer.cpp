#include "deserializer.h"

#include "runner.h"
#include "utils.h"

namespace dejsc {
namespace Deserializer {

using namespace v8::internal;

MaybeHandle<SharedFunctionInfo> DeserializeCacheData(Cache::CachedCode &data)
{
    std::string fake_source(' ', data.GetSourceHash());
    MaybeHandle<SharedFunctionInfo> info;

    Runner::RunInV8([&](v8::Isolate *isolate, v8::Local<v8::Context> &context) -> int {
        Isolate* iso = reinterpret_cast<i::Isolate*>(isolate);
        Handle<String> orig_source = iso->factory()
                                        ->NewStringFromUtf8(CStrVector(fake_source.c_str()))
                                        .ToHandleChecked();
        info = CodeSerializer::Deserialize(
            iso, data.GetScriptData().get(), orig_source, v8::ScriptCompiler::kNoCompileOptions);
        return 0;
    });

    return info;
}

MaybeHandle<SharedFunctionInfo> DeserializeCacheData(ScriptData &data)
{
    Cache::CachedCode cached_code(&data);
    return DeserializeCacheData(cached_code);
}

MaybeHandle<SharedFunctionInfo> DeserializeCacheData(v8::ScriptCompiler::CachedData &data)
{
    Cache::CachedCode cached_code(&data);
    return DeserializeCacheData(cached_code);
}

} // namespace Deserializer
} // namespace dejsc