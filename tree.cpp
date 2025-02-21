#include "tree.h"
#include <algorithm>  // for std::lower_bound
#include <cstring>    // for memcpy
#include <immintrin.h> // for the AVX2
#include <iostream>

MyTree::MyNode::MyNode(uint64_t key, void* val, size_t valbytes) {
	this->key = key;
	this->val = malloc(valbytes);
	memcpy(this->val, val, valbytes);
	this->parent = NULL;
	this->right = NULL;
	this->left = NULL;
	this->valcnt = valbytes/sizeof(uint32_t);
}

// MyTree::BPlusNode::BPlusNode(bool isleaf) {
//     this->isLeaf = isleaf;
//     this->parent = nullptr;
//     this->next = nullptr;
// }


MyTree::MyTree() {
    root = new BPlusNode(true);
}


MyTree::BPlusNode* MyTree::Find(uint64_t key) {
    if (!root) return nullptr;
    BPlusNode* leaf = FindLeaf(key);
    if (!leaf) return nullptr;

    // 1. Do a normal left-to-right search in this leaf to see if any record has 'key'
    int found_i = -1;
    for (int i = 0; i < leaf->keys.size(); i++) {
        if (leaf->keys[i] == key) {
            found_i = i;
            break;
        }
    }

    // 2. Now move left while previous slots also have the same key.
    //    This ensures we return the "left-most" duplicate, matching BST logic.
    while (found_i > 0 && leaf->keys[found_i - 1] == key) {
        found_i--;
    }
    // return leaf->values[found_i];
    return leaf;

}


MyTree::BPlusNode* MyTree::FindLeaf(uint64_t key) {
    MyTree::BPlusNode* cur = root;
    while (cur && !cur->isLeaf) {
        // pick which child to descend
        int i = 0;
        while (i < cur->keys.size() && key >= cur->keys[i]) {
            i++;
        }
        cur = cur->children[i];
    }
    return cur;
}



void MyTree::SplitChild(BPlusNode* parent, int index, BPlusNode* child) {
    BPlusNode* newChild = new BPlusNode(child->isLeaf);

    parent->children.insert(parent->children.begin() + index + 1, newChild);

    parent->keys.insert(parent->keys.begin() + index, child->keys[BPT_ORDER - 1]);

    newChild->keys.assign(child->keys.begin() + BPT_ORDER, child->keys.end());
    child->keys.resize(BPT_ORDER - 1);
    
    if (!child->isLeaf) {
        newChild->children.assign(child->children.begin() + BPT_ORDER, 
        child->children.end());

        child->children.resize(BPT_ORDER);
    }

    if (child->isLeaf) {
        newChild->values.assign(child->values.begin() + BPT_ORDER, child->values.end());
        child->values.resize(BPT_ORDER - 1);

        newChild->next = child->next;
        child->next = newChild;
    }
}


void MyTree::insertNonFull(BPlusNode* node, uint64_t key, void* val) {
    if (node->isLeaf) {
        auto it = upper_bound(node->keys.begin(), node->keys.end(), key);
        size_t index = it - node->keys.begin();

        node->keys.insert(it, key);

        node->values.insert(node->values.begin() + index, val);
    }
    else {
        int i = node->keys.size() - 1;
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;
        if (node->children[i]->keys.size() == 2 * BPT_ORDER - 1) {
            MyTree::SplitChild(node, i, node->children[i]);
            if (key > node->keys[i]) {
                i++;
            }
        }
        insertNonFull(node->children[i], key, val);
    }
}

void MyTree::Insert(MyNode* nn) {
    size_t valbytes = nn->valcnt * sizeof(uint32_t);
    void* new_val = malloc(valbytes);
    memcpy(new_val, nn->val, valbytes);

    if (root == nullptr) {
        root = new BPlusNode(true);
        root->keys.push_back(nn->key);
        root->values.push_back(new_val);
    }
    else {
        if (root->keys.size() == 2 * BPT_ORDER - 1) {
            BPlusNode* newRoot = new BPlusNode();
            newRoot->children.push_back(root);
            MyTree::SplitChild(newRoot, 0, root);
            root = newRoot;
        }
        MyTree::insertNonFull(root, nn->key, new_val);
    }

}



void traverse_queries(MyTree* tree,
    std::vector<std::tuple<MyTree::MyNode*, uint64_t>> queries,
    int tid,
    int threadcnt,
    uint64_t* ret) 
{
    uint64_t ans_sum = 0;

    for (size_t i = tid; i < queries.size(); i += threadcnt) {
        std::tuple<MyTree::MyNode*, uint64_t> q = queries[i];
        MyTree::MyNode* from = std::get<0>(q);
        uint64_t to = std::get<1>(q);

        MyTree::BPlusNode* nn = tree->Find(from->key);
        if (!nn) {
            std::cout << "Key not found: " << from->key << std::endl;
            continue;
        }
        if (!nn->values.empty()) {
            std::cout << "BPlusNode value[0]" << nn->values[0] << std::endl;
        }
    }
    *ret = ans_sum;

}