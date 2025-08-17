#pragma once

#include <iostream>
#include <string>
#include <cstdarg>

void inline log_printf(const char* level, const char* file, int line, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    va_end(args);

    std::cout << "[" << level << "] [" << file << ":" << line << "] " << buffer << std::endl;
}

#define LOGF_INFO(fmt, ...) log_printf("INFO", __FILE__, __LINE__, fmt, __VA_ARGS__);
#define LOGF_WARN(fmt, ...) log_printf("WARN", __FILE__, __LINE__, fmt, __VA_ARGS__);
#define LOGF_ERROR(fmt, ...) log_printf("ERROR", __FILE__, __LINE__, fmt, __VA_ARGS__);
	