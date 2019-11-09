#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "icosoc.h"
#include "sprites.h"

int ledstatus = 0;

void irq_handler(uint32_t irq_mask, uint32_t *regs)
{
	if (irq_mask & 1) {
		ledstatus = ledstatus >= 30 ? 1 : ledstatus+1;
		if (ledstatus > 15)
			icosoc_ledstrip_set(1 << (30-ledstatus));
		else
			icosoc_ledstrip_set(1 << ledstatus);
		icosoc_timer(1000000);
	}
}

int main()
{
	volatile uint32_t *videomem = (void*)(0x20000000 + 1 * 0x10000);

	icosoc_ledstrip_dir(0xffff);
	icosoc_irq(irq_handler);
	icosoc_maskirq(0);
	icosoc_timer(1000000);

	setbuf(stdout, NULL);
	putchar(0);

	for (int k = 0;; k++)
	{
		volatile uint32_t *panel0 = videomem;
		volatile uint32_t *panel1 = videomem;

		if (k % 2 == 0)
			panel0 += 1024;
		else
			panel1 += 1024;

		// statically display sprite0 on one panel
		for (int i = 0; i < 1024; i++)
			panel0[i] = sprite0[i];

		// animate sprite1/sprite2 on other panel
		for (int l = 0; l < 3; l++)
		{
			icosoc_leds((1 << (k%2)) | 4);

			for (int i = 0; i < 1024; i++)
				panel1[i] = sprite1[i];

			icosoc_leds(1 << (k%2));

			for (int i = 0; i < 1000000; i++)
				asm volatile ("" ::: "memory");

			icosoc_leds((1 << (k%2)) | 4);

			for (int i = 0; i < 1024; i++)
				panel1[i] = sprite2[i];

			icosoc_leds(1 << (k%2));

			for (int i = 0; i < 1000000; i++)
				asm volatile ("" ::: "memory");
		}
	}
}

