#pragma once

#include "node.h"
#include <vector>

class bintree
{
private:

	void insert(int value, node *n);
	//bool search(int value, node *n);

	void delete_tree(node *n);

	int _count;

	node *root;

public:
	bintree();
	~bintree();

	void insert(int value);
	//bool search(int value);

	void delete_tree();

	node getRoot();

	int getCount();
};
