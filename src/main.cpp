#include <string>
#include <fstream>
#include <cstdint>

#include "CLI/CLI.hpp"

#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"

#include "utils.h"
#include "shell.h"
#include "runner.h"

#define PROGRAM_NAME "dejsc"


int main(int argc, char* argv[]) {
    CLI::App app{PROGRAM_NAME};

    app.require_subcommand(1);

    std::string js_filename;
    CLI::App* compile_subapp = app.add_subcommand("compile", "Compile JavaScript file into v8 bytenode cache file.");
    compile_subapp->add_option("file", js_filename, "JavaScript file.");
    compile_subapp->callback([&]() {
        std::string output_file = dejsc::StringUtil::remove_filename_ext(js_filename) + ".jsc";
        std::cout << "writing cache to: " << output_file << std::endl;
        dejsc::Runner::RunInV8([&](v8::Isolate* isolate, v8::Local<v8::Context> &context) -> int {
            dejsc::Runner::CompileToCache(js_filename, output_file, isolate, context);
            return 0;
        }, false);
    });

    std::string js_cache_filename;
    std::string jscode;
    bool use_cache;
    CLI::App* run_subapp = app.add_subcommand("run", "Run JavaScript or v8 bytenode cache file.");
    run_subapp->add_option("-f,--file", js_cache_filename, "JavaScript or cache file.");
    run_subapp->add_option("-c,--cache", js_cache_filename, "Use v8 bytenode cache file.");
    run_subapp->add_option("-e,--execute", jscode, "Execute argument given to -e option directly.");
    run_subapp->callback([&]() {
        dejsc::Runner::RunInV8([&](v8::Isolate* isolate, v8::Local<v8::Context> &context) -> int {
            if (!js_cache_filename.empty()) {
                return dejsc::Runner::RunJavaScriptFile(js_cache_filename, isolate);
            } else if (!jscode.empty()) {
                return dejsc::Runner::RunJavaScriptCode(jscode, isolate);
            } else {
                return 0;
            }
        }, true);
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
