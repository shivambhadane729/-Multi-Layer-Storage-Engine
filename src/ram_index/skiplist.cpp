#include "../../include/ram_index/skiplist.h"
#include <cstdlib>
#include <random>

SkipNode::SkipNode(uint64_t k, const Record& v, int level) 
    : key(k), value(v), forward(level, nullptr) {}

SkipList::SkipList(int max_level, float p) 
    : max_level_(max_level), p_(p), level_(1) {
    Record empty_record = {0, {0}};
    header_ = new SkipNode(0, empty_record, max_level_);
}

SkipList::~SkipList() {
    SkipNode* current = header_;
    while (current != nullptr) {
        SkipNode* next = current->forward[0];
        delete current;
        current = next;
    }
}

int SkipList::randomLevel() const {
    int lvl = 1;
    // Simple random level generation
    while (((float)rand() / RAND_MAX) < p_ && lvl < max_level_) {
        lvl++;
    }
    return lvl;
}

void SkipList::insert(uint64_t key, const Record& value) {
    std::vector<SkipNode*> update(max_level_, nullptr);
    SkipNode* current = header_;

    for (int i = level_ - 1; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->key < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];

    // If key exists, update value
    if (current != nullptr && current->key == key) {
        current->value = value;
        return;
    }

    int new_level = randomLevel();
    if (new_level > level_) {
        for (int i = level_; i < new_level; i++) {
            update[i] = header_;
        }
        level_ = new_level;
    }

    SkipNode* new_node = new SkipNode(key, value, new_level);
    for (int i = 0; i < new_level; i++) {
        new_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = new_node;
    }
}

bool SkipList::search(uint64_t key, Record& out_record) const {
    SkipNode* current = header_;
    for (int i = level_ - 1; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->key < key) {
            current = current->forward[i];
        }
    }
    current = current->forward[0];
    if (current != nullptr && current->key == key) {
        out_record = current->value;
        return true;
    }
    return false;
}

bool SkipList::remove(uint64_t key) {
    std::vector<SkipNode*> update(max_level_, nullptr);
    SkipNode* current = header_;

    for (int i = level_ - 1; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->key < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];

    if (current != nullptr && current->key == key) {
        for (int i = 0; i < level_; i++) {
            if (update[i]->forward[i] != current) {
                break;
            }
            update[i]->forward[i] = current->forward[i];
        }

        delete current;

        // Decrease total level if header forward pointer is null
        while (level_ > 1 && header_->forward[level_ - 1] == nullptr) {
            level_--;
        }
        return true;
    }
    return false;
}
