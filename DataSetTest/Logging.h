#pragma once

enum class LogLevel
{
    Fatal   = 0,
    Error   = 1,
    Warning = 2,
    Debug   = 3,
    Info    = 4,
    Verbose = 5,
    MaxLogLevels
};

void SetLogLevel(LogLevel const level);
void LogToConsole(bool const log_to_console);
void __cdecl LogMessage(LogLevel const level, char const *format, ...);

#define LOG(level, format, ...) LogMessage(LogLevel::##level, format, __VA_ARGS__);
#define LOGF(format, ...) LOG(Fatal, format, __VA_ARGS__);
#define LOGE(format, ...) LOG(Error, format, __VA_ARGS__);
#define LOGW(format, ...) LOG(Warning, format, __VA_ARGS__);
#define LOGD(format, ...) LOG(Debug, format, __VA_ARGS__);
#define LOGI(format, ...) LOG(Info, format, __VA_ARGS__);
#define LOGV(format, ...) LOG(Verbose, format, __VA_ARGS__);
