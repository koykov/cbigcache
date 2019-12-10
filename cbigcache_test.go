package cbigcache

import (
	"testing"
	"time"
)

func TestMake(t *testing.T) {
	config := DefaultConfig(1 * time.Minute)
	cache, err := NewCBigCache(config)
	t.Log(cache, err)
}
