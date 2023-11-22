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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MetaTheme; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * Based on code from Plastik engine from KDE and color management code from QT.
 *
 * Qt GUI Toolkit
 * Copyright (C) 1999-2003 Trolltech AS.  All rights reserved.
 *
 * Plastik widget style for KDE 3
 * Copyright (C) 2003 Sandro Giessl <ceebx@users.sourceforge.net>
 *
 * based on the KDE style "dotNET":
 *
 * Copyright (C) 2001-2002, Chris Lee <clee@kde.org>
 *                          Carsten Pfeiffer <pfeiffer@kde.org>
 *                          Karol Szwed <gallium@kde.org>
 * Drawing routines completely reimplemented from KDE3 HighColor, which was
 * originally based on some stuff from the KDE2 HighColor.
 *
 * based on drawing routines of the style "Keramik":
 *
 * Copyright (c) 2002 Malte Starostik <malte@kde.org>
 *           (c) 2002,2003 Maksim Orlovich <mo002j@mail.rochester.edu>
 * based on the KDE3 HighColor Style
 * Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
 *           (C) 2001-2002 Fredrik Höglund  <fredrik@kde.org>
 * Drawing routines adapted from the KDE2 HCStyle,
 * Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *           (C) 2000 Dirk Mueller          <mueller@kde.org>
 *           (C) 2001 Martijn Klingens      <klingens@kde.org>
 * Progressbar code based on KStyle,
 * Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "metatheme.h"
#include "metathemeInt.h"

#define _contrast 7

#define DATA(name) ((ENGINE_DATA *)engine->data)->name

typedef struct {
   MT_COLOR col_dark1, col_dark2, col_white, col_black, col_grey, col_selected, col_button, col_tooltip;
   MT_GC *gc_dark1, *gc_dark2, *gc_white, *gc_black, *gc_grey, *gc_selected, *gc_tooltip;
   MT_PIXMAP *pattern_pixmap[2];
   int enhanced, xp, highlight_notebook;
} ENGINE_DATA;

static MT_ENGINE *engine;
static MT_WIDGET_DATA *widget_data;

enum ColorType {
   ButtonContour,
   ButtonSurface,
   PanelContour,
   PanelLight,
   PanelLight2,
   PanelDark,
   PanelDark2
};

enum SurfaceFlags {
   Draw_Left =          0x00000001,
   Draw_Right =         0x00000002,
   Draw_Top =           0x00000004,
   Draw_Bottom =        0x00000008,
   Highlight_Left =     0x00000010, /* surface */
   Highlight_Right =    0x00000020, /* surface */
   Highlight_Top =      0x00000040, /* surface */
   Highlight_Bottom =   0x00000080, /* surface */
   Is_Sunken =          0x00000100, /* surface */
   Is_Horizontal =      0x00000200, /* surface */
   Is_Highlight =       0x00000400, /* surface */
   Is_Default =         0x00000800, /* surface */
   Is_Disabled =        0x00001000,
   Round_UpperLeft =    0x00002000,
   Round_UpperRight =   0x00004000,
   Round_BottomLeft =   0x00008000,
   Round_BottomRight =  0x00010000,
   Draw_Full =          0x00020000  /* contour */
};

#define DEFAULT_SURFACE_FLAGS (Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight)

enum TabPosition {
   First = 0,
   Middle,
   Last,
   Single /* only one tab! */
};

/* check mark: */

#define CHECKMARK_SIZE 9 /* 9x9 */

static unsigned char checkmark_aa_bits[] = {
   0x45, 0x01, 0x28, 0x00, 0x11, 0x01, 0x82, 0x00, 0x44, 0x00, 0x82, 0x00,
   0x11, 0x01, 0x28, 0x00, 0x45, 0x01};
static unsigned char checkmark_dark_bits[] = {
   0x82, 0x00, 0x45, 0x01, 0xaa, 0x00, 0x54, 0x00, 0x28, 0x00, 0x74, 0x00,
   0xea, 0x00, 0xc5, 0x01, 0x82, 0x00};
static unsigned char checkmark_light_bits[] = {
   0x00, 0xfe, 0x82, 0xfe, 0x44, 0xfe, 0x28, 0xfe, 0x10, 0xfe, 0x08, 0xfe,
   0x04, 0xfe, 0x02, 0xfe, 0x00, 0xfe};
/*
not used now:
static unsigned char checkmark_tristate_bits[] = {
   0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0xff, 0x01,
   0x00, 0x00, 0xff, 0x01, 0x00, 0x00};
*/

/* radio button: */

static unsigned char radiobutton_mask_bits[] = {
   0xf8, 0x03, 0xfc, 0x07, 0xfe, 0x0f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f,
   0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xfe, 0x0f, 0xfc, 0x07,
   0xf8, 0x03};
static unsigned char radiobutton_contour_bits[] = {
   0xf0, 0x01, 0x0c, 0x06, 0x02, 0x08, 0x02, 0x08, 0x01, 0x10, 0x01, 0x10,
   0x01, 0x10, 0x01, 0x10, 0x01, 0x10, 0x02, 0x08, 0x02, 0x08, 0x0c, 0x06,
   0xf0, 0x01};
static unsigned char radiobutton_aa_inside_bits[] = {
   0x00, 0x00, 0x10, 0x01, 0x04, 0x04, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x04, 0x04, 0x10, 0x01,
   0x00, 0x00};
static unsigned char radiobutton_aa_outside_bits[] = {
   0x08, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00,
   0x08, 0x02};
static unsigned char radiobutton_highlight1_bits[] = {
   0x00, 0x00, 0xf0, 0x01, 0x1c, 0x07, 0x04, 0x04, 0x06, 0x0c, 0x02, 0x08,
   0x02, 0x08, 0x02, 0x08, 0x06, 0x0c, 0x04, 0x04, 0x1c, 0x07, 0xf0, 0x01,
   0x00, 0x00};
static unsigned char radiobutton_highlight2_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x18, 0x03, 0x08, 0x02, 0x04, 0x04,
   0x04, 0x04, 0x04, 0x04, 0x08, 0x02, 0x18, 0x03, 0xe0, 0x00, 0x00, 0x00,
   0x00, 0x00};

/* radio mark: */

#define RADIOMARK_SIZE 9 /* 9x9 */

static unsigned char radiomark_aa_bits[] = {
   0x00, 0x00, 0x44, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x82, 0x00, 0x44, 0x00, 0x00, 0x00};
static unsigned char radiomark_dark_bits[] = {
   0x00, 0x00, 0x38, 0x00, 0x44, 0x00, 0xf2, 0x00, 0xfa, 0x00, 0xfa, 0x00,
   0x7c, 0x00, 0x38, 0x00, 0x00, 0x00};
static unsigned char radiomark_light_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x04, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


void metatheme_init(MT_ENGINE *engine)
{
   MT_CONFIG *cfg;
   const char *tmp;

   /* initialization: */
   engine->data = (void *)malloc(sizeof(ENGINE_DATA));

   /* palette and colors stuff: */
   mt_color_set(DATA(col_black),     0,   0,   0);
   mt_color_set(DATA(col_dark1),    64,  64,  64);
   mt_color_set(DATA(col_dark2),   128, 128, 128);
   mt_color_set(DATA(col_white),   255, 255, 255);
   mt_color_set(DATA(col_white),   255, 255, 255);
   mt_color_set(DATA(col_tooltip), 255, 255, 225);

   DATA(col_grey)     = engine->palette[MT_BACKGROUND];
   DATA(col_selected) = engine->palette[MT_SELECTED_BACKGROUND];
   DATA(col_button)   = engine->palette[MT_BUTTON_BACKGROUND];

   engine->palette[MT_BUTTON_LIGHT]    = DATA(col_white);
   engine->palette[MT_BUTTON_MIDLIGHT] = DATA(col_grey);
   engine->palette[MT_BUTTON_DARK]     = DATA(col_dark1);
   engine->palette[MT_BUTTON_MID]      = DATA(col_dark2);
   engine->palette[MT_BUTTON_SHADOW]   = DATA(col_black);

   /* size metrics: */
   mt_set_metric_size(engine, MT_BUTTON_BORDER,             3, 3);
   mt_set_metric_size(engine, MT_BUTTON_TEXT_OFFSET,        1, 1);
   mt_set_metric_size(engine, MT_BUTTON_TOGGLE_BORDER,      3, 3);
   mt_set_metric_size(engine, MT_BUTTON_TOGGLE_TEXT_OFFSET, 1, 1);
   mt_set_metric_size(engine, MT_TOOLBAR_ITEM_BORDER,       3, 3);
   mt_set_metric_size(engine, MT_TOOLBAR_ITEM_TEXT_OFFSET,  1, 1);
   mt_set_metric_size(engine, MT_MENUBAR_BORDER,            0, 0); /* TODO */
   mt_set_metric_size(engine, MT_MENUBAR_ITEM_BORDER,       2, 2);
   mt_set_metric_size(engine, MT_MENUBAR_ITEM_TEXT_OFFSET,  1, 1);
   mt_set_metric_size(engine, MT_MENU_BORDER,               2, 2);
   mt_set_metric_size(engine, MT_MENU_ITEM_BORDER,          2, 2);
   mt_set_metric_size(engine, MT_PROGRESSBAR_BORDER,        2, 2);

   /* metrics: */
   mt_set_metric(engine, MT_BORDER,                         2);
   mt_set_metric(engine, MT_SCROLLBAR_SIZE,                 16);
   mt_set_metric(engine, MT_CHECKBOX_SIZE,                  13);
   mt_set_metric(engine, MT_SCALE_SLIDER_SIZE,              11);
   mt_set_metric(engine, MT_STATICBOX_TEXT_OFFSET,          5); /* TODO */
   mt_set_metric(engine, MT_MENUBAR_ITEM_SPACING,           3);
   mt_set_metric(engine, MT_MENU_SEPARATOR_HEIGHT,          9);
   mt_set_metric(engine, MT_NOTEBOOK_OVERLAP,               2);
   mt_set_metric(engine, MT_NOTEBOOK_TAB_OVERLAP,           1);
   mt_set_metric(engine, MT_NOTEBOOK_TEXT_OFFSET,           2);
   mt_set_metric(engine, MT_NOTEBOOK_ARROW_WIDTH,           12);
   mt_set_metric(engine, MT_NOTEBOOK_IS_FILLED,             0);
   mt_set_metric(engine, MT_CHOICE_BUTTON_WIDTH,            16);
   mt_set_metric(engine, MT_TOOLBAR_ITEM_DROPDOWN_WIDTH,    13);
   mt_set_metric(engine, MT_SPLITTER_WIDTH,                 5);
   mt_set_metric(engine, MT_MENU_TEAROFF_HEIGHT,            5);
   mt_set_metric(engine, MT_DETACHED_MENU_TEAROFF_HEIGHT,   10);

   cfg = mt_get_config(engine->library);
   tmp = mt_get_config_option(cfg, "enhanced", "plastik");
   DATA(enhanced) = (tmp && !strcmp(tmp, "1"));
   tmp = mt_get_config_option(cfg, "highlight_notebook", "plastik");
   DATA(highlight_notebook) = (tmp && !strcmp(tmp, "1"));
   
   tmp = mt_get_config_option(engine->config, "xp", "settings");
   DATA(xp) = (tmp && !strcmp(tmp, "1"));

   if (DATA(xp)) {
      DATA(enhanced) = 0;

      mt_set_metric_size(engine, MT_BUTTON_BORDER,             4, 4);
      mt_set_metric_size(engine, MT_BUTTON_TEXT_OFFSET,        0, 0);
      mt_set_metric_size(engine, MT_MENUBAR_ITEM_BORDER,       2, 2);
      mt_set_metric_size(engine, MT_MENUBAR_ITEM_TEXT_OFFSET,  0, 0);
      mt_set_metric_size(engine, MT_MENU_BORDER,               3, 3);

      mt_set_metric(engine, MT_MENUBAR_ITEM_SPACING,           0);
      mt_set_metric(engine, MT_NOTEBOOK_IS_FILLED,             1);
   }

   if (DATA(highlight_notebook)) {
      mt_set_metric(engine, MT_NOTEBOOK_IS_FILLED,             1);
   }
}


