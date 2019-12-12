package cbigcache

const (
	// Verbosity level constants.
	// Note that higher value includes by default lowest levels. So, VerboseLevelWarning includes
	// VerboseLevelException and VerboseLevelError as well.
	// Log nothing.
	VerboseLevelNone ConfigVerboseLevel = 0
	// Log only exceptions.
	VerboseLevelException ConfigVerboseLevel = 1
	// Log errors as well.
	VerboseLevelError ConfigVerboseLevel = 2
	// Log warning.
	VerboseLevelWarning ConfigVerboseLevel = 3
	// Log info messages with level 1.
	VerboseLevelDebug1 ConfigVerboseLevel = 4
	// Log info messages with level 2.
	VerboseLevelDebug2 ConfigVerboseLevel = 5
	// Log info messages with level 3.
	VerboseLevelDebug3 ConfigVerboseLevel = 6

	// Success.
	ErrorCodeOk ErrorCode = 0
	// Shard not found for given key.
	ErrorCodeNoShard ErrorCode = 1
	// No space available to save the data and max size limit exceeded.
	// Note that this error means that no enough space in shard, not the whole cache.
	ErrorCodeNoSpace ErrorCode = 2
	// Internal error caught.
	// Multiple such error may mark the shard as corrupted.
	ErrorCodeInternal ErrorCode = 3
	// Key not found in cache.
	ErrorCodeKeyNotFound ErrorCode = 4
	// Lifetime of the key is exceeded, but it still don't evicted.
	ErrorCodeKeyExpired ErrorCode = 5
	// You try to write the data under a key, that already exists in the cache.
	// To ignore this kind of error set config.ForceSet to true.
	ErrorCodeKeyExists ErrorCode = 6
	// Buffer that you reserved for data is too small.
	ErrorCodeBufLenLow ErrorCode = 7

	// Cache sizes.
	Byte     MemorySize = 1
	Kilobyte            = Byte * 1024
	Megabyte            = Kilobyte * 1024
	Gigabyte            = Megabyte * 1024
	Terabyte            = Gigabyte * 1024
)
