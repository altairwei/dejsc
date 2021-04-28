// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <CLI/CLI.hpp>

#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"
#include "v8/src/handles/handles.h"
#include "v8/src/objects/shared-function-info.h"

static Handle<SharedFunctionInfo> CompileScript(
    Isolate* isolate, Handle<String> source, Handle<String> name,
    ScriptData* cached_data, v8::ScriptCompiler::CompileOptions options) {
  return Compiler::GetSharedFunctionInfoForScript(
             isolate, source, Compiler::ScriptDetails(name),
             v8::ScriptOriginOptions(), nullptr, cached_data, options,
             ScriptCompiler::kNoCacheNoReason, NOT_NATIVES_CODE)
      .ToHandleChecked();
}

static Handle<SharedFunctionInfo> CompileScriptAndProduceCache(
    Isolate* isolate, Handle<String> source, Handle<String> name,
    ScriptData** script_data, v8::ScriptCompiler::CompileOptions options) {
  Handle<SharedFunctionInfo> sfi =
      Compiler::GetSharedFunctionInfoForScript(
          isolate, source, Compiler::ScriptDetails(name),
          v8::ScriptOriginOptions(), nullptr, nullptr, options,
          ScriptCompiler::kNoCacheNoReason, NOT_NATIVES_CODE)
          .ToHandleChecked();
  std::unique_ptr<ScriptCompiler::CachedData> cached_data(
      ScriptCompiler::CreateCodeCache(ToApiHandle<UnboundScript>(sfi)));
  uint8_t* buffer = NewArray<uint8_t>(cached_data->length);
  MemCopy(buffer, cached_data->data, cached_data->length);
  *script_data = new i::ScriptData(buffer, cached_data->length);
  (*script_data)->AcquireDataOwnership();
  return sfi;
}

int main(int argc, char* argv[]) {

}