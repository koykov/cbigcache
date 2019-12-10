#include <exception>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <cmath>
#include "const.h"
#include "debug.h"
#include "helpers.h"
#include "shard.h"
#include "types.h"

Shard::Shard(uint idx, uint64 max_size, uint64 expire_dur_ns, debug *dbg_p) {
    this->mux.lock();

    if (dbg_p == nullptr) {
        dbg_p = new debug(VERBOSE_LVL_NONE);
    }
    this->dbg = dbg_p;

    this->idx = idx;
    this->sz_max = max_size;
    this->sz_max = max_size;
    this->sz_page = uint64(this->sz_max * float(float(DEF_SHARD_PAGE_SIZE_PRCNT) / 100));
    this->sz_used = 0;
    this->sz_alloc = 0;
    this->sz_free = this->sz_max;

    this->idx_used = std::map<uint64, shard_entry_root*>{};
    this->idx_free = std::list<shard_entry_free*>{1, new shard_entry_free{0, this->sz_max}};
    this->idx_expire = std::map<uint64, std::map<uint64, bool>>{{}};

    uint shard_page_cnt = 100 / DEF_SHARD_PAGE_SIZE_PRCNT + 1;
    for (uint i = 0; i < shard_page_cnt; i++) {
        auto page_n = new shard_page{nullptr, 0, 0};
        this->data[i] = page_n;
    }
    this->dbg->l2("shrd #%d: %d pages prepared", this->idx, shard_page_cnt);
    this->page_reserve();

    this->expire_ns = expire_dur_ns;

    this->mux.unlock();
}

Shard::~Shard() {
    for (auto d : this->data) {
        delete d.second;
    }
    this->data.clear();
    this->idx_used.clear();
    this->idx_free.clear();
    this->idx_expire.clear();
}

uint Shard::get_idx() {
    return this->idx;
}

void Shard::page_reserve() {
    shard_page *page = this->data[this->page_init_cnt];
    page->payload = new byte[this->sz_page];
    page->addr_lo = this->addr_hi;
    page->addr_hi = this->addr_hi + this->sz_page;

    this->sz_alloc += this->sz_page;
    this->addr_hi += this->sz_page;

    this->dbg->l1("shrd #%d: page #%d (lo: %ld, hi: %ld) reserved",
            this->idx, this->page_init_cnt, page->addr_lo, page->addr_hi);

    this->page_init_cnt++;
}

error Shard::fset(uint64 key, const byte *bytes) {
    this->mux.lock();
    auto err = this->__set(key, bytes, true);
    this->mux.unlock();
    return err;
}

error Shard::set(uint64 key, const byte *bytes) {
    this->mux.lock();
    auto err = this->__set(key, bytes, false);
    this->mux.unlock();
    return err;
}

