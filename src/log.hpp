#ifndef VXIO_LOG_HPP
#define VXIO_LOG_HPP

#include "assert.hpp"
#include "build.hpp"

#include VXIO_FWDHEADER(string)

namespace voxelio {

enum class LogLevel : unsigned {
    FAILURE = 0,
    /** Signals that an error has occured but the program can continue (failed HTTP connection, I/O eror, etc.) */
    ERROR = 1,
    /** Warns the user about unusual behavior. (could not clear some data, had to retry some operation, etc.) */
    WARNING = 2,
    /** Default level on release builds. Used for general messages. */
    INFO = 3,
    /** Default level on debug builds. Used for messages that are only relevant to the developer. */
    DEBUG = 4,
    /** Like debug, but meant for messages which are irrelevant most of the time. */
    DETAIL = 5,
    /** Logging at this level does not even compile on release builds. Used for otherwise irrelevant messages when
       bug-fixing. */
    SPAM = 6,
    /** Used for extremely verbose logging of tiny operations. May cause huge slowdown during normal operations and
 should not normally be enabled, even on debug builds.*/
    SUPERSPAM = 7,
};

constexpr const char *nameOf(LogLevel level)
{
    switch (level) {
    case LogLevel::FAILURE: return "FAILURE";
    case LogLevel::ERROR: return "ERROR";
    case LogLevel::WARNING: return "WARNING";
    case LogLevel::INFO: return "INFO";
    case LogLevel::DEBUG: return "DEBUG";
    case LogLevel::DETAIL: return "DETAIL";
    case LogLevel::SPAM: return "SPAM";
    case LogLevel::SUPERSPAM: return "SUPERSPAM";
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

constexpr const char *fixedWidthNameOf(LogLevel level)
{
    switch (level) {
    case LogLevel::FAILURE: return "FAIL";
    case LogLevel::ERROR: return "EROR";
    case LogLevel::WARNING: return "WARN";
    case LogLevel::INFO: return "INFO";
    case LogLevel::DEBUG: return "DBUG";
    case LogLevel::DETAIL: return "DTAL";
    case LogLevel::SPAM: return "SPAM";
    case LogLevel::SUPERSPAM: return "SSPM";
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

constexpr bool operator<=(LogLevel x, LogLevel y)
{
    return static_cast<unsigned>(x) <= static_cast<unsigned>(y);
}

void logRaw(const char *msg);
void logRaw(const std::string &msg);

void log(const char *msg, LogLevel level, const char *file, const char *function, size_t line);
void log(const std::string &msg, LogLevel level, const char *file, const char *function, size_t line);

extern thread_local LogLevel logLevel;

#ifdef VXIO_DEBUG
#define VXIO_LOG_IMPL(level, msg, file, function, line)                                \
    if (::voxelio::LogLevel::level <= ::voxelio::logLevel) {                           \
        ::voxelio::log((msg), ::voxelio::LogLevel::level, (file), (function), (line)); \
    }
#else
#define VXIO_LOG_IMPL(level, msg, file, function, line)                                    \
    if constexpr (::voxelio::LogLevel::level < ::voxelio::LogLevel::SPAM) {                \
        if (::voxelio::LogLevel::level <= ::voxelio::logLevel) {                           \
            ::voxelio::log((msg), ::voxelio::LogLevel::level, (file), (function), (line)); \
        }                                                                                  \
    }
#endif

#define VXIO_LOG(level, msg) VXIO_LOG_IMPL(level, msg, __FILE__, __func__, __LINE__)

}  // namespace voxelio

#endif  // LOG_HPP
