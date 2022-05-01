#ifndef SRC_UTILS_H
#define SRC_UTILS_H

#include <string>
#include <vector>
#include <cstdint>

#include <v8/include/v8.h>

namespace dejsc {

namespace IO {

std::string read_file(const char *filename);
std::string read_file(const std::string &filename);
std::vector<uint8_t> read_binary(const std::string &filename);

void write_file(const std::string &filename, const char *data, const int length);

} // namespace IO


namespace StrUtil {

std::string remove_filename_ext(const std::string &filename);
v8::Local<v8::String> v8_str(const char* x);

} // namespace StrUtil

} // namespace dejsc

#endif // SRC_UTILS_H