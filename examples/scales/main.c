#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

#define SCALE_B_CSN  0x80
#define SCALE_B_DIN  0x40
#define SCALE_B_DOUT 0x20
#define SCALE_B_SCLK 0x10

#define SCALE_A_CSN  0x08
#define SCALE_A_DIN  0x04
#define SCALE_A_DOUT 0x02
#define SCALE_A_SCLK 0x01

int scale_a = 0, scale_b = 0;
int scale_a_min = 0xffffff;
int scale_b_min = 0xffffff;

void scales_init()
{
	icosoc_scales_set(SCALE_B_CSN | SCALE_B_DIN | SCALE_B_SCLK | SCALE_A_CSN | SCALE_A_DIN | SCALE_A_SCLK);
	icosoc_scales_dir(SCALE_B_CSN | SCALE_B_DIN | SCALE_B_SCLK | SCALE_A_CSN | SCALE_A_DIN | SCALE_A_SCLK);
	icosoc_scales_set(              SCALE_B_DIN | SCALE_B_SCLK |               SCALE_A_DIN | SCALE_A_SCLK);
}

bool scales_ready()
{
	return (icosoc_scales_get() & (SCALE_B_DOUT | SCALE_A_DOUT)) == 0;
}

void scales_read()
{
	for (int i = 0, cmd = 0x38; i < 8; i++) {
		icosoc_scales_set(((cmd & 0x80) ? (SCALE_B_DIN | SCALE_A_DIN) : 0));
		icosoc_scales_set(((cmd & 0x80) ? (SCALE_B_DIN | SCALE_A_DIN) : 0) | SCALE_B_SCLK | SCALE_A_SCLK);
		cmd = cmd << 1;
	}

	scale_a = 0, scale_b = 0;
	for (int i = 0; i < 24; i++) {
		icosoc_scales_set(SCALE_B_DIN | SCALE_A_DIN);
		icosoc_scales_set(SCALE_B_DIN | SCALE_A_DIN | SCALE_B_SCLK | SCALE_A_SCLK);
		uint32_t scale_bits = icosoc_scales_get();
		scale_a = (scale_a << 1) | ((scale_bits & SCALE_A_DOUT) != 0);
		scale_b = (scale_b << 1) | ((scale_bits & SCALE_B_DOUT) != 0);
	}
}

int main()
{
	icosoc_ledstrip_dir(0xffff);
	scales_init();

	for (int i = 0;; i++)
	{
		while (!scales_ready()) { /* wait */ }
		scales_read();

		if (scale_a < scale_a_min)
			scale_a_min = scale_a;

		if (scale_b < scale_b_min)
			scale_b_min = scale_b;

		int a = scale_a - scale_a_min;
		int b = scale_b - scale_b_min;
		int p = (20 * a) / (a+b+1) - 2;

		if (p < 0) p = 0;
		if (p > 15) p = 15;

		icosoc_ledstrip_set(1 << p);

		if (i % 32 == 0) {
			printf("---------------\n");
			printf("Scale A: %06x (min=%06x)\n", scale_a, scale_a_min);
			printf("Scale B: %06x (min=%06x)\n", scale_b, scale_b_min);
			printf("Position: %d\n", p);
		}
	}
}

