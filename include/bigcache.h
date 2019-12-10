#ifndef CBIGCACHE_BIGCACHE_H
#define CBIGCACHE_BIGCACHE_H

#include <map>
#include <string>
#include <thread>
#include "const.h"
#include "debug.h"
#include "shard.h"
#include "ts_counter.h"
#include "types.h"

/**
 * Main class.
 */
class BigCache {
public:
    /**
     * The constructor.
     *
     * @param config JSON string with configs
     */
    explicit BigCache(const std::string &config);

    /**
     * The destructor.
     */
    ~BigCache();

    /**
     * Set byte array <code>data</code> to cache under the key <code>key</code>.
     *
     * @param key  string key
     * @param data byte array
     * @return error code
     */
    error set(const std::string &key, const byte *data);

    /**
     * Get bytes of the entry corresponding to key <code>key</code>.
     *
     * @param key   string key
     * @param buf   output buffer
     * @param len   max length of the buffer
     * @param len_f actual length of the entry bytes in the cache, output var
     * @return error code
     */
    error get(const std::string &key, byte* (&buf), uint len);

    /**
     * Evict the entry corresponding to key <code>key</code>.
     *
     * @param key string key
     * @return error code
     */
    error evict(const std::string &key);

    /**
     * Expiration supervisor thread control worker.
     * Spawns a child threads for each shard and calculate expiration timings.
     */
    void expire_ctl();

    /**
     * Bulk expiration operations over chunk of four shards.
     *
     * @param shrd0
     * @param shrd1
     * @param shrd2
     * @param shrd3
     */
    void expire_shard(Shard *shrd0, Shard *shrd1, Shard *shrd2, Shard *shrd3);

    /**
     * Do an expiration operation on a single shard.
     *
     * @param shrd
     */
    void expire_shard_singe(Shard *shrd);

    /**
     * Vacuuming supervisor thread control worker.
     * Spawns a child threads for each shard and calculate vacuuming timings.
     */
    void vacuum_ctl();

    /**
     * Bulk vacuuming operations over chunk of four shards.
     *
     * @param shrd0
     * @param shrd1
     * @param shrd2
     * @param shrd3
     */
    void vacuum_shard(Shard *shrd0, Shard *shrd1, Shard *shrd2, Shard *shrd3);

    /**
     * Do an expiration operation on a single shard.
     *
     * @param shrd
     */
    void vacuum_shard_singe(Shard *shrd);

    void freeze();

private:
    /**
     * Debugger instance.
     */
    debug *dbg;

    /**
     * Helper mask to determine index of the shard to set/get data.
     */
    uint shard_mask = 0;

    /**
     * Shards count in the cache.
     */
    uint shards_cnt = DEF_SHARDS_CNT;

    /**
     * Shards storage.
     */
    std::map<uint, Shard*> shards;

    /**
     * Write data despite key already exists or not.
     * Otherwise ERR_KEY_EXISTS will return on attempt to write over existing key.
     */
    bool force_set = false;

    /**
     * Max size of the whole cache.
     * Note that this param defines only size of payload data and doesn't contain size of indices, etc...
     * Measure: bytes.
     */
    uint64 max_size = DEF_MAX_SIZE;

    /**
     * Lifetime of every entry in the cache.
     * Every entry after this period will be evicted from the cache.
     * Measure: nanoseconds.
     */
    uint64 expire_ns = DEF_EXPIRE_NS;

    /**
     * Expiration supervisor thread.
     * This thread just control expiration timing and spawn child threads that makes all direct work of expiration.
     */
    std::thread expire_thr;

    /**
     * Flag to stop expire thread.
     * Note, setting this flag to true wouldn't stop expiration operations that already started and executed. This flag
     * only forbids the further running of expiration operations.
     */
    bool expire_thr_stop_sig = false;

    // todo remove if unused
    ts_counter *expire_cntr;

    /**
     * Determine the period of automatic vacuuming.
     * Please note, auto vacuum wouldn't freeze the whole cache at the one moment. It will check and perform vacuum
     * operation over the separate shards in the cache.
     * Measure: nanoseconds.
     */
    uint64 vacuum_ns = DEF_VACUUM_NS;

    /**
     * Vacuuming supervisor thread.
     * This thread just control vacuuming timing and spawn child threads that makes all direct work of vacuuming.
     */
    std::thread vacuum_thr;

    /**
     * Flag to stop vacuum thread.
     * Note, setting this flag to true wouldn't stop vacuuming operations that already started and executed. This flag
     * only forbids the further running of vacuuming operations.
     */
    bool vacuum_thr_stop_sig = false;

    // todo remove if unused
    ts_counter *vacuum_cntr;

    /**
     * Get pointer to the shard corresponding tha key's hash.
     *
     * @param key hash ot the key
     * @return Shard object
     */
    Shard *get_shard(uint64 key);
};

#endif //CBIGCACHE_BIGCACHE_H
