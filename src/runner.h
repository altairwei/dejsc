#ifndef SRC_RUNNER_H
#define SRC_RUNNER_H

#include <functional>

#include "v8/include/libplatform/libplatform.h"
#include "v8/include/v8.h"

namespace dejsc {
namespace Runner {

void CompileToCache(
    const std::string &js_filename,
    const std::string &output_filename,
    v8::Isolate*,
    v8::Platform*,
    v8::Local<v8::Context> &context
);

void RunBytecodeCache(const std::string &cache_filename);
int RunJavaScriptFile(const std::string &js_filename);

int RunInV8(
    std::function<int (v8::Isolate*, v8::Platform*, v8::Local<v8::Context>&)> callback,
    bool use_shell_context = false
);
    
} // namespace Runner
} // namespace dejsc

#endif // SRC_RUNNER_H