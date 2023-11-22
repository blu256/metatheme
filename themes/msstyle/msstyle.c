/**
 * This file is part of MetaTheme.
 * Copyright (c) 2005 Martin Dvorak <jezek2@advel.cz>
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

#include <stdio.h>
#include <malloc.h>
#include "metatheme.h"
#include "metathemeInt.h"
#include "msstyle-loader.h"
#include "msstyle-ini.h"

#define DATA(name) ((ENGINE_DATA *)engine->data)->name

typedef struct {
   MT_COLOR col_dark1, col_dark2, col_white, col_black, col_grey, col_selected, col_yellow, col_tooltip;
   MT_GC *gc_dark1, *gc_dark2, *gc_white, *gc_black, *gc_grey, *gc_selected, *gc_yellow, *gc_tooltip;

   MT_ENGINE *redmond;
   ms_theme_t *ms;
   INI_FILE *ini;
   
   int flat_menus;
   MT_COLOR col_menu, col_menu_border, col_menu_border2, col_menu_item;
   MT_COLOR col_disabled, col_progressbar;

   int progress_chunk_size;
   int progress_space_size;
} ENGINE_DATA;

static MT_ENGINE *engine;

#define INI_COLOR(col, section, name) \
   { \
      int r,g,b; \
      char *tmp = msstyle_ini_get_option(DATA(ini), section, name); \
      if (tmp) { \
         sscanf(tmp, "%d %d %d", &r, &g, &b); \
         mt_color_set(col, r, g, b); \
      } \
   }

#define INI_PALETTE(type, section, name) \
   INI_COLOR(engine->palette[type], section, name)

#define INI_INT(i, section, name) \
   { \
      char *tmp = msstyle_ini_get_option(DATA(ini), section, name); \
      if (tmp) sscanf(tmp, "%d", &i); \
   }


void metatheme_init(MT_ENGINE *engine)
{
   char *tmp = NULL;
   int i;
   MT_CONFIG *cfg;
   const char *theme, *variant;

   /* initialization: */
   engine->data = (void *)calloc(1, sizeof(ENGINE_DATA));

   cfg = mt_get_config(engine->library);
   theme = mt_get_config_option(cfg, "fname", "msstyle");
   variant = mt_get_config_option(cfg, "ini", "msstyle");

   if (!theme || !variant) {
      fprintf(stderr, "This theme must be configured first.\n");
      goto redmond;
   }

   /* convert to absolute path: */
   if (theme[0] != '/') {
      char *home = getenv("HOME");
      tmp = malloc(strlen(home) + 21 + strlen(theme) + 1);
      strcpy(tmp, home);
      strcat(tmp, "/.metatheme/msstyles/");
      strcat(tmp, theme);
      theme = tmp;
   }

   DATA(ms) = ms_open_theme(theme);
   if (!DATA(ms)) {
      goto redmond;
   }

   if (tmp) free(tmp);

   DATA(redmond) = NULL;

   DATA(ini) = ms_load_ini(DATA(ms), variant);

   tmp = msstyle_ini_get_option(DATA(ini), "SysMetrics", "FlatMenus");
   DATA(flat_menus) = (tmp && !strcasecmp(tmp, "True"));

   INI_COLOR(DATA(col_menu),         "SysMetrics", "Menu");
   INI_COLOR(DATA(col_menu_border),  "SysMetrics", "BtnShadow");
   INI_COLOR(DATA(col_menu_border2), "SysMetrics", "BtnHighlight");
   INI_COLOR(DATA(col_menu_item),    "SysMetrics", "MenuHilight");
   INI_COLOR(DATA(col_disabled),     "SysMetrics", "GrayText");
   INI_COLOR(DATA(col_progressbar),  "Progress.Bar", "FillColorHint");
   
   tmp = msstyle_ini_get_option(DATA(ini), "SysMetrics", "ScrollbarWidth");
   sscanf(tmp, "%d", &i);
   mt_set_metric(engine, MT_SCROLLBAR_SIZE, i);
   mt_set_metric(engine, MT_CHOICE_BUTTON_WIDTH, i+2);

   INI_INT(DATA(progress_chunk_size), "Progress", "ProgressChunkSize");
   INI_INT(DATA(progress_space_size), "Progress", "ProgressSpaceSize");

   INI_PALETTE(MT_BACKGROUND,          "SysMetrics", "BtnFace");
   INI_PALETTE(MT_BUTTON_BACKGROUND,   "SysMetrics", "BtnFace");
   INI_PALETTE(MT_BUTTON_FOREGROUND,   "Button.PushButton", "TextColor");
   INI_PALETTE(MT_DISABLED_FOREGROUND, "SysMetrics", "GrayText");
   INI_PALETTE(MT_SELECTED_BACKGROUND, "SysMetrics", "Highlight");
   INI_PALETTE(MT_SELECTED_FOREGROUND, "SysMetrics", "HighlightText");

   INI_PALETTE(MT_BUTTON_LIGHT,        "Globals", "EdgeHighLightColor");
   INI_PALETTE(MT_BUTTON_MIDLIGHT,     "Globals", "EdgeLightColor");
   INI_PALETTE(MT_BUTTON_DARK,         "Globals", "EdgeDkShadowColor");
   INI_PALETTE(MT_BUTTON_MID,          "Globals", "EdgeShadowColor");

   mt_color_set(DATA(col_black),     0,   0,   0);
   mt_color_set(DATA(col_dark1),    64,  64,  64);
   mt_color_set(DATA(col_dark2),   128, 128, 128);
   mt_color_set(DATA(col_white),   255, 255, 255);
   mt_color_set(DATA(col_yellow),  245, 219, 149);
   mt_color_set(DATA(col_tooltip), 255, 255, 225);

   DATA(col_grey)     = engine->palette[MT_BACKGROUND];
   DATA(col_selected) = engine->palette[MT_SELECTED_BACKGROUND];

   engine->palette[MT_BUTTON_SHADOW]   = DATA(col_black);

   /* size metrics: */
   mt_set_metric_size(engine, MT_BUTTON_BORDER,             3, 3);
   mt_set_metric_size(engine, MT_BUTTON_TEXT_OFFSET,        0, 0);
   mt_set_metric_size(engine, MT_BUTTON_TOGGLE_BORDER,      3, 3);
   mt_set_metric_size(engine, MT_BUTTON_TOGGLE_TEXT_OFFSET, 0, 0);
   mt_set_metric_size(engine, MT_TOOLBAR_ITEM_BORDER,       3, 3);
   mt_set_metric_size(engine, MT_TOOLBAR_ITEM_TEXT_OFFSET,  0, 0);
   mt_set_metric_size(engine, MT_MENUBAR_BORDER,            0, 0); /* TODO */
   mt_set_metric_size(engine, MT_MENUBAR_ITEM_BORDER,       4, 3);
   mt_set_metric_size(engine, MT_MENUBAR_ITEM_TEXT_OFFSET,  0, 0);
   mt_set_metric_size(engine, MT_MENU_BORDER,               3, 3);
   mt_set_metric_size(engine, MT_MENU_ITEM_BORDER,          2, 2);
   mt_set_metric_size(engine, MT_PROGRESSBAR_BORDER,        3, 3);

   /* metrics: */
   mt_set_metric(engine, MT_BORDER,                         2);
   mt_set_metric(engine, MT_CHECKBOX_SIZE,                  13);
   mt_set_metric(engine, MT_SCALE_SLIDER_SIZE,              11);
   mt_set_metric(engine, MT_STATICBOX_TEXT_OFFSET,          5); /* TODO */
   mt_set_metric(engine, MT_MENUBAR_ITEM_SPACING,           0);
   mt_set_metric(engine, MT_MENU_SEPARATOR_HEIGHT,          9);
   mt_set_metric(engine, MT_NOTEBOOK_OVERLAP,               1);
   mt_set_metric(engine, MT_NOTEBOOK_TAB_OVERLAP,           3);
   mt_set_metric(engine, MT_NOTEBOOK_TEXT_OFFSET,           2);
   mt_set_metric(engine, MT_NOTEBOOK_ARROW_WIDTH,           17);
   mt_set_metric(engine, MT_NOTEBOOK_IS_FILLED,             1);
   mt_set_metric(engine, MT_TOOLBAR_ITEM_DROPDOWN_WIDTH,    12);
   mt_set_metric(engine, MT_SPLITTER_WIDTH,                 3);
   mt_set_metric(engine, MT_MENU_TEAROFF_HEIGHT,            5);
   mt_set_metric(engine, MT_DETACHED_MENU_TEAROFF_HEIGHT,   10);

   return;

