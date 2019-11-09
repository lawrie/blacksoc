#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "icosoc.h"

uint32_t colors[9] = {
	0x000000,
	0xff0000,
	0x00ffff,
	0x00ff00,
	0x660066,
	0x2222ff,
	0xffff00,
	0xff0000,
	0xffffff
};

uint32_t banner_data[5*96];
int banner_pos = 0;

const char *banner[5] = {
	"#..........##..#..................***..*..**..**....*...*..@..........@@@...................@...",
	"#..##..##.#...###..##..#.##.#.#...*..*.*.*...*......*...*..@..@@..@@..@..@..@@...@@@.@.@@.@@@...",
	"#.#...#.#..#...#..#..#.##..#.#.#..***..*..*..*...**..*.*...@.@...@..@.@@@..@..@.@..@.@@..@..@...",
	"#.#...##....#..#..#..#.#...#.#.#..*..*.*...*.*.......*.*...@.@...@..@.@..@.@..@.@.@@.@...@..@...",
	"#..##..##.##...##..##..#...#.#.#..*..*.*.**...**......*....@..@@..@@..@@@...@@...@@@.@....@@@..."
};

int stackptr[3];
int stacks[3][8];

void draw_banner(int offset)
{
	uint32_t *pixel = (void*)(0x20010000 + 8);
	const uint32_t *dat = banner_data;

	for (int i = 0; i < 5; i++)
	{
		for (int k = 0, p = offset; k < 32; k++) {
			*pixel = dat[p];
			pixel += 32;
			if (++p == 96) p = 0;
		}
		pixel -= 32*32 - 1;
		dat += 96;
	}
}

void draw_stack(int idx, int x, int y)
{
	uint32_t *pixel = (void*)(0x20010000 + 4*(32*(x-8) + y));

	for (int i = 0; i < 8; i++)
	{
		int q = stacks[idx][i];
		uint32_t c = colors[q];

		for (int k = -8; k < -q; k++, pixel += 32)
			*pixel = 0;

		for (int k = -q; k < q; k++, pixel += 32)
			*pixel = c;

		for (int k = q; k < 8; k++, pixel += 32)
			*pixel = 0;

		pixel -= 16*32 + 1;
	}
}

void update_screen()
{
	draw_stack(0, 8, 29);
	draw_stack(1, 24, 29);
	draw_stack(2, 16, 20);
}

void hanoi(int from, int to, int via, int n)
{
	if (n > 0)
		hanoi(from, via, to, n-1);

	int from_ptr = --stackptr[from];
	int to_ptr = stackptr[to]++;
	int k = stacks[from][from_ptr];
	stacks[from][from_ptr] = 0;
	stacks[to][to_ptr] = k;

	update_screen();

	for (int k = 0; k < 10; k++)
	{
		draw_banner(banner_pos);

		if (++banner_pos == 96)
			banner_pos = 0;

		for (int i = 0; i < 100000; i++)
			asm volatile ("");
	}

	if (n > 0)
		hanoi(via, to, from, n-1);
}

int main()
{
	stackptr[0] = 7;
	stackptr[1] = 0;
	stackptr[2] = 0;

	for (int i = 0; i < 8; i++) {
		stacks[0][i] = i > 6 ? 0 : 7 - i;
		stacks[1][i] = 0;
		stacks[2][i] = 0;
	}

	for (int y = 0; y < 5; y++)
	for (int x = 0; x < 96; x++)
		switch (banner[y][x]) {
			case '.': banner_data[96*y+x] = 0x000000; break;
			case '#': banner_data[96*y+x] = 0xff0000; break;
			case '*': banner_data[96*y+x] = 0x00ff00; break;
			case '@': banner_data[96*y+x] = 0x2222ff; break;
		}
	
	for (int x = 0; x < 16; x++) {
		icosoc_panel_setpixel(   x, 30, 64, 64, 64);
		icosoc_panel_setpixel(16+x, 30, 64, 64, 64);
		icosoc_panel_setpixel( 8+x, 21, 64, 64, 64);
	}

	while (1) {
		hanoi(0, 2, 1, 6);
		hanoi(2, 0, 1, 6);
	}
}

