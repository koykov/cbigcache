#include "debug.h"
#include "export.h"
#include "bigcache.h"

CBigCache cbc_new(char *config) {
    auto *cbc = new BigCache(config);
    return (void*) cbc;
}

void cbc_free(CBigCache *ptr) {
    auto *cbc = (BigCache*) ptr;
    delete cbc;
}

error cbc_set(CBigCache *cbc_ptr, char *key, byte *data) {
    auto *cbc = (BigCache*) cbc_ptr;
    auto err = cbc->set(key, data);
    return err;
}

error cbc_get(CBigCache *cbc_ptr, char *key, byte *buf, uint len) {
    auto *cbc = (BigCache*) cbc_ptr;
    return cbc->get(key, buf, len);
}

error cbc_evict(CBigCache *cbc_ptr, char *key) {
    auto *cbc = (BigCache*) cbc_ptr;
    return cbc->evict(key);
}