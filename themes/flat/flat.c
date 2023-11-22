/**
 * This file is part of MetaTheme.
 * Copyright (c) 2004 Martin Dvorak <jezek2@advel.cz>
 *
 * MetaTheme is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MetaTheme is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MetaTheme; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <malloc.h>
#include "metatheme.h"
#include "metathemeInt.h"

#define DATA(name) ((ENGINE_DATA *)engine->data)->name

typedef struct {
	MT_COLOR col_dark1, col_dark2, col_white, col_black, col_grey, col_selected,
		col_selectedfg, col_yellow, col_tooltip;
	MT_GC *gc_dark1, *gc_dark2, *gc_white, *gc_black, *gc_grey, *gc_selected, *gc_yellow, *gc_tooltip;
} ENGINE_DATA;

static MT_ENGINE *engine;


void metatheme_init(MT_ENGINE *engine)
{
	/* initialization: */
	engine->data = (void *)calloc(1, sizeof(ENGINE_DATA));

	/* palette and colors stuff: */
	mt_color_set(DATA(col_black),	  0,  0,	0);
	mt_color_set(DATA(col_dark1),	 64,  64,  64);
	mt_color_set(DATA(col_dark2),   128, 128, 128);
	mt_color_set(DATA(col_white),   255, 255, 255);
	mt_color_set(DATA(col_yellow),  245, 219, 149);
	mt_color_set(DATA(col_tooltip),	255, 255, 225);

	DATA(col_grey)			= engine->palette[MT_BACKGROUND];
	DATA(col_selected)		= engine->palette[MT_SELECTED_BACKGROUND];
	DATA(col_selectedfg)	= engine->palette[MT_SELECTED_FOREGROUND];

	engine->palette[MT_BUTTON_LIGHT]	= DATA(col_white);
	engine->palette[MT_BUTTON_MIDLIGHT]	= DATA(col_grey);
	engine->palette[MT_BUTTON_DARK]		= DATA(col_dark1);
	engine->palette[MT_BUTTON_MID]		= DATA(col_dark2);
	engine->palette[MT_BUTTON_SHADOW]	= DATA(col_black);

	/* size metrics: */
	mt_set_metric_size(engine, MT_BUTTON_BORDER,				2, 2);
	mt_set_metric_size(engine, MT_BUTTON_TEXT_OFFSET,			1, 1);
	mt_set_metric_size(engine, MT_BUTTON_TOGGLE_BORDER,			2, 2);
	mt_set_metric_size(engine, MT_BUTTON_TOGGLE_TEXT_OFFSET,	1, 1);
	mt_set_metric_size(engine, MT_TOOLBAR_ITEM_BORDER,			1, 1);
	mt_set_metric_size(engine, MT_TOOLBAR_ITEM_TEXT_OFFSET,		1, 1);
	mt_set_metric_size(engine, MT_MENUBAR_BORDER,				0, 0); /* TODO */
	mt_set_metric_size(engine, MT_MENUBAR_ITEM_BORDER,			2, 2);
	mt_set_metric_size(engine, MT_MENUBAR_ITEM_TEXT_OFFSET,		1, 1);
	mt_set_metric_size(engine, MT_MENU_BORDER,					2, 2);
	mt_set_metric_size(engine, MT_MENU_ITEM_BORDER,				2, 2);
	mt_set_metric_size(engine, MT_PROGRESSBAR_BORDER,			2, 2);

	/* metrics: */
	mt_set_metric(engine, MT_BORDER,							1);
	mt_set_metric(engine, MT_SCROLLBAR_SIZE,					14);
	mt_set_metric(engine, MT_CHECKBOX_SIZE,						13);
	mt_set_metric(engine, MT_SCALE_SLIDER_SIZE,					11);
	mt_set_metric(engine, MT_STATICBOX_TEXT_OFFSET,				3); /* TODO */
	mt_set_metric(engine, MT_MENUBAR_ITEM_SPACING,				0);
	mt_set_metric(engine, MT_MENU_SEPARATOR_HEIGHT,				3);
	mt_set_metric(engine, MT_NOTEBOOK_OVERLAP,					1);
	mt_set_metric(engine, MT_NOTEBOOK_TAB_OVERLAP,				1);
	mt_set_metric(engine, MT_NOTEBOOK_TEXT_OFFSET,				2);
	mt_set_metric(engine, MT_NOTEBOOK_ARROW_WIDTH,				15);
	mt_set_metric(engine, MT_NOTEBOOK_IS_FILLED,				0);
	mt_set_metric(engine, MT_CHOICE_BUTTON_WIDTH,				16+2);
	mt_set_metric(engine, MT_TOOLBAR_ITEM_DROPDOWN_WIDTH,		13);
	mt_set_metric(engine, MT_SPLITTER_WIDTH,					3);
	mt_set_metric(engine, MT_MENU_TEAROFF_HEIGHT,				5);
	mt_set_metric(engine, MT_DETACHED_MENU_TEAROFF_HEIGHT,		10);

	return;
}


