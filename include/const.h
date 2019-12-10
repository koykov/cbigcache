#ifndef CBIGCACHE_CONST_H
#define CBIGCACHE_CONST_H

#include "types.h"

/**
 * Default constants.
 */

/**
 * Default count of the shards.
 */
const uint DEF_SHARDS_CNT = 1024;

/**
 * Size of shard's page in percent.
 * Value: 10 %
 */
const uint DEF_SHARD_PAGE_SIZE_PRCNT = 10;

/**
 * The part of the whole available memory that can be used for the cache.
 * This value uses when max size of the cache aren't provided by the developer.
 * Value: 1/2 of available memory
 */
const float DEF_MAX_SIZE_AVAIL_FACTOR = .5;

/**
 * Default value of max size of the cache.
 * Uses when max size wasn't provided by developer and couldn't calculate available space of the memory.
 * Value: 10 GB
 */
const uint64 DEF_MAX_SIZE = 10737418240;

/**
 * Default lifetime period.
 * Value: 60 sec
 */
const uint64 DEF_EXPIRE_NS = 60000000000;

/**
 * Default auto vacuum period.
 * Value: 10 min
 */
const uint64 DEF_VACUUM_NS = 600000000000;

/**
 * Min/max constants.
 */

const uint MIN_SHARDS_CNT = 4;
const uint MAX_SHARDS_CNT = 4096;

/**
 * Minimal value of the lifetime period.
 * Value: 1 sec
 */
const uint64 MIN_EXPIRE_NS = 1000000000;

/**
 * Minimal value of the auto vacuum period.
 * Value: 1 min
 */
const uint64 MIN_VACUUM_NS = 60000000000;

/**
 * Verbose level.
 * Every level includes the lower levels. So, warnings level includes errors and exceptions as well.
 */

/**
 * Disable debug.
 */
const uint VERBOSE_LVL_NONE = 0;

/**
 * Display only exceptions.
 * This level doesn't include signal like SIGSEGV.
 * Target: stderr
 */
const uint VERBOSE_LVL_EXCP = 1;

/**
 * Display errors.
 * Target: stderr
 */
const uint VERBOSE_LVL_ERR = 2;

/**
 * Display warning.
 * Target: stdout
 */
const uint VERBOSE_LVL_WARN = 3;

/**
 * Display verbose messages with level 1.
 * Target: stdout
 */
const uint VERBOSE_LVL_DBG1 = 4;

/**
 * Display verbose messages with level 2.
 * Target: stdout
 */
const uint VERBOSE_LVL_DBG2 = 5;

/**
 * Display verbose messages with level 3.
 * Target: stdout
 */
const uint VERBOSE_LVL_DBG3 = 6;

/**
 * Error constants.
 */

/**
 * No error.
 */
const error ERR_OK = 0;

/**
 * Shard not found to perform IO operation.
 */
const error ERR_NO_SHARD = 1;

/**
 * Max size limit exceeded or failed alloc.
 */
const error ERR_NO_SPACE = 2;

/**
 * Internal exception was caught.
 */
const error ERR_INTERNAL = 3;

/**
 * Key not found in the cache.
 */
const error ERR_KEY_NOT_FOUND = 4;

/**
 * Key found but it marked as expired and will evicted soon.
 */
const error ERR_KEY_EXPIRED = 5;

/**
 * Key already exists during attempt to save data under the same key.
 */
const error ERR_KEY_EXISTS = 6;

/**
 * Insufficient buffer length. It means that entry's data is bigger that buffer.
 */
const error ERR_BUF_LEN_LOW = 7;

#endif //CBIGCACHE_CONST_H
