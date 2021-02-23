#ifndef VXIO_LOG_HPP
#define VXIO_LOG_HPP
/*
 * log.hpp
 * -----------
 * Provides basic logging functionality.
 * The current log level can be set thread-locally using voxelio::logLevel
 */

#include "assert.hpp"
#include "build.hpp"

#include VXIO_FWDHEADER(string)

namespace voxelio {

enum class LogLevel : unsigned {
    /** No messages get logged at this level. Use this as a log level to completely disable logging. */
    NONE,
    /** Signals an imminent program failure. (failed assertion, thread crash, hardware errors) */
    FAILURE,
    /** Signals that an error has occured but the program can continue (failed HTTP connection, I/O eror, etc.) */
    ERROR,
    /** Warns the user about unusual behavior. (could not clear some data, had to retry some operation, etc.) */
    WARNING,
    /** Default level on release builds. Used for general messages. */
    INFO,
    /** Default level on debug builds. Used for messages that are only relevant to the developer. */
    DEBUG,
    /** Like debug, but meant for messages which are irrelevant most of the time. */
    DETAIL,
    /** Logging at this level does not even compile on release builds. Used for otherwise irrelevant messages when
       bug-fixing. */
    SPAM,
    /** Used for extremely verbose logging of tiny operations. May cause huge slowdown during normal operations and
 should not normally be enabled, even on debug builds.*/
    SUPERSPAM,
};

constexpr const char *nameOf(LogLevel level)
{
    switch (level) {
    case LogLevel::NONE: return "NONE";
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
    case LogLevel::NONE: return "NONE";
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

constexpr bool operator<(LogLevel x, LogLevel y)
{
    return static_cast<unsigned>(x) < static_cast<unsigned>(y);
}

constexpr bool operator>=(LogLevel x, LogLevel y)
{
    return static_cast<unsigned>(x) >= static_cast<unsigned>(y);
}

constexpr bool operator>(LogLevel x, LogLevel y)
{
    return static_cast<unsigned>(x) > static_cast<unsigned>(y);
}

struct SourceLocation {
    const char *file;
    const char *function;
    usize line;
};

// CONFIGURATION =======================================================================================================

/// The function pointer type of the logging callback.
using LogCallback = void (*)(const char *msg);
/// The function pointer type of the logging formatter.
using LogFormatter = void (*)(const char *msg, LogLevel level, SourceLocation location);

namespace detail {

extern LogLevel logLevel;
extern LogCallback logBackend;
extern LogFormatter logFormatter;

}  // namespace detail

/**
 * @brief Sets the logging backend callback for voxelio.
 * By default, voxelio logs to std::cout.
 * This behavior can be changed using this function.
 *
 * If the async flag is set, voxelio will automatically wrap the callback in a mutex-guarded logging function.
 * This way the callback will be thread-safe, even if it doesn't contain any locking code itself.
 *
 * @param callback the callback or nullptr if the behavior should be reset to default
 * @param async true if voxelio should automatically ensure thread-safety using a mutex
 */
void setLogBackend(LogCallback callback, bool async = false);

/**
 * @brief Sets the logging formatter to the given function or resets it to default if the pointer is nullptr.
 * The formatter should format messages and metadata and then pass it on to the backend using logRaw().
 * @param formatter the formatter
 */
void setLogFormatter(LogFormatter formatter);

inline LogLevel getLogLevel()
{
    return detail::logLevel;
}

inline void setLogLevel(LogLevel level)
{
    detail::logLevel = level;
}

inline bool isLoggable(LogLevel level)
{
    return level <= detail::logLevel;
}

// LOGGING FUNCTIONS ===================================================================================================

/// Directly invokes the logging callback.
inline void logRaw(const char *msg)
{
    detail::logBackend(msg);
}

/// Directly invokes the logging callback.
inline void logRaw(const std::string &msg)
{
    detail::logBackend(msg.c_str());
}

inline void log(const char *msg, LogLevel level, SourceLocation location)
{
    detail::logFormatter(msg, level, location);
}

inline void log(const std::string &msg, LogLevel level, SourceLocation location)
{
    detail::logFormatter(msg.c_str(), level, location);
}

/// The default format function. Invoking this bypasses the logFormatter and goes straight to the backend.
void defaultFormat(const char *msg, LogLevel level, SourceLocation location);

#ifdef VXIO_DEBUG
#define VXIO_LOG_IMPL(level, msg, file, function, line)                                  \
    if (::voxelio::isLoggable(::voxelio::LogLevel::level)) {                             \
        ::voxelio::log((msg), ::voxelio::LogLevel::level, {(file), (function), (line)}); \
    }
#else
#define VXIO_LOG_IMPL(level, msg, file, function, line)                                      \
    if constexpr (::voxelio::LogLevel::level < ::voxelio::LogLevel::SPAM) {                  \
        if (::voxelio::isLoggable(::voxelio::LogLevel::level)) {                             \
            ::voxelio::log((msg), ::voxelio::LogLevel::level, {(file), (function), (line)}); \
        }                                                                                    \
    }
#endif

#define VXIO_LOG(level, msg) VXIO_LOG_IMPL(level, msg, __FILE__, __func__, __LINE__)

}  // namespace voxelio

#endif  // LOG_HPP