void metatheme_realize(MT_ENGINE *engine)
{
	MT_IMAGE *img1, *img2;
	int i,j,offset;

	DATA(gc_dark1)		= mt_gc_new_with_foreground(&DATA(col_dark1));
	DATA(gc_dark2)		= mt_gc_new_with_foreground(&DATA(col_dark2));
	DATA(gc_white)		= mt_gc_new_with_foreground(&DATA(col_white));
	DATA(gc_black)		= mt_gc_new_with_foreground(&DATA(col_black));
	DATA(gc_grey)		= mt_gc_new_with_foreground(&DATA(col_grey));
	DATA(gc_selected)	= mt_gc_new_with_foreground(&DATA(col_selected));
	DATA(gc_yellow)		= mt_gc_new_with_foreground(&DATA(col_yellow));
	DATA(gc_tooltip)	= mt_gc_new_with_foreground(&DATA(col_tooltip));
}


void metatheme_exit(MT_ENGINE *engine)
{
	mt_gc_destroy(DATA(gc_dark1));
	mt_gc_destroy(DATA(gc_dark2));
	mt_gc_destroy(DATA(gc_white));
	mt_gc_destroy(DATA(gc_black));
	mt_gc_destroy(DATA(gc_grey));
	mt_gc_destroy(DATA(gc_selected));
	mt_gc_destroy(DATA(gc_tooltip));

	free(engine->data);
}


static void alpha_blend_colors(MT_COLOR *result, MT_COLOR *bg, MT_COLOR *fg, int a)
{
	int alpha = a, inv_alpha;

	if (alpha > 255) alpha = 255;
	if (alpha < 0) alpha = 0;
	inv_alpha = 255 - alpha;

	result->r = fg->r * inv_alpha/255 + bg->r*alpha/255;
	result->g = fg->g * inv_alpha/255 + bg->g*alpha/255;
	result->b = fg->b * inv_alpha/255 + bg->b*alpha/255;
} 


static void get_hsv(MT_COLOR *col, int *h, int *s, int *v)
{
	int r = col->r;
	int g = col->g;
	int b = col->b;
	unsigned int max = r; /* maximum RGB component */
	int whatmax = 0; /* r=>0, g=>1, b=>2 */
	unsigned int min = r; /* find minimum value */
	int delta;

	if ((unsigned int)g > max) {
		max = g;
		whatmax = 1;
	}

	if ((unsigned int)b > max) {
		max = b;
		whatmax = 2;
	}

	if ((unsigned int)g < min) min = g;
	if ((unsigned int)b < min) min = b;

	delta = max-min;

	*v = max; /* calc value */
	*s = max? (510*delta+max)/(2*max) : 0;

	if (*s == 0) {
		*h = -1; /* undefined hue */
	}
	else {
		switch (whatmax) {
			case 0: /* red is max component */
				if (g >= b) {
					*h = (120*(g-b)+delta)/(2*delta);
				}
				else {
					*h = (120*(g-b+delta)+delta)/(2*delta) + 300;
				}
			break;

			case 1: /* green is max component */
				if (b > r) {
					*h = 120 + (120*(b-r)+delta)/(2*delta);
				}
				else {
					*h = 60 + (120*(b-r+delta)+delta)/(2*delta);
				}
				break;

			case 2: /* blue is max component */
				if (r > g) {
					*h = 240 + (120*(r-g)+delta)/(2*delta);
				}
				else {
					*h = 180 + (120*(r-g+delta)+delta)/(2*delta);
				}
				break;
		}
	}
} 


static void draw_gradient(MT_WINDOW *win, int x, int y, int width, int height, MT_COLOR *basecolor, int orientation)
{
	MT_COLOR color1, color2;
		  
	int h, s, v; 

	get_hsv(basecolor, &h, &s, &v); 

	if (v >= 160)
	{
		color2 = *basecolor;
		alpha_blend_colors(&color1, basecolor, &DATA(col_black), 128);
	}
	else		 
	{
		color1 = *basecolor;
		alpha_blend_colors(&color2, basecolor, &DATA(col_white), 128);
	}
			
	mt_draw_gradient(win, DATA(gc_grey), x, y, x+width-1, y+height-1, &color1, &color2, orientation);
}


static void draw_box(MT_WINDOW *win, int x, int y, int width, int height, int down)
{
	if (!down) {
		mt_draw_line(win, DATA(gc_white), x, y, x+width-1, y);
		mt_draw_line(win, DATA(gc_white), x, y, x, y+height-1);
		mt_draw_line(win, DATA(gc_dark2), x+width-1, y, x+width-1, y+height-1);
		mt_draw_line(win, DATA(gc_dark2), x, y+height-1, x+width-1, y+height-1);
	}
	else {
		mt_draw_line(win, DATA(gc_dark2), x, y, x+width-1, y);
		mt_draw_line(win, DATA(gc_dark2), x, y, x, y+height-1);
		mt_draw_line(win, DATA(gc_white), x+width-1, y, x+width-1, y+height-1);
		mt_draw_line(win, DATA(gc_white), x, y+height-1, x+width-1, y+height-1);
	}
}


