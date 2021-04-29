#include <string>
#include <fstream>

#include "CLI/CLI.hpp"

#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"

#include "utils.h"

using namespace std;

#define PROGRAM_NAME "dejsc"


static inline v8::Local<v8::String> v8_str(const char* x) {
  return v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), x,
                                 v8::NewStringType::kNormal)
      .ToLocalChecked();
}


void CompileToCache(const std::string &js_filename, const std::string &output_filename)
{
    string source = dejsc::IO::read_file(js_filename);

    v8::ScriptCompiler::CachedData* cache;
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

    v8::Isolate* isolate = v8::Isolate::New(create_params);
    {
        v8::Isolate::Scope iscope(isolate);
        v8::HandleScope scope(isolate);
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope cscope(context);
        v8::Local<v8::String> source_string = v8_str(source.c_str());
        v8::ScriptOrigin script_origin(v8_str(js_filename.c_str()));
        v8::ScriptCompiler::Source source(source_string, script_origin);
        v8::ScriptCompiler::CompileOptions option =
            v8::ScriptCompiler::kNoCompileOptions;
        v8::Local<v8::Script> script =
            v8::ScriptCompiler::Compile(context, &source, option).ToLocalChecked();
        cache = v8::ScriptCompiler::CreateCodeCache(script->GetUnboundScript());
    }

    // Dispose the isolate
    isolate->Dispose();
    delete create_params.array_buffer_allocator;

    dejsc::IO::write_file(output_filename, (char *)cache->data, cache->length);
}


int main(int argc, char* argv[]) {
    CLI::App app{PROGRAM_NAME};

    app.require_subcommand(1);

    string js_filename;
    CLI::App* count_subapp = app.add_subcommand("compile", "Compile javascript file into v8 bytenode cache file.");
    count_subapp->add_option("file", js_filename, "JavaScript file.");
    count_subapp->callback([&]() {
        string output_file = dejsc::StringUtil::remove_filename_ext(js_filename) + ".jsc";
        cout << "writing cache to: " << output_file << endl;
        CompileToCache(js_filename, output_file);
    });

    // Initialize V8.
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    CLI11_PARSE(app, argc, argv);

    // Tear down V8.
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();

    return 0;
}