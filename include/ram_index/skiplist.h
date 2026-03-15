#pragma once
#include <vector>
#include <cstdint>
#include "../common/record.h"

class SkipNode {
public:
    uint64_t key;
    Record value;
    std::vector<SkipNode*> forward;

    SkipNode(uint64_t k, const Record& v, int level);
};

class SkipList {
public:
    SkipList(int max_level = 16, float p = 0.5);
    ~SkipList();

    // Insert or update a record
    void insert(uint64_t key, const Record& value);

    // Search for a record, returns true if found
    bool search(uint64_t key, Record& out_record) const;

    // Remove a record, returns true if successfully removed
    bool remove(uint64_t key);

private:
    int randomLevel() const;

    int max_level_;
    float p_;
    int level_;
    SkipNode* header_;
};
