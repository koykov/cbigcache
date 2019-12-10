#include <thread>
#include <vector>
#include "bigcache.h"
#include "debug.h"
#include "hash.h"
#include "helpers.h"
#include "json.h"

BigCache::BigCache(const std::string &config) {
    this->dbg = new debug(VERBOSE_LVL_DBG1);

    if (config.length() > 0) {
        this->dbg->l1("init config: %s", config.c_str());

        std::shared_ptr<Json> jc(new Json());
        jc->unmarshal(config, 0);

        uint verbose_lvl = jc->get_i("verbose_lvl", VERBOSE_LVL_ERR);
        if (verbose_lvl < VERBOSE_LVL_NONE || verbose_lvl > VERBOSE_LVL_DBG3) {
            this->dbg->warn("verbosity level %d is out of range %d and %d, suppress debugging",
                    verbose_lvl, VERBOSE_LVL_NONE, VERBOSE_LVL_DBG3);
            verbose_lvl = VERBOSE_LVL_NONE;
        }
        this->dbg->set_lvl(verbose_lvl);

        this->shards_cnt = jc->get_inz("shards_cnt", DEF_SHARDS_CNT);
        if (this->shards_cnt < MIN_SHARDS_CNT || this->shards_cnt > MAX_SHARDS_CNT) {
            this->dbg->warn("shards count %d is out of min/max range %d-%d, fallback to default %d",
                    this->shards_cnt, MIN_SHARDS_CNT, MAX_SHARDS_CNT, DEF_SHARDS_CNT);
            this->shards_cnt = DEF_SHARDS_CNT;
        }
        if (!is_pow2(this->shards_cnt)) {
            this->dbg->warn("shards count provided %d isn't a power of 2, fallback to default %d",
                    this->shards_cnt, DEF_SHARDS_CNT);
            this->shards_cnt = DEF_SHARDS_CNT;
        }

        this->force_set = jc->get_b("force_set", false);

        this->max_size = jc->get_inz("max_size", uint64(avail_mem_b() * DEF_MAX_SIZE_AVAIL_FACTOR));
        if (this->max_size <= 0) {
            this->dbg->warn("couldn't determine cache max size, fallback to default %ld b", DEF_MAX_SIZE);
            this->max_size = DEF_MAX_SIZE;
        }
        this->expire_ns = jc->get_i("expire_ns", DEF_EXPIRE_NS);
        if (this->expire_ns < MIN_EXPIRE_NS) {
            this->dbg->warn("expire time %ld ns is less than minimum %ld ns, fallback to default %ld ns",
                    this->expire_ns, MIN_EXPIRE_NS, DEF_EXPIRE_NS);
            this->expire_ns = DEF_EXPIRE_NS;
        }
        this->vacuum_ns = jc->get_i("vacuum_ns", DEF_VACUUM_NS);
        if (this->vacuum_ns < MIN_VACUUM_NS) {
            this->dbg->warn("vacuum time %ld ns is less than minimum %ld ns, fallback to default %ld ns",
                    this->vacuum_ns, MIN_VACUUM_NS, DEF_VACUUM_NS);
            this->vacuum_ns = DEF_VACUUM_NS;
        }
    }

    this->shard_mask = this->shards_cnt - 1;

    uint64 shard_size = this->max_size / this->shards_cnt;
    for (uint i = 0; i < this->shards_cnt; i++) {
        this->shards[i] = new Shard(i, shard_size, this->expire_ns, this->dbg);
        this->dbg->l2("shrd #%d inited at ptr %p with size %ld b", i, this->shards[i], shard_size);
    }

    this->dbg->l1("cache inited with params:\n\t-shards: %ld\n\t-shard mask: %d\n\t-max size: %ld b\n\t-expire: %ld ns\n\t-vacuum: %ld ns",
             this->shards_cnt, this->shard_mask, this->max_size, this->expire_ns, this->vacuum_ns);

    // Init expire supervisor thread.
    this->expire_cntr = new ts_counter();
    this->expire_thr = std::thread(&BigCache::expire_ctl, this);
    this->dbg->l2("thr_e #%x: inited and started", this->expire_thr.get_id());

    // Init vacuum supervisor thread.
    this->vacuum_cntr = new ts_counter();
    this->vacuum_thr = std::thread(&BigCache::vacuum_ctl, this);
    this->dbg->l2("thr_v #%x: inited and started", this->vacuum_thr.get_id());
}

BigCache::~BigCache() {
    this->freeze();

    // Sync supervisor threads
    this->expire_thr.join();
    this->vacuum_thr.join();

    for (uint i = 0; i < this->shards_cnt; i++) {
        delete this->shards[i];
    }
    this->shards.clear();
    delete this->expire_cntr;
    delete this->vacuum_cntr;
}

