#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{
	for (uint16_t i = 0; i <= 0xfff; i++)
	{
		icosoc_ssd0_set(i);
		printf("[%03x] 7-segment test\n", i);

		for (int i = 0; i < 100000; i++)
			asm volatile ("");
	}
}

