#include <unistd.h>
#include <string>

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
    for (int i = 0; i+1 < v.size();i++) {
        std::cout << (v[i + 1].time - v[i].time) / 1000.0 << "ms  \t\t, "
                  << v[i + 1].msg << std::endl;
    }
}

int VmPeak() {
    int pid = getpid();
    std::string cmd = "cat /proc/" + std::to_string(pid) + "/status | grep VmPeak";
    system(cmd.c_str());
    return 0;
}