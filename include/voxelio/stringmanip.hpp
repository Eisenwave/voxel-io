#ifndef VXIO_STRINGMANIP_HPP
#define VXIO_STRINGMANIP_HPP
/*
 * stringmanip.hpp
 * -----------
 * Provides lightweight string manipulation functions.
 */

#include "primitives.hpp"

#include <string>
#include <vector>

namespace voxelio {

// IN-PLACE CONVERSIONS ================================================================================================

/**
 * @brief Converts a string to upper case in-place.
 * @param str the string
 */
void toUpperCase(std::string &str);

/**
 * @brief Converts a string to lower case in-place.
 * @param str the string
 */
void toLowerCase(std::string &str);

/**
 * @brief Left-trims whitespace in a string in-place.
 * @param str the string
 */
void ltrim(std::string &str);

/**
 * @brief Right-trims whitespace in a string in-place.
 * @param str the string
 */
void rtrim(std::string &str);

/**
 * @brief Trims whitespace left and right in a string in-place.
 * @param str the string
 */
void trim(std::string &str);

/**
 * @brief Replaces all instances of one char in the string with a different char.
 * @param str the string
 * @param old the old char
 * @param nue the new char
 */
void replaceChar(std::string &str, char old, char nue);

// COPYING CONVERSIONS =================================================================================================

std::string lpad(const std::string &str, usize length, char c = ' ');
std::string rpad(const std::string &str, usize length, char c = ' ');

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

std::vector<std::string> splitAtDelimiter(const std::string &str, char delimiter, usize maxParts = 0);

// FORMATTING ==========================================================================================================

/**
 * Formats a function like C's printf but writes the result into a std::string.
 *
 * @param format the null-terminated format string
 * @return the formatted string
 */
std::string format(const char *format, ...);

}  // namespace voxelio

#endif  // STRINGMANIP_HPP