void metatheme_realize(MT_ENGINE *engine)
{
   MT_IMAGE *img1, *img2;
   int i,j,offset;

   DATA(gc_dark1)    = mt_gc_new_with_foreground(&DATA(col_dark1));
   DATA(gc_dark2)    = mt_gc_new_with_foreground(&DATA(col_dark2));
   DATA(gc_white)    = mt_gc_new_with_foreground(&DATA(col_white));
   DATA(gc_black)    = mt_gc_new_with_foreground(&DATA(col_black));
   DATA(gc_grey)     = mt_gc_new_with_foreground(&DATA(col_grey));
   DATA(gc_selected) = mt_gc_new_with_foreground(&DATA(col_selected));
   DATA(gc_tooltip)  = mt_gc_new_with_foreground(&DATA(col_tooltip));

   /* pattern pixmap: */
   img1 = mt_image_new(64, 64);
   img2 = mt_image_new(64, 64);
   mt_image_fill(img1, &DATA(col_grey));
   mt_image_fill(img2, &DATA(col_dark1));
   offset = 0;
   for (i=0; i<64; i++) {
      for (j=offset; j<64; j+=2) {
         mt_image_set_pixel(img1, j, i, &DATA(col_white));
         mt_image_set_pixel(img2, j, i, &DATA(col_black));
      }
      offset = (offset==1)? 0:1;
   }
   DATA(pattern_pixmap)[0] = mt_pixmap_new_from_image(img1);
   DATA(pattern_pixmap)[1] = mt_pixmap_new_from_image(img2);
   mt_image_destroy(img1);
   mt_image_destroy(img2);
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

   mt_pixmap_destroy(DATA(pattern_pixmap)[0]);
   mt_pixmap_destroy(DATA(pattern_pixmap)[1]);

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


static void render_contour(MT_WINDOW *win, int x, int y, int width, int height, MT_COLOR *bg, MT_COLOR *fg, int flags)
{
   int drawLeft = flags&Draw_Left;
   int drawRight = flags&Draw_Right;
   int drawTop = flags&Draw_Top;
   int drawBottom = flags&Draw_Bottom;
   int disabled = flags&Is_Disabled;
   int full = flags&Draw_Full;

   int x2 = x+width-1;
   int y2 = y+height-1;

   MT_COLOR col_contour = *fg, col_mid1, col_mid2;
   MT_GC *gc1, *gc2, *gc_bg;

   if (disabled) {
      mt_color_dark(&col_contour, bg, 150);
   }
    
   alpha_blend_colors(&col_mid1, bg, &col_contour, 50);
   alpha_blend_colors(&col_mid2, bg, &col_contour, 150);
   gc1 = mt_gc_new_with_foreground(&col_mid1);
   gc2 = mt_gc_new_with_foreground(&col_mid2);
   if (full) {
      gc_bg = mt_gc_new_with_foreground(bg);
   }

   /* sides: */
   if (drawLeft) {
      mt_draw_line(win, gc1, x, drawTop?y+2:y, x, drawBottom?y2-2:y2);
   }
   if (drawRight) {
      mt_draw_line(win, gc1, x2, drawTop?y+2:y, x2, drawBottom?y2-2:y2);
   }
   if (drawTop) {
      mt_draw_line(win, gc1, drawLeft?x+2:x, y, drawRight?x2-2:x+width-1, y);
   }
   if (drawBottom) {
      mt_draw_line(win, gc1, drawLeft?x+2:x, y2, drawRight?x2-2:x+width-1, y2);
   }

   /* edges: */
   if (drawLeft && drawTop) {
      if (!(flags&Round_UpperLeft)) {
         mt_draw_point(win, gc1, x+1, y);
         mt_draw_point(win, gc1, x, y+1);

         /* anti-alias: */
         mt_draw_point(win, gc2, x, y);
      }
      else {
         mt_draw_point(win, gc1, x+1, y+1);

         /* anti-alias: */
         mt_draw_point(win, gc2, x+1, y);
         mt_draw_point(win, gc2, x, y+1);

         if (full) mt_draw_point(win, gc_bg, x, y);
      }
   }
   
   if (drawLeft && drawBottom) {
      if (!(flags&Round_BottomLeft)) {
         mt_draw_point(win, gc1, x+1, y2);
         mt_draw_point(win, gc1, x, y2-1);

         /* anti-alias: */
         mt_draw_point(win, gc2, x, y2);
      }
      else {
         mt_draw_point(win, gc1, x+1, y2-1);

         /* anti-alias: */
         mt_draw_point(win, gc2, x+1, y2);
         mt_draw_point(win, gc2, x, y2-1);

         if (full) mt_draw_point(win, gc_bg, x, y2);
      }
   }

   if (drawRight && drawTop) {
      if (!(flags&Round_UpperRight)) {
         mt_draw_point(win, gc1, x2-1, y);
         mt_draw_point(win, gc1, x2, y+1);

         /* anti-alias: */
         mt_draw_point(win, gc2, x2, y);
      }
      else {
         mt_draw_point(win, gc1, x2-1, y+1);

         /* anti-alias: */
         mt_draw_point(win, gc2, x2-1, y);
         mt_draw_point(win, gc2, x2, y+1);

         if (full) mt_draw_point(win, gc_bg, x2, y);
      }
   }

   if (drawRight && drawBottom) {
      if (!(flags&Round_BottomRight)) {
         mt_draw_point(win, gc1, x2-1, y2);
         mt_draw_point(win, gc1, x2, y2-1);

         /* anti-alias: */
         mt_draw_point(win, gc2, x2, y2);
      }
      else {
         mt_draw_point(win, gc1, x2-1, y2-1);

         /* anti-alias: */
         mt_draw_point(win, gc2, x2-1, y2);
         mt_draw_point(win, gc2, x2, y2-1);

         if (full) mt_draw_point(win, gc_bg, x2, y2);
      }
   }

   mt_gc_destroy(gc1);
   mt_gc_destroy(gc2);
   if (full) mt_gc_destroy(gc_bg);
}


static void render_surface(MT_WINDOW *win, int x, int y, int width, int height, MT_COLOR *bg, MT_COLOR *btn, MT_COLOR *hlt, int intensity, int flags)
{
   int drawLeft = flags&Draw_Left;
   int drawRight = flags&Draw_Right;
   int drawTop = flags&Draw_Top;
   int drawBottom = flags&Draw_Bottom;
   int roundUpperLeft = flags&Round_UpperLeft;
   int roundUpperRight = flags&Round_UpperRight;
   int roundBottomLeft = flags&Round_BottomLeft;
   int roundBottomRight = flags&Round_BottomRight;
   int sunken = flags&Is_Sunken;
   int horizontal = flags&Is_Horizontal;
   int disabled = flags&Is_Disabled;
   int highlight = 0;
   int highlightLeft = 0;
   int highlightRight = 0;
   int highlightTop = 0;
   int highlightBottom = 0;

   MT_COLOR col_base, col_top1, col_top2, col_bottom1, col_bottom2, col_tmp;
   MT_GC *gc;

   int x2 = x+width-1;
   int y2 = y+height-1;

   int w,h;

   /* only highlight if not sunken & not disabled: */
   if (!sunken && !disabled) {
      highlight = (flags&Is_Highlight);
      highlightLeft = (flags&Highlight_Left);
      highlightRight = (flags&Highlight_Right);
      highlightTop = (flags&Highlight_Top);
      highlightBottom = (flags&Highlight_Bottom);
   }

   alpha_blend_colors(&col_base, bg, disabled? bg:btn, 10);
   if (disabled) {
      intensity = 2;
   }
   else if (highlight) {
      /* blend this _slightly_ with the background: */
      alpha_blend_colors(&col_base, &col_base, hlt, 240);
   }
   else if (sunken) {
      /* enforce a common sunken-style: */
      mt_color_dark(&col_base, &col_base, 100+intensity);
      intensity = _contrast/2;
   }

   /* Some often needed colors (1 more intensive than 2 and 3): */

   /* col_top1: */
   if (sunken) {
      mt_color_dark(&col_tmp, &col_base, 100+intensity*2);
   }
   else {
      mt_color_light(&col_tmp, &col_base, 100+intensity*2);
   }
   alpha_blend_colors(&col_top1, &col_base, &col_tmp, 80);

   /* col_top2: */
   if (sunken) {
      mt_color_dark(&col_tmp, &col_base, 100+intensity);
   }
   else {
      mt_color_light(&col_tmp, &col_base, 100+intensity);
   }
   alpha_blend_colors(&col_top2, &col_base, &col_tmp, 80);

   /* col_bottom1: */
   if (sunken) {
      mt_color_light(&col_tmp, &col_base, 100+intensity*2);
   }
   else {
      mt_color_dark(&col_tmp, &col_base, 100+intensity*2);
   }
   alpha_blend_colors(&col_bottom1, &col_base, &col_tmp, 80);

   /* col_bottom2: */
   if (sunken) {
      mt_color_light(&col_tmp, &col_base, 100+intensity);
   }
   else {
      mt_color_dark(&col_tmp, &col_base, 100+intensity);
   }
   alpha_blend_colors(&col_bottom2, &col_base, &col_tmp, 80);

   gc = mt_gc_new_with_foreground(&DATA(col_grey));

   /* sides: */
   if (drawLeft) {
      if (horizontal) {
         h = height;

         if (roundUpperLeft || !drawTop) h--;
         if (roundBottomLeft || !drawBottom) h--;
         
         mt_draw_gradient(win, gc, x, (roundUpperLeft&&drawTop)?y+1:y, 1, h, &col_top1, &col_base, MT_VERTICAL);
      }
      else {
         mt_gc_set_foreground(gc, &col_top1);
         mt_draw_line(win, gc, x, (roundUpperLeft&&drawTop)?y+1:y, x, (roundBottomLeft&&drawBottom)?y2-1:y2);
      }
   }
   if (drawRight) {
      if (horizontal) {
         h = height;
         /**
          * TODO: there's still a bogus in it: when edge4 is Thick
          *       and we don't whant to draw the Top, we have a unpainted area
          */
         if (roundUpperRight || !drawTop) h--;
         if (roundBottomRight || !drawBottom) h--;

         mt_draw_gradient(win, gc, x2, (roundUpperRight&&drawTop)?y+1:y, 1, h, &col_base, &col_bottom1, MT_VERTICAL);
      }
      else {
         mt_gc_set_foreground(gc, &col_bottom1);
         mt_draw_line(win, gc, x2, (roundUpperRight&&drawTop)?y+1:y, x2, (roundBottomRight&&drawBottom)?y2-1:y2);
      }
   }
   if (drawTop) {
      if (horizontal) {
         mt_gc_set_foreground(gc, &col_top1);
         mt_draw_line(win, gc, (roundUpperLeft&&drawLeft)?x+1:x, y, (roundUpperRight&&drawRight)?x2-1:x2, y);
      }
      else {
         w = width;
         if (roundUpperLeft || !drawLeft) w--;
         if (roundUpperRight || !drawRight) w--;
         
         mt_draw_gradient(win, gc, (roundUpperLeft&&drawLeft)?x+1:x, y, w, 1, &col_top1, &col_top2, MT_HORIZONTAL);
      }
   }
   if (drawBottom) {
      if (horizontal) {
         mt_gc_set_foreground(gc, &col_bottom1);
         mt_draw_line(win, gc, (roundBottomLeft&&drawLeft)?x+1:x, y2, (roundBottomRight&&drawRight)?x2-1:x2, y2);
      }
      else {
         w = width;
         if (roundBottomLeft || !drawLeft) w--;
         if (roundBottomRight || !drawRight) w--;

         mt_draw_gradient(win, gc, (roundBottomLeft&&drawLeft)?x+1:x, y2, w, 1, &col_bottom2, &col_bottom1, MT_HORIZONTAL);
      }
   }

   /* button area: */
   w = width;
   h = height;
   if (drawLeft) w--;
   if (drawRight) w--;
   if (drawTop) h--;
   if (drawBottom) h--;
   
   if (horizontal) {
      mt_draw_gradient(win, gc, drawLeft?x+1:x, drawTop?y+1:y, w, h, &col_top2, &col_bottom2, MT_VERTICAL);
   }
   else {
      mt_draw_gradient(win, gc, drawLeft?x+1:x, drawTop?y+1:y, w, h, &col_top2, &col_bottom2, MT_HORIZONTAL);
   }

   /* highlighting: */
   if (highlightTop) {
      alpha_blend_colors(&col_tmp, &col_top1, hlt, 80);
      mt_gc_set_foreground(gc, &col_tmp);
      mt_draw_line(win, gc, (roundUpperLeft&&drawLeft)?x+1:x, y, (roundUpperRight&&drawRight)?x2-1:x2, y);

      alpha_blend_colors(&col_tmp, &col_top2, hlt, 150);
      mt_gc_set_foreground(gc, &col_tmp);
      mt_draw_line(win, gc, highlightLeft?x+1:x, y+1, highlightRight?x2-1:x2, y+1);
   }
   if (highlightBottom) {
      alpha_blend_colors(&col_tmp, &col_bottom1, hlt, 80);
      mt_gc_set_foreground(gc, &col_tmp);
      mt_draw_line(win, gc, (roundBottomLeft&&drawLeft)?x+1:x, y2, (roundBottomRight&&drawRight)?x2-1:x2, y2);

      alpha_blend_colors(&col_tmp, &col_bottom2, hlt, 150);
      mt_gc_set_foreground(gc, &col_tmp);
      mt_draw_line(win, gc, highlightLeft?x+1:x, y2-1, highlightRight?x2-1:x2, y2-1);
   }
   if (highlightLeft) {
      alpha_blend_colors(&col_tmp, &col_top1, hlt, 80);
      mt_gc_set_foreground(gc, &col_tmp);
      mt_draw_line(win, gc, x, (roundUpperLeft&&drawTop)?y+1:y, x, (roundBottomLeft&&drawBottom)?y2-1:y2);

      alpha_blend_colors(&col_tmp, &col_top2, hlt, 150);
      mt_gc_set_foreground(gc, &col_tmp);
      mt_draw_line(win, gc, x+1, highlightTop?y+1:y, x+1, highlightBottom?y2-1:y2);
   }
   if (highlightRight) {
      alpha_blend_colors(&col_tmp, &col_bottom1, hlt, 80);
      mt_gc_set_foreground(gc, &col_tmp);
      mt_draw_line(win, gc, x2, (roundUpperRight&&drawTop)?y+1:y, x2, (roundBottomRight&&drawBottom)?y2-1:y2);

      alpha_blend_colors(&col_tmp, &col_bottom2, hlt, 150);
      mt_gc_set_foreground(gc, &col_tmp);
      mt_draw_line(win, gc, x2-1, highlightTop?y+1:y, x2-1, highlightBottom?y2-1:y2);
   }

   mt_gc_destroy(gc);
}


static void get_color(MT_COLOR *result, int type, int enabled)
{
   MT_COLOR col_tmp;

   switch (type) {
      case ButtonContour:
         if (enabled) {
            if (widget_data) {
               mt_color_dark(result, &widget_data->background_color, 130+_contrast*8);
            }
            else {
               mt_color_dark(result, &DATA(col_grey), 130+_contrast*8);
            }
         }
         else {
            mt_color_dark(result, &DATA(col_grey), 120+_contrast*8);
         }
         break;

      case PanelContour:
         mt_color_dark(result, &DATA(col_grey), 160+_contrast*8);
         break;

      case PanelDark:
         mt_color_dark(&col_tmp, &DATA(col_grey), 120+_contrast*5);
         alpha_blend_colors(result, &DATA(col_grey), &col_tmp, 110);
         break;

      case PanelDark2:
         mt_color_dark(&col_tmp, &DATA(col_grey), 110+_contrast*5);
         alpha_blend_colors(result, &DATA(col_grey), &col_tmp, 110);
         break;

      case PanelLight:
         mt_color_light(&col_tmp, &DATA(col_grey), 120+_contrast*5);
         alpha_blend_colors(result, &DATA(col_grey), &col_tmp, 110);
         break;

      case PanelLight2:
         mt_color_light(&col_tmp, &DATA(col_grey), 110+_contrast*5);
         alpha_blend_colors(result, &DATA(col_grey), &col_tmp, 110);
         break;

      default:
         *result = DATA(col_grey);
   }
}


static void render_dot(MT_WINDOW *win, int x, int y, MT_COLOR *col_base, int thick, int sunken)
{
   MT_COLOR col_tmp, col_top, col_bottom;
   MT_GC *gc;

   /* col_top: */
   if (sunken) {
      mt_color_dark(&col_tmp, col_base, 130);
   }
   else {
      mt_color_light(&col_tmp, col_base, 150);
   }
   alpha_blend_colors(&col_top, col_base, &col_tmp, 70);

   /* col_bottom: */
   if (sunken) {
      mt_color_light(&col_tmp, col_base, 150);
   }
   else {
      mt_color_dark(&col_tmp, col_base, 130);
   }
   alpha_blend_colors(&col_bottom, col_base, &col_tmp, 70);

   gc = mt_gc_new_with_foreground(&col_top);
   mt_draw_line(win, gc, x, y, x+1, y);
   mt_draw_point(win, gc, x, y+1);

   mt_gc_set_foreground(gc, &col_bottom);
   if (thick) {
      mt_draw_line(win, gc, x+1, y+2, x+2, y+2);
      mt_draw_point(win, gc, x+2, y+1);
   }
   else {
      mt_draw_point(win, gc, x+1, y+1);
   }

   mt_gc_destroy(gc);
}


static void render_panel(MT_WINDOW *win, int x, int y, int width, int height, int pseudo3d, int sunken, int thick, int full)
{
   MT_COLOR col_tmp;
   MT_GC *gc;
   int x2 = x+width-1;
   int y2 = y+height-1;
   int contourFlags = DEFAULT_SURFACE_FLAGS;

   if (full) contourFlags |= Draw_Full;

   if (DATA(xp) && !pseudo3d && sunken && !thick) {
      mt_color_set(col_tmp, 158, 158, 145);
   }
   else {
      get_color(&col_tmp, PanelContour, 1);
   }

   render_contour(win, x, y, width, height, &DATA(col_grey), &col_tmp, contourFlags);

   if (pseudo3d) {
      if (sunken) {
         get_color(&col_tmp, PanelDark, 1);
      }
      else {
         get_color(&col_tmp, PanelLight, 1);
      }
      gc = mt_gc_new_with_foreground(&col_tmp);
      mt_draw_line(win, gc, x+2, y+1, x2-2, y+1);
      mt_draw_line(win, gc, x+1, y+2, x+1, y2-2);

      if (sunken) {
         get_color(&col_tmp, PanelLight, 1);
      }
      else {
         get_color(&col_tmp, PanelDark, 1);
      }
      mt_gc_set_foreground(gc, &col_tmp);
      mt_draw_line(win, gc, x+2, y2-1, x2-2, y2-1);
      mt_draw_line(win, gc, x2-1, y+2, x2-1, y2-2);

      mt_gc_destroy(gc);
   }
}


static void render_button(MT_WINDOW *win, int x, int y, int width, int height, int sunken, int mouseOver, int horizontal, int enabled, int full, int xp, int defaultButton)
{
   int contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight;
   int surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight;
   MT_COLOR col_btn, col_tmp, col_tmp2, col_tmp3;
   MT_GC *gc;
   
   if (!enabled) contourFlags |= Is_Disabled;
   if (full) contourFlags |= Draw_Full;
   
   /* Plastik XP modification: */
   if (DATA(xp) && xp && enabled) {
      mt_color_set(col_btn, 0, 60, 116);

      col_tmp = DATA(col_button);
      if (sunken) {
         mt_color_dark(&col_tmp, &col_tmp, 115);
      }
      else {
         mt_color_dark(&col_tmp, &col_tmp, 101);
      }

      gc = mt_gc_new_with_foreground(&col_tmp);

      /* background: */
      mt_draw_rectangle(win, gc, 1, x+1, y+4, x+width-2, y+height-5);

      /* top shadow: */
      if (!sunken) {
         mt_color_light(&col_tmp2, &col_tmp, 125);
         mt_color_light(&col_tmp3, &col_tmp, 110);
         mt_draw_gradient(win, gc, x+1, y+1, width-2, 3, &col_tmp2, &col_tmp3, MT_VERTICAL);
         mt_color_light(&col_tmp3, &col_tmp, 103);
         mt_draw_gradient(win, gc, x+1, y+4, width-2, height/3, &col_tmp3, &col_tmp, MT_VERTICAL);
      }
      else {
         mt_color_dark(&col_tmp2, &col_tmp, 115);
         mt_draw_gradient(win, gc, x+1, y+1, width-2, 3, &col_tmp2, &col_tmp, MT_VERTICAL);
      }

      /* bottom shadow: */
      if (sunken) {
         mt_color_light(&col_tmp2, &col_tmp, 105);
      }
      else {
         mt_color_dark(&col_tmp2, &col_tmp, 115);
      }
      mt_draw_gradient(win, gc, x+1, y+height-4, width-2, 3, &col_tmp, &col_tmp2, MT_VERTICAL);

      if (mouseOver) defaultButton = 0;

      if ((mouseOver || defaultButton) && !sunken) {
         if (defaultButton) {
            mt_color_set(col_tmp, 206, 231, 255);
            mt_color_set(col_tmp2, 188, 212, 246);
         }
         else {
            mt_color_set(col_tmp, 255, 248, 230);
            mt_color_set(col_tmp2, 252, 217, 141);
         }
         mt_draw_gradient(win, gc, x+1, y+1, width-2, 2, &col_tmp, &col_tmp2, MT_VERTICAL);

         if (defaultButton) {
            mt_color_set(col_tmp, 137, 173, 228);
         }
         else {
            mt_color_set(col_tmp, 248, 180, 53);
         }
         mt_draw_gradient(win, gc, x+1, y+3, 2, height-6, &col_tmp2, &col_tmp, MT_VERTICAL);
         mt_draw_gradient(win, gc, x+width-3, y+3, 2, height-6, &col_tmp2, &col_tmp, MT_VERTICAL);

         if (defaultButton) {
            mt_color_set(col_tmp2, 105, 130, 238);
         }
         else {
            mt_color_set(col_tmp2, 229, 151, 0);
         }
         mt_draw_gradient(win, gc, x+1, y+height-3, width-2, 2, &col_tmp, &col_tmp2, MT_VERTICAL);
      }

      mt_gc_destroy(gc);
   }
   else {
      get_color(&col_btn, ButtonContour, 1);
   }

   render_contour(win, x, y, width, height, &DATA(col_grey), &col_btn, contourFlags);
   if (horizontal) surfaceFlags |= Is_Horizontal;

   if (!enabled) {
      surfaceFlags |= Is_Disabled;
   }
   else {
      if (sunken) {
         surfaceFlags |= Is_Sunken;
      }
      else {
         if (mouseOver) {
            surfaceFlags |= Is_Highlight;
            if(horizontal) {
               surfaceFlags |= Highlight_Top;
               surfaceFlags |= Highlight_Bottom;
            }
            else {
               surfaceFlags |= Highlight_Left;
               surfaceFlags |= Highlight_Right;
            }
         }
      }
   }

   if (DATA(xp) && xp && enabled) return;

   col_btn = DATA(col_button);
   if (widget_data) col_btn = widget_data->background_color;
   render_surface(win, x+1, y+1, width-2, height-2, &DATA(col_grey), &col_btn, &DATA(col_selected), _contrast, surfaceFlags);
}


static void render_tab(MT_WINDOW *win, int x, int y, int width, int height, int mouseOver, int selected, int bottom, int pos, int triangular) /* default values: 0, 0, 0, Middle, 0 */
{
   int reverseLayout = 0;
   int isFirst = (pos == First) || (pos == Single);
   int isLast = (pos == Last);
   int isSingle = (pos == Single);
   MT_RECTANGLE Rc, Rs, Rb;
   MT_COLOR col_tmp, col_tmp2;
   MT_GC *gc;
   int contourFlags, surfaceFlags;
   int x2 = x+width-1;
   int y2 = y+height-1;

   /* TODO: */
   mouseOver = 0;
   
   gc = mt_gc_new_with_foreground(&DATA(col_grey));

   /* MetaTheme specific: */
   if (DATA(xp)) {
      if (!bottom) {
         mt_color_set(col_tmp, 252, 252, 254);
      }
      else {
         mt_color_set(col_tmp, 244, 243, 238);
      }
      mt_gc_set_foreground(gc, &col_tmp);
      mt_draw_rectangle(win, gc, 1, x+2, y+2, x2-2, y2-2);
   }
   else if (DATA(highlight_notebook)) {
      mt_color_light(&col_tmp, &DATA(col_grey), 105);
      mt_gc_set_foreground(gc, &col_tmp);
      mt_draw_rectangle(win, gc, 1, x+2, y+2, x2-2, y2-2);
   }

   if (selected) {
      if (!bottom) {
         mt_draw_rectangle(win, gc, 1, x, y2-2, x2, y2);
      }
      else {
         mt_draw_rectangle(win, gc, 1, x, y, x2, y+2);
      }
   }

   if (selected) {
      /* the top part of the tab which is nearly the same for all positions: */
      if (!bottom) {
         if (isFirst && !reverseLayout) {
            mt_rectangle_set(Rc, x, y, width-1, height-3);
         }
         else if (isFirst && reverseLayout) {
            mt_rectangle_set(Rc, x+1, y, width-1, height-3);
         }
         else {
            mt_rectangle_set(Rc, x+1, y, width-2, height-3);
         }
      }
      else {
         if (isFirst && !reverseLayout) {
            mt_rectangle_set(Rc, x, y+3, width-1, height-3);
         }
         else if (isFirst && reverseLayout) {
            mt_rectangle_set(Rc, x+1, y+3, width-1, height-3);
         }
         else {
            mt_rectangle_set(Rc, x+1, y+3, width-2, height-3);
         }
      }

      /* the resulting surface: */
      mt_rectangle_set(Rs, Rc.x+1, bottom? Rc.y:Rc.y+1, Rc.width-2, Rc.height-1);

      /* the area where the fake border shoudl appear: */
      mt_rectangle_set(Rb, x, bottom? y:(Rc.y+Rc.height-1)+1, width, height-Rc.height);

      contourFlags = Draw_Left|Draw_Right;
      if (!bottom) {
         contourFlags |= Draw_Top|Round_UpperLeft|Round_UpperRight;
      }
      else {
         contourFlags |= Draw_Bottom|Round_BottomLeft|Round_BottomRight;
      }

      get_color(&col_tmp, PanelContour, 1);
      render_contour(win, Rc.x, Rc.y, Rc.width, Rc.height, &DATA(col_grey), &col_tmp, contourFlags);
      
      /* surface: */
      if (!bottom) {
         get_color(&col_tmp, PanelLight, 1);
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, Rs.x+1, Rs.y, (Rs.x+Rs.width-1)-1, Rs.y);
         
         get_color(&col_tmp, PanelLight, 1);
         get_color(&col_tmp2, PanelLight2, 1);
         mt_draw_gradient(win, gc, Rs.x, Rs.y+1, 1, Rs.height-1, &col_tmp, &col_tmp2, MT_VERTICAL);

         get_color(&col_tmp, PanelDark, 1);
         get_color(&col_tmp2, PanelDark2, 1);
         mt_draw_gradient(win, gc, Rs.x+Rs.width-1, Rs.y+1, 1, Rs.height-1, &col_tmp, &col_tmp2, MT_VERTICAL);
      }
      else {
         mt_color_dark(&col_tmp, &DATA(col_grey), 160);
         alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 100);
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, Rs.x+1, Rs.y+Rs.height-1, (Rs.x+Rs.width-1)-1, Rs.y+Rs.height-1);

         get_color(&col_tmp, PanelLight, 1);
         get_color(&col_tmp2, PanelLight2, 1);
         mt_draw_gradient(win, gc, Rs.x, Rs.y, 1, Rs.height-1, &col_tmp, &col_tmp2, MT_VERTICAL);

         get_color(&col_tmp, PanelDark, 1);
         get_color(&col_tmp2, PanelDark2, 1);
         mt_draw_gradient(win, gc, Rs.x+Rs.width-1, Rs.y, 1, Rs.height-1, &col_tmp, &col_tmp2, MT_VERTICAL);
      }
      
      /* some "position specific" paintings... */

      /* draw parts of the inactive tabs around: */
      if (!isSingle) {
         get_color(&col_tmp, ButtonContour, 1);
         alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 50);
         mt_gc_set_foreground(gc, &col_tmp);

         if ((!isFirst && !reverseLayout) || (!isLast && reverseLayout)) {
            mt_draw_point(win, gc, x, bottom? (triangular? y2-2:y2-3) : (triangular? y+2:y+3));
            render_surface(win, x, bottom? y+3:(triangular? y+3:y+4), 1, (triangular? height-6:height-7), &DATA(col_grey), &DATA(col_button), &DATA(col_selected), _contrast, Draw_Top|Draw_Bottom|Is_Horizontal);
         }
         
         if ((!isLast&&!reverseLayout) || (!isFirst&&reverseLayout)) {
            mt_draw_point(win, gc, x2, bottom? (triangular? y2-2:y2-3) : (triangular? y+2:y+3));
            render_surface(win, x2, bottom? y+3:(triangular?y+3:y+4), 1, (triangular? height-6:height-7), &DATA(col_grey), &DATA(col_button), &DATA(col_selected), _contrast, Draw_Top|Draw_Bottom|Is_Horizontal);
         }
      }

      /* left connection from the panel border to the tab: */
      if (isFirst && !reverseLayout) {
         get_color(&col_tmp, PanelContour, 1);
         alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 50);
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, Rb.x, Rb.y, Rb.x, Rb.y+Rb.height-1);

         get_color(&col_tmp, PanelLight, 1);
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, Rb.x+1, Rb.y, Rb.x+1, Rb.y+Rb.height-1);
      }
      else if (isFirst && reverseLayout) {
         /*
         p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
         p->drawLine(Rb.right(), Rb.y(), Rb.right(), Rb.bottom() );
         p->setPen(getColor(g,PanelDark) );
         p->drawLine(Rb.right()-1, Rb.y(), Rb.right()-1, Rb.bottom() );
         */
      }

      /* rounded connections to the panel: */
      if (!bottom) {
         /* left: */
         if ((!isFirst && !reverseLayout) || (reverseLayout)) {
            get_color(&col_tmp, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 50);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x, Rb.y);

            get_color(&col_tmp, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 150);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x, Rb.y+1);
            mt_draw_point(win, gc, Rb.x+1, Rb.y);
         }

         /* right: */
         if ((!reverseLayout) || (!isFirst && reverseLayout)) {
            get_color(&col_tmp, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 50);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x+Rb.width-1, Rb.y);

            get_color(&col_tmp, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 150);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x+Rb.width-1, Rb.y+1);
            mt_draw_point(win, gc, (Rb.x+Rb.width-1)-1, Rb.y);
         }
      }
      else {
         /* left: */
         if ((!isFirst && !reverseLayout) || (reverseLayout)) {
            get_color(&col_tmp, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 50);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x, Rb.y+Rb.height-1);

            get_color(&col_tmp, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 150);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x, (Rb.y+Rb.height-1)-1);
            mt_draw_point(win, gc, Rb.x+1, (Rb.y+Rb.height-1));
         }

         /* right: */
         if ((!reverseLayout) || (!isFirst && reverseLayout)) {
            get_color(&col_tmp, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 50);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x+Rb.width-1, Rb.y+Rb.height-1);

            get_color(&col_tmp, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 150);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x+Rb.width-1, (Rb.y+Rb.height-1)-1);
            mt_draw_point(win, gc, (Rb.x+Rb.width-1)+1, (Rb.y+Rb.height-1));
         }
      }
   }
   else {
      /* inactive tabs: */
      
      /* the top part of the tab which is nearly the same for all positions: */
      if (isFirst&&reverseLayout) {
         mt_rectangle_set(Rc, x+1, (bottom? y+2:(triangular? y+2:y+3)), width-2, (triangular? height-4:height-5));
      }
      else {
         mt_rectangle_set(Rc, x+1, (bottom? y+2:(triangular? y+2:y+3)), width-1, (triangular? height-4:height-5));
      }
      
      if ((isFirst && !reverseLayout) || (isLast && reverseLayout)) {
         mt_rectangle_set(Rs, Rc.x+1, bottom? Rc.y:Rc.y+1, Rc.width-2, Rc.height-1);
      }
      else {
         mt_rectangle_set(Rs, Rc.x, bottom? Rc.y:Rc.y+1, Rc.width-1, Rc.height-1);
      }

      /* the area where the fake border shoudl appear: */
      mt_rectangle_set(Rb, x, bottom? y:(Rc.y+Rc.height-1)+1, width, 2);

      if (!bottom) {
         if ((isFirst && !reverseLayout) || (isLast && reverseLayout)) {
            contourFlags = Draw_Left|Draw_Right|Draw_Top|Round_UpperLeft;
         }
         else if ((isLast && !reverseLayout) || (isFirst && reverseLayout)) {
            contourFlags = Draw_Right|Draw_Top|Round_UpperRight;
         }
         else {
            contourFlags = Draw_Right|Draw_Top;
         }
      }
      else {
         if ((isFirst && !reverseLayout) || (isLast && reverseLayout)) {
            contourFlags = Draw_Left|Draw_Right|Draw_Bottom|Round_BottomLeft;
         }
         else if ((isLast && !reverseLayout) || (isFirst && reverseLayout)) {
            contourFlags = Draw_Right|Draw_Bottom|Round_BottomRight;
         }
         else {
            contourFlags = Draw_Right|Draw_Bottom;
         }
      }

      get_color(&col_tmp, ButtonContour, 1);
      render_contour(win, Rc.x, Rc.y, Rc.width, Rc.height, &DATA(col_grey), &col_tmp, contourFlags);

      surfaceFlags = Is_Horizontal;
      if (mouseOver) {
         surfaceFlags |= Highlight_Top;
         surfaceFlags |= Is_Highlight;
      }
      if ((isFirst && !reverseLayout) || (isLast&&reverseLayout)) {
         if (!bottom)
            surfaceFlags |= Draw_Left|Draw_Top|Draw_Bottom|Round_UpperLeft;
         else
            surfaceFlags |= Draw_Left|Draw_Top|Draw_Bottom|Round_BottomLeft;
      }
      else if ((isLast && !reverseLayout) || (isFirst && reverseLayout)) {
         if(!bottom)
            surfaceFlags |= Draw_Right|Draw_Top|Draw_Bottom|Round_UpperRight;
         else
            surfaceFlags |= Draw_Right|Draw_Top|Draw_Bottom|Round_BottomRight;
      }
      else {
         surfaceFlags |= Draw_Top|Draw_Bottom;
      }
      
      render_surface(win, Rs.x, Rs.y, Rs.width, Rs.height, &DATA(col_grey), &DATA(col_button), &DATA(col_selected), _contrast, surfaceFlags);

      /* some "position specific" paintings... */
      /* fake parts of the panel border */
      if(!bottom) {
         get_color(&col_tmp, PanelContour, 1);
         alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 50);
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, Rb.x, Rb.y, (isLast && !reverseLayout)? (Rb.x+Rb.width-1):(Rb.x+Rb.width-1)-1, Rb.y);

         get_color(&col_tmp, PanelLight, 1);
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, Rb.x, Rb.y+1, (isLast && !reverseLayout)? (Rb.x+Rb.width-1):(Rb.x+Rb.width-1)-1, Rb.y+1);
      }
      else {
         get_color(&col_tmp, PanelContour, 1);
         alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 50);
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, Rb.x, Rb.y+Rb.height-1, (isLast && !reverseLayout)? (Rb.x+Rb.width-1):(Rb.x+Rb.width-1)-1, Rb.y+Rb.height-1);

         get_color(&col_tmp, PanelDark, 1);
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, Rb.x, (Rb.y+Rb.height-1)-1, (isLast && !reverseLayout)? (Rb.x+Rb.width-1):(Rb.x+Rb.width-1)-1, (Rb.y+Rb.height-1)-1);
      }

      /* fake the panel border edge / of the inactive tab: */
      if (isFirst && !reverseLayout) {
         if (!bottom) {
            get_color(&col_tmp, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 50);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x+1, Rb.y+1);

            get_color(&col_tmp, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 150);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x, Rb.y+1);

            mt_draw_point(win, DATA(gc_grey), Rb.x, Rb.y);

            get_color(&col_tmp, ButtonContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 50);
            get_color(&col_tmp2, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &col_tmp, &col_tmp2, 150);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x+1, Rb.y);
         }
         else {
            get_color(&col_tmp, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 50);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x+1, (Rb.y+Rb.height-1)-1);

            get_color(&col_tmp, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 150);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x, (Rb.y+Rb.height-1)-1);

            mt_draw_point(win, DATA(gc_grey), Rb.x, Rb.y+Rb.height-1);

            get_color(&col_tmp, ButtonContour, 1);
            alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 50);
            get_color(&col_tmp2, PanelContour, 1);
            alpha_blend_colors(&col_tmp, &col_tmp, &col_tmp2, 150);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_point(win, gc, Rb.x+1, Rb.y+Rb.height-1);
         }
      }
      /*
      TODO:
      if (isFirst && reverseLayout) {
         if (!bottom) {
            p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
            p->drawPoint(Rb.right()-1, Rb.y()+1 );
            p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 150) );
            p->drawPoint(Rb.right(), Rb.y()+1 );
            p->setPen(g.background() );
            p->drawPoint(Rb.right(), Rb.y() );
            p->setPen(alphaBlendColors( alphaBlendColors(g.background(), getColor(g, ButtonContour), 50), getColor(g,PanelContour), 150) );
            p->drawPoint(Rb.right()-1, Rb.y() );
         }
         else {
            p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 50) );
            p->drawPoint(Rb.right()-1, Rb.bottom()-1 );
            p->setPen(alphaBlendColors(g.background(), getColor(g,PanelContour), 150) );
            p->drawPoint(Rb.right(), Rb.bottom()-1 );
            p->setPen(g.background() );
            p->drawPoint(Rb.right(), Rb.bottom() );
            p->setPen(alphaBlendColors( alphaBlendColors(g.background(), getColor(g, ButtonContour), 50), getColor(g,PanelContour), 150) );
            p->drawPoint(Rb.right()-1, Rb.bottom() );
         }
      }
      */
   }

   mt_gc_destroy(gc);
}