static void draw_box_alpha(MT_WINDOW *win, int x, int y, int width, int height, int down, int alpha)
{
	if (!down) {
		MT_COLOR col;
		MT_GC *gc;

		alpha_blend_colors(&col, &DATA(col_grey), &DATA(col_white), alpha);
		gc = mt_gc_new_with_foreground(&col);
		mt_draw_line(win, gc, x+width-1, y, x+width-1, y+height-1);
		mt_draw_line(win, gc, x, y+height-1, x+width-1, y+height-1);		
		mt_gc_destroy(gc);
		mt_draw_line(win, DATA(gc_dark2), x+width-1, y, x+width-1, y+height-1);
		mt_draw_line(win, DATA(gc_dark2), x, y+height-1, x+width-1, y+height-1);
	}
	else {
		MT_COLOR col;
		MT_GC *gc;

		mt_draw_line(win, DATA(gc_dark2), x, y, x+width-1, y);
		mt_draw_line(win, DATA(gc_dark2), x, y, x, y+height-1);
		alpha_blend_colors(&col, &DATA(col_grey), &DATA(col_white), alpha);
		gc = mt_gc_new_with_foreground(&col);
		mt_draw_line(win, gc, x+width-1, y, x+width-1, y+height-1);
		mt_draw_line(win, gc, x, y+height-1, x+width-1, y+height-1);		
		mt_gc_destroy(gc);
	}
}

static void draw_3box(MT_WINDOW *win, int x, int y, int width, int height, int down, int notside)
{
	if (!down) {
		if (notside != 1)
			mt_draw_line(win, DATA(gc_white), x, y, x+width-1, y);
		if (notside != 2)
			mt_draw_line(win, DATA(gc_white), x, y, x, y+height-1);
		if (notside != 3)
			mt_draw_line(win, DATA(gc_dark2), x+width-1, y, x+width-1, y+height-1);
		if (notside != 4)
			mt_draw_line(win, DATA(gc_dark2), x, y+height-1, x+width-1, y+height-1);
	}
	else {
		if (notside != 1)
			mt_draw_line(win, DATA(gc_dark2), x, y, x+width-1, y);
		if (notside != 2)
			mt_draw_line(win, DATA(gc_dark2), x, y, x, y+height-1);
		if (notside != 3)
			mt_draw_line(win, DATA(gc_white), x+width-1, y, x+width-1, y+height-1);
		if (notside != 4)
			mt_draw_line(win, DATA(gc_white), x, y+height-1, x+width-1, y+height-1);
	}
}


static void draw_simple_box_with_gap(MT_WINDOW *win, int x, int y, int width, int height, MT_GC *gc1, MT_GC *gc2, int gap_position, int gap_x, int gap_width)
{
	/* TODO: needs proper calculations */

	if (gap_position == MT_POSITION_TOP || gap_position == MT_POSITION_BOTTOM) {
		if (gap_x < 0) {
			gap_width -= -gap_x;
			gap_x = 0;
		}
		if (gap_x+gap_width >= width) {
			gap_width = width - gap_x - 1;
		}
	}
	else {
		if (gap_x < 0) {
			gap_width -= -gap_x;
			gap_x = 0;
		}
		if (gap_x+gap_width > height) {
			gap_width = height - gap_x;
		}
	}
	
	if (gap_position == MT_POSITION_TOP) {
		mt_draw_line(win, gc1, x, y, x+gap_x, y);
		if (gap_x+gap_width < width-1) mt_draw_line(win, gc1, x+gap_x+gap_width-1, y, x+width-2, y);
	}

	if (gap_position == MT_POSITION_BOTTOM) {
		mt_draw_line(win, gc2, x, y+height-1, x+gap_x, y+height-1);
		if (gap_x+gap_width < width-1) mt_draw_line(win, gc2, x+gap_x+gap_width-1, y+height-1, x+width-2, y+height-1);
	}

	if (gap_position == MT_POSITION_LEFT) {
		mt_draw_line(win, gc1, x, y, x, y+gap_x);
		if (gap_x+gap_width < height-1) mt_draw_line(win, gc1, x, y+gap_x+gap_width-1, x, y+height-2);
	}

	if (gap_position == MT_POSITION_RIGHT) {
		mt_draw_line(win, gc2, x+width-1, y, x+width-1, y+gap_x);
		if (gap_x+gap_width < height-1) mt_draw_line(win, gc2, x+width-1, y+gap_x+gap_width-1, x+width-1, y+height-2);
	}

	if (gap_position != MT_POSITION_TOP)	mt_draw_line(win, gc1, x, y, x+width-2, y);
	if (gap_position != MT_POSITION_LEFT)	mt_draw_line(win, gc1, x, y, x, y+height-2);
	if (gap_position != MT_POSITION_RIGHT)	mt_draw_line(win, gc2, x+width-1, y, x+width-1, y+height-1);
	if (gap_position != MT_POSITION_BOTTOM)	mt_draw_line(win, gc2, x, y+height-1, x+width-1, y+height-1);
} 


static void draw_check(MT_WINDOW *win, int x, int y, MT_GC *gc)
{
	int i;

	for (i=0; i<3; i++) {
		mt_draw_line(win, gc, x, y+2+i, x+2, y+2+2+i);
		mt_draw_line(win, gc, x+3, y+3+i, x+6, y+0+i);
	}
} 


static void draw_radio(MT_WINDOW *win, int x, int y, int width, int height, MT_GC *gc)
{
	mt_draw_rectangle(win, gc, 1, x, y+1, x+width-1, y+height-2);
	mt_draw_line(win, gc, x+1, y, x+width-2, y);
	mt_draw_line(win, gc, x+1, y+height-1, x+width-2, y+height-1);
} 


