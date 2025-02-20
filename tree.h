#ifndef __MY_TREE_H__
#define __MY_TREE_H__

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <chrono>
#include <vector>
#include <tuple>
#include <thread>
#include <map>

class MyTree {
public:
    // Forward-declare MyNode so BPlusNode can store pointers to it.
    class MyNode;

    // Nested B+ tree node structure
    class BPlusNode {
    public:
        bool     isLeaf;
        int      count;         
        uint64_t keys[4];       
        // For an internal node, children[i] is another BPlusNode
        // For a leaf, children[] are not used
        BPlusNode* children[5]; 

        // For leaf nodes only, we store actual data in records
        MyNode*   records[4];  

        BPlusNode* next;   // next leaf in the leaf-level linked list
        BPlusNode* parent; // parent pointer for splits

        // Simple constructor
        BPlusNode(bool leaf) {
            isLeaf = leaf;
            count  = 0;
            parent = nullptr;
            next   = nullptr;
            for(int i=0; i<4; i++){
                keys[i]      = 0;
                records[i]   = nullptr;
                children[i]  = nullptr;
            }
            children[4] = nullptr;
        }
    };

    // The data object we store in the leaves
    class MyNode {
    public:
        MyNode(uint64_t key, void* val, size_t valbytes);
        // Distance function as before
        uint32_t Distance(void* val);

        // Key + Value
        uint64_t key;
        void*    val;
        size_t   valcnt;

        // BST-related fields (unused now, but kept for main.cpp compatibility)
        MyNode*  parent;
        MyNode*  left;
        MyNode*  right;

        // B+ bookkeeping: which leaf, and at which index
        BPlusNode* leaf;
        int         leafIndex;
    };

    // Public methods
    MyTree();
    void Insert(MyNode* node);
    MyNode* Find(uint64_t key);
    MyNode* Next(MyNode* node);

    // For "golden" reference checking
    std::multimap<uint64_t, MyNode*>::iterator FindGolden(uint64_t key);
    std::multimap<uint64_t, MyNode*>::iterator NextGolden(std::multimap<uint64_t, MyNode*>::iterator it);
    std::multimap<uint64_t, MyNode*>::iterator GoldenEnd() { return golden.end(); }

private:
    // Root of the B+ tree
    BPlusNode* root = nullptr;

    // The golden multimap
    std::multimap<uint64_t, MyNode*> golden;

    // B+ tree private helpers
    BPlusNode* findLeaf(uint64_t key);
    void insertInLeaf(BPlusNode* leaf, MyNode* record);
    void splitLeaf(BPlusNode* leaf);
    void splitInternal(BPlusNode* internal);
};

// Same traversal function as before
void traverse_queries(MyTree* tree,
                      std::vector<std::tuple<MyTree::MyNode*, uint64_t>> queries,
                      int tid,
                      int threadcnt,
                      uint64_t* ret);

#endif

