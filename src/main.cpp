#include <string>
#include <fstream>
#include <cstdint>

#include <CLI11/CLI11.hpp>
#include <v8/include/libplatform/libplatform.h>
#include <v8/include/v8.h>

#include "utils.h"
#include "shell.h"
#include "runner.h"
#include "cache.h"

#define PROGRAM_NAME "dejsc"


int main(int argc, char* argv[]) {
    CLI::App app{PROGRAM_NAME};

    app.require_subcommand(1);

    std::string js_filename;
    CLI::App* compile_subapp = app.add_subcommand("compile", "Compile JavaScript file into v8 bytenode cache file.");
    compile_subapp->add_option("file", js_filename, "JavaScript file.");
    compile_subapp->callback([&]() {
        std::string output_file = dejsc::StrUtil::remove_filename_ext(js_filename) + ".jsc";
        std::cout << "writing cache to: " << output_file << std::endl;
        dejsc::Runner::RunInV8([&](v8::Isolate* isolate, v8::Local<v8::Context> &context) -> int {
            dejsc::Runner::CompileToCache(js_filename, output_file, isolate, context);
            return 0;
        }, false);
    });

    std::string jscode;
    std::string cache_filename;
    CLI::App* run_subapp = app.add_subcommand("run", "Run JavaScript or v8 bytenode cache file.");
    run_subapp->add_option("-f,--file", js_filename, "JavaScript or cache file.");
    run_subapp->add_option("-c,--cache", cache_filename, "Use v8 bytenode cache file.");
    run_subapp->add_option("-e,--execute", jscode, "Execute argument given to -e option directly.");
    run_subapp->callback([&]() {
        dejsc::Runner::RunInV8([&](v8::Isolate* isolate, v8::Local<v8::Context> &context) -> int {
            if (!js_filename.empty()) {
                return dejsc::Runner::RunJavaScriptFile(js_filename, isolate, context);
            } else if (!jscode.empty()) {
                return dejsc::Runner::RunJavaScriptCode(jscode, isolate, context);
            } else if (!cache_filename.empty()) {
                return dejsc::Runner::RunBytecodeCache(cache_filename, isolate, context);
            }
        }, true);
    });

    CLI::App* shell_subapp = app.add_subcommand("shell", "Enter a simple v8 shell.");
    shell_subapp->callback([&]() {
        dejsc::Runner::RunInV8([&](v8::Isolate* isolate, v8::Local<v8::Context> &context) -> int {
            dejsc::Shell::RunShell(context, dejsc::Runner::gDefaultPlatform.get());
            return 0;
        }, true);
    });

    CLI::App* detect_subapp = app.add_subcommand("detect", "Check v8 bytenode cache.");
    detect_subapp->add_option("file", cache_filename, "v8 bytenode cache file.");
    detect_subapp->callback([&]() {
        std::vector<uint8_t> raw = dejsc::IO::read_binary(cache_filename);
        v8::ScriptCompiler::CachedData cache(raw.data(), raw.size());
        dejsc::Cache::CachedCode cached_code(&cache);
        cached_code.PrintHeaderInfo();
    });

    // Initialize V8.
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
    v8::V8::InitializePlatform(dejsc::Runner::gDefaultPlatform.get());
    v8::V8::Initialize();

    CLI11_PARSE(app, argc, argv);

    // Tear down V8.
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();

    return 0;
}
