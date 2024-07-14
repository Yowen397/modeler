#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <time.h>

std::string Path2Filename(const std::string &path_);

int parse_arg(int, char **);

clock_t my_clock();

class Timer {
  public:
    std::string msg;
    clock_t time;

    Timer(const std::string &msg_) {
        time = my_clock();
        msg = msg_;
    }

    static void outputTime(std::vector<Timer> &v);
};

int VmPeak();

// 定义ANSI escape codes用于终端颜色输出
constexpr auto RED = "\033[31m";      // 红色
constexpr auto GREEN = "\033[32m";    // 绿色
constexpr auto YELLOW = "\033[33m";   // 黄色
constexpr auto BLUE = "\033[34m";     // 蓝色
constexpr auto RESET = "\033[0m";     // 重置颜色

void parse_ini(std::unordered_map<std::string, std::string>& um, const std::string& ini_);

/**
 * 读取整个文件的内容，返回一个std::string
 * @param filePath 文件路径
*/
std::string readFileFromPath(const std::string& filePath);

/**
 * 获取当前使用的内存大小，单位：kB
*/
std::size_t VmSize();
