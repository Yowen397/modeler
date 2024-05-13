#include <unistd.h>
#include <string>
#include <fstream>
#include <regex>

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

    //argv[2]规定为.ini文件
    if (argc <= 2) {
        std::cerr << "no ini file" << std::endl;
        exit(-1);
    }
    extern std::string path_ini;
    path_ini = argv[2];

    return 0;
}

void parse_ini(std::unordered_map<std::string, std::string>& um, const std::string& ini_)
{
    std::ifstream fin(ini_);
    if (!fin) {
        std::cerr << RED << "can't open file [" << ini_ << "]" << RESET << std::endl;
        exit(-1);
    }

    std::string line;
    while (std::getline(fin, line)) {
        std::regex re_jump("\\s*;.*|\\s*#.*|^\\s*");
        if (std::regex_match(line, re_jump))
            continue;

        std::regex re_use(".*=.*");
        if (!std::regex_match(line, re_use)) {
            std::cerr << RED << "parse .ini file error, line :" << std::endl
                      << line << RESET << std::endl;
            exit(-1);
        }
        std::string option = line.substr(0, line.find_first_of("=")),
                    arg = line.substr(line.find_first_of("=") + 1);
        um[option] = arg;
    }

    return;
}

/**
 * replace all clock() function to get better precision in time analysis, clock()
 * offers minimum unit is 10ms. my_clock() is 0.001ms
 * @return
 */
clock_t my_clock() {
    timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    return ts.tv_sec*1000000+ts.tv_nsec/1000;
}

void Timer::outputTime(std::vector<Timer> &v) {
    // for (auto t: v) {
    //     std::cout << t.time/1000.0 << "ms\t, " << t.msg << std::endl;
    // }
    std::cout.flags(std::cout.fixed);
    std::cout.precision(3);
    for (int i = 0; i + 1 < v.size(); i++) {
        std::cout   << (v[i + 1].time - v[i].time) / 1000.0 << "ms  \t\t, "
                  << v[i + 1].msg << std::endl;
    }
    std::cout.unsetf(std::cout.fixed);
}

int VmPeak() {
    int pid = getpid();
    std::string cmd = "cat /proc/" + std::to_string(pid) + "/status | grep VmPeak";
    system(cmd.c_str());
    return 0;
}