static void draw_bitmap(MT_WINDOW *win, MT_GC *gc, int x, int y, int w, int h, unsigned char *bits, int inv)
{
   int i,j,offset;
   unsigned char m;

   offset = 0;
   for (i=0; i<h; i++) {
      m = 0x01;
      for (j=0; j<w; j++) {
         if ((!inv && (bits[offset] & m)) || (inv && !(bits[offset] & m))) {
            mt_draw_point(win, gc, x+j, y+i);
         }
         m <<= 1;
         if (m == 0) {
            offset++;
            m = 0x01;
         }
      }
      if (m != 0x01) offset++;
   }
}


static void draw_check(MT_WINDOW *win, int x, int y, int state)
{
   MT_COLOR col_content, col_checkmark, col_tmp;
   MT_GC *gc;

   col_content = (state & MT_DISABLED)? DATA(col_grey) : DATA(col_white);
   col_checkmark = (state & MT_DISABLED)? DATA(col_grey) : DATA(col_selected);

   if (!(state & MT_MOUSE_ACTIVE)) {
      alpha_blend_colors(&col_content, &col_content, &col_checkmark, 150);
   }

   if (state & MT_ACTIVE) {
      mt_color_dark(&col_tmp, &col_checkmark, 150);
      alpha_blend_colors(&col_tmp, &col_content, &col_tmp, 50);
      gc = mt_gc_new_with_foreground(&col_tmp);
      draw_bitmap(win, gc, x, y, CHECKMARK_SIZE, CHECKMARK_SIZE, checkmark_dark_bits, 0);

      mt_color_dark(&col_tmp, &col_checkmark, 125);
      alpha_blend_colors(&col_tmp, &col_content, &col_tmp, 50);
      mt_gc_set_foreground(gc, &col_tmp);
      draw_bitmap(win, gc, x, y, CHECKMARK_SIZE, CHECKMARK_SIZE, checkmark_light_bits, 0);

      mt_color_dark(&col_tmp, &col_checkmark, 150);
      alpha_blend_colors(&col_tmp, &col_content, &col_tmp, 150);
      mt_gc_set_foreground(gc, &col_tmp);
      draw_bitmap(win, gc, x, y, CHECKMARK_SIZE, CHECKMARK_SIZE, checkmark_aa_bits, 0);

      mt_gc_destroy(gc);
   }
}


