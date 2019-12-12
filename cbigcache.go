package cbigcache

/*
#cgo CFLAGS: -I${SRCDIR}/include
#cgo LDFLAGS: -lcbigcache
#include <stdlib.h>
#include <sys/types.h>
#include "include/export.h"
*/
import "C"
import (
	"bytes"
	"unsafe"
)

// CBigCache is a fast in-memory cache.
// The main idea is inspired by BigCache written in pure Go, but release has a lot of differences.
// The main difference is in allocation algorithm. Go's BC uses builtin copy() to allocate more memory, CBC just
// allocates new page in the shard, without copying.
// Also all the data stores in a heap using CGO wrapper, therefore GC will not touch neither data or indexes at all!
type CBigCache struct {
	// Instance of C.CBigCache.
	handler C.CBigCache
	// Flag to check is cache alive or dead.
	alive bool
	// Maximum buffer length that will used for read the data.
	maxBufSize uint
}

// Init new instance of BigCache.
func NewCBigCache(config *Config) (*CBigCache, error) {
	// Init debugger first.
	verboseLevel := C.uint(config.VerboseLevel)
	C.cbc_debug_init(verboseLevel)

	// Prepare config and create new instance of CBigCache.
	configJson, _ := config.Marshal()
	configJsonC := C.CString(configJson)
	defer C.free(unsafe.Pointer(configJsonC))
	cbc := &CBigCache{
		handler:    C.cbc_new(configJsonC),
		alive:      true,
		maxBufSize: 1,
	}
	return cbc, nil
}

// Releases all unsafe memory and mark the instance as dead.
func (c *CBigCache) Free() error {
	if !c.alive {
		return ErrorCacheIsDead
	}
	ptrCbc := (*C.CBigCache)(unsafe.Pointer(c.handler))
	c.alive = false
	C.cbc_free(ptrCbc)
	return ErrorOk
}

// Save bytes under a given key in cache.
func (c *CBigCache) Set(key string, data []byte) error {
	if !c.alive {
		return ErrorCacheIsDead
	}

	ptrKey := C.CString(key)
	defer C.free(unsafe.Pointer(ptrKey))

	// Convert slice of bytes to C-like bytes (unsigned chars).
	dataLen := uint(len(data))
	ptrData := (*C.uchar)(unsafe.Pointer(&data[0]))

	// Call the C.CBigCache instance.
	ptrCbc := (*C.CBigCache)(unsafe.Pointer(c.handler))
	errCode := ErrorCode(C.cbc_set(ptrCbc, ptrKey, ptrData))

	// Update maximum buffer size for further reads.
	if errCode == ErrorCodeOk && dataLen > c.maxBufSize {
		c.maxBufSize = dataLen
	}

	return errorRegistry[errCode]
}

// Gets bytes for a given key.
// Successful result is a filled slice of bytes and count of read bytes.
// In any other cases the third result var will contain a corresponding error.
func (c *CBigCache) Get(key string) ([]byte, uint, error) {
	if !c.alive {
		return nil, 0, ErrorCacheIsDead
	}

	ptrKey := C.CString(key)
	defer C.free(unsafe.Pointer(ptrKey))

	// +1 to max length need to save zero byte.
	lenMax := c.maxBufSize + 1
	ptrLenMax := C.uint(lenMax)

	// Prepare buffer.
	buf := make([]byte, lenMax)
	ptrBuf := (*C.uchar)(unsafe.Pointer(&buf[0]))

	// Call the C.CBigCache instance.
	ptrCbc := (*C.CBigCache)(unsafe.Pointer(c.handler))
	errCode := ErrorCode(C.cbc_get(ptrCbc, ptrKey, ptrBuf, ptrLenMax))

	if errCode != ErrorCodeOk {
		return nil, 0, errorRegistry[errCode]
	}

	// Get the actual length of a result to trim useless bytes at the end of th result.
	lenActual := lenMax
	if zeroIndex := bytes.IndexByte(buf, byte('\x00')); zeroIndex != -1 {
		lenActual = uint(zeroIndex)
	}

	// Return lenActual bytes of result.
	return buf[:lenActual], lenActual, nil
}
