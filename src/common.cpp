#include "common.h"

std::string Path2Filename(const std::string &path_) {
    std::size_t pos = path_.rfind("/");

    return std::string(path_.substr(pos)); 
}