error Shard::__set(uint64 key, const byte *bytes, bool force) {
    error err = ERR_OK;

    try {
        uint64 sz_b = byte_len(bytes);

        if (sz_b == 0) {
            this->dbg->warn("shrd #%d: key %ld no data", this->idx, key);
            return ERR_BUF_LEN_LOW;
        }

        if (this->idx_used.count(key) > 0) {
            if (!force) {
                this->dbg->err("shrd #%d: key %ld already exists in shard #%d", this->idx, key);
                return ERR_KEY_EXISTS;
            }

            if (this->__evict(key, true) == ERR_INTERNAL) {
                return ERR_INTERNAL;
            }
        }

        if (this->sz_alloc - this->sz_used < sz_b) {
            this->dbg->l1("shrd #%d: hasn't enough free allocated space %ld b of %ld b to save %ld b. try to reserve new page",
                     this->idx, this->sz_alloc - this->sz_used, this->sz_alloc, sz_b);
            if (this->sz_used + sz_b > this->sz_max) {
                this->dbg->warn("shrd #%d: can't reserve new page, shard max size limit %ld b will exceeded",
                        this->idx, this->sz_max);
                return ERR_NO_SPACE;
            }
            this->page_reserve();
        }

        uint64 expire = unix_time_now_ns() + this->expire_ns;

        // Make the root for used entries queue.
        auto root = new shard_entry_root;
        root->expire = expire;
        root->total_len = 0;
        root->root = new shard_entry_used;
        auto cur = root->root;
        this->idx_used[key] = root;
        this->reg_expire(expire, key);

        uint64 remained = sz_b;
        while (remained > 0) {
            auto free = this->idx_free.front();
            if (free == nullptr) {
                std::stringstream ss;
                ss << "shrd #" << this->idx << ": couldn't find free block in shard #";
                throw std::runtime_error(ss.str());
            }
            this->dbg->l2("shrd #%d: found free block with len %ld at offset %ld", this->idx, free->len, free->offset);
            // Check how many bytes we can push into free block.
            // case 1: remained > block len - only a part.
            // case 2: remained <= block len - all bytes.
            uint64 len = remained > free->len ? free->len : remained;

            // Prepare current block.
            cur->offset = free->offset;
            cur->len = len;
            cur->next = nullptr;
            root->total_len += len;

            // Push bytes.
            for (ulong i = free->offset; i < free->offset + len; i++) {
                this->set_byte(i, bytes[i - cur->offset]);
                remained--;
            }
            this->dbg->l3("shrd #%d: %ld bytes of %ld has been saved", this->idx, cur->len, sz_b);

            // Bad case: we fill the whole free block but still have bytes to push.
            if (remained > 0) {
                this->dbg->l3("shrd #%d: %ld bytes remained to save. increase used queue with new block",
                         this->idx, remained);
                auto cur_n = new shard_entry_used;
                cur->next = cur_n;
                cur = cur_n;
            }

            // Remove old free block
            this->idx_free.pop_front();

            // Free block was largest that bytes to push. We have some rest of space.
            if (free->len > len) {
                this->dbg->l3("shrd #%d: space left (%ld b) in free block. register it as a smaller free block and write at the end of free index",
                         this->idx, free->len - len);
                // Make new free block from the rest.
                auto free_n = new shard_entry_free;
                free_n->offset = free->offset + len;
                free_n->len = free->len - len;
                // Push it at the end of free index.
                this->idx_free.push_back(free_n);
            }
        }

        // Update used/free metrics.
        this->sz_free -= sz_b;
        this->sz_used += sz_b;

        this->dbg->l2("shrd #%d: now used %ld b, has free %ld b", this->idx, this->sz_used, this->sz_free);

    } catch (std::exception &e) {
        this->dbg->excp(e.what());
        err = ERR_INTERNAL;
    }

    return err;
}

error Shard::get(uint64 key, byte* (&buf), uint len) {
    this->mux.lock();
    auto err = this->__get(key, buf, len);
    this->mux.unlock();
    return err;
}

error Shard::__get(uint64 key, byte* (&buf), uint len) {
    error err = ERR_OK;
    try {
        // check entry exists in shard
        if (this->idx_used.count(key) == 0) {
            this->dbg->warn("shrd #%d: key %ld not found", this->idx, key);
            return ERR_KEY_NOT_FOUND;
        }
        auto root = this->idx_used[key];
        if (root->total_len == 0) {
            this->dbg->warn("shrd #%d: entry on key %ld is empty", this->idx, key);
            return ERR_KEY_NOT_FOUND;
        }

        // check if entry already expired
        if (root->expire < unix_time_now_ns()) {
            this->dbg->warn("shrd #%d: key %ld found, but it's expired", this->idx, key);
            return ERR_KEY_EXPIRED;
        }

        if (root->total_len > len) {
            this->dbg->warn("shrd #%d: supposed buffer length %d b for key %ld is too small. actual len is %d",
                    this->idx, len, key, root->total_len);
            return ERR_BUF_LEN_LOW;
        }
        auto used = root->root;
        uint c = 0;
        // Walk over the used blocks linked list and read corresponding bytes.
        while (used) {
            uint64 addr_hi_b = used->offset + used->len;
            // Fill output buffer with the data of current used block.
            for (uint64 i = used->offset; i < addr_hi_b; i++) {
                buf[c] = this->get_byte(i);
                c++;
            }

            // Shift to the next block in linked list.
            used = used->next;
        }
        this->dbg->l3("shrd #%d: %ld bytes of %ld has been read", this->idx, c, root->total_len);
        buf[c] = '\000';

    } catch (std::exception &e) {
        this->dbg->excp(e.what());
        err = ERR_INTERNAL;
    }

    return err;
}