redmond:
   DATA(redmond) = engine->toolkit->load_engine(engine->library, "redmond");
   if (!DATA(redmond)) {
      fprintf(stderr, "Can't fallback to Redmond.\n");
      exit(1);
   }
   
   for (i=0; i<MT_COLOR_MAX; i++) {
      DATA(redmond)->palette[i] = engine->palette[i];
   }

   if (DATA(redmond)->refcount == 1) {
      DATA(redmond)->init(DATA(redmond));
   }

   for (i=0; i<MT_METRIC_MAX; i++) {
      engine->metric[i] = DATA(redmond)->metric[i];
   }

   for (i=0; i<MT_METRIC_SIZE_MAX; i++) {
      engine->metric_size[i] = DATA(redmond)->metric_size[i];
   }
}


void metatheme_realize(MT_ENGINE *engine)
{
   MT_IMAGE *img1, *img2;
   int i,j,offset;

   if (DATA(redmond)) {
      DATA(redmond)->realize(DATA(redmond));
      return;
   }

   DATA(gc_dark1)    = mt_gc_new_with_foreground(&DATA(col_dark1));
   DATA(gc_dark2)    = mt_gc_new_with_foreground(&DATA(col_dark2));
   DATA(gc_white)    = mt_gc_new_with_foreground(&DATA(col_white));
   DATA(gc_black)    = mt_gc_new_with_foreground(&DATA(col_black));
   DATA(gc_grey)     = mt_gc_new_with_foreground(&DATA(col_grey));
   DATA(gc_selected) = mt_gc_new_with_foreground(&DATA(col_selected));
   DATA(gc_yellow)   = mt_gc_new_with_foreground(&DATA(col_yellow));
   DATA(gc_tooltip)  = mt_gc_new_with_foreground(&DATA(col_tooltip));
}


void metatheme_exit(MT_ENGINE *engine)
{
   if (DATA(redmond)) {
      DATA(redmond)->exit(DATA(redmond));
      engine->toolkit->unload_engine(DATA(redmond));
      return;
   }

   mt_gc_destroy(DATA(gc_dark1));
   mt_gc_destroy(DATA(gc_dark2));
   mt_gc_destroy(DATA(gc_white));
   mt_gc_destroy(DATA(gc_black));
   mt_gc_destroy(DATA(gc_grey));
   mt_gc_destroy(DATA(gc_selected));
   mt_gc_destroy(DATA(gc_tooltip));

   ms_close_theme(DATA(ms));

   free(engine->data);
}


