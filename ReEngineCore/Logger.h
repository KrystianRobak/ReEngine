#pragma once

#include <iostream>
#include <iomanip>
#include <cstdarg>
#include <cstdio>

// ANSI colors
#define COLOR_RESET   "\033[0m"
#define COLOR_INFO    "\033[32m"  // green
#define COLOR_WARN    "\033[33m"  // yellow
#define COLOR_ERROR   "\033[31m"  // red

// Fixed widths
constexpr int LEVEL_WIDTH = 8;
constexpr int MSG_WIDTH = 80;
constexpr int FILE_WIDTH = 25;   // now aligns based on just filename

inline const char* basename(const char* path) {
    const char* file = path;
    for (const char* p = path; *p; p++) {
        if (*p == '\\' || *p == '/')
            file = p + 1;
    }
    return file;
}

inline void log_printf(const char* level, const char* color, const char* file, int line, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    va_end(args);

    std::cout
        << "[" << color << std::left << std::setw(LEVEL_WIDTH) << level << COLOR_RESET << "] "
        << std::left << std::setw(MSG_WIDTH) << buffer << " | "
        << " FILE: " << std::left << std::setw(FILE_WIDTH) << basename(file) << " | "
        << " LINE: " << line
        << std::endl;
}

// Macros
#define LOGF_INFO(fmt, ...)  log_printf("INFO",  COLOR_INFO,  __FILE__, __LINE__, fmt, ##__VA_ARGS__);
#define LOGF_WARN(fmt, ...)  log_printf("WARN",  COLOR_WARN,  __FILE__, __LINE__, fmt, ##__VA_ARGS__);
#define LOGF_ERROR(fmt, ...) log_printf("ERROR", COLOR_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__);
