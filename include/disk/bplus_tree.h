#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>

const int ORDER = 4; // Max children

class BPlusNode {
public:
    bool is_leaf;
    std::vector<uint64_t> keys;
    std::vector<BPlusNode*> children;
    std::vector<uint64_t> values; // Offsets for leaves
    BPlusNode* next; // Next leaf

    BPlusNode(bool leaf);
};

class BPlusTree {
public:
    BPlusTree();
    ~BPlusTree();

    void insert(uint64_t key, uint64_t value);
    bool search(uint64_t key, uint64_t& out_value) const;

private:
    BPlusNode* root_;

    void insertInternal(uint64_t key, BPlusNode* cursor, BPlusNode* child);
    BPlusNode* findParent(BPlusNode* cursor, BPlusNode* child) const;
};