#define DRAW_IMAGE() \
   if (s.width > 0 && s.height > 0 && d.width > 0 && d.height > 0) \
   mt_draw_image(win, DATA(gc_grey), img, s.x, s.y, s.width, s.height, d.x, d.y, d.width, d.height)

#define DRAW_TILED_IMAGE() \
   if (s.width > 0 && s.height > 0 && d.width > 0 && d.height > 0) \
   draw_tiled_image(win, DATA(gc_grey), img, s.x, s.y, s.width, s.height, d.x, d.y, d.width, d.height)

static void draw_tiled_image(MT_WINDOW *win, MT_GC *gc, MT_IMAGE *img, int xsrc, int ysrc, int wsrc, int hsrc, int xdest, int ydest, int width, int height)
{
   int x,y,w,h,width0;

   if (width <= 0 || height <= 0) return;

   y = ydest;
   width0 = width;
   while (height > 0) {
      h = MIN(hsrc, height);
      x = xdest;
      width = width0;
      while (width > 0) {
         w = MIN(wsrc, width);
         mt_draw_image(win, gc, img, xsrc, ysrc, w, h, x, y, w, h);
         x += w;
         width -= w;
      }
      y += h;
      height -= h;
   }
}


static void draw_stretched_box(MT_WINDOW *win, MT_IMAGE *img, MT_RECTANGLE *src, MT_RECTANGLE *dest, int bleft, int btop, int bright, int bbottom, int border_only, int tile)
{
   MT_RECTANGLE s, d;

   if (bleft + bright > dest->width) {
      bleft = dest->width / 2;
      bright = dest->width - bleft;
   }

   if (btop + bbottom > dest->height) {
      btop = dest->height / 2;
      bbottom = dest->height - btop;
   }
   
   if (btop > 0) {
      if (bleft > 0) {
         /* top left: */
         mt_rectangle_set(s, src->x, src->y, bleft, btop);
         mt_rectangle_set(d, dest->x, dest->y, bleft, btop);
         DRAW_IMAGE();
      }

      /* top: */
      mt_rectangle_set(s, src->x + bleft, src->y, src->width - bleft - bright, btop);
      mt_rectangle_set(d, dest->x + bleft, dest->y, dest->width - bleft - bright, btop);
      DRAW_IMAGE();

      if (bright > 0) {
         /* top right: */
         mt_rectangle_set(s, src->x + src->width - bright, src->y, bright, btop);
         mt_rectangle_set(d, dest->x + dest->width - bright, dest->y, bright, btop);
         DRAW_IMAGE();
      }
   }

   if (bleft > 0) {
      /* left: */
      mt_rectangle_set(s, src->x, src->y + btop, bleft, src->height - btop - bbottom);
      mt_rectangle_set(d, dest->x, dest->y + btop, bleft, dest->height - btop - bbottom);
      DRAW_IMAGE();
   }

   /* center: */
   if (!border_only) {
      mt_rectangle_set(s, src->x + bleft, src->y + btop, src->width - bleft - bright, src->height - btop - bbottom);
      mt_rectangle_set(d, dest->x + bleft, dest->y + btop, dest->width - bleft - bright, dest->height - btop - bbottom);
      if (tile) {
         DRAW_TILED_IMAGE();
      }
      else {
         DRAW_IMAGE();
      }
   }

   if (bright > 0) {
      /* right: */
      mt_rectangle_set(s, src->x + src->width - bright, src->y + btop, bright, src->height - btop - bbottom);
      mt_rectangle_set(d, dest->x + dest->width - bright, dest->y + btop, bright, dest->height - btop - bbottom);
      DRAW_IMAGE();
   }

   if (bbottom > 0) {
      if (bleft > 0) {
         /* bottom left: */
         mt_rectangle_set(s, src->x, src->y + src->height - bbottom, bleft, bbottom);
         mt_rectangle_set(d, dest->x, dest->y + dest->height - bbottom, bleft, bbottom);
         DRAW_IMAGE();
      }

      /* bottom: */
      mt_rectangle_set(s, src->x + bleft, src->y + src->height - bbottom, src->width - bleft - bright, bbottom);
      mt_rectangle_set(d, dest->x + bleft, dest->y + dest->height - bbottom, dest->width - bleft - bright, bbottom);
      DRAW_IMAGE();

      if (bright > 0) {
         /* bottom right: */
         mt_rectangle_set(s, src->x + src->width - bright, src->y + src->height - bbottom, bright, bbottom);
         mt_rectangle_set(d, dest->x + dest->width - bright, dest->y + dest->height - bbottom, bright, bbottom);
         DRAW_IMAGE();
      }
   }
}


static char *convert_name(char *buf, char *name)
{
   int i;

   strcpy(buf, name);
   for (i=strlen(buf)-1; i>=0; i--) {
      if (buf[i] == '\\' || buf[i] == '.') buf[i] = '_';
      else buf[i] = toupper(buf[i]);
   }
   return buf;
}


#define OPTION(name) msstyle_ini_get_class_option(DATA(ini), class1, class2, state_name, name)
#define OPTION_IS(name, value) ((option = OPTION(name)) && !strcasecmp(option, value))

#define DRAW_WIDGET(class1, class2, offset) \
   draw_widget(win, x, y, width, height, class1, class2, state, offset);

