#include "voxelio/log.hpp"

#include "voxelio/stringmanip.hpp"

#include <ctime>
#include <iostream>

namespace voxelio {

namespace ansi {

#define ASCII_ESC "\x1b"

constexpr const char *RESET = ASCII_ESC "[0m";

constexpr const char *FG_16C_BLK = ASCII_ESC "[38;5;0m";
// constexpr const char *FG_16C_RED = ASCII_ESC "[38;5;1m";
// constexpr const char *FG_16C_GRN = ASCII_ESC "[38;5;2m";
// constexpr const char *FG_16C_ORG = ASCII_ESC "[38;5;3m";
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

// constexpr const char *FG_BLK = ASCII_ESC "[30m";
// constexpr const char *FG_RED = ASCII_ESC "[31m";
// constexpr const char *FG_GRN = ASCII_ESC "[32m";
// constexpr const char *FG_YLW = ASCII_ESC "[33m";
// constexpr const char *FG_BLU = ASCII_ESC "[34m";
// constexpr const char *FG_MAG = ASCII_ESC "[35m";
// constexpr const char *FG_CYA = ASCII_ESC "[36m";
// constexpr const char *FG_WHT = ASCII_ESC "[37m";

// constexpr const char *FG_INTENSE_BLK = ASCII_ESC "[1;30m";
// constexpr const char *FG_INTENSE_RED = ASCII_ESC "[1;31m";
// constexpr const char *FG_INTENSE_GRN = ASCII_ESC "[1;32m";
// constexpr const char *FG_INTENSE_YLW = ASCII_ESC "[1;33m";
// constexpr const char *FG_INTENSE_BLU = ASCII_ESC "[1;34m";
// constexpr const char *FG_INTENSE_MAG = ASCII_ESC "[1;35m";
// constexpr const char *FG_INTENSE_CYA = ASCII_ESC "[1;36m";
// constexpr const char *FG_INTENSE_WHT = ASCII_ESC "[1;37m";

}  // namespace ansi

// constexpr const char* ISO8601_DATETIME = "%Y-%m-%d %H:%M:%S";
constexpr const char *ISO8601_TIME = "%H:%M:%S";

static std::time_t time()
{
    std::time_t rawTime;
    VXIO_ASSERTM(std::time(&rawTime) != -1, "Failed to get system time");
    return rawTime;
}

static std::tm *localtime(std::time_t time)
{
    std::tm *timeInfo = std::localtime(&time);
    VXIO_ASSERT_NOTNULL(timeInfo);
    return timeInfo;
}

static std::string currentIso8601Time()
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
    case LogLevel::FAILURE:
    case LogLevel::ERROR: return ansi::FG_16C_BRI_RED;
    case LogLevel::WARNING: return ansi::FG_16C_YLW;
    case LogLevel::INFO: return ansi::FG_16C_BRI_BLU;
    case LogLevel::DEBUG: return ansi::FG_16C_BRI_MAG;
    case LogLevel::DETAIL: return ansi::FG_16C_MAG;
    case LogLevel::SPAM: return ansi::FG_16C_BRI_GRA;
    case LogLevel::SUPERSPAM: return ansi::FG_16C_BLK;
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

void logRaw(const char *msg)
{
    std::cout << msg;
    std::cout.flush();
}

void logRaw(const std::string &msg)
{
    std::cout << msg;
    std::cout.flush();
}

void log(const std::string &msg, LogLevel level, const char *file, const char *, size_t line)
{
    std::string output;
    output.reserve(msg.size() + 64);

    output += "[";
    output += currentIso8601Time();
    output += "] [";
#ifdef __unix__
    output += prefixOf(level);
    output += fixedWidthNameOf(level);
    output += ansi::RESET;
#else
    std::cout << fixedWidthNameOf(level);
#endif
    output += "] ";
#ifdef __unix__
    output += ansi::FG_16C_BRI_GRA;
    output += basename(file);
    output += '@';
    output += voxelio::stringify(line);
    output += ": ";
    output += ansi::RESET;
#else
    std::cout << basename(file) << '@' << line << ": ";
#endif
    output += msg;
    output += '\n';

    logRaw(output);
}

void log(const char *msg, LogLevel level, const char *file, const char *function, size_t line)
{
    log(std::string{msg}, level, file, function, line);
}

thread_local LogLevel logLevel = LogLevel::INFO;

}  // namespace voxelio
