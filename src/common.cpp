#include "common.h"

std::string Path2Filename(const std::string &path_) {
    std::size_t pos = path_.rfind("/");
    if (pos < 0 || pos==std::string::npos)
        pos = 0;
    return std::string(path_.substr(pos + 1));
}


int parse_arg(int argc, char* argv[]) {
    //argv[0]肯定是文件调用名

    //argv[1]规定为combined.json，语法树
    if (argc <= 1) {
        std::cerr << "no input file" << std::endl;
        exit(-1);
    }
    extern std::string path_ast;
    path_ast = argv[1];

    return 0;
}