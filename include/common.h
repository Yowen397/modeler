#pragma once

#include <string>
#include <iostream>
#include <vector>
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