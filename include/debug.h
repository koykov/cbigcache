#ifndef CBIGCACHE_DEBUG_H
#define CBIGCACHE_DEBUG_H

#include "const.h"
#include "types.h"

class debug {
public:
    /**
     * The constructor.
     *
     * @param verbose_lvl one of VERBOSE_LVL_* const.
     */
    explicit debug(uint verbose_lvl);

    /**
     * Set new verbose level.
     *
     * @see debug::debug()
     * @param verbose_lvl
     */
    void set_lvl(uint verbose_lvl);

    /**
     * Display exception message.
     *
     * @param what e.what() message
     */
    void excp(const char *what);

    /**
     * Display formatted error message.
     *
     * @see http://www.cplusplus.com/reference/cstdio/printf
     * @param fmt
     * @param ...
     */
    void err(const char *fmt, ...);

    /**
     * Display formatted warning message.
     *
     * @see http://www.cplusplus.com/reference/cstdio/printf
     * @param fmt
     * @param ...
     */
    void warn(const char *fmt, ...);

    /**
     * Display formatted verbosity message with level 1.
     *
     * @see http://www.cplusplus.com/reference/cstdio/printf
     * @param fmt
     * @param ...
     */
    void l1(const char *fmt, ...);

    /**
     * Display formatted verbosity message with level 2.
     *
     * @see http://www.cplusplus.com/reference/cstdio/printf
     * @param fmt
     * @param ...
     */
    void l2(const char *fmt, ...);


    /**
     * Display formatted verbosity message with level 3.
     *
     * @see http://www.cplusplus.com/reference/cstdio/printf
     * @param fmt
     * @param ...
     */
    void l3(const char *fmt, ...);

private:
    /**
     * Verbosity level.
     */
    uint lvl = 0;

    /**
     * Internal printer.
     *
     * @param lvl
     * @param msg
     */
    void __p(uint lvl, const char *msg);
};

#endif //CBIGCACHE_DEBUG_H
