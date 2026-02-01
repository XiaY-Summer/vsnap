#pragma once

#include <string>
#include <time.h>

// 获取当前时间，返回 string 类型
inline std::string getCurrentTime() {
    time_t now = time(nullptr);
    char timeString[20];
#ifdef _WIN32
    tm localTime;
    localtime_s(&localTime, &now);
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &localTime);
#else
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", localtime(&now));
#endif
    return timeString;
}