static void draw_radio(MT_WINDOW *win, int x, int y, int state, MT_COLOR *col_bg)
{
   MT_COLOR col_contour, col_content, col_tmp, col_checkmark;
   MT_GC *gc;
   int surfaceFlags;
   int width = 13, height = 13;

   get_color(&col_contour, ButtonContour, !(state & MT_DISABLED));
   col_content = (state & MT_DISABLED)? DATA(col_grey) : DATA(col_white);
            
   surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
   if (state & MT_DISABLED) {
      surfaceFlags |= Is_Disabled;
   }
   else if (state & MT_HOVER) {
      alpha_blend_colors(&col_content, &col_content, &DATA(col_selected), 240);
   }
   
   render_surface(win, x, y, width, height, &DATA(col_grey), &col_content, &DATA(col_selected), (!(state & MT_DISABLED))?_contrast+3:(_contrast/2), surfaceFlags);
   /* TODO: */
   gc = mt_gc_new_with_foreground(col_bg);
   draw_bitmap(win, gc, x, y, 13, 13, radiobutton_mask_bits, 1);

   /* contour: */
   alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_contour, 50);
   mt_gc_set_foreground(gc, &col_tmp);
   draw_bitmap(win, gc, x, y, 13, 13, radiobutton_contour_bits, 0);

   /* anti-alias outside: */
   alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_contour, 150);
   mt_gc_set_foreground(gc, &col_tmp);
   draw_bitmap(win, gc, x, y, 13, 13, radiobutton_aa_outside_bits, 0);

   /* highlighting: */
   if (state & (MT_HOVER | MT_MOUSE_ACTIVE)) {
      alpha_blend_colors(&col_tmp, &col_content, &DATA(col_selected), 80);
      mt_gc_set_foreground(gc, &col_tmp);
      draw_bitmap(win, gc, x, y, 13, 13, radiobutton_highlight1_bits, 0);

      alpha_blend_colors(&col_tmp, &col_content, &DATA(col_selected), 150);
      mt_gc_set_foreground(gc, &col_tmp);
      draw_bitmap(win, gc, x, y, 13, 13, radiobutton_highlight2_bits, 0);
   }

   /* anti-alias inside, "above" the higlighting! */
   if (state & (MT_HOVER | MT_MOUSE_ACTIVE)) {
      alpha_blend_colors(&col_tmp, &DATA(col_selected), &col_contour, 180);
   }
   else {
      alpha_blend_colors(&col_tmp, &col_content, &col_contour, 180);
   }

   mt_gc_set_foreground(gc, &col_tmp);
   draw_bitmap(win, gc, x, y, 13, 13, radiobutton_aa_inside_bits, 0);

   /* radiomark: */
   col_content = (state & MT_DISABLED)? DATA(col_grey) : DATA(col_white);
   col_checkmark = (state & MT_DISABLED)? DATA(col_grey) : DATA(col_selected);
   
   if (state & MT_MOUSE_ACTIVE) {
      alpha_blend_colors(&col_checkmark, &col_content, &col_checkmark, 150);
   }

   if (state & (MT_ACTIVE | MT_MOUSE_ACTIVE)) {
      x += (13-9)/2;
      y += (13-9)/2;

      mt_color_dark(&col_tmp, &col_checkmark, 150);
      alpha_blend_colors(&col_tmp, &col_content, &col_tmp, 50);
      mt_gc_set_foreground(gc, &col_tmp);
      draw_bitmap(win, gc, x, y, CHECKMARK_SIZE, CHECKMARK_SIZE, radiomark_dark_bits, 0);

      mt_color_dark(&col_tmp, &col_checkmark, 125);
      alpha_blend_colors(&col_tmp, &col_content, &col_tmp, 50);
      mt_gc_set_foreground(gc, &col_tmp);
      draw_bitmap(win, gc, x, y, CHECKMARK_SIZE, CHECKMARK_SIZE, radiomark_light_bits, 0);

      mt_color_dark(&col_tmp, &col_checkmark, 150);
      alpha_blend_colors(&col_tmp, &col_content, &col_tmp, 150);
      mt_gc_set_foreground(gc, &col_tmp);
      draw_bitmap(win, gc, x, y, CHECKMARK_SIZE, CHECKMARK_SIZE, radiomark_aa_bits, 0);
   }

   mt_gc_destroy(gc);
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
         points[0].x = x;       points[0].y = y;
         points[1].x = x+w;     points[1].y = y;
         points[2].x = x+w/2;   points[2].y = y+h;
         break;

      case MT_ARROW_UP:
         points[0].x = x-1;     points[0].y = y+h;
         points[1].x = x+w/2;   points[1].y = y-1;
         points[2].x = x+w;     points[2].y = y+h;
         break;

      case MT_ARROW_LEFT:
         points[0].x = x+w;     points[0].y = y-1;
         points[1].x = x+w;     points[1].y = y+h;
         points[2].x = x;       points[2].y = y+h/2;
         break;

      case MT_ARROW_RIGHT:
         points[0].x = x;       points[0].y = y-1;
         points[1].x = x+w;     points[1].y = y+h/2;
         points[2].x = x;       points[2].y = y+h;
         break;
   }

   mt_draw_polygon(win, gc, 1, points, 3);
}


