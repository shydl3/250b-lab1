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

#ifndef __MY_TREE_H__
#define __MY_TREE_H__
class MyTree {
	public:
	class MyNode {
		public:
		MyNode(uint64_t key, void* val, size_t valbytes);
		void* val;
		size_t valcnt;
		uint64_t key;
		//uint32_t Distance(MyNode* node);
		uint32_t Distance(void* val);


		MyNode* parent = NULL;
		MyNode* right = NULL;
		MyNode* left = NULL;

		private:

	};
	
	MyTree();
	void Insert(MyNode* node);
	MyNode* Find(uint64_t key);
	MyNode* Next(MyNode* node);
	std::multimap<uint64_t,MyNode*>::iterator FindGolden(uint64_t key);
	std::multimap<uint64_t,MyNode*>::iterator NextGolden(std::multimap<uint64_t,MyNode*>::iterator it);
	std::multimap<uint64_t,MyNode*>::iterator GoldenEnd() {return golden.end();}
	
	private:

	MyNode* root = NULL;
	std::multimap<uint64_t, MyNode*> golden;

};

void traverse_queries(MyTree* tree, std::vector<std::tuple<MyTree::MyNode*, uint64_t>> queries, int tid, int threadcnt, uint64_t* ret);

#endif
