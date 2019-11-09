#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

void update_leds()
{
	static uint32_t status = 1;
	*(volatile uint32_t*)0x20000000 = (status++) & 7;
}

void irq_handler(uint32_t irq_mask, uint32_t *regs)
{
	// IRQ 0: timer interrupt
	if (irq_mask & 1)
	{
		// run IRQ payload
		update_leds();

		// restart timer
		icosoc_timer(1000000);
	}

	// IRQ 1, IRQ 2: SBREAK, ILLINS, or BUSERROR
	if (irq_mask & 6)
	{
		printf("System error!\n");

		// regs[0] is the return address of the IRQ handler, i.e. it points to the instruction _after_
		// the one that caused the IRQ. This is a heuristic that tries to detect if the instruction
		// before that is a 16 bit opcode or a 32 bit opcode. It is not perfect.. (This is only a
		// problem when compressed ISA is enabled. This will always do the right thing when the IRQ
		// is triggered from uncompressed code.)

		uint16_t *pc = (void*)(regs[0] - 4);
		unsigned int instr = (pc[1] << 16) | pc[0];

		if ((instr & 3) != 3) {
			pc = (void*)(regs[0] - 2);
			instr = pc[0];
		}

		printf("\n------------------------------------------------------------\n");

		if ((irq_mask & 4) != 0)
			printf("Bus error in ");

		if ((irq_mask & 2) != 0) {
			if (instr == 0x00100073 || instr == 0x9002)
				printf("SBREAK ");
			else
				printf("Illegal ");
		}

		printf("instruction at %p: 0x%0*x\n", pc, ((instr & 3) == 3) ? 8 : 4, instr);

		for (int i = 0; i < 8; i++)
		for (int k = 0; k < 4; k++)
		{
			int r = i + k*8;

			if (r == 0)
				printf("pc  ");
			else
				printf("x%-2d ", r);

			printf("0x%08x%s", (unsigned int)regs[r], k == 3 ? "\n" : "    ");
		}

		// calling sbreak within the IRQ handler will halt the system
		printf("STOP.\n");
		icosoc_sbreak();
	}

	// IRQ 8: Extirq
	if (irq_mask & (1 << 8))
	{
		printf("External IRQ: PIN=%d\n", icosoc_demoirq_read());
	}
}

int main()
{
	// register IRQ handler
	icosoc_irq(irq_handler);

	// enable IRQs
	icosoc_maskirq(0);

	// start timer (IRQ 0)
	icosoc_timer(1000000);

	// trigger IRQ 8 on any edge on PMOD1_1
	icosoc_demoirq_set_config(icosoc_demoirq_trigger_fe | icosoc_demoirq_trigger_re);

#if 0
	// trigger IRQ 1 by calling sbreak
	icosoc_sbreak();
#endif

#if 0
	// trigger IRQ 2 by unaligned memory access
	// (pointer value set by inline asm, so compiler can't see alignment)
	int *p;
	asm volatile ("addi %0, zero, 1" : "=r" (p));
	printf("%d\n", *p);
#endif

#if 0
	// trigger IRQ 1 by illegal instruction
	asm volatile (".word 0");
#endif

	while (1) { }
}

