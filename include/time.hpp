#pragma once

#include <string>
#include <time.h>

using namespace std;

// 获取当前时间，返回 string 类型
string getCurrentTime() {
    time_t now = time(nullptr);
    char timeString[20];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return timeString;
}