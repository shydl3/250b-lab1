#include "tree.h"

MyTree::MyNode::MyNode(uint64_t key, void* val, size_t valbytes) {
	this->key = key;
	this->val = malloc(valbytes);
	memcpy(this->val, val, valbytes);
	this->parent = NULL;
	this->right = NULL;
	this->left = NULL;
	this->valcnt = valbytes/sizeof(uint32_t);
}

//uint32_t MyTree::MyNode::Distance(MyTree::MyNode* node) {
uint32_t MyTree::MyNode::Distance(void* val) {
	uint32_t dist = 0;
	uint32_t* arr1 = (uint32_t*)this->val;
	uint32_t* arr2 = (uint32_t*)val;
	for ( size_t i = 0; i < this->valcnt; i++ ) {
		uint32_t d = (arr1[i] > arr2[i]) ? (arr1[i] - arr2[i]) : (arr2[i] - arr1[i]);
		dist += d;
	}
	return dist;
}

MyTree::MyTree() {
}

void 
MyTree::Insert(MyNode* node) {
	golden.insert(std::pair<uint64_t,MyTree::MyNode*>(node->key,node));

	if ( root == NULL ) {
		root = node;
		return;
	}

	MyNode* curpointer = root;
	bool done = false;
	while(!done) {
		if ( node->key >= curpointer->key ) {
			if ( curpointer->right == NULL ) {
				curpointer->right = node;
				node->parent = curpointer;
				done = true;
			} else {
				curpointer = curpointer->right;
			}
		} else {
			if ( curpointer->left == NULL ) {
				curpointer->left = node;
				node->parent = curpointer;
				done = true;
			} else {
				curpointer = curpointer->left;
			}
		}
	}
}
	
std::multimap<uint64_t,MyTree::MyNode*>::iterator 
MyTree::FindGolden(uint64_t key) {
	return golden.find(key);
}
std::multimap<uint64_t,MyTree::MyNode*>::iterator 
MyTree::NextGolden(std::multimap<uint64_t,MyTree::MyNode*>::iterator it) {
	return ++it;
}

MyTree::MyNode* 
MyTree::Find(uint64_t key) {
	MyNode* curpointer = root;
	while(true) {
		if ( key == curpointer->key ) {
			if ( curpointer->left != NULL && curpointer->left->key == key ) {
				curpointer = curpointer->left;
			} else return curpointer;
		} else if ( key > curpointer->key ) {
			if ( curpointer->right == NULL ) {
				return NULL;
			} else {
				curpointer = curpointer->right;
			}
		} else {
			if ( curpointer->left == NULL ) {
				return NULL;
			} else {
				curpointer = curpointer->left;
			}
		}
	}
}

MyTree::MyNode* 
MyTree::Next(MyTree::MyNode* node) {
	if ( !node ) return NULL;

	MyTree::MyNode* curpointer = node;

	if ( node->right != NULL ) {
		curpointer = node->right;
		while (curpointer->left) {
			curpointer = curpointer->left;
		}
		return curpointer;
	}
	// we are rightmost of th current branch

	while( true ) {
		if ( curpointer->parent == NULL ) return NULL;

		if ( curpointer->parent->right == curpointer ) {
			curpointer = curpointer->parent;
			continue;
		}

		curpointer = curpointer->parent;
		break;
	}

	return curpointer;
}

void traverse_queries(MyTree* tree, std::vector<std::tuple<MyTree::MyNode*, uint64_t>> queries, int tid, int threadcnt, uint64_t* ret) {
	uint64_t ans_sum = 0;
	uint64_t ans_sum_golden = 0;
	for ( size_t i = tid; i < queries.size(); i += threadcnt ) {
		std::tuple<MyTree::MyNode*, uint64_t> q = queries[i];
		MyTree::MyNode* from = std::get<0>(q);
		uint64_t to = std::get<1>(q);
		//printf( "Query: %lx %lx\n", from->key,  to );

		MyTree::MyNode* nn = tree->Find(from->key);
		if ( nn == NULL ) {
			//printf( "Not found!\n" );
		} else {
			uint64_t min_dist = nn->Distance(from->val);
			
			MyTree::MyNode* n = tree->Next(nn);
			while ( n != NULL ) {
				if ( n->key <= to ) {
					uint32_t ndist = n->Distance(from->val);
					if ( ndist < min_dist ) min_dist = ndist;
					//printf( "%lx ", n->key );
				}else break;
				n = tree->Next(n);
			}
			ans_sum += min_dist;
			//printf( "Found! min_dist: %lu\n", min_dist );
		}


		std::multimap<uint64_t,MyTree::MyNode*>::iterator it = tree->FindGolden(from->key);
		if ( it == tree->GoldenEnd() ) {
		} else {
			uint64_t min_dist = (*it).second->Distance(from->val);
			it = tree->NextGolden(it);

			while ( it != tree->GoldenEnd() ) {
				if ( (*it).second->key <= to ) {
					uint32_t ndist = (*it).second->Distance(from->val);
					if ( ndist < min_dist ) min_dist = ndist;
					//printf( "%lx ", (*it).second->key );
				} else break;
				
				it = tree->NextGolden(it);
			}
			ans_sum_golden += min_dist;
			//printf( "Golden Found! min_dist: %lu\n", min_dist );
		}

	}
	*ret = ans_sum;
	if ( ans_sum != ans_sum_golden ) {
		printf( "Results do not match the golden answer from std::multimap! %ld != %ld\n", ans_sum, ans_sum_golden );
	}
	//printf( "%d/%d -- %lx %lx\n", tid, threadcnt, *ret, ans_sum );
}

