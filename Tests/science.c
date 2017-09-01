#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define SIZE 1024
#define DATA 128

bool checkNumber(int number, bool *bloomFilter);
void registerNumber(int number, bool *bloomFilter);

int main()
{
	bool *bloomFilter = (bool*)calloc(SIZE, sizeof(bool));

	int *data = (int*)calloc(DATA, sizeof(bool));

	int i;
	for (i = 0; i < DATA; i++)
	{
		int num = rand() % DATA;
		registerNumber(num, bloomFilter);
		data[i] = num;
	}

	int number = 0;
	do
	{
		scanf_s("%i", &number);

		if (checkNumber(number, bloomFilter)) puts("maybe");
		else puts("definitiv nain");

	} while (number != -1);

	free(bloomFilter);
	free(data);
}

void registerNumber(int number, bool *bloomFilter)
{
	bloomFilter[number % SIZE] = true;
}

bool checkNumber(int number, bool *bloomFilter)
{
	return bloomFilter[number % SIZE];
}