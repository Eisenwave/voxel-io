#ifndef LOG_HPP
#define LOG_HPP

#include "assert.hpp"
#include "build.hpp"

#include <string>

namespace voxelio {

enum class LogLevel : unsigned {
    /** Signals that an error has occured but the program can continue (failed HTTP connection, I/O eror, etc.) */
    ERROR = 1,
    /** Warns the user about unusual behavior. (could not clear some data, had to retry some operation, etc.) */
    WARNING = 2,
    /** Default level on release builds. Used for general messages. */
    INFO = 3,
    /** Default level on debug builds. Used for messages that are only relevant to the developer. */
    DEBUG = 4,
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
    case LogLevel::ERROR: return "ERROR";
    case LogLevel::WARNING: return "WARNING";
    case LogLevel::INFO: return "INFO";
    case LogLevel::DEBUG: return "DEBUG";
    case LogLevel::SPAM: return "SPAM";
    case LogLevel::SUPERSPAM: return "SUPERSPAM";
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

constexpr const char *fixedWidthNameOf(LogLevel level)
{
    switch (level) {
    case LogLevel::ERROR: return "EROR";
    case LogLevel::WARNING: return "WARN";
    case LogLevel::INFO: return "INFO";
    case LogLevel::DEBUG: return "DBUG";
    case LogLevel::SPAM: return "SPAM";
    case LogLevel::SUPERSPAM: return "SSPM";
    }
    VXIO_DEBUG_ASSERT_UNREACHABLE();
}

void log(LogLevel level, std::string msg, const char *file, const char *function, size_t line);

extern thread_local LogLevel logLevel;

#define VXIO_LOG(level, msg)                                                                                   \
    if constexpr (::voxelio::build::DEBUG || ::voxelio::LogLevel::level < ::voxelio::LogLevel::SPAM) {         \
        if (static_cast<unsigned>(::voxelio::LogLevel::level) <= static_cast<unsigned>(::voxelio::logLevel)) { \
            ::voxelio::log(::voxelio::LogLevel::level, msg, __FILE__, __func__, __LINE__);                     \
        }                                                                                                      \
    }

}  // namespace voxelio

#endif  // LOG_HPP