static void draw_widget(MT_WINDOW *win, int x, int y, int width, int height, char *class1, char *class2, int state, int offset)
{
   char *state_name = "";
   int ms_state = 0;
   char *tmp, *option;
   char buf[128];
   int i, iw, ih, imgcount=1;
   MT_IMAGE *img;
   MT_RECTANGLE src, dest;
   MT_COLOR col;
   MT_GC *gc;

   if (state & MT_DISABLED) {
      state_name = "Disabled";
      ms_state = 3;
   }
   else if (state & MT_ACTIVE) {
      state_name = "Pressed";
      ms_state = 2;
   }
   else if (state & MT_HOVER) {
      state_name = "Hot";
      ms_state = 1;
   }

   if (offset >= 0) {
      offset += ms_state;
   }
   else {
      offset = -offset;
   }

   if (class1 && class2 && !strcmp(class1, "ComboBox") && !strcmp(class2, "DropDownButton")) {
      i = 0;
      tmp = OPTION("BorderSize");
      if (tmp) sscanf(tmp, "%d", &i);

      x += i;
      y += i;
      width -= i*2;
      height -= i*2;
   }

   if (OPTION_IS("BgType", "ImageFile")) {
      /* if (OPTION_IS("SizingType", "Stretch")) { */
         int bleft, btop, bright, bbottom;

         tmp = OPTION("SizingMargins");
         if (tmp) {
            sscanf(tmp, "%d,%d,%d,%d", &bleft, &bright, &btop, &bbottom);
         }
         else {
            bleft = 0;
            btop = 0;
            bright = 0;
            bbottom = 0;
         }

         tmp = OPTION("ImageFile");
         if (!tmp) tmp = OPTION("StockImageFile");

         if (!tmp && OPTION_IS("ImageSelectType", "DPI")) {
            if (OPTION_IS("MinDpi1", "96")) tmp = OPTION("ImageFile1");
            else if (OPTION_IS("MinDpi2", "96")) tmp = OPTION("ImageFile2");
            else if (OPTION_IS("MinDpi3", "96")) tmp = OPTION("ImageFile3");
         }
         if (!tmp) {
            printf("no image file for %s %s\n", class1, class2);
            return;
         }
         
         img = ms_load_widget(DATA(ms), engine, convert_name(buf, tmp), 0);
         if (!img) return;

         mt_image_get_size(img, &iw, &ih);

         if (OPTION_IS("Transparent", "true")) {
            tmp = OPTION("TransparentColor");
            if (tmp) {
               int r,g,b;
               sscanf(tmp, "%d %d %d", &r, &g, &b); \
               mt_color_set2(col, 0, 0, 0, 0);
               mt_image_replace_color(img, r, g, b, &col);
            }
         }

         /*
         if (!OPTION_IS("Transparent", "True")) {
            int i,j;
            MT_COLOR col;
            
            for (i=0; i<ih; i++) {
               for (j=0; j<iw; j++) {
                  mt_image_get_pixel(img, j, i, &col);
                  col.a = 255;
                  mt_image_set_pixel(img, j, i, &col);
               }
            }
         }
         */

         tmp = OPTION("ImageCount");
         if (tmp) {
            sscanf(tmp, "%d", &imgcount);
         }
         else {
            imgcount = 1;
         }

         if (offset >= imgcount) offset = 0;

         if (OPTION_IS("ImageLayout", "Vertical")) {
            ih /= imgcount;
            mt_rectangle_set(src, 0, ih*offset, iw, ih);
         }
         else if (OPTION_IS("ImageLayout", "Horizontal")) {
            iw /= imgcount;
            mt_rectangle_set(src, iw*offset, 0, iw, ih);
         }
         else {
            mt_rectangle_set(src, 0, 0, iw, ih);
         }

         if (OPTION_IS("SizingType", "TrueSize")) {
            /* if (src.width >= dest.width && src.height >= dest.height) { */
               mt_rectangle_set(dest, x+(width-iw)/2, y+(height-ih)/2, iw, ih);
               if (dest.width > 0 && dest.height > 0) {
                  mt_draw_image(win, DATA(gc_grey), img, src.x, src.y, src.width, src.height, dest.x, dest.y, dest.width, dest.height);
               }
            /* } */
         }
         else {
            mt_rectangle_set(dest, x, y, width, height);
            draw_stretched_box(win, img, &src, &dest, bleft, btop, bright, bbottom, OPTION_IS("BorderOnly", "True"), OPTION_IS("SizingType", "Tile"));
         }

      /* } */
   }
   else if (OPTION_IS("BgType", "BorderFill")) {
      mt_color_set(col, 0, 0, 0);
      gc = mt_gc_new_with_foreground(&col);

      tmp = OPTION("BorderColor");
      if (tmp) {
         sscanf(OPTION("BorderColor"), "%d %d %d", &col.r, &col.g, &col.b);
         mt_gc_set_foreground(gc, &col);
         mt_draw_rectangle(win, gc, 0, x, y, x+width-1, y+height-1);
      }

      tmp = OPTION("FillColor");
      if (tmp) {
         sscanf(tmp, "%d %d %d", &col.r, &col.g, &col.b);
         mt_gc_set_foreground(gc, &col);
         mt_draw_rectangle(win, gc, 1, x+1, y+1, x+width-2, y+height-2);
      }

      mt_gc_destroy(gc);
   }

   if (OPTION_IS("GlyphType", "ImageGlyph")) {
      tmp = OPTION("ImageFile2");
      if (!tmp) tmp = OPTION("GlyphImageFile");
      if (!tmp) return;

      img = ms_load_widget(DATA(ms), engine, convert_name(buf, tmp), 0);
      if (!img) return;

      mt_image_get_size(img, &iw, &ih);

      if (OPTION_IS("ImageLayout", "Vertical")) {
         ih /= imgcount;
         mt_rectangle_set(src, 0, ih*offset, iw, ih);
      }
      else if (OPTION_IS("ImageLayout", "Horizontal")) {
         iw /= imgcount;
         mt_rectangle_set(src, iw*offset, 0, iw, ih);
      }

      mt_rectangle_set(dest, x+(width-iw)/2, y+(height-ih)/2, iw, ih);
      if (src.width > 0 && src.height > 0 && dest.width >= 0 && dest.height >= 0) {
         mt_draw_image(win, DATA(gc_grey), img, src.x, src.y, src.width, src.height, dest.x, dest.y, dest.width, dest.height);
      }
   }
}


