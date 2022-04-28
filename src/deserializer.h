#ifndef DEJSC_DESERIALIZER_H
#define DEJSC_DESERIALIZER_H

#include <v8/src/snapshot/code-serializer.h>
#include <v8/src/handles/maybe-handles.h>
#include <v8/include/v8.h>

#include "cache.h"

namespace dejsc {
namespace Deserializer {

v8::internal::MaybeHandle<v8::internal::SharedFunctionInfo> DeserializeCacheData(v8::internal::ScriptData &data);
v8::internal::MaybeHandle<v8::internal::SharedFunctionInfo> DeserializeCacheData(Cache::CachedCode &data);
v8::internal::MaybeHandle<v8::internal::SharedFunctionInfo> DeserializeCacheData(v8::ScriptCompiler::CachedData &data);


} // namespace Deserializer
} // namespace dejsc

#endif // DEJSC_DESERIALIZER_H