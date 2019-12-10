package cbigcache

import (
	"encoding/json"
	"time"
)

type ConfigVerboseLevel uint

const (
	VerboseLevelException = iota
	VerboseLevelError
	VerboseLevelDebug1
	VerboseLevelDebug2
	VerboseLevelDebug3
)

type Config struct {
	Shards       uint `json:"shards"`
	Expire       time.Duration
	ExpireNs     uint64 `json:"expire_ns"`
	Vacuum       time.Duration
	VacuumNs     uint64 `json:"vacuum_ns"`
	MaxSizeByte  uint64 `json:"max_size"`
	MaxSizeStr   string
	Verbose      bool               `json:"verbose"`
	VerboseLevel ConfigVerboseLevel `json:"verbose_level"`
}

func DefaultConfig(expire time.Duration) *Config {
	return &Config{
		Shards:       1024,
		Expire:       expire,
		Vacuum:       0,
		MaxSizeByte:  0,
		Verbose:      false,
		VerboseLevel: VerboseLevelException,
	}
}

func (c *Config) Marshal() (string, error) {
	c.ExpireNs = uint64(c.Expire.Nanoseconds())
	c.VacuumNs = uint64(c.Vacuum.Nanoseconds())
	b, err := json.Marshal(c)
	return string(b), err
}
