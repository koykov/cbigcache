#ifndef CBIGCACHE_TS_COUNTER_H
#define CBIGCACHE_TS_COUNTER_H

#include <mutex>
#include "types.h"

class ts_counter {
public:
    void inc();
    void dec();
    uint get();
    bool is_zero();

private:
    uint val = 0;
    std::mutex mux;
};

#endif //CBIGCACHE_TS_COUNTER_H
