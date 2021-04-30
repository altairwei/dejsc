#include "runner.h"

#include "utils.h"
#include "shell.h"

static inline v8::Local<v8::String> v8_str(const char* x);
static inline v8::Local<v8::String> v8_str(const char* x) {
  return v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), x,
                                 v8::NewStringType::kNormal)
      .ToLocalChecked();
}

namespace dejsc {
namespace Runner {

std::unique_ptr<v8::Platform> gDefaultPlatform = v8::platform::NewDefaultPlatform();

void CompileToCache(
    const std::string &js_filename,
    const std::string &output_filename,
    v8::Isolate*,
    v8::Local<v8::Context> &context)
{
    std::string raw_code = dejsc::IO::read_file(js_filename);
    v8::ScriptCompiler::CachedData* cache;

    {
        v8::Local<v8::String> source_string = v8_str(raw_code.c_str());
        v8::ScriptOrigin script_origin(v8_str(js_filename.c_str()));
        v8::ScriptCompiler::Source source(source_string, script_origin);
        v8::ScriptCompiler::CompileOptions option =
            v8::ScriptCompiler::kNoCompileOptions;
        v8::Local<v8::Script> script =
            v8::ScriptCompiler::Compile(context, &source, option).ToLocalChecked();
        cache = v8::ScriptCompiler::CreateCodeCache(script->GetUnboundScript());
    }

    dejsc::IO::write_file(output_filename, (char *)cache->data, cache->length);
}


void RunBytecodeCache(const std::string &cache_filename)
{
    std::vector<uint8_t> raw = dejsc::IO::read_binary(cache_filename);
    v8::ScriptCompiler::CachedData cache(raw.data(), raw.size());

}


int RunJavaScriptCode(const std::string &jscode, v8::Isolate* isolate)
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


int RunJavaScriptFile(const std::string &js_filename, v8::Isolate* isolate)
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