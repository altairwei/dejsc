#include "utils.h"

#include <fstream>
#include <string>
#include <cerrno>

namespace dejsc {

namespace IO {

std::string read_file(const char *filename)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    throw(errno);
}

std::string read_file(const std::string &filename)
{
    return read_file(filename.c_str());
}

void write_file(const std::string &filename, const char *data, const int length)
{
    auto outputfile = std::fstream(filename, std::ios::out | std::ios::binary);
    outputfile.write(data, length);
    outputfile.close();
}

} // IO

namespace StringUtil {

std::string remove_filename_ext(const std::string &filename)
{
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos)
        return filename;
    return filename.substr(0, lastdot); 
}

} // StringUtil

} // dejsc