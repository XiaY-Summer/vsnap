#pragma once

#include <string>

using namespace std;

inline string Version = "0.1.0";

// 检查版本号格式是否正确
inline bool checkVersionFormat(const string& version) {
    // 检测版本号的.是否为两个
    if (count(version.begin(), version.end(), '.') != 2) {
        vsnap_log::printWarn("版本号格式不正确！版本号应为x.x.x格式！");
        return false;
    }
    // 检测版本号是否为x.x.x格式
    if (version.find(".") == string::npos) {
        vsnap_log::printWarn("版本号格式不正确！版本号应为x.x.x格式！");
        return false;
    }
    return true;
}

// 检查版本号是否为负数
inline bool checkVersionNegative(const string& version) {
    if (version.find("-") != string::npos) {
        vsnap_log::printWarn("版本号不能为负数！");
        return false;
    }
    return true;
}

// 检查版本号是否为0.0.0
inline bool checkVersionZero(const string& version) {
    if (version == "0.0.0") {
        vsnap_log::printWarn("版本号不能为0.0.0！");
        return false;
    }
    return true;
}

// 检测版本号是否为数字以外字符
inline bool checkVersionNumber(const string& version) {
    if (version.find_first_not_of("0123456789.") != string::npos) {
        vsnap_log::printWarn("版本号不能为数字以外字符！");
        return false;
    }
    return true;
}

// 统一检测版本号
inline bool checkVersion(const string& version) {
    if (!checkVersionFormat(version)) {
        return false;
    }
    if (!checkVersionNegative(version)) {
        return false;
    }
    if (!checkVersionZero(version)) {
        return false;
    }
    if (!checkVersionNumber(version)) {
        return false;
    }
    return true;
}