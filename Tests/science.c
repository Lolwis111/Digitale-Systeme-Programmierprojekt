#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

int main()
{
	printf("%i\n", isdigit(1));
	printf("%i\n", isdigit('1'));

	return 0;
}