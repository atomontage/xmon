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
#ifndef WIDGET_H_
#define WIDGET_H_

/* UI colors */
enum {
	COL_FG,
	COL_BG,
	COL_BGHI,
	COL_BGLO,

	COL_A,
	COL_B,
	COL_AB,

	NUM_UICOLORS
};

extern unsigned int uicolor[NUM_UICOLORS];
extern int bar_height;

int init_widgets(void);

void draw_frame(int x, int y, int w, int h, int depth);
void draw_bar(int x, int y, int w, int val, int total);
void draw_sep(int x, int y, int w);

#endif	/* WIDGET_H_ */
