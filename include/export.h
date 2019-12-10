#ifndef CBIGCACHE_EXPORT_H
#define CBIGCACHE_EXPORT_H

/**
 * @file External function to use in CGO wrapper.
 */

#ifdef __cplusplus
extern "C" {
#endif

    #include <stdint.h>
    #include "types.h"

    /**
     * Unnamed pointer type to use BigCache class externally.
     */
    typedef void* CBigCache;

    /**
     * Create new instance of the BigCache.
     *
     * @see BigCache::BigCache()
     * @param config JSON string
     * @return CBigCache object
     */
    CBigCache cbc_new(char *config);

    /**
     * Release all memory.
     * @param ptr CBigCache object
     */
    void cbc_free(CBigCache *ptr);

    /**
     * Set the data <code>date</code> to cache under the key <code>key</code>.
     *
     * @see BigCache::set()
     * @see ERR_* consts
     * @param cbc_ptr CBigCache object
     * @param key     string key
     * @param data    bytes array
     * @return error code
     */
    error cbc_set(CBigCache *cbc_ptr, char *key, byte *data);

    /**
     * Get the entry's data.
     *
     * Fill the buffer with the entry's bytes.
     * @see BigCache::get()
     * @param cbc_ptr CBigCache object
     * @param key     string key
     * @param buf     output buffer
     * @param len     max length of the buffer
     * @return error code
     */
    error cbc_get(CBigCache *cbc_ptr, char *key, byte *buf, uint len);

    /**
     * Evict entry from the cache.
     *
     * @param cbc_ptr CBigCache object
     * @param key     string key
     * @return error code
     */
    error cbc_evict(CBigCache *cbc_ptr, char *key);

#ifdef __cplusplus
}
#endif

#endif //CBIGCACHE_EXPORT_H