static void draw_pattern(MT_WINDOW *win, int type, int x, int y, int width, int height)
{
   mt_draw_pixmap_tiled(win, DATA(gc_grey), DATA(pattern_pixmap)[type], 64, 64, x, y, width, height);
}


static void draw_menu_check(MT_WINDOW *win, int x, int y, MT_GC *gc)
{
   int i;

   for (i=0; i<3; i++) {
      mt_draw_line(win, gc, x, y+2+i, x+2, y+2+2+i);
      mt_draw_line(win, gc, x+3, y+3+i, x+6, y+0+i);
   }
}


static void draw_menu_radio(MT_WINDOW *win, int x, int y, int width, int height, MT_GC *gc)
{
   mt_draw_rectangle(win, gc, 1, x, y+1, x+width-1, y+height-2);
   mt_draw_line(win, gc, x+1, y, x+width-2, y);
   mt_draw_line(win, gc, x+1, y+height-1, x+width-2, y+height-1);
}


void metatheme_draw_widget(MT_ENGINE *_engine, MT_WINDOW *win, MT_RECTANGLE *area, int type, int state, int x, int y, int width, int height, MT_WIDGET_DATA *data)
{
   MT_COLOR col_tmp, col_tmp2;
   MT_GC *gc;
   int contourFlags, surfaceFlags;
   int x2=x+width-1, y2=y+height-1;

   engine = _engine;
   widget_data = data;
   if (type != MT_BUTTON && type != MT_BUTTON_DEFAULT && type != MT_BUTTON_TOGGLE && data) {
      data->background_color = DATA(col_grey);
   }

   switch (type) {
      case MT_ARROW_UP:
      case MT_ARROW_DOWN:
      case MT_ARROW_LEFT:
      case MT_ARROW_RIGHT:
      {
         if (state & MT_DISABLED) {
            draw_arrow(win, DATA(gc_white), x+1, y+1, width, height, type);
            draw_arrow(win, DATA(gc_dark2), x, y, width, height, type);
         }
         else {
            draw_arrow(win, DATA(gc_black), x, y, width, height, type);
         }
         break;
      }

      case MT_BORDER_IN:
      case MT_BORDER_OUT:
      {
         render_panel(win, x, y, width, height, 1, (type == MT_BORDER_IN), 0, 1);
         break;
      }

      case MT_HLINE:
      {
         get_color(&col_tmp, PanelLight, 1);
         gc = mt_gc_new_with_foreground(&col_tmp);
         mt_draw_line(win, gc, x, y+1, x2, y+1);

         get_color(&col_tmp, PanelDark, 1);
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, x, y, x2, y);

         mt_gc_destroy(gc);
         break;
      }
      
      case MT_VLINE:
      {
         get_color(&col_tmp, PanelLight, 1);
         gc = mt_gc_new_with_foreground(&col_tmp);
         mt_draw_line(win, gc, x+1, y, x+1, y2);

         get_color(&col_tmp, PanelDark, 1);
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, x, y, x, y2);

         mt_gc_destroy(gc);
         break;
      }

      case MT_SCROLLED_WINDOW:
      {
         if (DATA(xp)) {
            mt_draw_rectangle(win, DATA(gc_white), 0, x+1, y+1, x+width-2, y+height-2);

            mt_color_set(col_tmp, 127, 157, 185);
            render_contour(win, x, y, width, height, &DATA(col_white), &col_tmp, DEFAULT_SURFACE_FLAGS);
         }
         else {
            render_contour(win, x, y, width, height, &DATA(col_grey), &DATA(col_dark2), DEFAULT_SURFACE_FLAGS);
         }
         break;
      }

      case MT_BUTTON:
      case MT_BUTTON_DEFAULT:
      case MT_BUTTON_TOGGLE:
      case MT_BUTTON_HEADER:
      {
         if (DATA(xp) && type == MT_BUTTON_HEADER) {
            x -= 3;
            y -= 3;
            width += 6;
            height += 3;
         }

         if (DATA(xp) && type != MT_BUTTON_HEADER) {
            mt_draw_rectangle(win, DATA(gc_grey), 0, x, y, x+width-1, y+height-1);

            /* button right light: */
            mt_color_light(&col_tmp, &DATA(col_grey), (state & MT_DISABLED)? 102 : 110);
            gc = mt_gc_new_with_foreground(&col_tmp);
            mt_draw_line(win, gc, x+3, y2, x2-3, y2);
            mt_draw_line(win, gc, x2, y+3, x2, y2-3);
            mt_draw_line(win, gc, x2-2, y2, x2, y2-2);
            
            /* top left shadow: */
            mt_color_dark(&col_tmp2, &DATA(col_grey), (state & MT_DISABLED)? 102 : 115);
            mt_gc_set_foreground(gc, &col_tmp2);
            mt_draw_line(win, gc, x+3, y, x2-3, y);
            mt_draw_line(win, gc, x, y+3, x, y2-3);
            mt_draw_line(win, gc, x, y+2, x+2, y);

            /* top right and bottom left corners: */
            alpha_blend_colors(&col_tmp, &col_tmp, &col_tmp2, 128);
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_line(win, gc, x2-2, y, x2, y+2);
            mt_draw_line(win, gc, x, y2-2, x+2, y2);

            mt_gc_destroy(gc);

            x += 1;
            y += 1;
            width -= 2;
            height -= 2;
         }

         if (type == MT_BUTTON_DEFAULT && !DATA(xp)) {
            contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Round_UpperLeft|Round_UpperRight|Round_BottomLeft|Round_BottomRight;
            if (state & MT_DISABLED) contourFlags |= Is_Disabled;
            mt_color_dark(&col_tmp, &DATA(col_grey), 120);
            render_contour(win, x, y, width, height, &DATA(col_grey), &col_tmp, contourFlags);

            x += 1;
            y += 1;
            width -= 2;
            height -= 2;
         }

         if (type == MT_BUTTON_HEADER) state &= ~MT_HOVER;
         
         render_button(win, x, y, width, height, state & MT_ACTIVE, state & MT_HOVER, 1, !(state & MT_DISABLED), (type != MT_BUTTON_DEFAULT) && !DATA(xp), 1, (type == MT_BUTTON_DEFAULT));

         if (DATA(xp) && type == MT_BUTTON_HEADER) {
            mt_color_dark(&col_tmp, &DATA(col_button), 120);
            gc = mt_gc_new_with_foreground(&col_tmp);
            mt_draw_line(win, gc, x+width-4, y+2, x+width-4, y+height-2);
            mt_gc_destroy(gc);
         }
         break;
      }

      case MT_MENUBAR_ITEM:
         if (DATA(xp) && (state & (MT_ACTIVE | MT_HOVER))) {
            mt_draw_rectangle(win, DATA(gc_selected), 1, x, y, x+width-1, y+height-1);
            break;
         }

      case MT_TOOLBAR_ITEM:
      case MT_TOOLBAR_ITEM_TOGGLE:
      {
         if (!(state & (MT_ACTIVE | MT_HOVER))) break;
         if (data) data->background_color = DATA(col_button);
         render_button(win, x, y, width, height, state & MT_ACTIVE, 0, 1, !(state & MT_DISABLED), 0, 0, 0);
         break;
      }
      
      case MT_TOOLBAR_ITEM_DROPDOWN:
      {
         int oldstate = state;
         
         if ((state & MT_ACTIVE) && (data->flags & MT_TOOLBAR_ITEM_DROPDOWN_ACTIVE)) {
            state &= ~MT_ACTIVE;
            state |= MT_HOVER;
         }
         
         metatheme_draw_widget(engine, win, area, MT_TOOLBAR_ITEM, state, x, y, width, height, data);

         state = oldstate;
         
         if ((state & MT_ACTIVE) && !(data->flags & MT_TOOLBAR_ITEM_DROPDOWN_ACTIVE)) {
            state &= ~MT_ACTIVE;
            state |= MT_HOVER;
         }

         x = x+width-13;
         width = 13;

         if (state & (MT_HOVER | MT_ACTIVE)) {
            contourFlags = Draw_Right|Draw_Top|Draw_Bottom|Round_UpperRight|Round_BottomRight;

            get_color(&col_tmp, ButtonContour, !(state & MT_DISABLED));
            render_contour(win, x, y, width, height, &DATA(col_grey), &col_tmp, contourFlags);
            gc = mt_gc_new_with_foreground(&col_tmp);
            mt_draw_line(win, gc, x, y, x, y+height-1);
            mt_gc_destroy(gc);
         }

         x++;
         y++;
         width -= 2;
         height -= 2;

         if (state & (MT_HOVER | MT_ACTIVE)) {
            surfaceFlags = contourFlags | Is_Horizontal;
            /*
            if (state & MT_HOVER) {
               surfaceFlags |= Is_Highlight;
               surfaceFlags |= Highlight_Top|Highlight_Left|Highlight_Right;
               if (type == MT_CHOICE_BUTTON) surfaceFlags |= Highlight_Bottom;
            }
            */
            if (state & MT_ACTIVE) surfaceFlags |= Is_Sunken;
            /*
            if (state & MT_DISABLED) surfaceFlags |= Is_Disabled;
            */
            render_surface(win, x, y, width, height, &DATA(col_grey), &DATA(col_button), &DATA(col_selected), _contrast, surfaceFlags);
         }

         x += 3;
         width -= 6;
         if (state & MT_ACTIVE) {
            y++;
         }
         metatheme_draw_widget(engine, win, area, MT_ARROW_DOWN, state, x, y, width, height, NULL);
         break;
      }

      case MT_TOOLBAR_SEPARATOR:
      {
         type = (data->orientation == MT_HORIZONTAL)? MT_VLINE : MT_HLINE;
         switch (type) {
            case MT_VLINE:
               y += 5;
               height -= 10;
               x++;
               break;

            case MT_HLINE:
               x += 5;
               width -= 10;
               y++;
               break;
         }
         metatheme_draw_widget(engine, win, area, type, state, x, y, width, height, NULL);
         break;
      }

      case MT_TOOLBAR_HANDLE:
      {
         int j, counter = 1;

         if (data->orientation == MT_HORIZONTAL) {
            int center = x+width/2;
            for (j = y+2; j<=y2-3; j+=3) {
               if (counter % 2 == 0) {
                  render_dot(win, center+1, j, &DATA(col_grey), 1, 1);
               }
               else {
                  render_dot(win, center-2, j, &DATA(col_grey), 1, 1);
               }
               counter++;
            }
         }
         else {
            int center = y+height/2;
            for (j=x+2; j<=x2-3; j+=3) {
               if (counter % 2 == 0) {
                  render_dot(win, j, center+1, &DATA(col_grey), 1, 1);
               }
               else {
                  render_dot(win, j, center-2, &DATA(col_grey), 1, 1);
               }
               counter++;
            }
         }
         break;
      }

      case MT_SCROLLBAR:
      {
         draw_pattern(win, 0, x, y, width, height);

         if (data->flags & MT_SCROLLBAR_SUBPAGE_ACTIVE) {
            if (data->orientation == MT_VERTICAL) {
               draw_pattern(win, 1, x, y+engine->metric[MT_SCROLLBAR_SIZE], width, data->handle_position);
               y += engine->metric[MT_SCROLLBAR_SIZE];
               height = data->handle_position;
            }
            else {
               draw_pattern(win, 1, x+engine->metric[MT_SCROLLBAR_SIZE], y, data->handle_position, height);
               x += engine->metric[MT_SCROLLBAR_SIZE];
               width = data->handle_position;
            }
         }
         else if (data->flags & MT_SCROLLBAR_ADDPAGE_ACTIVE) {
            int offset = engine->metric[MT_SCROLLBAR_SIZE] + data->handle_position + data->groove_size;
            
            if (data->orientation == MT_VERTICAL) {
               draw_pattern(win, 1, x, y+(offset & ~1), width, height-(offset & ~1) - engine->metric[MT_SCROLLBAR_SIZE]);
               y += offset;
               height -= offset + engine->metric[MT_SCROLLBAR_SIZE];
            }
            else {
               draw_pattern(win, 1, x+(offset & ~1), y, width-(offset & ~1) - engine->metric[MT_SCROLLBAR_SIZE], height);
               x += offset;
               width -= offset + engine->metric[MT_SCROLLBAR_SIZE];
            }
         }

         /* soften edges: */
         if (data->flags & (MT_SCROLLBAR_SUBPAGE_ACTIVE | MT_SCROLLBAR_ADDPAGE_ACTIVE)) {
            mt_draw_point(win, DATA(gc_dark2), x, y);
            mt_draw_point(win, DATA(gc_dark1), x+1, y);
            mt_draw_point(win, DATA(gc_dark1), x, y+1);

            mt_draw_point(win, DATA(gc_dark2), x+width-1, y);
            mt_draw_point(win, DATA(gc_dark1), x+width-2, y);
            mt_draw_point(win, DATA(gc_dark1), x+width-1, y+1);

            mt_draw_point(win, DATA(gc_dark2), x, y+height-1);
            mt_draw_point(win, DATA(gc_dark1), x+1, y+height-1);
            mt_draw_point(win, DATA(gc_dark1), x, y+height-2);

            mt_draw_point(win, DATA(gc_dark2), x+width-1, y+height-1);
            mt_draw_point(win, DATA(gc_dark1), x+width-2, y+height-1);
            mt_draw_point(win, DATA(gc_dark1), x+width-1, y+height-2);
         }
         break;
      }

      case MT_SCROLLBAR_ARROW_UP:
      case MT_SCROLLBAR_ARROW_DOWN:
      case MT_SCROLLBAR_ARROW_LEFT:
      case MT_SCROLLBAR_ARROW_RIGHT:
      {
         int w,h;

         if (data->flags & MT_SCROLLBAR_UNSCROLLABLE) state |= MT_DISABLED;
         
         contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Draw_Full;
         surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
         if (state & MT_ACTIVE) surfaceFlags|=Is_Sunken;

         if (state & MT_DISABLED) {
            contourFlags|=Is_Disabled;
            surfaceFlags|=Is_Disabled;
         }

         switch (type) {
            case MT_SCROLLBAR_ARROW_UP:
               contourFlags |= Round_UpperLeft|Round_UpperRight;
               surfaceFlags |= Round_UpperLeft|Round_UpperRight;
               break;

            case MT_SCROLLBAR_ARROW_DOWN:
               contourFlags |= Round_BottomLeft|Round_BottomRight;
               surfaceFlags |= Round_BottomLeft|Round_BottomRight;
               break;

            case MT_SCROLLBAR_ARROW_LEFT:
               contourFlags |= Round_UpperLeft|Round_BottomLeft|Is_Horizontal;
               surfaceFlags |= Round_UpperLeft|Round_BottomLeft|Is_Horizontal;
               break;

            case MT_SCROLLBAR_ARROW_RIGHT:
               contourFlags |= Round_UpperRight|Round_BottomRight|Is_Horizontal;
               surfaceFlags |= Round_UpperRight|Round_BottomRight|Is_Horizontal;
               break;
         }

         render_contour(win, x, y, width, height, &DATA(col_grey), &DATA(col_dark2), contourFlags);
         render_surface(win, x+1, y+1, width-2, height-2, &DATA(col_grey), &DATA(col_button), &DATA(col_white), (!(state & MT_DISABLED))?_contrast+3:(_contrast/2), surfaceFlags);
         
         w = width/2;
         h = height/2;
         x += (width-w)/2;
         y += (height-h)/2;

         if (BIT_IS_SET(state, MT_ACTIVE)) {
            x += 1;
            y += 1;
         }
         
         metatheme_draw_widget(engine, win, area, type - MT_SCROLLBAR_ARROW_UP + MT_ARROW_UP, state, x, y, w, h, NULL);
         break;
      }

      case MT_SCROLLBAR_HANDLE:
      {
         int d = 3;
         int n = (((data->orientation == MT_HORIZONTAL)? width : height) - 10) / d;
         int j,k;

         if (data->flags & MT_SCROLLBAR_UNSCROLLABLE) break;

         if (height < 4 || width < 4) break;

         contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
         if(state & MT_DISABLED) contourFlags|=Is_Disabled;

         get_color(&col_tmp, ButtonContour, 1);
         render_contour(win, x, y, width, height, &DATA(col_grey), &col_tmp, contourFlags);

         surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
         if (data->orientation == MT_HORIZONTAL) surfaceFlags|=Is_Horizontal;

         if (height >= 4) {
            render_surface(win, x+1, y+1, width-2, height-2, &DATA(col_grey), &DATA(col_button), &DATA(col_white), (!(state & MT_DISABLED))?_contrast+3:(_contrast/2), surfaceFlags);
         }

         /* dots part: */

         /*
         mt_color_dark(&col_tmp, &DATA(col_button), (state & MT_DISABLED)? 120:140);
         alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 50);
         */

         if (n > 5) n = 5;
         if (data->orientation == MT_VERTICAL) {
            for (j=0; j<n; j++) {
               int yPos = (y+height/2)-(n*3)/2+3*j+1;
               /*
               if (_scrollBarLines) {
                  p->drawLine(r.x()+1, yPos, r.right()-1, yPos);
               }
               */
               {
                  for(k=4; k<=12; k+=3) {
                     render_dot(win, x+k, yPos, &DATA(col_button), 0, 0);
                  }
               }
            }
         }
         else {
            for(j=0; j<n; j++) {
               int xPos = (x+width/2)-(n*3)/2+3*j+1;
               /*
               if(_scrollBarLines) {
                  p->drawLine(xPos, r.y()+1, xPos, r.bottom()-1);
               }
               */
               {
                  for(k=4; k<=12; k+=3) {
                     render_dot(win, xPos, y+k, &DATA(col_button), 0, 0);
                  }
               }
            }
         }
         break;
      }

      case MT_TOOLBAR:
      case MT_MENUBAR:
      {
         mt_draw_rectangle(win, DATA(gc_grey), 1, x, y, x2, y2);

         if (width > height) {
            get_color(&col_tmp, PanelLight, 1);
            gc = mt_gc_new_with_foreground(&col_tmp);
            mt_draw_line(win, gc, x, y, x2, y);
            mt_gc_destroy(gc);

            get_color(&col_tmp, PanelDark, 1);
            gc = mt_gc_new_with_foreground(&col_tmp);
            mt_draw_line(win, gc, x, y2, x2, y2);
            mt_gc_destroy(gc);
         }
         else {
            get_color(&col_tmp, PanelLight, 1);
            gc = mt_gc_new_with_foreground(&col_tmp);
            mt_draw_line(win, gc, x, y, x, y2);
            mt_gc_destroy(gc);

            get_color(&col_tmp, PanelDark, 1);
            gc = mt_gc_new_with_foreground(&col_tmp);
            mt_draw_line(win, gc, x2, y, x2, y2);
            mt_gc_destroy(gc);
         }
         break;
      }

      case MT_MENU:
      {
         if (DATA(xp)) {
            mt_color_set(col_tmp, 172, 168, 153);
            gc = mt_gc_new_with_foreground(&col_tmp);
            mt_draw_rectangle(win, gc, 0, x, y, x+width-1, y+height-1);
            mt_gc_destroy(gc);

            mt_draw_rectangle(win, DATA(gc_white), 0, x+1, y+1, x+width-2, y+height-2);
            mt_draw_rectangle(win, DATA(gc_white), 0, x+2, y+2, x+width-3, y+height-3);
            break;
         }

         mt_color_dark(&col_tmp, &DATA(col_grey), 200);
         render_contour(win, x, y, width, height, &DATA(col_grey), &col_tmp, Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);
         break;
      }

      case MT_MENU_ITEM:
      {
         if ((state & MT_SELECTED)) {
            if (DATA(xp)) {
               mt_draw_rectangle(win, DATA(gc_selected), 1, x, y, x+width-1, y+height-1);
            }
            else {
               render_surface(win, x, y, width, height, &DATA(col_grey), &DATA(col_selected), &DATA(col_selected), _contrast+3, Draw_Top|Draw_Bottom|Is_Horizontal);
            }
         }
         else {
            if (DATA(xp)) {
               col_tmp = DATA(col_white);
            }
            else {
               mt_color_light(&col_tmp, &DATA(col_grey), 105);
            }
            gc = mt_gc_new_with_foreground(&col_tmp);
            mt_draw_rectangle(win, gc, 1, x, y, x+width-1, y+height-1);
            mt_gc_destroy(gc);
         }
         break;
      }

      case MT_MENU_ITEM_CHECK:
      {
         if (BIT_IS_SET(state, MT_ACTIVE)) {
            draw_menu_check(win, x+(width-6)/2, y+(height-6)/2, BIT_IS_SET(state, MT_SELECTED)? DATA(gc_white) : DATA(gc_black));
         }
         break;
      }

      case MT_MENU_ITEM_RADIO:
      {
         if (BIT_IS_SET(state, MT_ACTIVE)) {
            draw_menu_radio(win, x+(width-6)/2, y+(height-6)/2, 6, 6, BIT_IS_SET(state, MT_SELECTED)? DATA(gc_white) : DATA(gc_black));
         }
         break;
      }

      case MT_MENU_ITEM_ARROW:
      {
         y += (height-7)/2;
         height = 7;

         if (state & MT_DISABLED) {
            draw_arrow(win, DATA(gc_white), x+1, y+1, width, height, MT_ARROW_RIGHT);
            draw_arrow(win, DATA(gc_dark2), x, y, width, height, MT_ARROW_RIGHT);
         }
         else {
            draw_arrow(win, (state & MT_SELECTED)? DATA(gc_white) : DATA(gc_black), x, y, width, height, MT_ARROW_RIGHT);
         }
         break;
      }

      case MT_MENU_ITEM_SEPARATOR:
      {
         int my = y + height/2;
         MT_COLOR col_base;

         if (DATA(xp)) {
            mt_draw_rectangle(win, DATA(gc_white), 1, x, y, x+width-1, y+height-1);

            mt_color_set(col_tmp, 172, 168, 153);
            gc = mt_gc_new_with_foreground(&col_tmp);
            mt_draw_line(win, gc, x+1, my, x+width-3, my);
            mt_gc_destroy(gc);
            break;
         }

         mt_color_light(&col_tmp, &DATA(col_grey), 105);
         gc = mt_gc_new_with_foreground(&col_tmp);
         mt_draw_rectangle(win, gc, 1, x, y, x+width-1, y+height-1);

         col_base = col_tmp;

         mt_color_dark(&col_tmp, &col_base, 120+_contrast*5);
         alpha_blend_colors(&col_tmp, &col_base, &col_tmp, 110);
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, x+5, my, x+width-10, my);

         mt_color_light(&col_tmp, &col_base, 120+_contrast*5);
         alpha_blend_colors(&col_tmp, &col_base, &col_tmp, 110);
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, x+5, my+1, x+width-10, my+1);

         mt_gc_destroy(gc);
         break;
      }

      case MT_MENU_TEAROFF:
      {
         MT_GC *gc1, *gc2;
         int i,max;
         
         metatheme_draw_widget(engine, win, area, MT_MENU_ITEM, state, x, y, width, height, data);

         x += 1;
         width -= 2;

         if (data->flags & MT_MENU_DETACHED) {
            draw_arrow(win, (state & MT_SELECTED)? DATA(gc_white) : DATA(gc_black), x, y+1, height, height-2, MT_ARROW_LEFT);
            y += 4;
            i = 10;
         }
         else {
            y += 1;
            i = 0;
         }

         get_color(&col_tmp, PanelDark, 1);
         gc1 = mt_gc_new_with_foreground(&col_tmp);

         get_color(&col_tmp, PanelLight, 1);
         gc2 = mt_gc_new_with_foreground(&col_tmp);

         while (i < width) {
            max = i+5;
            if (max >= width) max = width;
            
            mt_draw_line(win, gc1, x+i, y, x+max, y);
            mt_draw_line(win, gc2, x+i, y+1, x+max, y+1);

            i += 10;
         }

         mt_gc_destroy(gc1);
         mt_gc_destroy(gc2);
         break;
      }

      case MT_CHECK_BOX:
      case MT_CHECK_CELL:
      {
         MT_COLOR col_content;

         if (type == MT_CHECK_BOX && (data->flags & MT_DRAW_MASK)) {
            mt_draw_rectangle(win, DATA(gc_black), 1, x, y, x2, y2);
            return;
         }

         col_content = (state & MT_DISABLED)? DATA(col_grey) : DATA(col_white);
         contourFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom;
         if (state & MT_DISABLED) {
            contourFlags |= Is_Disabled;
         }

         get_color(&col_tmp, ButtonContour, 1);
         render_contour(win, x, y, width, height, &DATA(col_grey), &col_tmp, contourFlags);
         
         /* surface: */
         surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
         if (state & MT_DISABLED) {
            surfaceFlags |= Is_Disabled;
         }
         else if (state & (MT_HOVER | MT_MOUSE_ACTIVE)) {
            alpha_blend_colors(&col_content, &col_content, &DATA(col_selected), 240);
            surfaceFlags |= Is_Highlight;
            surfaceFlags |= Highlight_Left|Highlight_Right|Highlight_Top|Highlight_Bottom;
         }
         render_surface(win, x+1, y+1, width-2, height-2, &DATA(col_grey), &col_content, &DATA(col_selected), (!(state & MT_DISABLED))?_contrast+3:(_contrast/2), surfaceFlags);

         if (BIT_IS_SET(state, MT_ACTIVE)) {
            draw_check(win, x+(width-9)/2, y+(height-9)/2, state);
         }
         break;
      }

      case MT_RADIO_BOX:
      case MT_RADIO_CELL:
      {
         MT_COLOR *col_bg;

         x += (width - 13) / 2;
         y += (height - 13) / 2;

         if (type == MT_RADIO_BOX && (data->flags & MT_DRAW_MASK)) {
            draw_bitmap(win, DATA(gc_black), x, y, 13, 13, radiobutton_mask_bits, 0);
            return;
         }

         col_bg = &DATA(col_grey);
         if (type == MT_RADIO_CELL) {
            if (state & MT_SELECTED) {
               col_bg = &DATA(col_selected);
            }
            else {
               col_bg = &DATA(col_white);
            }
         }
         
         draw_radio(win, x, y, state, col_bg);
         break;
      }
      
      case MT_CHOICE:
         data->background_color = DATA(col_grey);
      case MT_SPINBUTTON:
      case MT_ENTRY_BORDER:
      {
         MT_COLOR col_content;

         contourFlags = DEFAULT_SURFACE_FLAGS | Draw_Full;
         if (type != MT_ENTRY_BORDER) {
            contourFlags &= ~(Draw_Right);
         }
         
         if (DATA(xp)) {
            mt_color_set(col_tmp, 127, 157, 185);
         }
         else {
            get_color(&col_tmp, ButtonContour, !(state & MT_DISABLED));
         }
         render_contour(win, x, y, width, height, &DATA(col_grey), &col_tmp, contourFlags);

         col_content = (state & MT_DISABLED)? DATA(col_grey) : DATA(col_white);

         if (DATA(xp)) {
            mt_color_set(col_tmp, 255, 255, 255);
         }
         else {
            mt_color_dark(&col_tmp, &col_content, 130);
         }
         gc = mt_gc_new_with_foreground(&col_tmp);
         mt_draw_line(win, gc, x+1, y+2, x+1, y2-2);
         mt_draw_line(win, gc, x+2, y+1, (type != MT_ENTRY_BORDER)? x2-1:x2-2, y+1);

         if (!DATA(xp)) {
            mt_color_light(&col_tmp, &col_content, 130);
         }
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, x+2, y2-1, (type != MT_ENTRY_BORDER)? x2-1:x2-2, y2-1);
         mt_draw_line(win, gc, x2-1, y+2, x2-1, y2-2);
         
         if (type != MT_ENTRY_BORDER) {
            get_color(&col_tmp, ButtonContour, !(state & MT_DISABLED));
            mt_gc_set_foreground(gc, &col_tmp);
            mt_draw_line(win, gc, x2, y, x2, y2);
         }

         mt_gc_destroy(gc);

         if (type == MT_CHOICE) {
            if (DATA(enhanced)) {
               metatheme_draw_widget(engine, win, area, MT_ENTRY, state, x+2, y+2, width-4, height-4, data);
            }
            else {
               mt_draw_rectangle(win, (state & MT_DISABLED)? DATA(gc_grey):DATA(gc_white), 1, x+2, y+2, x2-2, y2-2);
            }
         }
         
         break;
      }

      case MT_ENTRY:
      {
         MT_COLOR col_content;

         if (!DATA(enhanced)) return;

         col_content = DATA(col_white);

         surfaceFlags = Is_Sunken|Is_Horizontal;
         if (state & MT_DISABLED) {
            surfaceFlags |= Is_Disabled;
         }
         render_surface(win, x, y, width, height, &DATA(col_grey), &col_content, &DATA(col_selected), ((state & MT_DISABLED))?_contrast+3:(_contrast/2), surfaceFlags);
         break;
      }

      case MT_STATUSBAR_SECTION:
      {
         mt_color_dark(&col_tmp, &DATA(col_grey), 160);
         render_contour(win, x, y, width, height, &DATA(col_grey), &col_tmp, Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);
         break;
      }
      
      case MT_STATICBOX:
      {
         render_panel(win, x, y, width, height, 0, 1, 0, 0);

         if (data->gap_width > 0) {
            /*
            data->gap_x -= 3;
            data->gap_width += 6;
            */
            switch (data->gap_position) {
               case MT_POSITION_TOP:
                  mt_draw_line(win, DATA(gc_grey), x+data->gap_x, y, x+data->gap_x+data->gap_width-1, y);
                  break;
            }
         }

         break;
      }

      case MT_RESIZE_GRIP:
      {
         break;
      }

      case MT_NOTEBOOK:
      {
         render_panel(win, x, y, width, height, 1, 0, 1, 0);
         mt_draw_point(win, DATA(gc_grey), x2, y);
         mt_draw_point(win, DATA(gc_grey), x, y2);
         mt_draw_point(win, DATA(gc_grey), x2, y2);

         if (DATA(xp)) {
            MT_COLOR col_tmp2;
            
            mt_color_set(col_tmp, 252, 252, 254);
            mt_color_set(col_tmp2, 244, 243, 238);

            gc = mt_gc_new_with_foreground(&col_tmp2);
            mt_draw_gradient(win, gc, x+2, y+2, width-4, height/2-4, &col_tmp, &col_tmp2, MT_VERTICAL);
            mt_draw_rectangle(win, gc,1, x+2, y+2+height/2-4, x+width-3, y+height-3);
            mt_gc_destroy(gc);
         }

         if (DATA(highlight_notebook)) {
            mt_color_light(&col_tmp, &DATA(col_grey), 105);
            gc = mt_gc_new_with_foreground(&col_tmp);
            mt_draw_rectangle(win, gc, 1, x+2, y+2, x+width-3, y+height-3);
            mt_gc_destroy(gc);
         }
         break;
      }

      case MT_NOTEBOOK_TAB:
      {
         int pos = Middle;

         if (data->flags & MT_NOTEBOOK_FIRST_VISIBLE_TAB) data->flags |= MT_NOTEBOOK_FIRST_TAB;

         if (data->flags & MT_NOTEBOOK_FIRST_TAB) pos = First;
         if (data->flags & MT_NOTEBOOK_LAST_TAB) pos = (data->flags & MT_NOTEBOOK_FIRST_TAB)? Single : Last;

         switch (data->gap_position) {
            case MT_POSITION_TOP:
               render_tab(win, x, y, width, height, state & MT_HOVER, state & MT_SELECTED, 0, pos, 1);
               break;

            case MT_POSITION_BOTTOM:
               render_tab(win, x, y, width, height, state & MT_HOVER, state & MT_SELECTED, 1, pos, 1);
               break;
         }
         break;
      }

      case MT_NOTEBOOK_ARROW_LEFT:
      case MT_NOTEBOOK_ARROW_RIGHT:
      {
         metatheme_draw_widget(engine, win, area, MT_BUTTON, state, x, y, width, height, data);

         if (state & MT_ACTIVE) {
            x++;
            y++;
         }

         x += 4;
         width -= 8;
         y += 5;
         height -= 10;
         metatheme_draw_widget(engine, win, area, (type == MT_NOTEBOOK_ARROW_LEFT)? MT_ARROW_LEFT : MT_ARROW_RIGHT, state, x, y, width, height, data);
         break;
      }
         
      case MT_CHOICE_BUTTON:
         state &= ~(MT_ACTIVE);
      case MT_SPINBUTTON_UP:
      case MT_SPINBUTTON_DOWN:
      {
         int reverseLayout = 0;

         data->background_color = DATA(col_grey);

         if (data->flags & MT_SPINBUTTON_HOVER) state |= MT_HOVER;
         
         if (type == MT_SPINBUTTON_UP || type == MT_CHOICE_BUTTON) {
            contourFlags = Draw_Right|Draw_Top|Round_UpperLeft|Round_UpperRight;
            if (type == MT_CHOICE_BUTTON) {
               contourFlags |= Draw_Bottom|Round_BottomLeft|Round_BottomRight|Draw_Full;
            }

            get_color(&col_tmp, ButtonContour, !(state & MT_DISABLED));
            render_contour(win, x, y, width, height, &DATA(col_grey), &col_tmp, contourFlags);

            width -= 1;
            y++;
            height -= 1;
            if (type == MT_CHOICE_BUTTON) height--;

            surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;

            if (reverseLayout) {
               surfaceFlags |= Round_UpperLeft;
            }
            else {
               surfaceFlags |= Round_UpperRight;
            }
            if (type == MT_CHOICE_BUTTON) {
               surfaceFlags |= Round_BottomRight;
            }
            if (state & MT_HOVER) {
               surfaceFlags |= Is_Highlight;
               surfaceFlags |= Highlight_Top|Highlight_Left|Highlight_Right;
               if (type == MT_CHOICE_BUTTON) surfaceFlags |= Highlight_Bottom;
            }
            if (state & MT_ACTIVE) surfaceFlags |= Is_Sunken;
            /*
            if (state & MT_DISABLED) surfaceFlags |= Is_Disabled;
            */
            render_surface(win, x, y, width, height, &DATA(col_grey), &DATA(col_button), &DATA(col_selected), _contrast, surfaceFlags);
         }
         else {
            get_color(&col_tmp, ButtonContour, !(state & MT_DISABLED));

            gc = mt_gc_new_with_foreground(&col_tmp);
            mt_draw_line(win, gc, x, y, x2, y);
            mt_gc_destroy(gc);
            
            render_contour(win, x, y, width, height, &DATA(col_grey), &col_tmp, Draw_Right|Draw_Bottom|Round_BottomLeft|Round_BottomRight);

            width -= 1;
            y++;
            height -= 2;

            surfaceFlags = Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Is_Horizontal;
            if (reverseLayout) {
               surfaceFlags |= Round_BottomLeft;
            }
            else {
               surfaceFlags |= Round_BottomRight;
            }
            if (state & MT_HOVER) {
               surfaceFlags |= Is_Highlight;
               surfaceFlags |= Highlight_Bottom|Highlight_Left|Highlight_Right;
            }
            if (state & MT_ACTIVE) surfaceFlags |= Is_Sunken;
            /*
            if (state & MT_DISABLED) surfaceFlags |= Is_Disabled;
            */
            render_surface(win, x, y, width, height, &DATA(col_grey), &DATA(col_button), &DATA(col_selected), _contrast, surfaceFlags);

            y++;
         }

         x += 3;
         width -= 6;
         y += 2;
         height -= 4;
         if (type == MT_CHOICE_BUTTON) {
            x += 1;
            width -= 2;
         }
         
         metatheme_draw_widget(engine, win, area, (type == MT_SPINBUTTON_UP)? MT_ARROW_UP:MT_ARROW_DOWN, state, x, y, width, height, data);
         break;
      }

      case MT_SPLITTER:
      {
         MT_GC *gc1, *gc2;
         int k;

         mt_color_dark(&col_tmp, &DATA(col_grey), 130);
         render_contour(win, x, y, width, height, &DATA(col_grey), &col_tmp, Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);
         if (data->orientation == MT_HORIZONTAL) {
            if (height > 4) {
               mt_color_light(&col_tmp, &DATA(col_grey), 160);
               alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 110);
               gc1 = mt_gc_new_with_foreground(&col_tmp);

               mt_color_dark(&col_tmp, &DATA(col_grey), 160);
               alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 110);
               gc2 = mt_gc_new_with_foreground(&col_tmp);

               for (k = x+2*width/10; k < x+8*width/10; k+=5) {
                  int ycenter = y+height/2;
                  
                  mt_draw_line(win, gc1, k, ycenter-1, k+1, ycenter-1);
                  mt_draw_point(win, gc1, k, ycenter-1);

                  mt_draw_point(win, gc2, k+1, ycenter);
               }

               mt_gc_destroy(gc1);
               mt_gc_destroy(gc2);
            }
         }
         else {
            if (width > 4) {
               mt_color_light(&col_tmp, &DATA(col_grey), 160);
               alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 70);
               gc1 = mt_gc_new_with_foreground(&col_tmp);

               mt_color_dark(&col_tmp, &DATA(col_grey), 160);
               alpha_blend_colors(&col_tmp, &DATA(col_grey), &col_tmp, 70);
               gc2 = mt_gc_new_with_foreground(&col_tmp);

               for (k = y+2*height/10; k < y+8*height/10; k+=5) {
                  int xcenter = x+width/2;
                  mt_draw_line(win, gc1, xcenter-1, k, xcenter-1, k+1);
                  mt_draw_point(win, gc1, xcenter, k);

                  mt_draw_point(win, gc2, xcenter, k+1);
               }

               mt_gc_destroy(gc1);
               mt_gc_destroy(gc2);
            }
         }
         break;
      }

      case MT_SCALE:
      {
         switch (data->orientation) {
            case MT_HORIZONTAL:
               y += height/2-2;
               height = 4;
               break;

            case MT_VERTICAL:
               x += width/2-2;
               width = 4;
               break;
         }

         mt_color_dark(&col_tmp, &DATA(col_grey), (state & MT_DISABLED)? 130:150);
         render_contour(win, x, y, width, height, &DATA(col_grey), &col_tmp, Draw_Left|Draw_Right|Draw_Top|Draw_Bottom);
         break;
      }

      case MT_SCALE_HANDLE:
      {
         /* TODO: */
         render_button(win, x, y, width, height, 0, 0, (data->orientation == MT_HORIZONTAL), !(state & MT_DISABLED), 1, 0, 0);
         break;
      }

      case MT_PROGRESSBAR:
      {
         MT_COLOR col_content = (state & MT_DISABLED)? DATA(col_grey):DATA(col_white);
         get_color(&col_tmp, ButtonContour, !(state & MT_DISABLED));
         render_contour(win, x, y, width, height, &DATA(col_grey), &col_tmp, DEFAULT_SURFACE_FLAGS | Draw_Full);
         
         mt_color_dark(&col_tmp, &col_content, 105);
         gc = mt_gc_new_with_foreground(&col_tmp);
         mt_draw_line(win, gc, x+2, y+1, x2-2, y+1);
         mt_draw_line(win, gc, x+1, y+2, x+1, y2-2);
         
         mt_color_light(&col_tmp, &col_content, 105);
         mt_gc_set_foreground(gc, &col_tmp);
         mt_draw_line(win, gc, x+2, y2-1, x2-2, y2-1);
         mt_draw_line(win, gc, x2-1, y+2, x2-1, y2-2);

         mt_gc_destroy(gc);

         if (DATA(enhanced)) {
            surfaceFlags = Is_Sunken|Is_Horizontal;
            if (state & MT_DISABLED) {
               surfaceFlags |= Is_Disabled;
            }
            render_surface(win, x+2, y+2, width-4, height-4, &DATA(col_grey), &col_content, &DATA(col_selected), ((state & MT_DISABLED))?_contrast+3:(_contrast/2), surfaceFlags);
         }
         else {
            mt_draw_rectangle(win, (state & MT_DISABLED)? DATA(gc_grey):DATA(gc_white), 1, x+2, y+2, x2-2, y2-2);
         }
         break;
      }

      case MT_PROGRESSBAR_SLIDER:
      {
         MT_COLOR col_bg = (state & MT_DISABLED)? DATA(col_grey) : DATA(col_white);
         MT_COLOR col_fg = DATA(col_selected);

         if (state & MT_DISABLED) {
            mt_color_dark(&col_fg, &DATA(col_grey), 110);
         }

         if (DATA(enhanced)) {
            surfaceFlags = Is_Sunken|Is_Horizontal;
            if (state & MT_DISABLED) {
               surfaceFlags |= Is_Disabled;
            }
            render_surface(win, x, y, width, height, &DATA(col_grey), &col_bg, &DATA(col_selected), ((state & MT_DISABLED))?_contrast+3:(_contrast/2), surfaceFlags);
         }
         else {
            mt_draw_rectangle(win, (state & MT_DISABLED)? DATA(gc_grey):DATA(gc_white), 1, x, y, x2, y2);
         }

         if (width < 4 || height < 4) break;

         mt_color_dark(&col_tmp, &col_fg, 160);
         render_contour(win, x, y, width, height, &col_bg, &col_tmp, Draw_Left|Draw_Right|Draw_Top|Draw_Bottom|Round_UpperRight|Round_BottomRight|Round_UpperLeft|Round_BottomLeft);
         render_surface(win, x+1, y+1, width-2, height-2, &col_bg, &col_fg, &DATA(col_selected), 2*(_contrast/3), Draw_Right|Draw_Left|Draw_Top|Draw_Bottom|Round_UpperRight|Round_BottomRight|Round_UpperLeft|Round_BottomLeft|Is_Horizontal);
         break;
      }

      case MT_TOOLTIP:
      {
         mt_draw_rectangle(win, DATA(gc_black), 0, x, y, x+width-1, y+height-1);
         mt_draw_rectangle(win, DATA(gc_tooltip), 1, x+1, y+1, x+width-2, y+height-2);
         break;
      }
   }
}


void metatheme_draw_string(MT_ENGINE *engine, MT_WINDOW *win, int type, int state, MT_STRING *str)
{
   if (DATA(xp) && type == MT_MENUBAR_ITEM && (state & (MT_ACTIVE | MT_HOVER))) {
      mt_draw_string(win, str, 0, 0, &DATA(col_white));
      return;
   }

   if (state & MT_DISABLED) {
      mt_draw_string(win, str, 0, 0, &DATA(col_dark2));
      return;
   }

   mt_draw_string(win, str, 0, 0, NULL);
}
