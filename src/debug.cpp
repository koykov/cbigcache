#include <execinfo.h>
#include <helpers.h>
#include <stdarg.h>
#include <iostream>
#include <sstream>
#include "debug.h"

debug::debug(uint verbose_lvl) {
    this->lvl = verbose_lvl;
}

void debug::set_lvl(uint verbose_lvl) {
    this->lvl = verbose_lvl;
}

void debug::excp(const char *what) {
    if (this->lvl < VERBOSE_LVL_EXCP) {
        return;
    }

    void *buf[256];
    const int calls = backtrace(buf, sizeof(buf) / sizeof(void*));
    backtrace_symbols_fd(buf, calls, 1);
    std::stringstream ss;
    ss << "[ex] " << unix_time_now_fmt() << " e.what(): " << what << std::endl
       << "bt:" << std::endl << buf << std::endl;
    std::cerr << ss.str();
}

void debug::err(const char *fmt, ...) {
    if (this->lvl < VERBOSE_LVL_ERR) {
        return;
    }

    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);

    std::stringstream ss;
    ss << "[er] " << unix_time_now_fmt() << " " << buf << std::endl;
    std::cerr << ss.str();
}

void debug::warn(const char *fmt, ...) {
    if (this->lvl < VERBOSE_LVL_WARN) {
        return;
    }

    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);

    this->__p(VERBOSE_LVL_WARN, buf);
}

void debug::l1(const char *fmt, ...) {
    if (this->lvl < VERBOSE_LVL_DBG1) {
        return;
    }

    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);

    this->__p(VERBOSE_LVL_DBG1, buf);
}

void debug::l2(const char *fmt, ...) {
    if (this->lvl < VERBOSE_LVL_DBG2) {
        return;
    }

    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);

    this->__p(VERBOSE_LVL_DBG2, buf);
}

void debug::l3(const char *fmt, ...) {
    if (this->lvl < VERBOSE_LVL_DBG3) {
        return;
    }

    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);

    this->__p(VERBOSE_LVL_DBG3, buf);
}

void debug::__p(uint lvl, const char *msg) {
    std::string lvl_s;
    switch (lvl) {
        case VERBOSE_LVL_WARN:
            lvl_s = "wr";
            break;
        case VERBOSE_LVL_DBG1:
            lvl_s = "d1";
            break;
        case VERBOSE_LVL_DBG2:
            lvl_s = "d2";
            break;
        case VERBOSE_LVL_DBG3:
            lvl_s = "d3";
            break;
        default:
            lvl_s = "un";
            break;
    }
    std::stringstream ss;
    ss << "[" + lvl_s + "] " << unix_time_now_fmt() << " " << msg << std::endl;
    std::cout << ss.str();
}