error BigCache::set(const std::string &key, const byte *data) {
    auto hashKey = fnv64a(key);
    this->dbg->l3("set: key '%s' (hkey %ld), data %ld b", key.c_str(), hashKey, byte_len(data));
    auto shard = this->get_shard(hashKey);
    if (shard == nullptr) {
        this->dbg->err("shard not found for key '%s' (%ld)", key.c_str(), hashKey);
        return ERR_NO_SHARD;
    }
    this->dbg->l3("shrd #%d src_w", shard->get_idx());
    return this->force_set ? shard->fset(hashKey, data) : shard->set(hashKey, data);
}

error BigCache::get(const std::string &key, byte* (&buf), uint len) {
    auto hashKey = fnv64a(key);
    this->dbg->l3("get: key '%s' (hkey %ld), supposed buffer length %ld b", key.c_str(), hashKey, len);
    auto shard = this->get_shard(hashKey);
    if (shard == nullptr) {
        this->dbg->err("shard not found for key '%s' (%ld)", key.c_str(), hashKey);
        return ERR_NO_SHARD;
    }
    this->dbg->l3("shrd #%d src_r", shard->get_idx());
    return shard->get(hashKey, buf, len);
}

error BigCache::evict(const std::string &key) {
    auto hashKey = fnv64a(key);
    this->dbg->l3("evk: key '%s' (hkey %ld)", key.c_str(), hashKey);
    auto shard = this->get_shard(hashKey);
    if (shard == nullptr) {
        this->dbg->err("shard not found for key '%s' (%ld)", key.c_str(), hashKey);
        return ERR_NO_SHARD;
    }
    this->dbg->l3("shrd #%d src_e", shard->get_idx());
    return shard->evict(hashKey);
}

void BigCache::freeze() {
    this->expire_thr_stop_sig = true;
    this->vacuum_thr_stop_sig = true;
}

void BigCache::expire_ctl() {
    uint64 prev_took = 0;
    auto thr_e_id = std::this_thread::get_id();
    while (true) {
        bool skip = false;
        // Check and warn if previous expiration took more time than expire_ns.
        if (prev_took > this->expire_ns) {
            this->dbg->warn("thr_e #%x: prev expire took %ld ns > expire time %ld ns, skip expire cycle. advise: increase shards count",
                    thr_e_id, prev_took, this->expire_ns);
            // Wait, but skip current cycle.
            prev_took = 0;
            skip = true;
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(this->expire_ns - prev_took));
        if (skip) {
            continue;
        }

        auto time_s = unix_time_now_ns();

        this->dbg->l2("thr_e #%x: expire cycle start", thr_e_id);
        std::vector<std::thread> thr_c_pool{};
        if (this->shards_cnt >= 16) {
            // Init and start shards_cnt/4 child threads.
            // Each thread is responsible to do expiration check and cleanup for chunk of 4 shard.
            for (uint i = 0; i < this->shards_cnt; i = i + 4) {
                thr_c_pool.emplace_back(std::thread(&BigCache::expire_shard, this,
                        this->shards[i], this->shards[i+1], this->shards[i+2], this->shards[i+3]));
            }
        } else {
            // Init and start one thread for each shard.
            for (uint i = 0; i < this->shards_cnt; i++) {
                thr_c_pool.emplace_back(std::thread(&BigCache::expire_shard_singe, this, this->shards[i]));
            }
        }
        this->dbg->l2("thr_e #%x: %d child threads started", thr_e_id, thr_c_pool.size());
        for (auto &thr_c : thr_c_pool) {
            thr_c.join();
        }

        auto time_e = unix_time_now_ns();
        prev_took = time_e - time_s;

        this->dbg->l2("thr_e #%x: expire cycle finish, time took %ld ns", thr_e_id, prev_took);

        if (this->expire_thr_stop_sig) {
            this->dbg->l1("thr_e #%x: caught stop sig. exiting", thr_e_id);
            break;
        }
    }
}

void BigCache::expire_shard(Shard *shrd0, Shard *shrd1, Shard *shrd2, Shard *shrd3) {
    this->dbg->l2("thr_ec #%x: expire start on shrd #%d, #%d, #%d, #%d",
            std::this_thread::get_id(), shrd0->get_idx(), shrd1->get_idx(), shrd2->get_idx(), shrd3->get_idx());
    this->expire_cntr->inc();
    shrd0->bulk_expire();
    shrd1->bulk_expire();
    shrd2->bulk_expire();
    shrd3->bulk_expire();
    this->expire_cntr->dec();
    this->dbg->l2("thr_ec #%x: expire finish on shrd #%d, #%d, #%d, #%d",
            std::this_thread::get_id(), shrd0->get_idx(), shrd1->get_idx(), shrd2->get_idx(), shrd3->get_idx());
}

