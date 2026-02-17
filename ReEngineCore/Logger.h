#pragma once
#include <iostream>
#include <iomanip>
#include <cstdarg>
#include <cstdio>
#include <string_view>

enum class LogLevel { Info, Warn, Error, RegisterPass };

// ANSI colors
#define COLOR_RESET     "\033[0m"
#define COLOR_INFO      "\033[32m"
#define COLOR_WARN      "\033[33m"
#define COLOR_ERROR     "\033[31m"
#define COLOR_PASS      "\033[36m"

inline const char* basename(const char* path) {
    const char* file = path;
    for (const char* p = path; *p; p++) {
        if (*p == '\\' || *p == '/') file = p + 1;
    }
    return file;
}

inline void log_printf(LogLevel level, const char* file, int line, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    const char* levelStr = "INFO";
    const char* color = COLOR_INFO;

    switch (level) {
    case LogLevel::Warn:         levelStr = "WARN";       color = COLOR_WARN; break;
    case LogLevel::Error:        levelStr = "ERROR";      color = COLOR_ERROR; break;
    case LogLevel::RegisterPass: levelStr = "REGISTERED"; color = COLOR_PASS; break;
    }

    std::ostream& out = (level == LogLevel::Error) ? std::cerr : std::cout;

    out << "[" << color << std::left << std::setw(10) << levelStr << COLOR_RESET << "] "
        << std::left << std::setw(60) << buffer;

    if (level == LogLevel::Error || level == LogLevel::Warn) {
        out << " | " << COLOR_WARN << basename(file) << ":" << line << COLOR_RESET;
    }

    out << std::endl;
}


#define LOGF_INFO(fmt, ...)          log_printf(LogLevel::Info,         __FILE__, __LINE__, fmt, ##__VA_ARGS__);
#define LOGF_WARN(fmt, ...)          log_printf(LogLevel::Warn,         __FILE__, __LINE__, fmt, ##__VA_ARGS__);
#define LOGF_ERROR(fmt, ...)         log_printf(LogLevel::Error,        __FILE__, __LINE__, fmt, ##__VA_ARGS__);
#define LOGF_REGISTER_PASS(fmt, ...) log_printf(LogLevel::RegisterPass, __FILE__, __LINE__, fmt, ##__VA_ARGS__);