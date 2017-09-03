#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define SIZE 1000000

typedef struct pair
{
	uint32_t key;
	uint32_t value;
} pair;

int main()
{
	pair* hashmap = (pair*)malloc(sizeof(pair) * SIZE);

	if (NULL == hashmap) return 1;

	for (uint32_t i = 0; i < SIZE; i++) hashmap[i].key = UINT32_MAX;

	for (uint32_t i = 0; i < SIZE; i++)
	{
		uint32_t m = i % SIZE;

		while (hashmap[m % SIZE].key != UINT32_MAX)
		{
			m++;
		}

		hashmap[m % SIZE].key = m % SIZE;
		hashmap[m % SIZE].value = i;

		printf("%u ", m % SIZE);
	}

	puts("\n");

	for (uint32_t i = 0; i < SIZE; i++)
	{
		printf("(%u; %u) ", hashmap[i].key, hashmap[i].value);
	}

	return 0;
}