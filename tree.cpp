#include "tree.h"
#include <stdio.h>
#include <algorithm>
#include <iostream>

// ================== MyNode Implementation ==================

MyTree::MyNode::MyNode(uint64_t key, void* val, size_t valbytes)
{
    this->key   = key;
    this->val   = malloc(valbytes);
    memcpy(this->val, val, valbytes);
    this->valcnt = valbytes / sizeof(uint32_t);

    // Keep these for main.cpp compatibility (unused in B+ tree)
    this->parent = nullptr;
    this->left   = nullptr;
    this->right  = nullptr;

    // B+ references
    this->leaf      = nullptr;
    this->leafIndex = -1;
}

// Compare two arrays for “distance” as done in the original
uint32_t MyTree::MyNode::Distance(void* val) {
    uint32_t dist = 0;
    uint32_t* arr1 = (uint32_t*)this->val;
    uint32_t* arr2 = (uint32_t*)val;
    for (size_t i = 0; i < this->valcnt; i++) {
        uint32_t d = (arr1[i] > arr2[i]) ? (arr1[i] - arr2[i]) : (arr2[i] - arr1[i]);
        dist += d;
    }
    return dist;
}

// ================== MyTree Implementation ==================

MyTree::MyTree()
{
    // start with an empty tree
    root = nullptr;
}

// Insert node into both the B+ tree and the "golden" multimap
void MyTree::Insert(MyNode* node)
{
    // Insert into golden multimap reference
    //golden.insert({node->key, node});

    // If tree is empty, create the first leaf
    if (!root) {
        root = new BPlusNode(true); // a leaf
        root->count    = 1;
        root->keys[0]  = node->key;
        root->records[0] = node;
        node->leaf      = root;
        node->leafIndex = 0;
        return;
    }

    // Otherwise, find the leaf in which to insert
    BPlusNode* leaf = findLeaf(node->key);
    insertInLeaf(leaf, node);
}

// Return the *first* node that exactly matches 'key', or NULL
MyTree::MyNode* MyTree::Find(uint64_t key)
{
    if (!root) return nullptr;
    BPlusNode* leaf = findLeaf(key);
    if (!leaf) return nullptr;

    // Linear search in that leaf for the first matching key
    for (int i = 0; i < leaf->count; i++) {
        if (leaf->records[i] && leaf->records[i]->key == key) {
            return leaf->records[i];
        }
    }
    return nullptr;
}

// Return the next record in ascending key order, or NULL if none
MyTree::MyNode* MyTree::Next(MyNode* node)
{
    if (!node) return nullptr;
    BPlusNode* leaf = node->leaf;
    if (!leaf) return nullptr;

    // If there is another record in the same leaf, return it
    if (node->leafIndex + 1 < leaf->count) {
        return leaf->records[node->leafIndex + 1];
    }

    // Otherwise, jump to the next leaf
    BPlusNode* nxtLeaf = leaf->next;
    if (nxtLeaf && nxtLeaf->count > 0) {
        return nxtLeaf->records[0];
    }
    // None
    return nullptr;
}

// Golden reference lookups
// std::multimap<uint64_t, MyTree::MyNode*>::iterator 
// MyTree::FindGolden(uint64_t key)
// {
//     return golden.find(key);
// }
// std::multimap<uint64_t, MyTree::MyNode*>::iterator
// MyTree::NextGolden(std::multimap<uint64_t, MyTree::MyNode*>::iterator it)
// {
//     return ++it;
// }

// =============== B+ Tree Internal Helpers ===============

// Follow down the tree until we reach a leaf
MyTree::BPlusNode* MyTree::findLeaf(uint64_t key)
{
    MyTree::BPlusNode* cur = root;
    while (cur && !cur->isLeaf) {
        // pick which child to descend
        int i = 0;
        while (i < cur->count && key >= cur->keys[i]) {
            i++;
        }
        cur = cur->children[i];
    }
    return cur;
}

// Insert a record into a leaf, splitting if needed
void MyTree::insertInLeaf(BPlusNode* leaf, MyNode* record)
{
    uint64_t key = record->key;

    // Insert (key, record) in sorted order
    int i = leaf->count - 1;
    while (i >= 0 && leaf->keys[i] > key) {
        leaf->keys[i + 1]    = leaf->keys[i];
        leaf->records[i + 1] = leaf->records[i];
        if (leaf->records[i + 1]) {
            leaf->records[i + 1]->leafIndex = i + 1;
        }
        i--;
    }
    int insertPos = i + 1;

    leaf->keys[insertPos]    = key;
    leaf->records[insertPos] = record;
    leaf->count++;
    record->leaf      = leaf;
    record->leafIndex = insertPos;

    // Adjust indexes after
    for (int j = insertPos + 1; j < leaf->count; j++) {
        if (leaf->records[j]) {
            leaf->records[j]->leafIndex = j;
        }
    }

    // If overflow (simple order=4 => max 3 keys), split
    if (leaf->count == 4) {
        splitLeaf(leaf);
    }
}

