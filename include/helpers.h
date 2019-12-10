#ifndef CBIGCACHE_HELPERS_H
#define CBIGCACHE_HELPERS_H

#include <string>
#include <vector>
#include "types.h"

/**
 * Calculate length of byte array.
 *
 * @param data
 * @return uint64
 */
uint64 byte_len(const byte *data);

/**
 * Check if given number is power of two.
 *
 * @param n
 * @return bool
 */
bool is_pow2(uint n);

/**
 * Splits given string to the chunks using delimiter.
 *
 * @param s
 * @param delimiter
 * @return vector
 */
std::vector<std::string> split(const std::string& s, char delimiter);

/**
 * Trim left side of the string using cutset <code>chars</code>.
 *
 * @param str
 * @param chars
 * @return string
 */
std::string& ltrim(std::string& str, const std::string& chars);

/**
 * Trim right side of the string using cutset <code>chars</code>.
 *
 * @param str
 * @param chars
 * @return string
 */
std::string& rtrim(std::string& str, const std::string& chars);

/**
 * Trim the string using cutset <code>chars</code>.
 *
 * @param str
 * @param chars
 * @return string
 */
std::string& trim(std::string& str, const std::string& chars);

/**
 * Converts given string to int64 var.
 *
 * @param s
 * @return int64
 */
int64 atoi64(const std::string &s);

/**
 * Converts given string to float64 var.
 *
 * @param s
 * @return float64
 */
float64 atof64(const std::string &s);

/**
 * Converts given string to bool var.
 *
 * @param s
 * @return bool
 */
bool atob(const std::string &s);

/**
 * Returns current UNIX time in nanoseconds.
 *
 * @return uint64
 */
uint64 unix_time_now_ns();

/**
 * Returns current formatted UNIX time.
 *
 * @return string
 */
std::string unix_time_now_fmt();

/**
 * Returns available memory size in bytes.
 *
 * @return unsigned long
 */
ulong avail_mem_b();

#endif //CBIGCACHE_HELPERS_H
