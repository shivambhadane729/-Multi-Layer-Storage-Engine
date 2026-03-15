#include "../../include/cache/lru_cache.h"

LRUCache::LRUCache(size_t capacity) : capacity_(capacity) {}

bool LRUCache::get(uint64_t key, Record& out_record) {
    auto it = cache_map_.find(key);
    if (it == cache_map_.end()) {
        return false;
    }
    
    // Move the accessed item to the front of the list (Most Recently Used)
    items_.splice(items_.begin(), items_, it->second);
    
    out_record = it->second->second;
    return true;
}

EvictionResult LRUCache::put(uint64_t key, const Record& value) {
    EvictionResult result = {false, 0, {}};
    
    auto it = cache_map_.find(key);
    if (it != cache_map_.end()) {
        // Key already exists, update value and move to front
        it->second->second = value;
        items_.splice(items_.begin(), items_, it->second);
        return result; // No eviction
    }
    
    // Key doesn't exist, check capacity
    if (items_.size() >= capacity_ && capacity_ > 0) {
        // Evict Least Recently Used (back of the list)
        auto last = items_.back();
        result.evicted = true;
        result.key = last.first;
        result.value = last.second;
        
        cache_map_.erase(last.first);
        items_.pop_back();
    }
    
    // Insert new item at the front
    items_.emplace_front(key, value);
    cache_map_[key] = items_.begin();
    
    return result;
}

size_t LRUCache::size() const {
    return items_.size();
}

size_t LRUCache::get_capacity() const {
    return capacity_;
}
