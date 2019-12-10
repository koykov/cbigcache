#include "ts_counter.h"

void ts_counter::inc() {
    this->mux.lock();
    this->val++;
    this->mux.unlock();
}

void ts_counter::dec() {
    this->mux.lock();
    this->val--;
    this->mux.unlock();
}

uint ts_counter::get() {
    this->mux.lock();
    auto v = this->val;
    this->mux.unlock();
    return v;
}

bool ts_counter::is_zero() {
    this->mux.lock();
    auto b = this->val == 0;
    this->mux.unlock();
    return b;
}
