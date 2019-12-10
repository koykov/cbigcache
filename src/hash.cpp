#include "const.h"
#include "hash.h"

uint64 fnv64a(const std::string &s) {
    uint64 ret = HASH_FNV64A_OFFSET
    for (auto c : s) {
        ret ^= uint64(c);
        ret *= HASH_FNV64A_PRIME
    }
    return ret;
}