// Split a leaf node that has overflowed
void MyTree::splitLeaf(BPlusNode* leaf)
{
    BPlusNode* newLeaf = new BPlusNode(true);
    newLeaf->parent = leaf->parent;

    int mid = leaf->count / 2; // e.g. 4 => mid=2
    newLeaf->count = leaf->count - mid;
    for (int i = 0; i < newLeaf->count; i++) {
        newLeaf->keys[i]    = leaf->keys[mid + i];
        newLeaf->records[i] = leaf->records[mid + i];
        // fix references
        newLeaf->records[i]->leaf = newLeaf;
        newLeaf->records[i]->leafIndex = i;
    }
    leaf->count = mid;

    // link leaf list
    newLeaf->next = leaf->next;
    leaf->next    = newLeaf;

    // newLeaf's first key goes up
    uint64_t pushKey = newLeaf->keys[0];

    // If leaf was root, create new root
    if (leaf == root) {
        BPlusNode* newRoot = new BPlusNode(false);
        newRoot->count    = 1;
        newRoot->keys[0]  = pushKey;
        newRoot->children[0] = leaf;
        newRoot->children[1] = newLeaf;
        leaf->parent   = newRoot;
        newLeaf->parent= newRoot;
        root = newRoot;
        return;
    }

    // Otherwise, insert pushKey into the parent
    BPlusNode* parent = leaf->parent;
    int i = parent->count - 1;
    while (i >= 0 && parent->keys[i] > pushKey) {
        parent->keys[i + 1]      = parent->keys[i];
        parent->children[i + 2]  = parent->children[i + 1];
        i--;
    }
    int insertPos = i + 1;
    parent->keys[insertPos]     = pushKey;
    parent->children[insertPos+1] = newLeaf;
    parent->count++;
    newLeaf->parent = parent;

    // check for overflow in parent
    if (parent->count == 4) {
        splitInternal(parent);
    }
}

// Split an internal node that has overflowed
void MyTree::splitInternal(BPlusNode* internal)
{
    BPlusNode* newInternal = new BPlusNode(false);
    newInternal->parent = internal->parent;

    int mid = internal->count / 2; 
    uint64_t pushKey = internal->keys[mid];

    // Right half goes to newInternal
    newInternal->count = internal->count - mid - 1;
    for (int i = 0; i < newInternal->count; i++) {
        newInternal->keys[i]    = internal->keys[mid + 1 + i];
        newInternal->children[i] = internal->children[mid + 1 + i];
        if (newInternal->children[i]) {
            newInternal->children[i]->parent = newInternal;
        }
    }
    // last child pointer
    newInternal->children[newInternal->count] = internal->children[internal->count];
    if (newInternal->children[newInternal->count]) {
        newInternal->children[newInternal->count]->parent = newInternal;
    }

    internal->count = mid;

    // If splitting the root
    if (internal == root) {
        BPlusNode* newRoot = new BPlusNode(false);
        newRoot->count   = 1;
        newRoot->keys[0] = pushKey;
        newRoot->children[0] = internal;
        newRoot->children[1] = newInternal;
        internal->parent   = newRoot;
        newInternal->parent= newRoot;
        root = newRoot;
        return;
    }

    // Otherwise push up into the parent
    BPlusNode* parent = internal->parent;
    int i = parent->count - 1;
    while (i >= 0 && parent->keys[i] > pushKey) {
        parent->keys[i + 1] = parent->keys[i];
        parent->children[i + 2] = parent->children[i + 1];
        i--;
    }
    int insertPos = i + 1;
    parent->keys[insertPos] = pushKey;
    parent->children[insertPos + 1] = newInternal;
    parent->count++;
    newInternal->parent = parent;

    if (parent->count == 4) {
        splitInternal(parent);
    }
}

// =============== Unchanged traverse_queries ===============
void traverse_queries(MyTree* tree,
                      std::vector<std::tuple<MyTree::MyNode*, uint64_t>> queries,
                      int tid,
                      int threadcnt,
                      uint64_t* ret)
{
    uint64_t ans_sum = 0;
    //uint64_t ans_sum_golden = 0;

    int j = 0;

    for (size_t i = tid; i < queries.size(); i += threadcnt) {
        std::tuple<MyTree::MyNode*, uint64_t> q = queries[i];
        MyTree::MyNode* from = std::get<0>(q);
        uint64_t to = std::get<1>(q);

        MyTree::MyNode* nn = tree->Find(from->key);
        if (!nn) {
            // not found
        } else {
            uint64_t min_dist = nn->Distance(from->val);
            MyTree::MyNode* n = tree->Next(nn);
            while (n) {
                if (n->key <= to) {
                    uint32_t ndist = n->Distance(from->val);
                    if (ndist < min_dist) {
                        min_dist = ndist;
                    }
                } else {
                    break;
                }
                n = tree->Next(n);
            }
            std::cout << "Current min_dist: " << min_dist << std::endl; // Print min_dist
            ans_sum += min_dist;
            j ++;
            if (j == 10) exit(1);
        }

    //     auto it = tree->FindGolden(from->key);
    //     if (it == tree->GoldenEnd()) {
    //         // not found
    //     } else {
    //         uint64_t min_dist = (*it).second->Distance(from->val);
    //         it = tree->NextGolden(it);
    //         while (it != tree->GoldenEnd()) {
    //             if ((*it).second->key <= to) {
    //                 uint32_t ndist = (*it).second->Distance(from->val);
    //                 if (ndist < min_dist) {
    //                     min_dist = ndist;
    //                 }
    //             } else {
    //                 break;
    //             }
    //             it = tree->NextGolden(it);
    //         }
    //         ans_sum_golden += min_dist;
    //     }
    // }

    *ret = ans_sum;


    // if (ans_sum != ans_sum_golden) {
    //     printf("Results do not match the golden answer from std::multimap! %ld != %ld\n",
    //            ans_sum, ans_sum_golden);
    // }
    // printf( "%d/%d -- %lx %lx\n", tid, threadcnt, *ret, ans_sum );

    }
}