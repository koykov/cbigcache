package cbigcache

import (
	"encoding/json"
	"time"
)

// Config for CBigCache.
type Config struct {
	// Number of cache shards, value must be a power of two
	Shards uint `json:"shards_cnt"`
	// Rewrite existing keys on save.
	ForceSet bool `json:"force_set"`
	// Lifetime of the entries in the cache.
	// ExpireNs contains the same value in nanoseconds.
	// Recommend to omit Ns value because of it will calculate on base of Expire field.
	Expire   time.Duration `json:"-"`
	ExpireNs uint64        `json:"expire_ns"`
	// Auto vacuum period.
	// VacuumNs contains the same value in nanoseconds. You may omit Ns field.
	Vacuum   time.Duration `json:"-"`
	VacuumNs uint64        `json:"vacuum_ns"`
	// Cache max size in bytes.
	// Use MemorySize values.
	MaxSize MemorySize `json:"max_size"`
	// Level of a displayed verbose messages.
	// Use ConfigVerboseLevel values.
	VerboseLevel ConfigVerboseLevel `json:"verbose_lvl"`
}

// DefaultConfig initializes config with default values.
// Note than it omits max size value thus that value will sets as 1/2 of available memory. Therefore we recommend to
// make maximum two instances of CBigCache without size limit.
func DefaultConfig(expire time.Duration) *Config {
	return &Config{
		Shards:       1024,
		ForceSet:     false,
		Expire:       expire,
		Vacuum:       10 * time.Minute,
		MaxSize:      0,
		VerboseLevel: VerboseLevelNone,
	}
}

// Serialize config to a JSON object to send to C.CBigCache.
func (c *Config) Marshal() (string, error) {
	if c.ExpireNs == 0 {
		c.ExpireNs = uint64(c.Expire.Nanoseconds())
	}
	if c.VacuumNs == 0 {
		c.VacuumNs = uint64(c.Vacuum.Nanoseconds())
	}
	b, err := json.Marshal(c)
	return string(b), err
}
