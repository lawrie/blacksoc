#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "icosoc.h"

#undef DISPLAY_FLIP_X
#undef DISPLAY_FLIP_Y
#define DISPLAY_SWAP_XY

#define MIN_WEIGHT 20000
#define SUB_WEIGHT  1500

#define SCALE_B_CSN  0x80
#define SCALE_B_DIN  0x40
#define SCALE_B_DOUT 0x20
#define SCALE_B_SCLK 0x10

#define SCALE_A_CSN  0x08
#define SCALE_A_DIN  0x04
#define SCALE_A_DOUT 0x02
#define SCALE_A_SCLK 0x01

int scale_a, scale_b;
int scale_init_a, scale_init_b;

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

#define COLOR_BLACK   0x000000
#define COLOR_RED     0xff0000
#define COLOR_GREEN   0x00ff00
#define COLOR_BLUE    0x0000ff
#define COLOR_YELLOW  0xffff00
#define COLOR_CYAN    0x00ffff
#define COLOR_MAGENTA 0xff00ff
#define COLOR_WHITE   0xffffff

uint32_t block_colors[] = {
	COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA
};

uint32_t all_colors[] = {
	COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_CYAN, COLOR_MAGENTA, COLOR_WHITE
};

int block_map[12][12];

void setpixel(int x, int y, uint32_t color)
{
	if (x < 0 || x >= 3*32) return;
	if (y < 0 || y >= 3*32) return;

#ifdef DISPLAY_FLIP_X
	x = 3*32-1-x;
#endif

#ifdef DISPLAY_FLIP_Y
	y = 3*32-1-y;
#endif

#ifdef DISPLAY_SWAP_XY
	int t = x;
	x = y, y = t;
#endif

	int px = x / 32;
	int py = y / 32;

	x = x % 32;
	y = 31 - (y % 32);
	x = x + 32*(px + 3*py);

	*(uint32_t*)(0x20000000 + 1 * 0x10000 + 4*(32*x + y)) = color;
}

void draw_ball(int x, int y)
{
	for (int cx = 0; cx < 4; cx++)
	for (int cy = 0; cy < 4; cy++)
		if ((cx != 0 && cx != 3) || (cy != 0 && cy != 3))
			setpixel(x+cx, y+cy, COLOR_RED);
}

void erase_ball(int x, int y)
{
	for (int cx = 0; cx < 4; cx++)
	for (int cy = 0; cy < 4; cy++)
		if ((cx != 0 && cx != 3) || (cy != 0 && cy != 3))
			setpixel(x+cx, y+cy, COLOR_BLACK);
}

void draw_block(int x, int y, uint32_t color)
{
	for (int cx = 8*x; cx < 8*(x+1); cx++)
	for (int cy = 4*y+8; cy < 4*(y+1)+8; cy++)
		setpixel(cx, cy, color);

	// setpixel(8*x+0, 4*y+0, COLOR_BLACK);
	// setpixel(8*x+7, 4*y+0, COLOR_BLACK);
	// setpixel(8*x+0, 4*y+3, COLOR_BLACK);
	// setpixel(8*x+7, 4*y+3, COLOR_BLACK);
}

bool erase_block_group(int x, int y, int refcolor)
{
	if (x < 0 || x >= 12 || y < 0 || y >= 12)
		return false;

	if (block_map[x][y] == 0)
		return false;

	if (refcolor == 0)
		refcolor = block_map[x][y];

	if (refcolor != block_map[x][y])
		return false;

	draw_block(x, y, COLOR_BLACK);
	block_map[x][y] = 0;

	erase_block_group(x-1, y, refcolor);
	erase_block_group(x+1, y, refcolor);

	erase_block_group(x, y-1, refcolor);
	erase_block_group(x, y+1, refcolor);

	return true;
}

void draw_paddle(int x)
{
	setpixel(x+1, 3*32-2, COLOR_RED);
	setpixel(x+2, 3*32-2, COLOR_BLUE);
	setpixel(x+3, 3*32-2, COLOR_BLUE);
	setpixel(x+4, 3*32-2, COLOR_BLUE);
	setpixel(x+5, 3*32-2, COLOR_BLUE);
	setpixel(x+6, 3*32-2, COLOR_BLUE);
	setpixel(x+7, 3*32-2, COLOR_BLUE);
	setpixel(x+8, 3*32-2, COLOR_BLUE);
	setpixel(x+9, 3*32-2, COLOR_BLUE);
	setpixel(x+10, 3*32-2, COLOR_RED);

	setpixel(x+0, 3*32-1, COLOR_BLUE);
	setpixel(x+1, 3*32-1, COLOR_BLUE);
	setpixel(x+2, 3*32-1, COLOR_BLUE);

	setpixel(x+9, 3*32-1, COLOR_BLUE);
	setpixel(x+10, 3*32-1, COLOR_BLUE);
	setpixel(x+11, 3*32-1, COLOR_BLUE);
}

