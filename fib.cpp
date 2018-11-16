#include <iostream>											//my model used for ACM Contest
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
#include <cmath>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <queue>
#include <functional>
#include <list>
#include <set>
#include <sstream>
#define ll long long
#define INF 1000000007

using namespace std;

struct Fib
{
	Fib *left_child, *father, *left_sibling, *right_sibling;
	bool childcut;
	ll value;												//the count of each word
	int degree;
	string name;

	Fib(){
		left_child = nullptr;								//initialize the values
		father = nullptr;
		left_sibling = nullptr;
		right_sibling = nullptr;
		childcut = false;
		degree = 0;
		value = 0;
	}
};

Fib *node_max, *start;

unordered_map<string, Fib*> hash_map;						//use the STL unordered map as the hash_map

void join_subtree(Fib *subroot){							//add the subtree to the left of the start node
	subroot->left_sibling = start->left_sibling;
	subroot->right_sibling = start;
	start->left_sibling->right_sibling = subroot;
	start->left_sibling = subroot;
	subroot->father = nullptr;
	subroot->childcut = false;
}

void add_new_keyword(string name, int cnt){					//simply make a one node subtree and join it to the heap
	Fib *new_node = new Fib();
	new_node->value = cnt;
	new_node->name = name;
	hash_map[name] = new_node;
	join_subtree(new_node);
	if(new_node->value > node_max->value)
		node_max = new_node;
}

void remove_node(Fib *node){
	node->left_sibling->right_sibling = node->right_sibling;	//remove the node from the list
	node->right_sibling->left_sibling = node->left_sibling;
	if(node->father != nullptr)									//check if the node is tree root
	{
		if(node->value > node_max->value)
			node_max = node;
		if(node->father->left_child == node)
			node->father->left_child = node->right_sibling;
		node->father->degree --;
		if(node->father->degree == 0)
			node->father->left_child = nullptr;
		node->father = nullptr;
		join_subtree(node);										//if node is not root, join it to the heap
	}
	else{
		hash_map.erase(node->name);								//erase the node from the hash map
		delete(node);											//release the memory
	}
}

Fib* tree_meld(Fib *a, Fib *b){
	if(a->father != nullptr || b->father != nullptr || a->degree != b->degree){
		cerr << "meld error!" << endl;							//error check
		assert(0);
	}
	if(a->value > b->value)
		swap(a, b);												//make the node a has smaller value
	a->father = b;												//add a as the leftest child of b
	b->degree ++;
	a->left_sibling->right_sibling = a->right_sibling;
	a->right_sibling->left_sibling = a->left_sibling;
	if(b->left_child != nullptr){
		a->right_sibling = b->left_child;
		a->left_sibling = b->left_child->left_sibling;
		b->left_child->left_sibling->right_sibling = a;
		b->left_child->left_sibling = a;
		b->left_child = a;
	}
	else{
		a->left_sibling = a;
		a->right_sibling = a;
		b->left_child = a;
	}
	return b;
}

void add_count(Fib *node, int cnt){
	node->value += cnt;
	if(node->father != nullptr && node->value > node->father->value)
	{
		while(true){										//cascading cut
			Fib *fa = node->father;
			if(fa == nullptr)
				break;
			remove_node(node);
			if(!fa->childcut){
				if(fa->father != nullptr)
					fa->childcut = true;
				break;
			}
		}
	}
}

Fib *degs[64] = {0};
vector<pair<string,int>> sts;

string remove_max(){
	string name = node_max->name;
	for (Fib *i = node_max->left_child;;){					//join the subtree of the max node to the heap				
		if(i == nullptr) break;
		Fib *nxt = i->right_sibling;
		join_subtree(i);
		if(nxt == node_max->left_child) break;
		i = nxt;
	}
	sts.push_back(make_pair(node_max->name, node_max->value));  //store the node to reinsert later
	remove_node(node_max);									//remove the node with max value

	for (Fib *i = start->left_sibling; i != start; ){
		Fib *nxt = i->left_sibling;
		int deg = i->degree;
		if(degs[deg] != nullptr){							//merge the subtree with same degree
			Fib *res = tree_meld(degs[deg], i);
			degs[deg] = nullptr;
			while(degs[++deg] != nullptr){					//continiously checking.
				res = tree_meld(degs[deg], res);
				degs[deg] = nullptr;
			}
			degs[deg] = res;
		}
		else
			degs[deg] = i;
		i = nxt;
	}

	node_max = start;										//update the pointer of the max value node.
	for (Fib *i = start->left_sibling; i != start; i = i->left_sibling){
		if(i->value > node_max->value)
			node_max = i;
	}
	memset(degs, 0, sizeof(degs));							//reset the list of trees
	return name;
}

int main(int argc, char** argv)
{
	ios::sync_with_stdio(false);							//speed up stream IO (fin/fout)

	if(argc != 2){											//check arguments
		cerr << "Argument error!" << endl;
		return 0;
	}
	string filename = argv[1];
	ifstream fin(filename);
	ofstream fout("output_file.txt");

	start = new Fib();										//init start node
	start->left_sibling = start;
	start->right_sibling = start;
	start->name = "*start";
	node_max = start;
	do
	{
		string line;
		getline(fin, line);									//read each line from input file
		if(line[0] == '$')									//check the first character
		{
			string str;
			int count; 
			stringstream ss;								//use stringstream to separate the line
			ss << line;
			ss >> str >> count;
			str = str.substr(1);
			if(hash_map[str] == 0)							
				add_new_keyword(str, count);
			else
				add_count(hash_map[str], count);
		}
		if(line[0] >= '0' && line[0] <= '9')
		{
			int count;
			stringstream ss;
			ss << line;
			ss >> count;
			fout << remove_max();							//simply remove the largest nodes from heap and reinsert them.
			for (int i = 1; i < count; i++)
				fout << ',' << remove_max();
			fout << endl;

			for(auto i: sts)
				add_new_keyword(i.first, i.second);
			sts.clear();
		}
		if(line[0] == 's')
			break;
	}
	while(true);

	return 0;
}
