#include "runner.h"

#include <v8/src/snapshot/code-serializer.h>
#include <v8/src/objects/shared-function-info.h>
#include <v8/src/handles/handles.h>
#include <v8/src/codegen/compiler.h>

#include "utils.h"
#include "shell.h"
#include "cache.h"


namespace dejsc {
namespace Runner {

using namespace v8::internal;

std::unique_ptr<v8::Platform> gDefaultPlatform = v8::platform::NewDefaultPlatform();


static Handle<SharedFunctionInfo> CompileScript(
        Isolate* isolate, Handle<String> source, Handle<String> name,
        ScriptData* cached_data, v8::ScriptCompiler::CompileOptions options) {
    return Compiler::GetSharedFunctionInfoForScript(
                isolate, source, Compiler::ScriptDetails(name),
                v8::ScriptOriginOptions(), nullptr, cached_data, options,
                v8::ScriptCompiler::kNoCacheNoReason, NOT_NATIVES_CODE)
        .ToHandleChecked();
}


static Handle<SharedFunctionInfo> CompileScriptAndProduceCache(
        Isolate* isolate, Handle<String> source, Handle<String> name,
        ScriptData** script_data, v8::ScriptCompiler::CompileOptions options) {
    Handle<SharedFunctionInfo> sfi =
        Compiler::GetSharedFunctionInfoForScript(
            isolate, source, Compiler::ScriptDetails(name),
            v8::ScriptOriginOptions(), nullptr, nullptr, options,
            v8::ScriptCompiler::kNoCacheNoReason, NOT_NATIVES_CODE)
            .ToHandleChecked();
    std::unique_ptr<v8::ScriptCompiler::CachedData> cached_data(
        v8::ScriptCompiler::CreateCodeCache(v8::ToApiHandle<v8::UnboundScript>(sfi)));
    uint8_t* buffer = NewArray<uint8_t>(cached_data->length);
    MemCopy(buffer, cached_data->data, cached_data->length);
    *script_data = new i::ScriptData(buffer, cached_data->length);
    (*script_data)->AcquireDataOwnership();
    return sfi;
}


void CompileToCache(
    const std::string &js_filename,
    const std::string &output_filename,
    v8::Isolate*,
    v8::Local<v8::Context> &context)
{
    std::string raw_code = dejsc::IO::read_file(js_filename);
    v8::ScriptCompiler::CachedData* cache;

    {
        v8::Local<v8::String> source_string = StrUtil::v8_str(raw_code.c_str());
        v8::ScriptOrigin script_origin(StrUtil::v8_str(js_filename.c_str()));
        v8::ScriptCompiler::Source source(source_string, script_origin);
        v8::ScriptCompiler::CompileOptions option =
            v8::ScriptCompiler::kNoCompileOptions;
        v8::Local<v8::Script> script =
            v8::ScriptCompiler::Compile(context, &source, option).ToLocalChecked();
        cache = v8::ScriptCompiler::CreateCodeCache(script->GetUnboundScript());
    }

    dejsc::IO::write_file(output_filename, (char *)cache->data, cache->length);
}


int RunBytecodeCache(const std::string &cache_filename, v8::Isolate* isolate, v8::Local<v8::Context>& context)
{
    Cache::CachedCode cache(cache_filename);

    std::string fake_source(' ', cache.GetSourceHash());
    v8::Local<v8::String> source_string = StrUtil::v8_str(fake_source.c_str());
    v8::ScriptOrigin script_origin(StrUtil::v8_str(cache_filename.c_str()));
    v8::ScriptCompiler::Source source_obj(source_string, script_origin, cache.GetCachedData().get());
    v8::ScriptCompiler::CompileOptions option = v8::ScriptCompiler::kConsumeCodeCache;

    v8::Local<v8::Script> script;
    {
      i::DisallowCompilation no_compile(
          reinterpret_cast<i::Isolate*>(isolate));
      script = v8::ScriptCompiler::Compile(context, &source_obj, option)
                   .ToLocalChecked();
    }

    return 0;
}


int RunJavaScriptCode(const std::string &jscode, v8::Isolate* isolate, v8::Local<v8::Context>& context)
{
    // Execute argument given to -e option directly.
    v8::Local<v8::String> file_name =
        v8::String::NewFromUtf8(isolate, "unnamed",
                                v8::NewStringType::kNormal).ToLocalChecked();
    v8::Local<v8::String> source;
    if (!v8::String::NewFromUtf8(isolate, jscode.c_str(),
                                v8::NewStringType::kNormal)
            .ToLocal(&source)) {
        return 1;
    }
    bool success = dejsc::Shell::ExecuteString(isolate, source, file_name, false, true);
    while (v8::platform::PumpMessageLoop(gDefaultPlatform.get(), isolate)) continue;
    if (!success)
        return 1;
    else
        return 0;
}


int RunJavaScriptFile(const std::string &js_filename, v8::Isolate* isolate, v8::Local<v8::Context>& context)
{
    std::string raw_code = dejsc::IO::read_file(js_filename);
    // Use all other arguments as names of files to load and run.
    v8::Local<v8::String> file_name =
        v8::String::NewFromUtf8(isolate, js_filename.c_str(), v8::NewStringType::kNormal)
            .ToLocalChecked();
    v8::Local<v8::String> source;
    if (!dejsc::Shell::ReadFile(
            isolate, js_filename.c_str()).ToLocal(&source)) {
        fprintf(stderr, "Error reading '%s'\n", js_filename.c_str());
    }
    bool success = dejsc::Shell::ExecuteString(isolate, source, file_name, false, true);
    while (v8::platform::PumpMessageLoop(gDefaultPlatform.get(), isolate)) continue;
    if (!success)
        return 1;
    else
        return 0;
}


int RunInV8(
    std::function<int (v8::Isolate*, v8::Local<v8::Context>&) > callback,
    bool use_shell_context /*= false*/)
{
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator =
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);

    int result;
    {
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);

        v8::Local<v8::Context> context;
        if (use_shell_context)
            context = dejsc::Shell::CreateShellContext(isolate);
        else
            context = v8::Context::New(isolate);

        if (context.IsEmpty()) {
            fprintf(stderr, "Error creating context\n");
            return 1;
        }

        v8::Context::Scope context_scope(context);

        // Run given function in the environment that has been set up.
        result = callback(isolate, context);
    }

    // Dispose the isolate
    isolate->Dispose();
    delete create_params.array_buffer_allocator;

    return result;
}

} // namespace Runner
} // namespace dejsc