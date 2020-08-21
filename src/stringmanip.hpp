#ifndef VXIO_STRINGMANIP_HPP
#define VXIO_STRINGMANIP_HPP
/*
 * stringmanip.hpp
 * -----------
 * Provides lightweight string manipulation functions.
 */

#include <string>
#include <vector>

namespace voxelio {

// IN-PLACE CONVERSIONS ================================================================================================

/**
 * @brief Converts a string to upper case.
 * @param str the string
 */
void toUpperCase(std::string &str);

/**
 * @brief Converts a string to lower case.
 * @param str the string
 */
void toLowerCase(std::string &str);

void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);

// COPYING CONVERSIONS =================================================================================================

std::string lpad(const std::string &str, size_t length, char c = ' ');
std::string rpad(const std::string &str, size_t length, char c = ' ');

std::string substrBeforeFirst(const std::string &str, char delimiter);
std::string substrBeforeLast(const std::string &str, char delimiter);
std::string substrAfterFirst(const std::string &str, char delimiter);
std::string substrAfterLast(const std::string &str, char delimiter);

std::string dir(const std::string &str, char delimiter = '/');
std::string basename(const std::string &str, char delimiter = '/');
std::string noext(const std::string &str, char delimiter = '.');
std::string basenameNoext(const std::string &str);
std::string ext(const std::string &str, char delimiter = '.');

// SPLITTING ===========================================================================================================

std::vector<std::string> splitAtDelimiter(const std::string &str, char delimiter, size_t maxParts = 0);

// FORMATING ===========================================================================================================

/**
 * Formats a function like C's printf but writes the result into a std::string.
 *
 * @param format the null-terminated format string
 * @return the formatted string
 */
std::string format(const char *format, ...);

}  // namespace voxelio

#endif  // STRINGMANIP_HPP
