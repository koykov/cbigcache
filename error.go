package cbigcache

import "errors"

var (
	ErrorOk          error = nil
	ErrorNoShard           = errors.New("shard not found for given key")
	ErrorNoSpace           = errors.New("couldn't allocate memory to perform the operation")
	ErrorInternal          = errors.New("internal error caught, see the logs")
	ErrorKeyNotFound       = errors.New("key not found in cache")
	ErrorKeyExpired        = errors.New("key found, but expired")
	ErrorKeyExists         = errors.New("key already exists")
	ErrorBufLenLow         = errors.New("insufficient buffer length")

	ErrorCacheIsDead = errors.New("cache is dead now")

	errorRegistry = []error{
		ErrorCodeOk:          ErrorOk,
		ErrorCodeNoShard:     ErrorNoShard,
		ErrorCodeNoSpace:     ErrorNoSpace,
		ErrorCodeInternal:    ErrorInternal,
		ErrorCodeKeyNotFound: ErrorKeyNotFound,
		ErrorCodeKeyExpired:  ErrorKeyExpired,
		ErrorCodeKeyExists:   ErrorKeyExists,
		ErrorCodeBufLenLow:   ErrorBufLenLow,
	}
)
