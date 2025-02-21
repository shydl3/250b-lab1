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
#include <algorithm>

#define BPT_ORDER 4

class MyTree {
    public:
    // MyTree::MyNode is the original BST node obj
    // keep for compatability
    class MyNode{
        public:
        MyNode(uint64_t key, void* val, size_t valbytes);
        void* val;
        uint64_t key;
        size_t valcnt;
        uint32_t Distance(void* val);


		MyNode* parent = NULL;
		MyNode* right = NULL;
		MyNode* left = NULL;
    };


    class BPlusNode {
        public:
        bool isLeaf;
        BPlusNode* parent;
        BPlusNode* next;
        
        std::vector<uint64_t> keys; // 存储的键值
        std::vector<BPlusNode*> children; // 内部节点的子指针（仅内部节点使用

        std::vector<void*> values; // 叶子节点存储数据（仅叶子节点使用）
        std::vector<size_t> valbytes; // 存储每个 values[i] 的字节大小

        BPlusNode(bool leaf = false): isLeaf(leaf) {}

        uint32_t Distance(void* val);

    };



    MyTree();
    void Insert(MyNode* nn);

    BPlusNode* root;
    BPlusNode* FindLeaf(uint64_t key);
    
    void InsertIntoLeaf(BPlusNode* leaf, uint64_t key, void* val, size_t valcnt);

    void SplitChild(BPlusNode* parent, int index, BPlusNode* child);
    void insertNonFull(BPlusNode* node, uint64_t key);

};

void traverse_queries();


#endif