static void draw_arrow(MT_WINDOW *win, MT_GC *gc, int x, int y, int width, int height, int type)
{
	int w, h;
	MT_POINT points[3];
	
	if (type == MT_ARROW_UP || type == MT_ARROW_DOWN) {
		w = (width & 1)? width:width-1;
		h = w/2+1;
		if (h > height) {
			w = w * ((float)height/(float)h);
			w = (w & 1)? w:w-1;
			h = w/2+1;
			x += (width-w)/2;
		}
		y += (height-h)/2;
	}
	else {
		h = (height & 1)? height:height-1;
		w = h/2+1;
		if (w > width) {
			h = h * ((float)width/(float)w);
			h = (h & 1)? h:h-1;
			w = h/2+1;
			y += (height-h)/2;
		}
		x += (width-w)/2;
	}

	switch (type) {
		case MT_ARROW_DOWN:
			points[0].x = x;	points[0].y = y;
			points[1].x = x+w;	points[1].y = y;
			points[2].x = x+w/2;	points[2].y = y+h;
			break;

		case MT_ARROW_UP:
			points[0].x = x-1;	points[0].y = y+h;
			points[1].x = x+w/2;	points[1].y = y-1;
			points[2].x = x+w;	points[2].y = y+h;
			break;

		case MT_ARROW_LEFT:
			points[0].x = x+w;	points[0].y = y-1;
			points[1].x = x+w;	points[1].y = y+h;
			points[2].x = x;	points[2].y = y+h/2;
			break;

		case MT_ARROW_RIGHT:
			points[0].x = x;	points[0].y = y-1;
			points[1].x = x+w;	points[1].y = y+h/2;
			points[2].x = x;	points[2].y = y+h;
			break;
	}

	mt_draw_polygon(win, gc, 1, points, 3);
}


