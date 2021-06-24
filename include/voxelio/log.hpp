#ifndef VXIO_LOG_HPP
#define VXIO_LOG_HPP
/*
 * log.hpp
 * -----------
 * Provides basic logging functionality.
 * The current log level can be set thread-locally using voxelio::logLevel
 */

#include "build.hpp"
#include "builtin.hpp"

#include <string>

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
    /** Important messages, more relevant than regular info messages. */
    IMPORTANT,
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

constexpr const char *nameOf(LogLevel level) noexcept
{
    switch (level) {
    case LogLevel::NONE: return "NONE";
    case LogLevel::FAILURE: return "FAILURE";
    case LogLevel::ERROR: return "ERROR";
    case LogLevel::WARNING: return "WARNING";
    case LogLevel::IMPORTANT: return "IMPORTANT";
    case LogLevel::INFO: return "INFO";
    case LogLevel::DEBUG: return "DEBUG";
    case LogLevel::DETAIL: return "DETAIL";
    case LogLevel::SPAM: return "SPAM";
    case LogLevel::SUPERSPAM: return "SUPERSPAM";
    }
    VXIO_UNREACHABLE();
}

constexpr const char *fixedWidthNameOf(LogLevel level) noexcept
{
    switch (level) {
    case LogLevel::NONE: return "NONE";
    case LogLevel::FAILURE: return "FAIL";
    case LogLevel::ERROR: return "EROR";
    case LogLevel::WARNING: return "WARN";
    case LogLevel::IMPORTANT: return "IMPO";
    case LogLevel::INFO: return "INFO";
    case LogLevel::DEBUG: return "DBUG";
    case LogLevel::DETAIL: return "DTAL";
    case LogLevel::SPAM: return "SPAM";
    case LogLevel::SUPERSPAM: return "SSPM";
    }
    VXIO_UNREACHABLE();
}

constexpr bool operator<=(LogLevel x, LogLevel y) noexcept
{
    return static_cast<unsigned>(x) <= static_cast<unsigned>(y);
}

constexpr bool operator<(LogLevel x, LogLevel y) noexcept
{
    return static_cast<unsigned>(x) < static_cast<unsigned>(y);
}

constexpr bool operator>=(LogLevel x, LogLevel y) noexcept
{
    return static_cast<unsigned>(x) >= static_cast<unsigned>(y);
}

constexpr bool operator>(LogLevel x, LogLevel y) noexcept
{
    return static_cast<unsigned>(x) > static_cast<unsigned>(y);
}

// CONFIGURATION =======================================================================================================

/// The function pointer type of the logging callback.
using LogCallback = void (*)(const char *msg);
/// The function pointer type of the logging formatter.
using LogFormatter = void (*)(const char *msg, LogLevel level, SourceLocation location);

namespace detail {

extern LogLevel logLevel;
extern LogCallback logBackend;
extern LogFormatter logFormatter;

extern bool isTimestampLogging;
extern bool isLevelLogging;
extern bool isSourceLogging;

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
void setLogBackend(LogCallback callback, bool async = false) noexcept;

/**
 * @brief Sets the logging formatter to the given function or resets it to default if the pointer is nullptr.
 * The formatter should format messages and metadata and then pass it on to the backend using logRaw().
 * @param formatter the formatter
 */
void setLogFormatter(LogFormatter formatter) noexcept;

inline LogLevel getLogLevel() noexcept
{
    return detail::logLevel;
}

inline void setLogLevel(LogLevel level) noexcept
{
    detail::logLevel = level;
}

inline bool isLoggable(LogLevel level) noexcept
{
    return level <= detail::logLevel;
}

inline void enableLoggingTimestamp(bool enable) noexcept
{
    detail::isTimestampLogging = enable;
}

inline void enableLoggingLevel(bool enable) noexcept
{
    detail::isLevelLogging = enable;
}

inline void enableLoggingSourceLocation(bool enable) noexcept
{
    detail::isSourceLogging = enable;
}

// LOGGING FUNCTIONS ===================================================================================================

/// Directly invokes the logging callback.
inline void logRaw(const char *msg) noexcept
{
    detail::logBackend(msg);
}

/// Directly invokes the logging callback.
inline void logRaw(const std::string &msg) noexcept
{
    detail::logBackend(msg.c_str());
}

inline void log(const char *msg, LogLevel level, SourceLocation location) noexcept
{
    detail::logFormatter(msg, level, location);
}

inline void log(const std::string &msg, LogLevel level, SourceLocation location) noexcept
{
    detail::logFormatter(msg.c_str(), level, location);
}

/// The default format function. Invoking this bypasses the logFormatter and goes straight to the backend.
void defaultFormat(const char *msg, LogLevel level, SourceLocation location) noexcept;

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