void Shard::reg_expire(uint64 expire, uint64 key) {
    this->idx_expire[uint64(ceil(expire/1e9)*1e9)][key] = true;
    this->dbg->l3("shrd #%d: register expire moment %ld ns for %ld", this->idx, expire, key);
}

error Shard::bulk_expire() {
    error err = ERR_OK;

    this->dbg->l3("shrd #%d: bulk expire start", this->idx);

    try {
        auto now = unix_time_now_ns();

        std::map<uint64, std::map<uint64, bool>>::reverse_iterator it;
        for (it = this->idx_expire.rbegin(); it != this->idx_expire.rend(); ++it) {
            if (it->first + this->expire_ns <= now) {
                this->mux.lock();

                if (it->second.empty() || it->first == 0) {
                    if (it->first != 0) {
                        this->idx_expire.erase(it->first);
                    }
                    continue;
                }

                for (auto hkey : it->second) {
                    this->__evict(hkey.first, true, true);
                }
                this->idx_expire.erase(it->first);

                this->mux.unlock();
            }
        }

        this->mux.unlock();
    } catch (std::exception &e) {
        this->dbg->excp(e.what());
        err = ERR_INTERNAL;
    }

    this->dbg->l3("shrd #%d: bulk expire finish", this->idx);

    return err;
}

error Shard::evict(uint64 key) {
    this->mux.lock();
    error err = this->__evict(key);
    this->mux.unlock();
    return err;
}

error Shard::__evict(uint64 key, bool skip_check, bool skip_idx_clear) {
    error err = ERR_OK;

    try {
        // check entry exists in shard
        if (!skip_check) {
            if (this->idx_used.count(key) == 0) {
                this->dbg->warn("shrd #%d: key %ld not found", this->idx, key);
                return ERR_KEY_NOT_FOUND;
            }
        }

        auto root = this->idx_used[key];
        if (root == nullptr) {
            return ERR_KEY_NOT_FOUND;
        }
        // Sync balance.
        this->sz_used -= root->total_len;
        this->sz_free += root->total_len;

        // Try to remove key from the expire index.
        if (!skip_idx_clear) {
            this->idx_expire[root->expire].erase(key);
        }


        auto used = root->root;
        auto used_o = used;
        while (used) {
            // Make new free block on the base of used block.
            auto free_n = new shard_entry_free;
            free_n->offset = used->offset;
            free_n->len = used->len;
            // Push it at the end of free index.
            this->idx_free.push_back(free_n);

            this->dbg->l3("shrd #%d: mark mem free, offset %ld len %ld", this->idx, free_n->offset, free_n->len);

            // Shift to the next used block.
            used = used->next;
            // Free used block.
            // Note that the real data still remains in the shard, but turned into a garbage and will overwrite in the
            // future.
            delete used_o;
            // Save pointer for further free.
            used_o = used;
        }

        // Completely remove the entry from used index.
        this->idx_used.erase(key);
    } catch (std::exception &e) {
        this->dbg->excp(e.what());
        err = ERR_INTERNAL;
    }

    return err;
}

void Shard::set_byte(uint64 addr, byte b) {
    int idx_page = addr / this->sz_page;
    if (uint(idx_page) >= this->page_init_cnt) {
        this->dbg->err("shrd #%d: try to write to an unreserved page %d, available space: %ld b, free space: %ld b",
                this->idx, idx_page, this->sz_alloc - this->sz_used, this->sz_free);
        return;
    }
    this->data[idx_page]->payload[addr % this->sz_page] = b;
}

byte Shard::get_byte(uint64 addr) {
    int idx_page = addr / this->sz_page;
    if (uint(idx_page) >= this->page_init_cnt) {
        this->dbg->err("shrd #%d: try to read from an unreserved page %d, addr %ld",
                this->idx, idx_page, addr);
        return 0;
    }
    return this->data[idx_page]->payload[addr % this->sz_page];
}
