#pragma once

#include <iostream>
#include <string>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace vsnap_log {

// ANSI 转义序列
namespace ansi {
    constexpr const char* reset   = "\033[0m";
    constexpr const char* bold    = "\033[1m";
    constexpr const char* dim     = "\033[2m";
    constexpr const char* red     = "\033[31m";
    constexpr const char* green   = "\033[32m";
    constexpr const char* yellow  = "\033[33m";
    constexpr const char* blue    = "\033[34m";
    constexpr const char* cyan    = "\033[36m";
    constexpr const char* magenta = "\033[35m";
}

enum LogLevel {
    DEFAULT = 0,
    DETAIL = 1,
};
// 日志级别 0: 默认 1: 详细
int logLevel = 0;
inline bool colorEnabled = true;

// 检测是否为 TTY（终端），管道重定向时禁用颜色
inline bool isTty() {
#ifdef _WIN32
    return _isatty(_fileno(stdout));
#else
    return isatty(STDOUT_FILENO);
#endif
}

inline std::string color(const char* code, bool enable = true) {
    return (enable && colorEnabled && isTty()) ? code : "";
}

inline std::string reset(bool enable = true) {
    return (enable && colorEnabled && isTty()) ? ansi::reset : "";
}

inline void setColorEnabled(bool enabled) {
    colorEnabled = enabled;
}

inline std::ostream& success(std::ostream& out = std::cout) {
    out << color(ansi::green) << "✓ " << reset();
    return out;
}

inline std::ostream& error(std::ostream& out = std::cout) {
    out << color(ansi::red) << "✗ " << reset();
    return out;
}

inline std::ostream& warn(std::ostream& out = std::cout) {
    out << color(ansi::yellow) << "⚠ " << reset();
    return out;
}

inline std::ostream& info(std::ostream& out = std::cout) {
    out << color(ansi::cyan) << "ℹ " << reset();
    return out;
}

inline std::ostream& bullet(std::ostream& out = std::cout) {
    out << color(ansi::dim) << "  • " << reset();
    return out;
}

inline std::ostream& dim(std::ostream& out = std::cout) {
    out << color(ansi::dim);
    return out;
}

inline std::ostream& bold(std::ostream& out = std::cout) {
    out << color(ansi::bold);
    return out;
}

// 带样式的消息输出
inline void printSuccess(const std::string& msg) {
    success() << msg << std::endl;
}

inline void printError(const std::string& msg) {
    error() << msg << std::endl;
}

inline void printWarn(const std::string& msg) {
    if (logLevel >= 1) {
        warn() << msg << std::endl;
    }
    else {
        return;
    }
}

inline void printInfo(const std::string& msg) {
    info() << msg << std::endl;
}

} // namespace vsnap_log