void metatheme_draw_widget(MT_ENGINE *_engine, MT_WINDOW *win, MT_RECTANGLE *area, int type, int state, int x, int y, int width, int height, MT_WIDGET_DATA *data)
{
   engine = _engine;

	switch (type) {
		case MT_BASE:
			break;

		case MT_WINDOW_BASE:
			break;
			
		case MT_BORDER_IN:
		{
			draw_box(win, x, y, width, height, 1);
			break;
		}

		case MT_BORDER_OUT:
		{
			draw_box(win, x, y, width, height, 0);
			break;
		}

		case MT_HLINE:
		{
			mt_draw_line(win, DATA(gc_dark2), x, y, x+width-1, y);
			mt_draw_line(win, DATA(gc_white), x, y+1, x+width-1, y+1);
			break;
		}

		case MT_VLINE:
		{
			mt_draw_line(win, DATA(gc_dark2), x, y, x, y+height-1);
			mt_draw_line(win, DATA(gc_white), x+1, y, x+1, y+height-1);
			break;
		}

		case MT_FOCUS_TAB:
		case MT_FOCUS_CHECKBOX:
		{
			break;
		}

		case MT_RESIZE_GRIP:
		{
			x = x+width-1-13+3;
			y = y+height-1-13+3;
			width = 13;
			height = 13;
			
			mt_draw_line(win, DATA(gc_white), x+0, y+11, x+11, y+0);
			mt_draw_line(win, DATA(gc_dark2), x+1, y+11, x+11, y+1);
			mt_draw_line(win, DATA(gc_dark2), x+2, y+11, x+11, y+2);
			mt_draw_line(win, DATA(gc_grey),  x+3, y+11, x+11, y+3);

			mt_draw_line(win, DATA(gc_white), x+4, y+11, x+11, y+4);
			mt_draw_line(win, DATA(gc_dark2), x+5, y+11, x+11, y+5);
			mt_draw_line(win, DATA(gc_dark2), x+6, y+11, x+11, y+6);
			mt_draw_line(win, DATA(gc_grey),  x+7, y+11, x+11, y+7);

			mt_draw_line(win, DATA(gc_white), x+8, y+11, x+11, y+8);
			mt_draw_line(win, DATA(gc_dark2), x+9, y+11, x+11, y+9);
			mt_draw_line(win, DATA(gc_dark2), x+10, y+11, x+11, y+10);
			mt_draw_point(win, DATA(gc_grey), x+11, y+11);

			mt_draw_line(win, DATA(gc_grey), x+0, y+12, x+12, y+12);
			mt_draw_line(win, DATA(gc_grey), x+12, y+0, x+12, y+12);
			break;
		}

		case MT_ARROW_UP:
		case MT_ARROW_DOWN:
		case MT_ARROW_LEFT:
		case MT_ARROW_RIGHT:
		{
			if (!BIT_IS_SET(state, MT_DISABLED)) {
				draw_arrow(win, DATA(gc_black), x, y, width, height, type);
			}
			else {
				draw_arrow(win, DATA(gc_white), x+1, y+1, width, height, type);
				draw_arrow(win, DATA(gc_dark2), x, y, width, height, type);
			}
			break;
		}

		case MT_BUTTON:
		case MT_BUTTON_DEFAULT:
		case MT_BUTTON_TOGGLE:
		{
			draw_box(win, x, y, width, height, 1);
			if (!BIT_IS_SET(state, MT_ACTIVE))
			{
				draw_box(win, x+1, y+1, width-2, height-2, 0);
			}
			else if (type == MT_BUTTON_TOGGLE)
			{
				MT_COLOR col;
				MT_GC *gc;

				alpha_blend_colors(&col, &DATA(col_grey), &DATA(col_white), 200);
				gc = mt_gc_new_with_foreground(&col);
				mt_draw_rectangle(win, gc, 1, x+1, y+1, x+width-2, y+height-2);
				mt_gc_destroy(gc);
			}
			break;
		}

		case MT_BUTTON_HEADER:
		{
			if (BIT_IS_SET(state, MT_ACTIVE))
				draw_box(win, x, y, width, height, 1);
			else
				draw_box(win, x, y, width, height, 0);
			break;
		}

		case MT_MENU:
		{
			draw_box(win, x, y, width, height, 0);
			break;
		}
		
		case MT_MENUBAR_ITEM:
		{
			if (BIT_IS_SET(state, MT_ACTIVE)) { 
				draw_box(win, x, y, width, height, 1);
			}
			break;
		}
		
		case MT_MENU_ITEM:
		{
			if ((state & MT_SELECTED)) {
				mt_draw_rectangle(win, DATA(gc_selected), 1, x, y, x+width-1, y+height-1);
			}
			else {
				mt_draw_rectangle(win, DATA(gc_grey), 1, x, y, x+width-1, y+height-1);
			}
			break; 
		}

		case MT_MENU_ITEM_CHECK:
		{
			if (BIT_IS_SET(state, MT_ACTIVE)) {
				draw_check(win, x+(width-6)/2, y+(height-6)/2, BIT_IS_SET(state, MT_SELECTED)? DATA(gc_white) : DATA(gc_black));
			}
			break;
		}

		case MT_MENU_ITEM_RADIO:
		{
			if (BIT_IS_SET(state, MT_ACTIVE)) {
				draw_radio(win, x+(width-6)/2, y+(height-6)/2, 6, 6, BIT_IS_SET(state, MT_SELECTED)? DATA(gc_white) : DATA(gc_black));
			}
			break;
		}

		case MT_MENU_ITEM_ARROW:
		{
			y += (height-7)/2;
			height = 7;

			if (!BIT_IS_SET(state, MT_DISABLED)) {
				draw_arrow(win, BIT_IS_SET(state, MT_SELECTED)? DATA(gc_white) : DATA(gc_black), x, y, width, height, MT_ARROW_RIGHT);
			}
			else {
				draw_arrow(win, DATA(gc_white), x+1, y+1, width, height, MT_ARROW_RIGHT);
				draw_arrow(win, DATA(gc_dark2), x, y, width, height, MT_ARROW_RIGHT);
			}
			break; 
		}

		case MT_MENU_ITEM_SEPARATOR:
		{
			mt_draw_line(win, DATA(gc_dark2), x, y+1, x+width-1, y+1);
/*			mt_draw_line(win, DATA(gc_white), x, y+1, x+width-1, y+1);*/
			break;
		}

		case MT_MENU_TEAROFF:
		{
			if (state & MT_SELECTED) {
				mt_draw_rectangle(win, DATA(gc_selected), 1, x, y, x+width-1, y+height-1);
			}
			else if (data->flags & MT_MENU_DETACHED) {
				draw_gradient(win, x, y, width, height+1, &DATA(col_selected), MT_HORIZONTAL);
			}
			else {				
				mt_draw_gradient(win, DATA(gc_grey), x, y, x+width, y+height, &DATA(col_selected), &DATA(col_grey), MT_HORIZONTAL);
			}

			if (data->flags & MT_MENU_DETACHED)
			{
				MT_GC *gc = mt_gc_new_with_foreground(&DATA(col_selectedfg));
				draw_arrow(win, gc, x, y+1, height, height-2, MT_ARROW_LEFT);
				mt_gc_destroy(gc);
			}

			break; 
		}

		case MT_TOOLBAR:
		{
			break;
		}
		
		case MT_TOOLBAR_ITEM:
		case MT_TOOLBAR_ITEM_TOGGLE:
		{
			if (BIT_IS_SET(state, MT_ACTIVE)) {
				draw_box(win, x, y, width, height, 1);
				if (type == MT_BUTTON_TOGGLE)
				{
					MT_COLOR col;
					MT_GC *gc;

					alpha_blend_colors(&col, &DATA(col_grey), &DATA(col_white), 200);
					gc = mt_gc_new_with_foreground(&col);
					mt_draw_rectangle(win, gc, 1, x+1, y+1, x+width-2, y+height-2);
					mt_gc_destroy(gc);
				}
			}
			else if (BIT_IS_SET(state, MT_HOVER) && !(type == MT_TOOLBAR_ITEM_TOGGLE && BIT_IS_SET(state, MT_ACTIVE))) {
				draw_box(win, x, y, width, height, 0);
			} 
			break;
		}

		case MT_TOOLBAR_ITEM_DROPDOWN:
		{
			int oldstate = state;

			if ((state & MT_ACTIVE) && (data->flags & MT_TOOLBAR_ITEM_DROPDOWN_ACTIVE)) {
				state &= ~MT_ACTIVE;
				state |= MT_HOVER;
			}

			metatheme_draw_widget(engine, win, area, MT_TOOLBAR_ITEM, state, x, y, width - 13, height, NULL);

			state = oldstate;

			x = x+width-13;
			width = 13;
			metatheme_draw_widget(engine, win, area, MT_TOOLBAR_ITEM, state, x, y, width, height, NULL);

			x += 4;
			width -= 6;
			if (state & MT_ACTIVE) {
				x++;
				y++;
			}
			metatheme_draw_widget(engine, win, area, MT_ARROW_DOWN, state, x, y, width, height, NULL);
			break;
		}

		case MT_TOOLBAR_SEPARATOR:
		{
			if (data->orientation == MT_VERTICAL) {
				x += 2;
				width -= 4;
				mt_draw_line(win, DATA(gc_dark2), x, y, x+width-1, y);
				mt_draw_line(win, DATA(gc_white), x, y+1, x+width-1, y+1);
			}
			else {
				y += 2;
				height -= 4;
				mt_draw_line(win, DATA(gc_dark2), x, y, x, y+height-1);
				mt_draw_line(win, DATA(gc_white), x+1, y, x+1, y+height-1);
			}
			break;
		} 

		case MT_TOOLBAR_HANDLE:
		{
			if (data->orientation == MT_HORIZONTAL) {
				x += 1;
				width = 3;
				y += 2;
				height -= 4;
				draw_box(win, x, y, width, height, 0);
			}
			else {
				y += 1;
				height = 3;
				x += 2;
				width -= 4;
				draw_box(win, x, y, width, height, 0);
			} 
			break;
		}

		case MT_SCROLLBAR:
		{
			mt_draw_rectangle(win, DATA(gc_grey), 1, x, y, x+width-1, y+height-1);
			break;
		}

		case MT_SCROLLBAR_ARROW_UP:
		case MT_SCROLLBAR_ARROW_DOWN:
		case MT_SCROLLBAR_ARROW_LEFT:
		case MT_SCROLLBAR_ARROW_RIGHT:
		{
			if (data->flags & MT_SCROLLBAR_UNSCROLLABLE) state |= MT_DISABLED; 

			draw_box(win, x, y, width, height, 0);
			mt_draw_rectangle(win, DATA(gc_grey), 1, x+1, y+1, x+width-2, y+height-2);

			x += 3;
			width -= 6;
			y += 3;
			height -= 6; 
			if (state & MT_ACTIVE) {
				x += 1;
				y += 1;
			} 
			if (!BIT_IS_SET(state, MT_DISABLED)) {
				draw_arrow(win, DATA(gc_black), x, y, width, height, type - MT_SCROLLBAR_ARROW_UP + MT_ARROW_UP);
			}
			else {
				draw_arrow(win, DATA(gc_white), x+1, y+1, width, height, type - MT_SCROLLBAR_ARROW_UP + MT_ARROW_UP);
				draw_arrow(win, DATA(gc_dark2), x, y, width, height, type - MT_SCROLLBAR_ARROW_UP + MT_ARROW_UP);
			}
			break;
		}

		case MT_SCROLLBAR_HANDLE:
		{
			draw_box(win, x, y, width, height, 0);
			mt_draw_rectangle(win, DATA(gc_grey), 1, x+1, y+1, x+width-2, y+height-2);
			break;
		}

		case MT_PROGRESSBAR:
		{
			draw_box(win, x, y, width, height, 1);
			mt_draw_rectangle(win, DATA(gc_grey), 1, x+1, y+1, x+width-2, y+height-2);
			break;
		}

		case MT_PROGRESSBAR_SLIDER:
		{
			draw_gradient(win, x, y, width-1, height-1, &DATA(col_selected), MT_HORIZONTAL);
			break;
		}

		case MT_NOTEBOOK:
		{
			draw_simple_box_with_gap(win, x, y, width, height, DATA(gc_white), DATA(gc_dark2), data->gap_position+1, data->gap_x, data->gap_width);
			break;
		}
 
		case MT_NOTEBOOK_TAB:
		{
			int bottom_offset = 0;

			if (data->flags & MT_NOTEBOOK_FIRST_VISIBLE_TAB) {
				switch (data->gap_position) {
					case MT_POSITION_TOP:
						mt_draw_point(win, DATA(gc_grey), x, y+height-2);
						mt_draw_point(win, DATA(gc_white), x, y+height-1);
						break;
				}
			}

			if (state & MT_SELECTED) {
				bottom_offset = 1;
			}
			else {
				if (data->gap_position == MT_POSITION_TOP) {
					y += 1;
					height -= 1;
					mt_draw_line(win, DATA(gc_white), x+1, y+height-1, x+width-1, y+height-1);

					x += 1;
					width -= 2;
				}
				else if (data->gap_position == MT_POSITION_BOTTOM) {
					x += 1;
					width -= 2;
					height -= 1;
					mt_draw_line(win, DATA(gc_dark2), x, y, x+width-1, y);
				}
			}

			if (data->gap_position == MT_POSITION_TOP) {
				height -= 2;
			}
			else if (data->gap_position == MT_POSITION_BOTTOM) {
				y += 2;
				height -= 2;
			}

			switch (data->gap_position) {
				case MT_POSITION_TOP:
					mt_draw_rectangle(win, DATA(gc_grey), 1, x+1, y+1, x+width-3, y+height+bottom_offset);
					mt_draw_line(win, DATA(gc_white), x, y+2, x, y+height-1+bottom_offset);
					mt_draw_point(win, DATA(gc_white), x+1, y+1);
					mt_draw_line(win, DATA(gc_white), x+2, y, x+width-4, y);
					mt_draw_point(win, DATA(gc_dark2), x+width-3, y+1); 
					mt_draw_line(win, DATA(gc_dark2), x+width-2, y+2, x+width-2, y+height-1+bottom_offset);
					break;

				case MT_POSITION_BOTTOM:
					mt_draw_rectangle(win, DATA(gc_grey), 1, x+1, y-1-bottom_offset, x+width-3, y+height-2);
					mt_draw_line(win, DATA(gc_white), x, y-1-bottom_offset, x, y+height-3);
					mt_draw_point(win, DATA(gc_dark2), x+1, y+height-2);
					mt_draw_line(win, DATA(gc_dark2), x+2, y+height-1, x+width-4, y+height-1);
					mt_draw_point(win, DATA(gc_dark2), x+width-3, y+height-1); 
					mt_draw_line(win, DATA(gc_dark2), x+width-2, y-1-bottom_offset, x+width-2, y+height-2);
					break;

				case MT_POSITION_LEFT:
					mt_draw_line(win, DATA(gc_white), x+2, y, x+width-1+bottom_offset, y);
					mt_draw_point(win, DATA(gc_white), x+1, y+1);
					mt_draw_line(win, DATA(gc_white), x, y+2, x, y+height-3);
					mt_draw_point(win, DATA(gc_dark2), x+1, y+height-2);
					mt_draw_line(win, DATA(gc_dark2), x+2, y+height-1, x+width-1+bottom_offset, y+height-1); 
					break;

				case MT_POSITION_RIGHT:
					mt_draw_line(win, DATA(gc_white), x-bottom_offset*2, y, x+width-3, y);
					mt_draw_point(win, DATA(gc_white), x+width-2, y+1);
					mt_draw_line(win, DATA(gc_dark2), x+width-1, y+2, x+width-1, y+height-3);
					mt_draw_point(win, DATA(gc_dark2), x+width-2, y+height-2);
					mt_draw_line(win, DATA(gc_dark2), x-bottom_offset*2, y+height-1, x+width-3, y+height-1);
					break;
			}
			break;
		}

		case MT_NOTEBOOK_ARROW_LEFT:
		case MT_NOTEBOOK_ARROW_RIGHT:
		{
			mt_draw_rectangle(win, DATA(gc_grey), 1, x, y, x+width-1, y+height-2);

			draw_box(win, x, y, width, height-1, BIT_IS_SET(state, MT_ACTIVE));

			if (state & MT_ACTIVE) {
				x++;
				y++;
			}

			x += 3;
			width -= 6;
			y += 5;
			height -= 10;
			metatheme_draw_widget(engine, win, area, (type == MT_NOTEBOOK_ARROW_LEFT)? MT_ARROW_LEFT : MT_ARROW_RIGHT, state, x, y, width, height, data); 
			break;
		}

		case MT_CHECK_BUTTON:
		case MT_RADIO_BUTTON:
			break;

		case MT_CHECK_BOX:
		case MT_CHECK_CELL:
		{
			if (type == MT_CHECK_BOX && (data->flags & MT_DRAW_MASK)) {
				mt_draw_rectangle(win, DATA(gc_black), 1, x, y, x+width-1, y+height-1);
				return;
			}

			draw_box_alpha(win, x, y, width, height, 1, 128);

			mt_draw_rectangle(win, BIT_IS_SET(state, MT_MOUSE_ACTIVE)? DATA(gc_grey) : DATA(gc_white), 1, x+1, y+1, x+width-2, y+height-2);

			if (BIT_IS_SET(state, MT_ACTIVE)) {
				draw_check(win, x+(width-6)/2, y+(height-6)/2, DATA(gc_black));
			}
			break;
		}

		case MT_RADIO_BOX:
		case MT_RADIO_CELL:
		{
			width--;
			height--;
			if (width/3 != width/3.0)
				width -= 1;
			if (width/3 != width/3.0)
			{
				width -= 1;
				x+=1;
			}
			if (height/3 != height/3.0)
				height -= 1;
			if (height/3 != height/3.0)
			{
				height -= 1;
				y+=1;
			}

			if (type == MT_RADIO_BOX)
			{
				MT_POINT points[8];
				points[0].x = x+width/3; points[0].y = y;
				points[1].x = x+width/3*2; points[1].y = y;
				points[2].x = x+width; points[2].y = y+height/3;
				points[3].x = x+width; points[3].y = y+height/3*2;
				points[4].x = x+width/3*2; points[4].y = y+height;
				points[5].x = x+width/3; points[5].y = y+height;
				points[6].x = x; points[6].y = y+height/3*2;
				points[7].x = x; points[7].y = y+height/3;
			
				mt_draw_polygon(win, DATA(gc_white), 1, points, 8);
			}

			/*right*/
			mt_draw_line(win, DATA(gc_white), x+width, y+height/3, x+width, y+height/3*2);
			/*bottom*/
			mt_draw_line(win, DATA(gc_white), x+width/3, y+height, x+width/3*2, y+height);
			/*bottom right*/
			mt_draw_line(win, DATA(gc_white), x+width/3*2, y+height, x+width, y+height/3*2);
			/*top right*/
			mt_draw_line(win, DATA(gc_white), x+width/3*2, y, x+width, y+height/3);
			/*bottom left*/
			mt_draw_line(win, DATA(gc_white), x, y+height/3*2, x+width/3, y+height);

			/*top*/
			mt_draw_line(win, DATA(gc_dark2), x+width/3, y, x+width/3*2, y);
			/*left*/
			mt_draw_line(win, DATA(gc_dark2), x, y+height/3, x, y+height/3*2);
			/*top left*/
			mt_draw_line(win, DATA(gc_dark2), x, y+height/3, x+width/3, y);
			/*top right*/
			mt_draw_line(win, DATA(gc_dark2), x+width/3*2, y, x+width/6*5, y+height/6);
			/*bottom left*/
			mt_draw_line(win, DATA(gc_dark2), x, y+height/3*2, x+width/6, y+height/6*5);
			

			if (BIT_IS_SET(state, MT_ACTIVE)) {
				draw_radio(win, x+4, y+4, 5, 5, DATA(gc_black)); 
			}
			break;
		}

		case MT_CHOICE:
		{
			draw_3box(win, x, y, width, height, 1, 3);
			mt_draw_rectangle(win, DATA(gc_white), 1, x+1, y+1, x+width-1, y+height-2);
			break;
		}

		case MT_CHOICE_BUTTON:
		{
			mt_draw_rectangle(win, DATA(gc_grey), 1, x, y+1, x+width-3, y+height-2);
			draw_3box(win, x, y, width, height, 1, 2);
			if (!BIT_IS_SET(state, MT_ACTIVE))
			{
				draw_box(win, x+1, y+1, width-2, height-2, 0);
			}
			else
			{
				x += 1;
				y += 1;
			}
			x += 5;
			width -= 10;
			y += 3;
			height -= 6; 
			metatheme_draw_widget(engine, win, area, MT_ARROW_DOWN, state, x, y, width, height, data); 
			break;
		}

		case MT_SCALE:
		{
			switch (data->orientation) {
				case MT_HORIZONTAL:
					y += height/2-2;
					draw_box(win, x, y, width, 4, 1);
					break;

				case MT_VERTICAL:
					x += width/2-2;
					draw_box(win, x, y, 4, height, 1);
					break;
			}
			break;
		}

		case MT_SCALE_HANDLE:
		{
			draw_box(win, x, y, width, height, 0);
			mt_draw_rectangle(win, DATA(gc_grey), 1, x+1, y+1, x+width-2, y+height-2);
			break;
		}

		case MT_STATUSBAR_SECTION:
		{
			draw_box(win, x, y, width, height, 1);
			break;
		}

		case MT_STATICBOX:
		{
			data->gap_x -= 3;
			data->gap_width += 6;

			draw_simple_box_with_gap(win, x, y, width, height, DATA(gc_dark2), DATA(gc_white), data->gap_position, data->gap_x, data->gap_width);
			draw_simple_box_with_gap(win, x+1, y+1, width-2, height-2, DATA(gc_white), DATA(gc_dark2), data->gap_position, data->gap_x-1, data->gap_width); 
		 	break;
		}
			
		case MT_SPINBUTTON:
			draw_box(win, x, y, width+1, height, 1);
			break;
			
		case MT_SPINBUTTON_UP:
		case MT_SPINBUTTON_DOWN:
		{
			if (type == MT_SPINBUTTON_UP)
			{
				mt_draw_line(win, DATA(gc_dark2), x, y, x+width-1, y);
				mt_draw_line(win, DATA(gc_white), x+width-1, y, x+width-1, y+height-1);
				draw_box(win, x, y+1, width-1, height-1, 0);
				y += 3;
			}
			else
			{
				mt_draw_line(win, DATA(gc_white), x, y+height-1, x+width-1, y+height-1);
				mt_draw_line(win, DATA(gc_white), x+width-1, y, x+width-1, y+height-1);
				draw_box(win, x, y, width-1, height-2, 0);
				y += 2;
			}
			x += 2;
			width -= 5;
			height -= 5;
			if (state & MT_ACTIVE) {
				x += 1;
				y += 1;
				width -= 1;
				height -= 1;
			} 
			 if (!BIT_IS_SET(state, MT_DISABLED)) {
				draw_arrow(win, DATA(gc_black), x, y, width, height, type - MT_SPINBUTTON_UP + MT_ARROW_UP);
			}
			else {
				draw_arrow(win, DATA(gc_white), x+1, y+1, width, height, type - MT_SPINBUTTON_UP + MT_ARROW_UP);
				draw_arrow(win, DATA(gc_dark2), x, y, width, height, type - MT_SPINBUTTON_UP + MT_ARROW_UP);
			}
			break;
		}

		case MT_ENTRY:
			break;
		
		case MT_ENTRY_BORDER:
		case MT_SCROLLED_WINDOW:
		{
			draw_box_alpha(win, x, y, width, height, 1, 128);
			break;
		}

		case MT_TOOLTIP:
		{
			draw_box(win, x, y, width, height, 0);
			break;
		}

		case MT_SPLITTER:
		{
			break;
		}
	}
}


void metatheme_draw_string(MT_ENGINE *engine, MT_WINDOW *win, int type, int state, MT_STRING *str)
{
	if (state & MT_DISABLED) {
		mt_draw_string(win, str, 1, 1, &DATA(col_white));
	}

	mt_draw_string(win, str, 0, 0, NULL);
}
