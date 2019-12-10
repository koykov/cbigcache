#ifndef CBIGCACHE_SHARD_PAGE_H
#define CBIGCACHE_SHARD_PAGE_H

/**
 * @file Shard page struct.
 */

/**
 * Describes a page of shard.
 * Addresses fields contains addresses of internal memory of the shard, not addresses in virtual memory.
 */
struct shard_page {
    /**
     * Array of bytes with certain length.
     * @see Shard::sz_page
     */
    byte *payload;

    /**
     * Low memory address of the page.
     */
    uint64 addr_lo;

    /**
     * High memory address of the page.
     */
    uint64 addr_hi;
};

#endif //CBIGCACHE_SHARD_PAGE_H
