#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>

#include "bintree.h"

int main(int argc, char **argv);
void traverse(node *n, std::ofstream *output);

int maxNodes = 0;
int distanceMode = 0;

int main(int argc, char **argv)
{
	if (argc < 5)
	{
		std::cerr << "Missing arguments!" << std::endl;

		std::cout << "gc++ <infile> <outfile> <node count> <distance mode>" << std::endl;

		return 0;
	}

	maxNodes = std::stoi(argv[3], nullptr, 10);
	distanceMode = std::stoi(argv[4], nullptr, 10);

	if (distanceMode > 2)
	{
		std::cerr << "Valid distance modes are: " << std::endl
				  << "  0 - all reachable" << std::endl
				  << "  1 - nothing reachable" << std::endl
				  << "  2 - all random" << std::endl;
		return 0;
	}

	bintree tree1;
	std::vector<int> saveHouses;

	srand((unsigned int)time(NULL));

	std::ofstream output;
	output.open(argv[1]);

	for (int i = 0; i < (maxNodes - 1); i++)
	{
		if (i == 0)
		{
			output << "0 " << maxNodes << " " << maxNodes + 1 << std::endl;
		}

		tree1.insert(i);
		
		if (rand() % 10000 > 9500)
		{
			saveHouses.push_back(i);
		}
	}

	traverse(&tree1.getRoot(), &output);

	tree1.delete_tree();

	int elements = (int)saveHouses.size();
	int *array = (int*)malloc(sizeof(int) * elements);

	for (int i = 0; i < elements; i++)
	{
		int sh = saveHouses.back();
		output << sh << std::endl;
		saveHouses.pop_back();

		array[i] = sh;
	}

	output.close();
	output.open(argv[2]);

	qsort(array, elements, sizeof(int), [](const void *a, const void *b) { return *((int*)a) - *((int*)b); });

	for (int i = 0; i < elements; i++)
	{
		output << array[i] << std::endl;
	}

	output.close();

	return 0;
}

void traverse(node *n, std::ofstream *output)
{
	if (NULL == n->left && NULL == n->right)
	{
		if(distanceMode == 0)
			*output << n->value << " " << maxNodes << " 1" << std::endl;
		else if(distanceMode == 1)
			*output << n->value << " " << maxNodes << " " << (maxNodes - 1) << std::endl;
		else
			*output << n->value << " " << maxNodes << " " << (rand() % maxNodes) << std::endl;

		return;
	}

	if (NULL != n->left)
	{
		if (distanceMode == 0)
			*output << n->value << " " << n->left->value << " 1" << std::endl;
		else if (distanceMode == 1)
			*output << n->value << " " << n->left->value << " " << (maxNodes - 1) << std::endl;
		else
			*output << n->value << " " << n->left->value << " " << (rand() % maxNodes) << std::endl;

		traverse(n->left, output);
	}
		

	if (NULL != n->right)
	{
		if (distanceMode == 0)
			*output << n->value << " " << n->right->value << " 1" << std::endl;
		else if (distanceMode == 1)
			*output << n->value << " " << n->right->value << " " << (maxNodes - 1) << std::endl;
		else
			*output << n->value << " " << n->right->value << " " << (rand() % maxNodes) << std::endl;

		traverse(n->right, output);
	}
}