void erase_paddle(int x)
{
	for (int cx = x+1; cx < x+11; cx++)
		setpixel(cx, 3*32-2, COLOR_BLACK);

	for (int cx = x; cx < x+12; cx++)
		setpixel(cx, 3*32-1, COLOR_BLACK);
}

uint32_t xorshift32()
{
	static uint32_t x32 = 314159265;
	x32 ^= x32 << 13;
	x32 ^= x32 >> 17;
	x32 ^= x32 << 5;
	return x32;
}

void game()
{
	for (int by = 0; by < 12; by++)
	for (int bx = 0; bx < 12; bx++)
		block_map[bx][by] = 0;

	int x = 3, y = 3*32-30;
	int dx = 1, dy = -1;
	int paddle = 3*32/2-4;
	bool rebuild = false;
	bool hitsomething = false;

	while (1)
	{
		while (!scales_ready()) { /* wait */ }
		scales_read();

		// printf("%8d %8d\n", scale_a - scale_init_a, scale_b - scale_init_b);

		bool bounce_x = false, bounce_y = false;

		if (x == 0 && dx < 0) bounce_x = true;
		if (x+4 == 3*32 && dx > 0) bounce_x = true;

		if (y == 8 && dy < 0)
		{
			bounce_y = true;

			rebuild = true;
			for (int by = 0; by < 12; by++)
			for (int bx = 0; bx < 12; bx++)
				if (block_map[bx][by])
					rebuild = false;
		}

		if (rebuild && y % 4 == 0 && y > 8)
		{
			int by = (y / 4) - 3;

			for (int bx = 0; bx < 12; bx++)
			{
				block_map[bx][by] = (xorshift32() % (1 + sizeof(block_colors)/sizeof(*block_colors)));

				if (block_map[bx][by] != 0)
					draw_block(bx, by, block_colors[block_map[bx][by] - 1]);
			}

			if (by == 11)
				rebuild = false;
		}

		if (y > 3*32 + 8)
			return;

		if (dy < 0 && y > 8 && y%4 == 0)
		{
			int by = (y / 4) - 3;
			int bx1 = x / 8;
			int bx2 = (x+3) / 8;

			if (erase_block_group(bx1, by, 0))
				hitsomething = true, bounce_y = true;
			if (erase_block_group(bx2, by, 0))
				hitsomething = true, bounce_y = true;
		}

		if (dy > 0 && y+4 < 3*32 && y%4 == 0)
		{
			int by = (y / 4) - 1;
			int bx1 = x / 8;
			int bx2 = (x+3) / 8;

			if (erase_block_group(bx1, by, 0))
				hitsomething = true, bounce_y = true;
			if (erase_block_group(bx2, by, 0))
				hitsomething = true, bounce_y = true;
		}

		if (dx < 0 && x > 0 && x%8 == 0)
		{
			int bx = (x / 8) - 1;
			int by1 = y / 4 - 2;
			int by2 = (y+3) / 4 - 2;

			if (erase_block_group(bx, by1, 0))
				hitsomething = true, bounce_x = true;
			if (erase_block_group(bx, by2, 0))
				hitsomething = true, bounce_x = true;
		}

		if (dx > 0 && x+4 < 3*32 && x%8 == 4)
		{
			int bx = (x / 8) + 1;
			int by1 = y / 4 - 2;
			int by2 = (y+3) / 4 - 2;

			if (erase_block_group(bx, by1, 0))
				hitsomething = true, bounce_x = true;
			if (erase_block_group(bx, by2, 0))
				hitsomething = true, bounce_x = true;
		}

		erase_ball(x, y);
		erase_paddle(paddle);

		int weight_a = scale_a - scale_init_a - SUB_WEIGHT;
		int weight_b = scale_b - scale_init_b - SUB_WEIGHT;

		if (weight_a < 0) weight_a = 0;
		if (weight_b < 0) weight_b = 0;

		if (weight_a + weight_b > MIN_WEIGHT)
		{
			paddle = (4*32*weight_b) / (weight_a + weight_b) - (4+16);
		}
		else if (dy > 0 && y < 3*32-8)
		{
			int distance = (3*32-6) - y;
			int final_x = x + distance*dx;
			int final_dx = dx;

			while (final_x < 0 || final_x > 3*32-4) {
				if (final_x < 0)
					final_x = -final_x;
				else
					final_x = 2*(3*32-4) - final_x;
				final_dx = -final_dx;
			}

			if (hitsomething)
				final_x += 6*final_dx;

			if (paddle+4 > final_x)
				paddle -= xorshift32() % 2 ? 2 : 3;

			if (paddle+4 < final_x)
				paddle += xorshift32() % 2 ? 2 : 3;
		}

		if (paddle < 0) paddle = 0;
		if (paddle+12 > 3*32) paddle = 3*32-12;

		if (dy > 0 && (y == 3*32-6 || y == 3*32-5 || y == 3*32-4))
		{
			int w = y - (3*32-7);
			int delta = x - paddle;

			if (0 <= delta+w && delta-w < 9) {
				if (w > 1)
					dx = delta < 0 ? -1 : +1;
				bounce_y = true;
				hitsomething = false;
			}
		}

		if (bounce_x) dx = -dx;
		if (bounce_y) dy = -dy;

		x += dx;
		y += dy;

		draw_ball(x, y);
		draw_paddle(paddle);
	}
}

