#include <chrono>
#include <exception>
#include <string>
#include <sys/sysinfo.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include "types.h"

uint64 byte_len(const byte *data) {
    uint64 i;
    for (i = 0; data[i] != '\000'; i++) {}
    return i;
}

bool is_pow2(uint n) {
    return (n & (n - 1)) == 0;
}

std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> chunks;
    std::string chunk;
    std::istringstream token_ss(s);
    while (std::getline(token_ss, chunk, delimiter)) {
        chunks.push_back(chunk);
    }
    return chunks;
}

std::string& ltrim(std::string& str, const std::string& chars = " ") {
    str.erase(0, str.find_first_not_of(chars));
    return str;
}

std::string& rtrim(std::string& str, const std::string& chars = " ") {
    str.erase(str.find_last_not_of(chars) + 1);
    return str;
}

std::string& trim(std::string& str, const std::string& chars = " ") {
    return ltrim(rtrim(str, chars), chars);
}

int64 atoi64(const std::string &s) {
    return strtol(s.c_str(), nullptr, 10);
}

float64 atof64(const std::string &s) {
    return strtod(s.c_str(), nullptr);
}

bool atob(const std::string &s) {
    std::string sl;
    sl.resize(s.size());
    std::transform(s.begin(), s.end(), sl.begin(), ::tolower);
    return sl == "true";
}

uint64 unix_time_now_ns() {
    auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch());
    return now.count();
}

std::string unix_time_now_fmt() {
    const auto now = std::chrono::system_clock::now();
    const auto now_t = std::chrono::system_clock::to_time_t(now);
    const auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()) % 1000000;
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_t), "%Y-%m-%d %T")
        << '.' << std::setfill('0') << std::setw(6) << now_ns.count();
    return ss.str();
}

ulong avail_mem_b() {
    try {
        auto si = new struct sysinfo;
        si->freeram = 0L;
        sysinfo(si);
        return si->freeram;
    } catch (std::exception &e) {
        return 0;
    }
}