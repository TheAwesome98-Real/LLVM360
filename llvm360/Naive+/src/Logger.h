
#include "Shared.h"

enum LogLevel
{
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_DEBUG,
    LOG_FATAL
};

const char* getColor(LogLevel level)
{
    switch (level)
    {
    case LogLevel::LOG_INFO:    return "\033[32m"; // Green
    case LogLevel::LOG_WARNING: return "\033[33m"; // Yellow
    case LogLevel::LOG_ERROR:   return "\033[31m"; // Red
    case LogLevel::LOG_DEBUG:   return "\033[36m"; // Cyan
    case LogLevel::LOG_FATAL:   return "\033[35m"; // Magenta
    default: return "";
    }
}

void LOG_PRINT(LogLevel level, const char* name, const char* fmt, ...)
{
    printf("%s<%s> -> ", getColor(level), name);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\033[0m\n");
}

#define LOG_INFO(name, fmt, ...)    LOG_PRINT(LogLevel::LOG_INFO, name, fmt, __VA_ARGS__)
#define LOG_WARNING(name, fmt, ...) LOG_PRINT(LogLevel::LOG_WARNING, name, fmt, __VA_ARGS__)
#define LOG_ERROR(name, fmt, ...)   LOG_PRINT(LogLevel::LOG_ERROR, name, fmt, __VA_ARGS__)
#define LOG_DEBUG(name, fmt, ...)   LOG_PRINT(LogLevel::LOG_DEBUG, name, fmt, __VA_ARGS__)
#define LOG_FATAL(name, fmt, ...)   LOG_PRINT(LogLevel::LOG_FATAL, name, fmt, __VA_ARGS__)