void BigCache::expire_shard_singe(Shard *shrd) {
    this->dbg->l2("thr_ecs #%x: expire start on shrd #%d",
            std::this_thread::get_id(), shrd->get_idx());
    this->expire_cntr->inc();
    shrd->bulk_expire();
    this->expire_cntr->dec();
    this->dbg->l2("thr_ecs #%x: expire finish on shrd #%d",
            std::this_thread::get_id(), shrd->get_idx());
}

void BigCache::vacuum_ctl() {
    uint64 prev_took = 0;
    auto thr_v_id = std::this_thread::get_id();
    while (true) {
        bool skip = false;
        // Check and warn if previous vacuuming took more time than expire_ns.
        if (prev_took > this->vacuum_ns) {
            this->dbg->warn("thr_v #%x: prev vacuum took %ld ns > vacuum time %ld ns, skip vacuum cycle. advise: increase shards count",
                            thr_v_id, prev_took, this->vacuum_ns);
            // Wait, but skip current cycle.
            prev_took = 0;
            skip = true;
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(this->vacuum_ns - prev_took));
        if (skip) {
            continue;
        }

        auto time_s = unix_time_now_ns();

        this->dbg->l2("thr_v #%x: vacuum cycle start", thr_v_id);
        std::vector<std::thread> thr_c_pool{};
        if (this->shards_cnt >= 16) {
            // Init and start shards_cnt/4 child threads.
            // Each thread is responsible to do vacuuming check and cleanup for chunk of 4 shard.
            for (uint i = 0; i < this->shards_cnt; i = i + 4) {
                thr_c_pool.emplace_back(std::thread(&BigCache::vacuum_shard, this,
                                                    this->shards[i], this->shards[i+1], this->shards[i+2], this->shards[i+3]));
            }
        } else {
            // Init and start one thread for each shard.
            for (uint i = 0; i < this->shards_cnt; i++) {
                thr_c_pool.emplace_back(std::thread(&BigCache::vacuum_shard_singe, this, this->shards[i]));
            }
        }
        this->dbg->l2("thr_v #%x: %d child threads started", thr_v_id, thr_c_pool.size());
        for (auto &thr_c : thr_c_pool) {
            thr_c.join();
        }

        auto time_e = unix_time_now_ns();
        prev_took = time_e - time_s;

        this->dbg->l2("thr_v #%x: vacuum cycle finish, time took %ld ns", thr_v_id, prev_took);

        if (this->vacuum_thr_stop_sig) {
            this->dbg->l1("thr_v #%x: caught stop sig. exiting", thr_v_id);
            break;
        }
    }
}

void BigCache::vacuum_shard(Shard *shrd0, Shard *shrd1, Shard *shrd2, Shard *shrd3) {
    this->dbg->l2("thr_vc #%x: vacuum start on shrd #%d, #%d, #%d, #%d",
                  std::this_thread::get_id(), shrd0->get_idx(), shrd1->get_idx(), shrd2->get_idx(), shrd3->get_idx());
    this->vacuum_cntr->inc();
//    shrd0->bulk_vacuum();
//    shrd1->bulk_vacuum();
//    shrd2->bulk_vacuum();
//    shrd3->bulk_vacuum();
    this->vacuum_cntr->dec();
    this->dbg->l2("thr_ec #%x: vacuum finish on shrd #%d, #%d, #%d, #%d",
                  std::this_thread::get_id(), shrd0->get_idx(), shrd1->get_idx(), shrd2->get_idx(), shrd3->get_idx());
}

void BigCache::vacuum_shard_singe(Shard *shrd) {
    this->dbg->l2("thr_vcs #%x: vacuum start on shrd #%d",
                  std::this_thread::get_id(), shrd->get_idx());
    this->vacuum_cntr->inc();
//    shrd->bulk_vacuum();
    this->vacuum_cntr->dec();
    this->dbg->l2("thr_vcs #%x: vacuum finish on shrd #%d",
                  std::this_thread::get_id(), shrd->get_idx());
}

Shard *BigCache::get_shard(uint64 key) {
    auto shard_key = key & this->shard_mask;
    if (this->shards.count(shard_key) == 0) {
        return nullptr;
    }
    return this->shards[key & this->shard_mask];
}