static void draw_simple_box(MT_WINDOW *win, int x, int y, int width, int height, MT_GC *gc1, MT_GC *gc2, int edge_from_first_gc)
{
   if (edge_from_first_gc) {
      mt_draw_line(win, gc1, x, y, x+width-1, y);
      mt_draw_line(win, gc1, x, y, x, y+height-1);
      mt_draw_line(win, gc2, x+width-1, y+1, x+width-1, y+height-1);
      mt_draw_line(win, gc2, x+1, y+height-1, x+width-1, y+height-1);
   }
   else {
      mt_draw_line(win, gc1, x, y, x+width-2, y);
      mt_draw_line(win, gc1, x, y, x, y+height-2);
      mt_draw_line(win, gc2, x+width-1, y, x+width-1, y+height-1);
      mt_draw_line(win, gc2, x, y+height-1, x+width-1, y+height-1);
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

   if (gap_position != MT_POSITION_TOP)    mt_draw_line(win, gc1, x, y, x+width-2, y);
   if (gap_position != MT_POSITION_LEFT)   mt_draw_line(win, gc1, x, y, x, y+height-2);
   if (gap_position != MT_POSITION_RIGHT)  mt_draw_line(win, gc2, x+width-1, y, x+width-1, y+height-1);
   if (gap_position != MT_POSITION_BOTTOM) mt_draw_line(win, gc2, x, y+height-1, x+width-1, y+height-1);
}


static void draw_dotted_box(MT_WINDOW *win, int x, int y, int width, int height, MT_GC *gc)
{
   int i;
   int h_offset, v_offset;

   h_offset = (height & 1)? 0:1;
   v_offset = (width & 1)? 0:1;
   
   for (i=0; i<width; i+=2) {
      mt_draw_point(win, gc, x+i, y);
   }
   for (i=h_offset; i<width; i+=2) {
      mt_draw_point(win, gc, x+i, y+height-1);
   }

   for (i=0; i<height; i+=2) {
      mt_draw_point(win, gc, x, y+i);
   }
   for (i=v_offset; i<height; i+=2) {
      mt_draw_point(win, gc, x+width-1, y+i);
   }
}


static void draw_box(MT_WINDOW *win, int x, int y, int width, int height, int thick, int flatdown, int down, int default_btn)
{
   if (!down) {
      if (!thick) {
         if (default_btn) {
            draw_simple_box(win, x, y, width, height, DATA(gc_black), DATA(gc_black), 0);
            draw_simple_box(win, x+1, y+1, width-3, height-3, DATA(gc_white), DATA(gc_dark2), 1);
            mt_draw_line(win, DATA(gc_dark1), x+width-2, y+1, x+width-2, y+height-2);
            mt_draw_line(win, DATA(gc_dark1), x+1, y+height-2, x+width-2, y+height-2);
         }
         else {
            draw_simple_box(win, x+1, y+1, width-2, height-2, DATA(gc_white), DATA(gc_dark2), 0);
            draw_simple_box(win, x, y, width, height, DATA(gc_grey), DATA(gc_dark1), 0);
         }
      }
      else {
         draw_simple_box(win, x+1, y+1, width-2, height-2, DATA(gc_grey), DATA(gc_dark2), 0);
         draw_simple_box(win, x, y, width, height, DATA(gc_white), DATA(gc_dark1), 0);
      }
   }
   else {
      if (!thick && flatdown) {
         draw_simple_box(win, x+1, y+1, width-2, height-2, DATA(gc_dark2), DATA(gc_dark2), 0);
         draw_simple_box(win, x, y, width, height, DATA(gc_black), DATA(gc_black), 0);
      }
      else if (thick && flatdown) {
         draw_simple_box(win, x, y, width, height, DATA(gc_dark2), DATA(gc_dark2), 0);
      }
      else if (!thick && !flatdown) {
         draw_simple_box(win, x+1, y+1, width-2, height-2, DATA(gc_dark1), DATA(gc_grey), 0);
         draw_simple_box(win, x, y, width, height, DATA(gc_dark2), DATA(gc_white), 0);
      }
      else if (thick && !flatdown) {
         draw_simple_box(win, x, y, width, height, DATA(gc_dark2), DATA(gc_white), 0);
      }
   }
}


static void draw_box_with_gap(MT_WINDOW *win, int x, int y, int width, int height, int gap_position, int gap_x, int gap_width)
{
   draw_simple_box_with_gap(win, x+1, y+1, width-2, height-2, DATA(gc_grey), DATA(gc_dark2), gap_position, gap_x-1, gap_width);
   draw_simple_box_with_gap(win, x, y, width, height, DATA(gc_white), DATA(gc_dark1), gap_position, gap_x, gap_width);
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


void metatheme_draw_widget(MT_ENGINE *_engine, MT_WINDOW *win, MT_RECTANGLE *area, int type, int state, int x, int y, int width, int height, MT_WIDGET_DATA *data)
{
   engine = _engine;

   if (DATA(redmond)) {
      DATA(redmond)->draw_widget(DATA(redmond), win, area, type, state, x, y, width, height, data);
      return;
   }

   switch (type) {
      case MT_BASE:
         break;

      case MT_WINDOW_BASE:
         break;
         
      case MT_BORDER_IN:
      {
         DRAW_WIDGET("Edit", "EditText", 0);
         break;
      }

      case MT_BORDER_OUT:
      {
         DRAW_WIDGET("Edit", "EditText", 0);
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
         draw_dotted_box(win, x, y, width, height, DATA(gc_black));
         break;
      }

      case MT_RESIZE_GRIP:
      {
         /* DRAW_WIDGET("Status", "Gripper", 0); */
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
         DRAW_WIDGET("Button", "PushButton", 0);
         break;
      }

      case MT_BUTTON_HEADER:
      {
         DRAW_WIDGET("Header", "HeaderItem", 0);
         break;
      }

      case MT_MENU:
      {
         if (DATA(flat_menus)) {
            MT_COLOR col_tmp;
            MT_GC *gc;

            gc = mt_gc_new_with_foreground(&DATA(col_menu_border));
            mt_draw_rectangle(win, gc, 0, x, y, x+width-1, y+height-1);
            mt_gc_set_foreground(gc, &DATA(col_menu));
            mt_draw_rectangle(win, gc, 0, x+1, y+1, x+width-2, y+height-2);
            mt_draw_rectangle(win, gc, 0, x+2, y+2, x+width-3, y+height-3);
            mt_gc_destroy(gc);
            break;
         }

         draw_box(win, x, y, width, height, 0, 0, 0, 0);
         mt_draw_rectangle(win, DATA(gc_grey), 0, x+2, y+2, x+width-3, y+height-3);
         break;
      }
      
      case MT_MENUBAR_ITEM:
      case MT_MENU_ITEM:
      {
         MT_GC *gc;

         if (type == MT_MENUBAR_ITEM) {
            if (state & (MT_ACTIVE | MT_HOVER)) {
               state |= MT_SELECTED;
            }
            else break;
         }
         
         if ((state & MT_SELECTED)) {
            gc = mt_gc_new_with_foreground(&DATA(col_menu_item));
         }
         else {
            gc = mt_gc_new_with_foreground(&DATA(col_menu));
         }
         
         mt_draw_rectangle(win, gc, 1, x, y, x+width-1, y+height-1);

         if ((state & MT_SELECTED)) {
            mt_gc_set_foreground(gc, &engine->palette[MT_SELECTED_BACKGROUND]);
            mt_draw_rectangle(win, gc, 0, x, y, x+width-1, y+height-1);
         }

         mt_gc_destroy(gc);
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
         MT_GC *gc;
         
         gc = mt_gc_new_with_foreground(&DATA(col_menu));
         mt_draw_rectangle(win, gc, 1, x, y, x+width-1, y+height-1);

         int my = y + height/2 - 1;
         mt_gc_set_foreground(gc, &DATA(col_menu_border));
         mt_draw_line(win, gc, x+1, my, x+width-2, my);
         mt_gc_set_foreground(gc, &DATA(col_menu_border2));
         mt_draw_line(win, gc, x+1, my+1, x+width-2, my+1);

         mt_gc_destroy(gc);
         break;
      }

      case MT_MENU_TEAROFF:
      {
         int i,max;
         
         if ((state & MT_SELECTED)) {
            mt_draw_rectangle(win, DATA(gc_selected), 1, x, y, x+width-1, y+height-1);
         }
         else {
            mt_draw_rectangle(win, DATA(gc_grey), 1, x, y, x+width-1, y+height-1);
         }

         if (data->flags & MT_MENU_DETACHED) {
            draw_arrow(win, (state & MT_SELECTED)? DATA(gc_white) : DATA(gc_black), x, y+1, height, height-2, MT_ARROW_LEFT);
            y += 4;
            i = 10;
         }
         else {
            y += 1;
            i = 0;
         }

         while (i < width) {
            max = i+5;
            if (max >= width) max = width;
            
            mt_draw_line(win, DATA(gc_dark2), x+i, y, x+max, y);
            mt_draw_line(win, DATA(gc_white), x+i, y+1, x+max, y+1);

            i += 10;
         }
         break;
      }

      case MT_TOOLBAR:
      {
         //DRAW_WIDGET("Toolbar", NULL, 0);
         break;
      }
      
      case MT_TOOLBAR_ITEM:
      case MT_TOOLBAR_ITEM_TOGGLE:
      {
         int offset = 0;

         if (type == MT_TOOLBAR_ITEM_TOGGLE && (state & MT_ACTIVE)) {
            offset = (state & MT_HOVER)? -5 : -4;
         }
         
         DRAW_WIDGET("Toolbar", "Button", offset);
         break;
      }

      case MT_TOOLBAR_ITEM_DROPDOWN:
      {
         int oldstate = state;
         
         if ((state & MT_ACTIVE) && (data->flags & MT_TOOLBAR_ITEM_DROPDOWN_ACTIVE)) {
            state &= ~MT_ACTIVE;
            state |= MT_HOVER;
         }

         width -= 12;
         DRAW_WIDGET("Toolbar", "SplitButton", 0);
         
         state = oldstate;
         x += width;
         width = 12;
         DRAW_WIDGET("Toolbar", "SplitButtonDropdown", 0);
         break;
      }

      case MT_TOOLBAR_SEPARATOR:
      {
         DRAW_WIDGET("Toolbar", (data->orientation == MT_HORIZONTAL)? "Separator":"SeparatorVert", 0);
         break;
      }

      case MT_TOOLBAR_HANDLE:
      {
         if (data->orientation == MT_HORIZONTAL) {
            width = 6;
            DRAW_WIDGET("Rebar", "Gripper", 0);
         }
         else {
            height = 6;
            DRAW_WIDGET("Rebar", "GripperVert", 0);
         }
         break;
      }

      case MT_SCROLLBAR:
      {
         state = 0;
         DRAW_WIDGET("Scrollbar", (data->orientation == MT_HORIZONTAL)? "LowerTrackHorz":"LowerTrackVert", 0);
         break;
      }

      case MT_SCROLLBAR_ARROW_UP:
      case MT_SCROLLBAR_ARROW_DOWN:
      case MT_SCROLLBAR_ARROW_LEFT:
      case MT_SCROLLBAR_ARROW_RIGHT:
      {
         int offset = 0;

         if (data->flags & MT_SCROLLBAR_UNSCROLLABLE) state |= MT_DISABLED;

         switch (type) {
            case MT_SCROLLBAR_ARROW_UP:    offset = 0; break;
            case MT_SCROLLBAR_ARROW_DOWN:  offset = 1; break;
            case MT_SCROLLBAR_ARROW_LEFT:  offset = 2; break;
            case MT_SCROLLBAR_ARROW_RIGHT: offset = 3; break;
         }
         
         DRAW_WIDGET("Scrollbar", "ArrowBtn", offset*4);
         break;
      }

      case MT_SCROLLBAR_HANDLE:
      {
         if ((data->flags & MT_SCROLLBAR_UNSCROLLABLE)) break;

         DRAW_WIDGET("Scrollbar", (data->orientation == MT_HORIZONTAL)? "ThumbBtnHorz":"ThumbBtnVert", 0);
         DRAW_WIDGET("Scrollbar", (data->orientation == MT_HORIZONTAL)? "GripperHorz":"GripperVert", 0);
         break;
      }

      case MT_PROGRESSBAR:
      {
         DRAW_WIDGET("Progress", (data->orientation == MT_HORIZONTAL)? "Bar":"BarVert", 0);
         break;
      }

      case MT_PROGRESSBAR_SLIDER:
      {
         int chunk_size = DATA(progress_chunk_size) + DATA(progress_space_size);
         int i, count;

         if (data->orientation == MT_HORIZONTAL) {
            count = width / chunk_size;
            if (data->flags & MT_PROGRESSBAR_REVERSED) {
               x += width - DATA(progress_chunk_size);
            }
            width = DATA(progress_chunk_size);
            for (i=0; i<count; i++) {
               DRAW_WIDGET("Progress", "Chunk", 0);
               x += (data->flags & MT_PROGRESSBAR_REVERSED)? -chunk_size : chunk_size;
            }
         }
         else {
            count = height / chunk_size;
            if (!(data->flags & MT_PROGRESSBAR_REVERSED)) {
               y += height - DATA(progress_chunk_size);
            }
            height = DATA(progress_chunk_size);
            for (i=0; i<count; i++) {
               DRAW_WIDGET("Progress", "Chunk", 0);
               y += (data->flags & MT_PROGRESSBAR_REVERSED)? chunk_size : -chunk_size;
            }
         }
         break;
      }

      case MT_NOTEBOOK:
      {
         DRAW_WIDGET("Tab", "Pane", 0);
         break;
      }

      case MT_NOTEBOOK_TAB:
      {
         int offset = (state & MT_SELECTED)? -2 : 0;
         char name[32], *pos="", *other="";

         switch (data->gap_position) {
            case MT_POSITION_TOP: pos = "Top"; break;
         }

         if (data->flags & MT_NOTEBOOK_FIRST_VISIBLE_TAB) {
            data->flags |= MT_NOTEBOOK_FIRST_TAB;
         }
         data->flags &= MT_NOTEBOOK_FIRST_TAB | MT_NOTEBOOK_LAST_TAB;


         /*
         if (data->flags == (MT_NOTEBOOK_FIRST_TAB | MT_NOTEBOOK_LAST_TAB)) {
            other = "BothEdge";
         }
         else if (data->flags & MT_NOTEBOOK_FIRST_TAB) {
            other = "LeftEdge";
         }
         else if (data->flags & MT_NOTEBOOK_LAST_TAB) {
            other = "RightEdge";
         }
         */

         if (data->flags & MT_NOTEBOOK_FIRST_TAB) {
            other = "LeftEdge";
         }

         if (!(state & MT_SELECTED)) {
            x += 2;
            y += 2;
            height -= 3;
            width -= 3;
         }
         else {
            height += 1;
         }

         /* fixes for themes: */
         if ((state & MT_SELECTED) && (data->flags & MT_NOTEBOOK_FIRST_TAB) && !strcmp(DATA(ms)->ms_theme_info->name, "BlackMesa")) {
            x++;
            width--;
         }

         sprintf(name, "%sTabItem%s", pos, other);
         DRAW_WIDGET("Tab", name, offset);
         break;
      }

      case MT_NOTEBOOK_ARROW_LEFT:
      case MT_NOTEBOOK_ARROW_RIGHT:
      {
         mt_draw_rectangle(win, DATA(gc_grey), 1, x, y, x+width-1, y+height-1);

         mt_draw_line(win, DATA(gc_white), x, y+height-2, x+width-1, y+height-2);
         if (type == MT_NOTEBOOK_ARROW_RIGHT) {
            mt_draw_line(win, DATA(gc_dark1), x+width-1, y+height-2, x+width-1, y+height-1);
            mt_draw_point(win, DATA(gc_dark2), x+width-2, y+height-1);
         }

         y = y+height - width - 4;
         height = width;

         draw_box(win, x, y, width, height, 0, 0, BIT_IS_SET(state, MT_ACTIVE), 0);

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
      case MT_RADIO_BOX:
      case MT_RADIO_CELL:
      {
         int offset = 0;

         if (type == MT_CHECK_BOX && (data->flags & MT_DRAW_MASK)) {
            mt_draw_rectangle(win, DATA(gc_black), 1, x, y, x+width-1, y+height-1);
            return;
         }

         if (state & MT_ACTIVE) offset = 4;

         state &= ~MT_ACTIVE;
         if (state & MT_MOUSE_ACTIVE) state |= MT_ACTIVE;

         DRAW_WIDGET("Button", (type == MT_RADIO_BOX || type == MT_RADIO_CELL)? "RadioButton" : "CheckBox", offset);
         break;
      }

      case MT_CHOICE:
      {
         width += engine->metric[MT_CHOICE_BUTTON_WIDTH];
         DRAW_WIDGET("ComboBox", NULL, 0);
         break;
      }

      case MT_CHOICE_BUTTON:
      {
         char *tmp;
         int border;

         x -= 16; width += 16;
         DRAW_WIDGET("ComboBox", NULL, 0);
         x += 16; width -= 16;

         DRAW_WIDGET("ComboBox", "DropDownButton", 0);
         break;
      }

      case MT_SCALE:
      {
         if (data->orientation == MT_VERTICAL) {
            x += width/2 - 3;
            width = 6;
         }
         else {
            y += height/2 - 3;
            height = 6;
         }
         
         DRAW_WIDGET("TrackBar", (data->orientation == MT_VERTICAL)? "TrackVert" : "Track", 0);
         break;
      }

      case MT_SCALE_HANDLE:
      {
         DRAW_WIDGET("TrackBar", (data->orientation == MT_VERTICAL)? "ThumbVert" : "Thumb", 0);
         break;
      }

      case MT_STATUSBAR_SECTION:
         draw_box(win, x, y, width, height, 1, 0, 1, 0);
         break;

      case MT_STATICBOX:
         DRAW_WIDGET("Button", "GroupBox", 0);

         if (data->gap_width > 0) {
            mt_draw_rectangle(win, DATA(gc_grey), 1, x+data->gap_x-1, y, x+data->gap_x+data->gap_width, y+10);
         }
         break;
         
      case MT_SPINBUTTON:
         width += 8;
         /* DRAW_WIDGET("Spin", NULL, 0); */
         DRAW_WIDGET("Edit", "EditText", 0);
         break;
         
      case MT_SPINBUTTON_UP:
      {
         x -= 10; width += 10;
         DRAW_WIDGET("Edit", "EditText", 0);
         x += 10; width -= 10;

         width--;
         y++;
         height--;
         
         if (data->flags & MT_SPINBUTTON_HOVER) state &= ~MT_HOVER;
         DRAW_WIDGET("Spin", "Up", 0);
         break;
      }
      
      case MT_SPINBUTTON_DOWN:
      {
         x -= 10; width += 10;
         DRAW_WIDGET("Edit", "EditText", 0);
         x += 10; width -= 10;

         width--;
         height--;

         if (data->flags & MT_SPINBUTTON_HOVER) state &= ~MT_HOVER;
         DRAW_WIDGET("Spin", "Down", 0);
         break;
      }

      case MT_ENTRY:
         break;
      
      case MT_ENTRY_BORDER:
      {
         DRAW_WIDGET("Edit", "EditText", 0);
         break;
      }
      
      case MT_SCROLLED_WINDOW:
      {
         DRAW_WIDGET("ListView", NULL, 0);
         break;
      }

      case MT_TOOLTIP:
      {
         mt_draw_rectangle(win, DATA(gc_black), 0, x, y, x+width-1, y+height-1);
         mt_draw_rectangle(win, DATA(gc_tooltip), 1, x+1, y+1, x+width-2, y+height-2);
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
   if (DATA(redmond)) {
      DATA(redmond)->draw_string(DATA(redmond), win, type, state, str);
      return;
   }

   if (state & MT_DISABLED) {
      mt_draw_string(win, str, 0, 0, &DATA(col_disabled));
      return;
   }

   if (type == MT_MENUBAR_ITEM && (state & (MT_SELECTED | MT_HOVER))) {
      mt_draw_string(win, str, 0, 0, &engine->palette[MT_SELECTED_FOREGROUND]);
      return;
   }

   if (type == MT_PROGRESSBAR_SLIDER) {
      mt_draw_string(win, str, 0, 0, &engine->palette[MT_FOREGROUND]);
      return;
   }

   mt_draw_string(win, str, 0, 0, NULL);
}
