#include "voxelio/log.hpp"

#include "voxelio/build.hpp"
#include "voxelio/stringmanip.hpp"

#include <ctime>
#include <iostream>
#include <mutex>

namespace voxelio {

namespace {
namespace ansi {

#define ASCII_ESC "\x1b"

constexpr const char *RESET = ASCII_ESC "[0m";

constexpr const char *FG_16C_BLK = ASCII_ESC "[38;5;0m";
// constexpr const char *FG_16C_RED = ASCII_ESC "[38;5;1m";
constexpr const char *FG_16C_GRN = ASCII_ESC "[38;5;2m";
constexpr const char *FG_16C_ORG = ASCII_ESC "[38;5;3m";
// constexpr const char *FG_16C_BLU = ASCII_ESC "[38;5;4m";
constexpr const char *FG_16C_MAG = ASCII_ESC "[38;5;5m";
// constexpr const char *FG_16C_CYA = ASCII_ESC "[38;5;6m";
constexpr const char *FG_16C_BRI_GRA = ASCII_ESC "[38;5;7m";
// constexpr const char *FG_16C_GRA = ASCII_ESC "[38;5;8m";
constexpr const char *FG_16C_BRI_RED = ASCII_ESC "[38;5;9m";
// constexpr const char *FG_16C_BRI_GRN = ASCII_ESC "[38;5;10m";
constexpr const char *FG_16C_YLW = ASCII_ESC "[38;5;11m";
constexpr const char *FG_16C_BRI_BLU = ASCII_ESC "[38;5;12m";
constexpr const char *FG_16C_BRI_MAG = ASCII_ESC "[38;5;13m";
// constexpr const char *FG_16C_BRI_CYA = ASCII_ESC "[38;5;14m";
// constexpr const char *FG_16C_WHT = ASCII_ESC "[38;5;15m";

#if 0
constexpr const char *FG_BLK = ASCII_ESC "[30m";
constexpr const char *FG_RED = ASCII_ESC "[31m";
constexpr const char *FG_GRN = ASCII_ESC "[32m";
constexpr const char *FG_YLW = ASCII_ESC "[33m";
constexpr const char *FG_BLU = ASCII_ESC "[34m";
constexpr const char *FG_MAG = ASCII_ESC "[35m";
constexpr const char *FG_CYA = ASCII_ESC "[36m";
constexpr const char *FG_WHT = ASCII_ESC "[37m";

constexpr const char *FG_INTENSE_BLK = ASCII_ESC "[1;30m";
constexpr const char *FG_INTENSE_RED = ASCII_ESC "[1;31m";
constexpr const char *FG_INTENSE_GRN = ASCII_ESC "[1;32m";
constexpr const char *FG_INTENSE_YLW = ASCII_ESC "[1;33m";
constexpr const char *FG_INTENSE_BLU = ASCII_ESC "[1;34m";
constexpr const char *FG_INTENSE_MAG = ASCII_ESC "[1;35m";
constexpr const char *FG_INTENSE_CYA = ASCII_ESC "[1;36m";
constexpr const char *FG_INTENSE_WHT = ASCII_ESC "[1;37m";
#endif

}  // namespace ansi

// constexpr const char* ISO8601_DATETIME = "%Y-%m-%d %H:%M:%S";
constexpr const char *ISO8601_TIME = "%H:%M:%S";

std::time_t time()
{
    std::time_t rawTime;
    VXIO_ASSERTM(std::time(&rawTime) != -1, "Failed to get system time");
    return rawTime;
}

std::tm *localtime(std::time_t time)
{
    std::tm *timeInfo = std::localtime(&time);
    VXIO_ASSERT_NOTNULL(timeInfo);
    return timeInfo;
}

std::string currentIso8601Time()
{
    constexpr size_t resultLength = 8;

    std::tm *timeInfo = localtime(time());
    char buffer[resultLength + 1];
    std::strftime(buffer, sizeof(buffer), ISO8601_TIME, timeInfo);

    return {buffer, resultLength};
}

constexpr const char *prefixOf(LogLevel level)
{
    switch (level) {
    case LogLevel::NONE: return ansi::FG_16C_ORG;
    case LogLevel::FAILURE: return ansi::FG_16C_BRI_RED;
    case LogLevel::ERROR: return ansi::FG_16C_BRI_RED;
    case LogLevel::WARNING: return ansi::FG_16C_YLW;
    case LogLevel::IMPORTANT: return ansi::FG_16C_GRN;
    case LogLevel::INFO: return ansi::FG_16C_BRI_BLU;
    case LogLevel::DEBUG: return ansi::FG_16C_BRI_MAG;
    case LogLevel::DETAIL: return ansi::FG_16C_MAG;
    case LogLevel::SPAM: return ansi::FG_16C_BRI_GRA;
    case LogLevel::SUPERSPAM: return ansi::FG_16C_BLK;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

void logToCout(const char *msg)
{
    std::cout << msg;
    std::cout.flush();
};

static LogCallback asyncLogCallback = nullptr;
static std::mutex asyncLogMutex;

void logToAsyncCallback(const char *msg)
{
    std::lock_guard<std::mutex> lock{asyncLogMutex};
    asyncLogCallback(msg);
}

}  // namespace

LogLevel detail::logLevel = LogLevel::INFO;
LogCallback detail::logBackend = &logToCout;
LogFormatter detail::logFormatter = &defaultFormat;
bool detail::isSourceLogging = true;

void defaultFormat(const char *msg, LogLevel level, SourceLocation location)
{
#ifdef VXIO_UNIX
#define VXIO_IF_UNIX(code) code
#define VXIO_IF_WINDOWS(code)
#else
#define VXIO_IF_UNIX(code)
#define VXIO_IF_WINDOWS(code) code
#endif

    std::string output;

    output += "[";
    output += currentIso8601Time();
    output += "] [";
    VXIO_IF_UNIX(output += prefixOf(level));
    output += fixedWidthNameOf(level);
    VXIO_IF_UNIX(output += ansi::RESET);
    output += "] ";

    if (detail::isSourceLogging) {
        VXIO_IF_UNIX(output += ansi::FG_16C_BRI_GRA);
        output += basename(location.file, VXIO_IF_WINDOWS('\\') VXIO_IF_UNIX('/'));
        output += '@';
        output += voxelio::stringify(location.line);
        output += ": ";
        VXIO_IF_UNIX(output += ansi::RESET);
    }

    output += msg;
    output += '\n';

    logRaw(output);
}

void setLogBackend(LogCallback callback, bool async)
{
    if (callback == nullptr) {
        callback = &logToCout;
    }
    if (not async) {
        detail::logBackend = callback;
    }
    else {
        asyncLogCallback = callback;
        detail::logBackend = &logToAsyncCallback;
    }
}

void setLogFormatter(LogFormatter callback)
{
    detail::logFormatter = callback == nullptr ? &defaultFormat : callback;
}

}  // namespace voxelio
