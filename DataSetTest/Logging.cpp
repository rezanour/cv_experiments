#include "Precomp.h"
#include "Logging.h"

// Set default log level based on debug/release build
#ifdef _DEBUG
static LogLevel s_log_level = LogLevel::Debug;
#else
static LogLevel s_log_level = LogLevel::Warning;
#endif

static bool s_log_to_console = false;

static char const *LevelLabel(LogLevel const level)
{
    switch (level)
    {
    case LogLevel::Fatal:   return "FTL";
    case LogLevel::Error:   return "ERR";
    case LogLevel::Warning: return "WRN";
    case LogLevel::Debug:   return "DBG";
    case LogLevel::Info:    return "INF";
    case LogLevel::Verbose: return "VRB";
    default:                return "UNK";
    }
}

void SetLogLevel(LogLevel const level)
{
    s_log_level = level;
}

void LogToConsole(bool const log_to_console)
{
    s_log_to_console = log_to_console;
}

void __cdecl LogMessage(LogLevel const level, char const *format, ...)
{
    if (static_cast<int32_t>(level) > static_cast<int32_t>(s_log_level))
    {
        // Filtered out
        return;
    }

    SYSTEMTIME local_time{};
    GetLocalTime(&local_time);

    char message[2048]{};

    // Fill in standard prefix
    sprintf_s(message, "%04u-%02u-%02u %02u:%02u:%02u.%03u  [%s]: ",
        local_time.wYear, local_time.wMonth, local_time.wDay,
        local_time.wHour, local_time.wMinute, local_time.wSecond,
        local_time.wMilliseconds, LevelLabel(level));

    // Concatenate on the provided message
    va_list list;
    va_start(list, format);
    vsprintf_s(message + strlen(message), sizeof(message) - strlen(message), format, list);
    va_end(list);

    strcat_s(message, "\n");
    OutputDebugStringA(message);

    if (s_log_to_console)
    {
        printf(message);
    }

    if (LogLevel::Fatal == level)
    {
        if (IsDebuggerPresent())
        {
            abort();
        }
        else
        {
            exit(-1);
        }
    }
}
