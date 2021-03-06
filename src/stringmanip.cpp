#include "voxelio/stringmanip.hpp"

#include <algorithm>
#include <cstdarg>
#include <iomanip>
#include <sstream>
#include <cctype>

namespace voxelio {

static_assert (std::is_same_v<std::string::size_type, size_t>);

static void transformEachChar(std::string &str, int(*transform)(int))
{
    std::transform(str.begin(), str.end(), str.begin(), transform);
}

void toUpperCase(std::string &str)
{
    transformEachChar(str, std::toupper);
}

void toLowerCase(std::string &str)
{
    transformEachChar(str, std::tolower);
}

static constexpr const char *SPACE_CHARS = " \f\t\n\r";

void ltrim(std::string &s)
{
    usize i = s.find_first_not_of(SPACE_CHARS);
    s.erase(0, i);
}

void rtrim(std::string &s)
{
    usize i = s.find_last_not_of(SPACE_CHARS);
    s.erase(i + 1);
}

void trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}

void replaceChar(std::string &s, char old, char nue)
{
    std::replace(s.begin(), s.end(), old, nue);
}

std::string lpad(const std::string &str, size_t length, char c)
{
    std::stringstream stream;
    stream << std::setw(static_cast<int>(length)) << std::setfill(c) << str;
    return stream.str();
}

std::string rpad(const std::string &str, size_t length, char c)
{
    std::string result = str;
    if (str.size() < length) {
        result.resize(length, c);
    }
    return result;
}

std::string substrBeforeFirst(const std::string &str, char delimiter)
{
    const auto pos = str.find_first_of(delimiter);
    return pos == std::string::npos ? str : str.substr(0, pos);
}

std::string substrBeforeLast(const std::string &str, char delimiter)
{
    const auto pos = str.find_last_of(delimiter);
    return pos == std::string::npos ? str : str.substr(0, pos);
}

std::string substrAfterFirst(const std::string &str, char delimiter)
{
    const auto pos = str.find_first_of(delimiter);
    return pos == std::string::npos ? str : str.substr(pos + 1, str.size());
}

std::string substrAfterLast(const std::string &str, char delimiter)
{
    const auto pos = str.find_last_of(delimiter);
    return pos == std::string::npos ? str : str.substr(pos + 1, str.size());
}

std::string dir(const std::string &str, char delimiter)
{
    return substrBeforeLast(str, delimiter);
}

std::string basename(const std::string &str, char delimiter)
{
    return substrAfterLast(str, delimiter);
}

std::string noext(const std::string &str, char delimiter)
{
    return substrBeforeLast(str, delimiter);
}

std::string ext(const std::string &str, char delimiter)
{
    return substrAfterLast(str, delimiter);
}

std::string basenameNoext(const std::string &str)
{
    return noext(basename(str));
}

std::string vformat(const char *fmt, va_list ap)
{
    constexpr size_t initialSize = 1024;

    // Allocate a buffer on the stack that's big enough for us almost
    // all the time.  Be prepared to allocate dynamically if it doesn't fit.
    size_t size = initialSize;
    char stackbuf[initialSize];
    std::vector<char> dynamicbuf;
    char *buf = &stackbuf[0];
    va_list ap_copy;

    while (true) {
        // Try to vsnprintf into our buffer.
        va_copy(ap_copy, ap);
        int needed = vsnprintf(buf, size, fmt, ap);
        va_end(ap_copy);

        // NB. C99 (which modern Linux and OS X follow) says vsnprintf
        // failure returns the length it would have needed.  But older
        // glibc and current Windows return -1 for failure, i.e., not
        // telling us how much was needed.
        if (needed <= static_cast<int>(size) && needed >= 0) {
            // It fit fine so we're done.
            return std::string(buf, static_cast<size_t>(needed));
        }

        // vsnprintf reported that it wanted to write more characters
        // than we allotted.  So try again using a dynamic buffer.  This
        // doesn't happen very often if we chose our initial size well.
        size = (needed > 0) ? static_cast<size_t>(needed + 1) : (size * 2);
        dynamicbuf.resize(size);
        buf = &dynamicbuf[0];
    }
}

std::string format(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    std::string buf = vformat(fmt, ap);
    va_end(ap);
    return buf;
}

std::vector<std::string> splitAtDelimiter(const std::string &str, char delimiter, size_t max)
{
    std::vector<std::string> result;
    std::string::size_type prev_pos = 0, pos = 0;

    while ((max == 0 || result.size() < max) && (pos = str.find(delimiter, pos)) != std::string::npos) {
        std::string substring = str.substr(prev_pos, pos - prev_pos);
        result.push_back(std::move(substring));
        prev_pos = ++pos;
    }

    // last word
    std::string substring = str.substr(prev_pos, pos - prev_pos);
    result.push_back(std::move(substring));
    return result;
}

}  // namespace voxelio
