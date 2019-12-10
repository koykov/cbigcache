#ifndef CBIGCACHE_SHARD_ENTRY_H
#define CBIGCACHE_SHARD_ENTRY_H

/**
 * @file Shard's internal structs.
 */

/**
 * Describes a block of usage data.
 */
struct shard_entry_used {
    /**
     * Start address in the shard's memory.
     */
    uint64 offset;

    /**
     * Length of the block.
     */
    uint len;

    /**
     * Pointer to the next block if current block isn't enough to store the entry.
     */
    shard_entry_used *next;
};

/**
 * Describes entry in usage index.
 */
struct shard_entry_root {
    /**
     * Total length of all block of usage data.
     */
    uint total_len;

    /**
     * Expire moment in nanoseconds.
     */
    uint64 expire;

    /**
     * Pointer to the first block of usage data.
     */
    shard_entry_used *root;
};

/**
 * Describes free block in shard's memory.
 */
struct shard_entry_free {
    /**
     * Start address in the shard's memory.
     */
    uint64 offset;

    /**
     * Length of the block.
     */
    uint64 len;
};

#endif //CBIGCACHE_SHARD_ENTRY_H
