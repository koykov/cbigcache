#ifndef CBIGCACHE_HASH_H
#define CBIGCACHE_HASH_H

/**
 * @file Implementation of FNV-64 hashing function.
 */

#include <string>
#include "types.h"

/**
 * FNV-64 offset basis.
 */
#define HASH_FNV64A_OFFSET 14695981039346656037U;

/**
 * FNV-64 prime.
 */
#define HASH_FNV64A_PRIME 1099511628211;

/**
 * Calculate FNV-64 hash of the given string <code>s</code>.
 *
 * @param s
 * @return hash
 */
uint64 fnv64a(const std::string &s);

#endif //CBIGCACHE_HASH_H
