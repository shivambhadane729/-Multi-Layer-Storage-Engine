#include "../../include/disk/bplus_tree.h"

BPlusNode::BPlusNode(bool leaf) : is_leaf(leaf), next(nullptr) {}

BPlusTree::BPlusTree() {
    root_ = nullptr;
}

BPlusTree::~BPlusTree() {
    // Basic memory leak here, normally we should write a recursive destructor or use smart pointers.
    // Given the prototype nature, keeping it simple.
}

bool BPlusTree::search(uint64_t key, uint64_t& out_value) const {
    if (root_ == nullptr) return false;

    BPlusNode* cursor = root_;
    while (!cursor->is_leaf) {
        int i;
        for (i = 0; i < cursor->keys.size(); i++) {
            if (key < cursor->keys[i]) break;
        }
        cursor = cursor->children[i];
    }

    for (int i = 0; i < cursor->keys.size(); i++) {
        if (cursor->keys[i] == key) {
            out_value = cursor->values[i];
            return true;
        }
    }
    return false;
}

void BPlusTree::insertInternal(uint64_t key, BPlusNode* cursor, BPlusNode* child) {
    if (cursor->keys.size() < ORDER - 1) {
        int i = 0;
        while (i < cursor->keys.size() && key > cursor->keys[i]) {
            i++;
        }
        cursor->keys.insert(cursor->keys.begin() + i, key);
        cursor->children.insert(cursor->children.begin() + i + 1, child);
    } else {
        BPlusNode* newInternal = new BPlusNode(false);
        std::vector<uint64_t> virtualKeys = cursor->keys;
        std::vector<BPlusNode*> virtualChildren = cursor->children;

        int i = 0;
        while (i < virtualKeys.size() && key > virtualKeys[i]) {
            i++;
        }
        virtualKeys.insert(virtualKeys.begin() + i, key);
        virtualChildren.insert(virtualChildren.begin() + i + 1, child);

        cursor->keys.clear();
        cursor->children.clear();

        int mid = virtualKeys.size() / 2;
        uint64_t upKey = virtualKeys[mid];

        for (int j = 0; j < mid; j++) {
            cursor->keys.push_back(virtualKeys[j]);
            cursor->children.push_back(virtualChildren[j]);
        }
        cursor->children.push_back(virtualChildren[mid]);

        for (int j = mid + 1; j < virtualKeys.size(); j++) {
            newInternal->keys.push_back(virtualKeys[j]);
            newInternal->children.push_back(virtualChildren[j]);
        }
        newInternal->children.push_back(virtualChildren.back());

        if (cursor == root_) {
            BPlusNode* newRoot = new BPlusNode(false);
            newRoot->keys.push_back(upKey);
            newRoot->children.push_back(cursor);
            newRoot->children.push_back(newInternal);
            root_ = newRoot;
        } else {
            insertInternal(upKey, findParent(root_, cursor), newInternal);
        }
    }
}

BPlusNode* BPlusTree::findParent(BPlusNode* cursor, BPlusNode* child) const {
    BPlusNode* parent = nullptr;
    if (cursor->is_leaf || cursor->children[0]->is_leaf) {
        return nullptr;
    }
    for (int i = 0; i < cursor->children.size(); i++) {
        if (cursor->children[i] == child) {
            parent = cursor;
            return parent;
        } else {
            parent = findParent(cursor->children[i], child);
            if (parent != nullptr) return parent;
        }
    }
    return parent;
}

void BPlusTree::insert(uint64_t key, uint64_t value) {
    if (root_ == nullptr) {
        root_ = new BPlusNode(true);
        root_->keys.push_back(key);
        root_->values.push_back(value);
        return;
    }

    BPlusNode* cursor = root_;
    BPlusNode* parent = nullptr;

    while (!cursor->is_leaf) {
        parent = cursor;
        int i;
        for (i = 0; i < cursor->keys.size(); i++) {
            if (key < cursor->keys[i]) break;
        }
        cursor = cursor->children[i];
    }

    if (cursor->keys.size() < ORDER - 1) {
        int i = 0;
        while (i < cursor->keys.size() && key > cursor->keys[i]) {
            i++;
        }
        cursor->keys.insert(cursor->keys.begin() + i, key);
        cursor->values.insert(cursor->values.begin() + i, value);
    } else {
        BPlusNode* newLeaf = new BPlusNode(true);
        std::vector<uint64_t> virtualKeys = cursor->keys;
        std::vector<uint64_t> virtualValues = cursor->values;

        int i = 0;
        while (i < virtualKeys.size() && key > virtualKeys[i]) {
            i++;
        }
        virtualKeys.insert(virtualKeys.begin() + i, key);
        virtualValues.insert(virtualValues.begin() + i, value);

        cursor->keys.clear();
        cursor->values.clear();

        int mid = virtualKeys.size() / 2;
        for (int j = 0; j < mid; j++) {
            cursor->keys.push_back(virtualKeys[j]);
            cursor->values.push_back(virtualValues[j]);
        }
        for (int j = mid; j < virtualKeys.size(); j++) {
            newLeaf->keys.push_back(virtualKeys[j]);
            newLeaf->values.push_back(virtualValues[j]);
        }

        newLeaf->next = cursor->next;
        cursor->next = newLeaf;

        if (cursor == root_) {
            BPlusNode* newRoot = new BPlusNode(false);
            newRoot->keys.push_back(newLeaf->keys[0]);
            newRoot->children.push_back(cursor);
            newRoot->children.push_back(newLeaf);
            root_ = newRoot;
        } else {
            insertInternal(newLeaf->keys[0], parent, newLeaf);
        }
    }
}
