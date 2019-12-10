#ifndef CBIGCACHE_SHARD_H
#define CBIGCACHE_SHARD_H

#include <map>
#include <mutex>
#include <list>
#include "const.h"
#include "shard_page.h"
#include "shard_entry.h"
#include "types.h"

/**
 * Cache shard class.
 */
class Shard {
public:
    /**
     * The constructor.
     *
     * @param idx       shard's index
     * @param max_size  max size of the shard
     * @param expire_ns lifetime period of the entry in the shard
     * @param debug_p   Debugger object
     */
    Shard(uint idx, uint64 max_size, uint64 expire_ns, debug *dbg_p);

    /**
     * The destructor.
     */
    ~Shard();

    /**
     * Get shard's index.
     *
     * @return shard index
     */
    uint get_idx();

    /**
     * Set the entry bytes in the shard.
     *
     * @see Shard::_set()
     * @param key   hash key
     * @param bytes bytes array
     * @return error code
     */
    error set(uint64 key, const byte *bytes);

    /**
     * Force set of entry's bytes.
     *
     * Works the same as Shard::set(), but overwrites existing keys.
     * @see Shard::_set()
     * @param key   hash key
     * @param bytes bytes array
     * @return error code
     */
    error fset(uint64 key, const byte *bytes);

    /**
     * Get entry bytes from the shard.
     *
     * @param key   hash key
     * @param buf   output buffer
     * @param len   max length of the buffer
     * @param len_f actual length of the buffer
     * @return error code
     */
    error get(uint64 key, byte* (&buf), uint len);

    /**
     * Start bulk expiration.
     *
     * During this operation all expired entries will be evicted.
     * @see Shard::evict()
     * @return error code
     */
    error bulk_expire();

    /**
     * Evict entry from the shard.
     *
     * In fact the entry will be removed only from the index. The real data of the entry will become a garbage.
     * @param key hash key
     * @return error code
     */
    error evict(uint64 key);

private:
    /**
     * Debugger instance.
     */
    debug *dbg;

    /**
     * Pages storage.
     */
    std::map<uint, shard_page*> data;

    /**
     * Shard index.
     */
    uint idx = 0;

    /**
     * Max size of the shard.
     * Defines only size of the payload. Indices and all other helper data stores separate in the memory.
     * Measure: bytes.
     */
    uint64 sz_max = 0;

    /**
     * Page size.
     * Measure: bytes.
     */
    uint64 sz_page = 0;

    /**
     * Usage size of the shard.
     * Measure: bytes.
     */
    uint64 sz_used = 0;

    /**
     * Free size on shard.
     * Measure: bytes.
     */
    uint64 sz_free = 0;

    /**
     * Allocated size in shard.
     * Measure: bytes.
     */
    uint64 sz_alloc = 0;

    /**
     * High address of the shard.
     * Note it's internal address, not address in virtual memory.
     */
    uint64 addr_hi = 0;

    /**
     * Lifetime period.
     * Measure: nanoseconds.
     */
    uint64 expire_ns = 0;

    /**
     * How many pages already allocated.
     */
    uint page_init_cnt = 0;

    /**
     * Index of usage data.
     * The key is a hash of entry's string key.
     * The value is an linked list of use queue.
     * Complexity: O(log n)
     * @see shard_entry_root
     */
    std::map<uint64, shard_entry_root*> idx_used;

    /**
     * Index of free blocks in the shard.
     * Stores a list of free blocks in the shard.
     * Complexity: O(1)
     * @see shard_entry_free
     */
    std::list<shard_entry_free*> idx_free;

    /**
     * Index of keys expiration time.
     * Stores a pair <expire time in ns, hashes of keys>.
     * Complexity: O(log n)
     */
    std::map<uint64, std::map<uint64, bool>> idx_expire;

    /**
     * Mutex to acquire access to write in the shard.
     * Uses for whole types of write operations: set, evict, vacuum.
     */
    std::mutex mux;

    /**
     * Allocate memory for the new page.
     * Calls when more space required.
     */
    void page_reserve();

    /**
     * Internal setter function.
     *
     * Caution! Call of this func should be protect with mutex.
     * @param key   hash key
     * @param bytes bytes array
     * @param force rewrite existing key flag
     * @return error code
     */
    error __set(uint64 key, const byte *bytes, bool force);

    /**
     * Internal getter function.
     *
     * Caution! Call of this func should be protect with mutex.
     * @param key   hash key
     * @param buf   output buffer
     * @param len   max length of the buffer
     * @param len_f actual length of the buffer
     * @return error code
     */
    error __get(uint64 key, byte* (&buf), uint len);

    /**
     * Register key in expiration index.
     *
     * @param expire UNIX time in nanoseconds
     * @param key    hash key
     */
    void reg_expire(uint64 expire, uint64 key);

    /**
     * Internal eviction function.
     *
     * Caution! Call of this func should be protect with mutex.
     * @param key hash key
     * @return error code
     */
    error __evict(uint64 key, bool skip_check = false, bool skip_idx_clear = false);

    /**
     * Set the byte value at the address <code>addr</code>.
     *
     * Note <code>addr</code> is an internal address, not address in virtual memory.
     * @param addr address in shard
     * @param b    single byte to save
     */
    void set_byte(uint64 addr, byte b);

    /**
     * Get the byte at the address <code>addr</code>.
     *
     * Note <code>addr</code> is an internal address, not address in virtual memory.
     * @param addr address in shard
     * @return single byte
     */
    byte get_byte(uint64 addr);
};

#endif //CBIGCACHE_SHARD_H
