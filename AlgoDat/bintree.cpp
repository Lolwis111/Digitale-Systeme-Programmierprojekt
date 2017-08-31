#include "bintree.h"
#include <cstdlib>

bintree::bintree()
{
	root = NULL;
	_count = 0;
}

bintree::~bintree()
{
	delete_tree(root);

	_count = 0;
}

void bintree::delete_tree()
{
	delete_tree(root);

	_count = 0;
}

void bintree::delete_tree(node *n)
{
	if (NULL != n)
	{
		delete_tree(n->left);
		delete_tree(n->right);
		 
		n = NULL;
		delete n;
	}
}

int bintree::getCount()
{
	return _count;
}

void bintree::insert(int value)
{
	if (NULL == root)
	{
		root = new node;
		root->value = value;
		root->left = NULL;
		root->right = NULL;
	}
	else
	{
		insert(value, root);
	}

	_count++;
}

void bintree::insert(int value, node *n)
{
	int val = rand() % 10000;

	if (val < 5000)
	{
		if (NULL != n->left)
		{
			insert(value, n->left);
		}
		else
		{
			n->left = new node;
			n->left->value = value;
			n->left->left = NULL;
			n->left->right = NULL;
		}
	}
	else
	{
		if (NULL != n->right)
		{
			insert(value, n->right);
		}
		else
		{
			n->right = new node;
			n->right->value = value;
			n->right->left = NULL;
			n->right->right = NULL;
		}
	}
}

/*
bool bintree::search(int value)
{
	return search(value, root);
}

bool bintree::search(int value, node *n)
{
	if (n == NULL)
	{
		return false;
	}
	else
	{
		if (value == n->value) return true;
		if (value < n->value) return search(value, n->left);
		else return search(value, n->right);
	}
}
*/

node bintree::getRoot()
{
	return *root;
}