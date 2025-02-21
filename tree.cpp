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
        newChild->next = child->next;
        child->next = newChild;
    }
}


void MyTree::insertNonFull(BPlusNode* node, uint64_t key) {
    if (node->isLeaf) {
        node->keys.insert(upper_bound(node->keys.begin(),
        node->keys.end(),
        key),
key);
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
        insertNonFull(node->children[i], key);
    }
}

void MyTree::Insert(MyNode* nn) {
    if (root == nullptr) {
        root = new BPlusNode(true);
        root->keys.push_back(nn->key);
    }
    else {
        if (root->keys.size() == 2 * BPT_ORDER - 1) {
            BPlusNode* newRoot = new BPlusNode();
            newRoot->children.push_back(root);
            MyTree::SplitChild(newRoot, 0, root);
            root = newRoot;
        }
        MyTree::insertNonFull(root, nn->key);
    }

}
