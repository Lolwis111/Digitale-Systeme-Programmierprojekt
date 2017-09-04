#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

bool strToLong(char*, uint32_t*);

int main()
{
	char *str1 = "1 1 1";

	uint32_t val = 0;
	if (strToLong(str1, &val))
	{
		fprintf(stdout, "\n%u", val);
	}
	else
	{
		fputs("\nERROR\n", stderr);
	}

	return 0;
}

bool strToLong(char *str, uint32_t *result)
{
	uint64_t res = 0;

	if (NULL == str || *str < '0' || *str > '9') return false;

	res = 0;

	for (size_t i = 0; i < 11; i++)
	{
		if (*str < '0' || *str > '9') break;

		res *= 10;
		res += *str - '0';
		*str++;
	}

	if (res > 3999999999)
	{
		*result = UINT32_MAX;
		return false;
	}

	*result = (uint32_t)res;

	return true;
}