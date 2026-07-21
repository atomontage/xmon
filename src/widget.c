/*
xmon - a minimal lightweight cross-platform system monitoring utility
Copyright (C) 2025-2026  John Tsiombikas <nuclear@mutantstargoat.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include "widget.h"
#include "xmon.h"
#include "options.h"

unsigned int uicolor[NUM_UICOLORS];
int bar_height;

static unsigned int col_bar;

int init_widgets(void)
{
	int i, bar_thick;

	for(i=0; i<NUM_UICOLORS; i++) {
		uicolor[i] = alloc_color(opt.vis.uicolor[i].r, opt.vis.uicolor[i].g,
				opt.vis.uicolor[i].b);
	}

	col_bar = uicolor[COL_A];

	bar_thick = BEVEL * 2;
	if(bar_thick < 4) bar_thick = 4;
	bar_height = BEVEL * 2 + bar_thick;
	return 0;
}


static void point(struct point *p, int x, int y)
{
	p->x = x;
	p->y = y;
}

void draw_frame(int x, int y, int w, int h, int depth)
{
	int bevel;
	struct point v[4];

	if(depth == 0) return;

	bevel = abs(depth);

	if(bevel == 1) {
		/*
		point(v, x, y + h - 1);
		point(v + 1, x, y);
		point(v + 2, x + w - 1, y);

		XSetForeground(dpy, gc, opt.vis.uicolor[depth > 0 ? COL_BGHI : COL_BGLO].pixel);
		XDrawLines(dpy, win, gc, v, 3, CoordModeOrigin);

		point(v, x + w - 1, y);
		point(v + 1, x + w - 1, y + h - 1);
		point(v + 2, x, y + h - 1);

		XSetForeground(dpy, gc, opt.vis.uicolor[depth > 0 ? COL_BGLO : COL_BGHI].pixel);
		XDrawLines(dpy, win, gc, v, 3, CoordModeOrigin);
		*/
	} else {
		set_color(uicolor[depth > 0 ? COL_BGHI : COL_BGLO]);

		point(v, x, y);
		point(v + 1, x + bevel, y + bevel);
		point(v + 2, x + bevel, y + h - bevel);
		point(v + 3, x, y + h);
		draw_poly(v, 4);

		point(v, x, y);
		point(v + 1, x + w, y);
		point(v + 2, x + w - bevel, y + bevel);
		point(v + 3, x + bevel, y + bevel);
		draw_poly(v, 4);

		set_color(uicolor[depth > 0 ? COL_BGLO : COL_BGHI]);

		point(v, x + w, y);
		point(v + 1, x + w, y + h);
		point(v + 2, x + w - bevel, y + h - bevel);
		point(v + 3, x + w - bevel, y + bevel);
		draw_poly(v, 4);

		point(v, x + w, y + h);
		point(v + 1, x, y + h);
		point(v + 2, x + bevel, y + h - bevel);
		point(v + 3, x + w - bevel, y + h - bevel);
		draw_poly(v, 4);
	}
}


void draw_bar(int x, int y, int w, int val, int total)
{
	int bar, max_bar, bar_thick;

	bar_thick = BEVEL * 2;
	if(bar_thick < 4) bar_thick = 4;
	max_bar = w - BEVEL * 2;

	if(val >= total) {
		bar = max_bar;
	} else {
		bar = val * max_bar / total;
	}

	draw_frame(x, y, w, bar_height, -BEVEL);
	y += BEVEL;

	set_color(col_bar);
	draw_rect(x + BEVEL, y, bar, bar_thick);

	if(BEVEL) {
		/* if we have bevels, the trough is visible just with the background color */
		set_color(uicolor[COL_BG]);
	} else {
		/* without bevels, let's paint it lighter */
		set_color(uicolor[COL_BGHI]);
	}
	draw_rect(x + BEVEL + bar, y, max_bar - bar, bar_thick);
}

void draw_sep(int x, int y, int w)
{
	set_color(uicolor[COL_BGLO]);
	draw_rect(x, y - BEVEL, w, BEVEL);
	set_color(uicolor[COL_BGHI]);
	draw_rect(x, y, w, BEVEL);
}