int compar_int(const void *vp1, const void *vp2)
{
	const int *ip1 = vp1, *ip2 = vp2;
	if (*ip1 < *ip2) return -1;
	if (*ip1 > *ip2) return +1;
	return 0;
}

const char banner_data[] =
"................................................................................................"
".#..........##..#...................***..*..**..**....*...*..@..........@@@...................@."
".#..##..##.#...###..##..#.##.#.#....*..*.*.*...*......*...*..@..@@..@@..@..@..@@...@@@.@.@@.@@@."
".#.#...#.#..#...#..#..#.##..#.#.#...***..*..*..*...**..*.*...@.@...@..@.@@@..@..@.@..@.@@..@..@."
".#.#...##....#..#..#..#.#...#.#.#...*..*.*...*.*.......*.*...@.@...@..@.@..@.@..@.@.@@.@...@..@."
".#..##..##.##...##..##..#...#.#.#...*..*.*.**...**......*....@..@@..@@..@@@...@@...@@@.@....@@@."
"................................................................................................"
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

void init()
{
	for (int y = 0; y < 3*32; y++)
	for (int x = 0; x < 3*32; x++)
	{
		uint32_t color = COLOR_BLACK;

		if (y < 8)
		{
			switch (banner_data[96*y+x])
			{
			case '#':
				color = COLOR_CYAN;
				break;
			case '*':
				color = COLOR_RED;
				break;
			case '@':
				color = COLOR_MAGENTA;
				break;
			case 'x':
				color = x % 2 ? COLOR_BLUE : COLOR_RED;
				break;
			}
		}

		setpixel(x, y, color);
	}

	scales_init();

	scale_init_a = 0;
	scale_init_b = 0;

	while (!scales_ready()) { /* wait */ }
	scales_read();

	int initvals_a[7];
	int initvals_b[7];

	for (int i = 0; i < 7; i++) {
		while (!scales_ready()) { /* wait */ }
		scales_read();
		initvals_a[i] = scale_a;
		initvals_b[i] = scale_b;
	}

	qsort(initvals_a, 7, sizeof(int), compar_int);
	qsort(initvals_b, 7, sizeof(int), compar_int);

	scale_init_a = initvals_a[3];
	scale_init_b = initvals_b[3];
}

int main()
{
	init();

	while (1)
	{
		game();

		for (int y = 8; y < 3*32; y++)
		{
			if (y % 4 == 0) {
				while (!scales_ready()) { /* wait */ }
				scales_read();
			}
			for (int x = 0; x < 3*32; x++)
				if (xorshift32() % 4 == 0)
					setpixel(x, y, all_colors[xorshift32() % (sizeof(all_colors)/sizeof(*all_colors))]);
		}

		for (int y = 8; y < 3*32; y++)
		{
			if (y % 4 == 0) {
				while (!scales_ready()) { /* wait */ }
				scales_read();
			}
			for (int x = 0; x < 3*32; x++)
				if (xorshift32() % 2 == 0)
					setpixel(x, y, all_colors[xorshift32() % (sizeof(all_colors)/sizeof(*all_colors))]);
		}

		for (int y = 8; y < 3*32; y++)
		{
			if (y % 4 == 0) {
				while (!scales_ready()) { /* wait */ }
				scales_read();
			}
			for (int x = 0; x < 3*32; x++)
				setpixel(x, y, COLOR_BLACK);
		}
	}
}

