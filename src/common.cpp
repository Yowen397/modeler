#include "common.h"

std::string Path2Filename(const std::string &path_) {
    std::size_t pos = path_.rfind("/");
    if (pos < 0 || pos==std::string::npos)
        pos = 0;
    return std::string(path_.substr(pos + 1));
}