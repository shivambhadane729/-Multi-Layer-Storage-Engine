#pragma once
#include <unordered_map>
#include <list>
#include <cstdint>
#include "../common/record.h"

// Result struct to handle data promotion/demotion between layers
struct EvictionResult {
    bool evicted;
    uint64_t key;
    Record value;
};

class LRUCache {
public:
    explicit LRUCache(size_t capacity);

    // Get a record from the cache. Returns true if found, false otherwise.
    // If found, the record is copied into `out_record` and marked as MRU.
    bool get(uint64_t key, Record& out_record);

    // Put a record into the cache. 
    // Returns EvictionResult indicating if a record was evicted.
    EvictionResult put(uint64_t key, const Record& value);

    size_t size() const;
    size_t get_capacity() const;

private:
    size_t capacity_;
    std::list<std::pair<uint64_t, Record>> items_;
    std::unordered_map<uint64_t, std::list<std::pair<uint64_t, Record>>::iterator> cache_map_;
};
