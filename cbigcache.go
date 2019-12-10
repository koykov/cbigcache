package cbigcache

/*
#cgo CFLAGS: -I${SRCDIR}/include
#cgo LDFLAGS: -L${SRCDIR}/src
#include <stdlib.h>
#include "include/export.h"
*/
import "C"
import (
	"github.com/koykov/cbigcache/src"
	"unsafe"
)

type CBigCache struct {
	handler C.CBigCache
}

func NewCBigCache(config *Config) (*CBigCache, error) {
	src.CppInclude()

	configJson, _ := config.Marshal()
	configJsonC := C.CString(configJson)
	defer C.free(unsafe.Pointer(configJsonC))
	cbc := &CBigCache{handler: C.make_cbigcache(configJsonC)}
	return cbc, nil
}
