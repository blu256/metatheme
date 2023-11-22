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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <gmodule.h>
#include "mt_gtk2.h"
#include "mt_gtk2_patches.h"
#include "metathemeInt.h"

#ifdef HAVE_CAIRO
#include <gdk/gdkx.h>
#include <cairo-xlib.h>
#endif

#define CAIRO_ACCEL

MetaThemeStyle *mt_style = NULL;
MT_ENGINE *mt_default_engine = NULL;
int mt_swap_buttons = 0;

static GList *mt_engines = NULL;
static MT_ENGINE *mt_first_engine = NULL;
static GtkWidget *comm_window = NULL;

MT_RECTANGLE mt_clip_rect;

static int _draw_pixmap_not_clipping = 0;
static GHashTable *widget_data = NULL;

#ifdef HAVE_CAIRO
static cairo_surface_t *_cairo_surface = NULL;
static GdkPixbuf *_cairo_buf = NULL;
static MT_RECTANGLE _cairo_rect;
static GdkWindow *_cairo_win;
#endif


#ifdef USE_AUTO_CLIPPING
static void _set_clipping(MT_GC *gc)
{
   /*
   MT_GTK_GC *mtgc = (MT_GTK_GC *)gc;
   if (mtgc->clip.x != mt_clip_rect.x || mtgc->clip.y != mt_clip_rect.y ||
       mtgc->clip.width != mt_clip_rect.width || mtgc->clip.height != mt_clip_rect.height) {
      mtgc->clip = mt_clip_rect;
      if (mt_clip_rect.width == -1) {
         gdk_gc_set_clip_rectangle(mtgc->gc, NULL);
      }
      else {
         gdk_gc_set_clip_rectangle(mtgc->gc, (GdkRectangle *)(&mt_clip_rect));
      }
   }
   */
   MT_GTK_GC *mtgc = (MT_GTK_GC *)gc;

   if (mt_clip_rect.width == -1) {
      gdk_gc_set_clip_rectangle(mtgc->gc, NULL);
   }
   else {
      gdk_gc_set_clip_rectangle(mtgc->gc, (GdkRectangle *)(&mt_clip_rect));
   }
   /*printf("%d %d %d %d\n", mt_clip_rect.x, mt_clip_rect.y, mt_clip_rect.width, mt_clip_rect.height);
   gdk_gc_set_clip_rectangle(mtgc->gc, (GdkRectangle *)(&mt_clip_rect));*/
}


static void _unset_clipping(MT_GC *gc)
{
   MT_GTK_GC *mtgc = (MT_GTK_GC *)gc;

   gdk_gc_set_clip_rectangle(mtgc->gc, NULL);
}
#else
#define _set_clipping(gc)
#define _unset_clipping(gc)
#endif


static MT_GC *_mt_gc_new_with_foreground(MT_COLOR *col)
{
   GdkGCValues gc_values;
   GdkGCValuesMask gc_values_mask;
   GdkColor color;
   GdkGC *gc;
   MT_GTK_GC *mtgc;
   
   color.red = col->r * 256;
   color.green = col->g * 256;
   color.blue = col->b * 256;
   if (!gdk_colormap_alloc_color(GTK_STYLE(mt_style)->colormap, &color, FALSE, TRUE)) return NULL;
   
   gc_values_mask = GDK_GC_FOREGROUND;
   gc_values.foreground = color;
   gc = gtk_gc_get(GTK_STYLE(mt_style)->depth, GTK_STYLE(mt_style)->colormap, &gc_values, gc_values_mask);

   mtgc = (MT_GC *)malloc(sizeof(MT_GTK_GC));
   mtgc->gc = gc;
   mtgc->clip.width = -1;
   mtgc->clip.height = -1;
   return mtgc;
}


static void _mt_gc_destroy(MT_GC *gc)
{
   gtk_gc_release((GdkGC *)((MT_GTK_GC *)gc)->gc);
   free(gc);
}


static void _mt_gc_set_clip_rectangle(MT_RECTANGLE *rect, ...)
{
   MT_GC *gc;
   va_list ap;
   
   va_start(ap, rect);
   while (1) {
      gc = va_arg(ap, MT_GC *);
      if (!gc) break;

      gdk_gc_set_clip_rectangle((GdkGC *)((MT_GTK_GC *)gc)->gc, (GdkRectangle *)rect);
   }

   va_end(ap);
}


static MT_RECTANGLE *_mt_gc_get_clip_rectangle(MT_GC *gc)
{
   return &(((MT_GTK_GC *)gc)->clip);
}


static void _mt_gc_set_foreground(MT_GC *gc, MT_COLOR *col)
{
   GdkGCValues gc_values;
   GdkGCValuesMask gc_values_mask;
   GdkColor color;
   GdkGC *gdkgc;
   MT_GTK_GC *mtgc = (MT_GTK_GC *)gc;
   
   color.red = col->r * 256;
   color.green = col->g * 256;
   color.blue = col->b * 256;
   if (!gdk_colormap_alloc_color(GTK_STYLE(mt_style)->colormap, &color, FALSE, TRUE)) return;
   
   gc_values_mask = GDK_GC_FOREGROUND;
   gc_values.foreground = color;
   gdkgc = gtk_gc_get(GTK_STYLE(mt_style)->depth, GTK_STYLE(mt_style)->colormap, &gc_values, gc_values_mask);

   gtk_gc_release(mtgc->gc);
   mtgc->gc = gdkgc;
}


static MT_PIXMAP *_mt_pixmap_new_from_xpm(char **xpm_data)
{
   return gdk_pixmap_create_from_xpm_d(gdk_get_default_root_window(), NULL, NULL, xpm_data);
}


static MT_PIXMAP *_mt_pixmap_new_from_image(MT_IMAGE *img)
{
   GdkPixmap *pixmap;
   int width, height;
   
   width = gdk_pixbuf_get_width((GdkPixbuf *)img);
   height = gdk_pixbuf_get_height((GdkPixbuf *)img);
   pixmap = gdk_pixmap_new(gdk_get_default_root_window(), width, height, GTK_STYLE(mt_style)->depth);
   gdk_pixbuf_render_to_drawable((GdkPixbuf *)img, pixmap, NULL, 0, 0, 0, 0, width, height, GDK_RGB_DITHER_NONE, 0, 0);
   return (MT_PIXMAP *)pixmap;
}


static void _mt_pixmap_destroy(MT_PIXMAP *pixmap)
{
   gdk_pixmap_unref(pixmap);
}


static MT_IMAGE *_mt_image_new(int width, int height)
{
   return gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
}


static MT_IMAGE *_mt_image_new_from_xpm(char **xpm_data)
{
   GdkPixbuf *pb;
   
   pb = gdk_pixbuf_new_from_xpm_data((const char **)xpm_data);
   return pb;
}


static MT_IMAGE *_mt_image_copy(MT_IMAGE *img)
{
   return gdk_pixbuf_copy((GdkPixbuf *)img);
}


static void _mt_image_destroy(MT_IMAGE *image)
{
   gdk_pixbuf_unref((GdkPixbuf *)image);
}


static void _mt_image_replace_color(MT_IMAGE *img, int r, int g, int b, MT_COLOR *col)
{
   guchar *p = gdk_pixbuf_get_pixels((GdkPixbuf *)img);
   int width, height, channels;
   int i;

   width = gdk_pixbuf_get_width((GdkPixbuf *)img);
   height = gdk_pixbuf_get_height((GdkPixbuf *)img);
   channels = gdk_pixbuf_get_n_channels((GdkPixbuf *)img);

   for (i=0; i<width*height; i++) {
      if (p[i*channels+0] == r && p[i*channels+1] == g && p[i*channels+2] == b) {
         p[i*channels+0] = col->r;
         p[i*channels+1] = col->g;
         p[i*channels+2] = col->b;
         p[i*channels+3] = col->a;
      }
   }
}


static void _mt_image_set_pixel(MT_IMAGE *img, int x, int y, MT_COLOR *col)
{
   guchar *p = gdk_pixbuf_get_pixels((GdkPixbuf *)img);
   int width = gdk_pixbuf_get_width((GdkPixbuf *)img);
   int channels = gdk_pixbuf_get_n_channels((GdkPixbuf *)img);
   
   p[(y*width+x)*channels+0] = col->r;
   p[(y*width+x)*channels+1] = col->g;
   p[(y*width+x)*channels+2] = col->b;
   if (channels >= 4) p[(y*width+x)*channels+3] = col->a;
}


static void _mt_image_get_pixel(MT_IMAGE *img, int x, int y, MT_COLOR *col)
{
   guchar *p = gdk_pixbuf_get_pixels((GdkPixbuf *)img);
   int width = gdk_pixbuf_get_width((GdkPixbuf *)img);
   int channels = gdk_pixbuf_get_n_channels((GdkPixbuf *)img);
   
   col->r = p[(y*width+x)*channels+0];
   col->g = p[(y*width+x)*channels+1];
   col->b = p[(y*width+x)*channels+2];
   if (channels >= 4) col->a = p[(y*width+x)*channels+3];
}


static void _mt_image_fill(MT_IMAGE *img, MT_COLOR *col)
{
   guchar *p = gdk_pixbuf_get_pixels((GdkPixbuf *)img);
   int width, height, channels;
   int i;

   width = gdk_pixbuf_get_width((GdkPixbuf *)img);
   height = gdk_pixbuf_get_height((GdkPixbuf *)img);
   channels = gdk_pixbuf_get_n_channels((GdkPixbuf *)img);

   for (i=0; i<width*height; i++) {
      p[i*channels+0] = col->r;
      p[i*channels+1] = col->g;
      p[i*channels+2] = col->b;
      if (channels >= 4) p[i*channels+3] = col->a;
   }
}


static void _mt_image_get_size(MT_IMAGE *img, int *width, int *height)
{
   *width = gdk_pixbuf_get_width((GdkPixbuf *)img);
   *height = gdk_pixbuf_get_height((GdkPixbuf *)img);
}


static void _mt_draw_point(MT_WINDOW *win, MT_GC *gc, int x, int y)
{
   _set_clipping(gc);
   gdk_draw_point((GdkWindow *)win, ((MT_GTK_GC *)gc)->gc, x, y);
   _unset_clipping(gc);
}


static void _mt_draw_line(MT_WINDOW *win, MT_GC *gc, int x1, int y1, int x2, int y2)
{
   _set_clipping(gc);
   gdk_draw_line((GdkWindow *)win, ((MT_GTK_GC *)gc)->gc, x1, y1, x2, y2);
   _unset_clipping(gc);
}


static void _mt_draw_rectangle(MT_WINDOW *win, MT_GC *gc, int filled, int x1, int y1, int x2, int y2)
{
   if (x1 > x2 || y1 > y2) return;

   if (filled) {
      x2++;
      y2++;
   }

   _set_clipping(gc);
   gdk_draw_rectangle((GdkWindow *)win, ((MT_GTK_GC *)gc)->gc, filled, x1, y1, x2-x1, y2-y1);
   _unset_clipping(gc);
}


static void _mt_draw_polygon(MT_WINDOW *win, MT_GC *gc, int filled, MT_POINT *points, int npoints)
{
   _set_clipping(gc);
   gdk_draw_polygon((GdkWindow *)win, ((MT_GTK_GC *)gc)->gc, filled, (GdkPoint *)points, npoints);
   _unset_clipping(gc);
}


static void _mt_draw_string(MT_WINDOW *win, MT_STRING *str, int x_offset, int y_offset, MT_COLOR *color)
{
   MT_GTK_STRING *s = str;
   PangoLayout *layout = s->layout;
   int state_type = s->state_type;

   if (!color && s->color) color = s->color;
   if (color && !pango_layout_get_text(layout)) color = NULL;

   if (color) {
      PangoAttrList *attrlist;
      PangoAttribute *attr;

      layout = pango_layout_copy(layout);

      attrlist = pango_layout_get_attributes(layout);
      if (attrlist) {
         pango_attr_list_ref(attrlist);
      }
      else {
         attrlist = pango_attr_list_new();
      }

      if (color) {
         attr = pango_attr_foreground_new(color->r << 8, color->g << 8, color->b << 8);
         attr->start_index = 0;
         attr->end_index = strlen(pango_layout_get_text(layout));
         pango_attr_list_insert_before(attrlist, attr);
      }
      
      pango_layout_set_attributes(layout, attrlist);
      pango_attr_list_unref(attrlist);

      state_type = GTK_STATE_NORMAL;
   }

   s->parent_class->draw_layout(s->style, (GdkWindow *)win, state_type, s->use_text,
                                s->area, s->widget, s->detail, s->x + x_offset, s->y + y_offset, layout);

   if (color) {
      g_object_unref(layout);
   }
}


static void _mt_draw_pixmap(MT_WINDOW *win, MT_GC *gc, MT_PIXMAP *pixmap, int xsrc, int ysrc, int xdest, int ydest, int width, int height)
{
   if (!_draw_pixmap_not_clipping) _set_clipping(gc);
   gdk_draw_drawable((GdkWindow *)win, ((MT_GTK_GC *)gc)->gc, (GdkPixmap *)pixmap, xsrc, ysrc, xdest, ydest, width, height);
   if (!_draw_pixmap_not_clipping) _unset_clipping(gc);
}


static void _mt_draw_pixmap_tiled(MT_WINDOW *win, MT_GC *gc, MT_PIXMAP *pixmap, int srcwidth, int srcheight, int xdest, int ydest, int width, int height)
{
   int x,y,w,h,width0;

   _set_clipping(gc);
   _draw_pixmap_not_clipping = 1;
   
   y = ydest;
   width0 = width;
   while (height > 0) {
      h = MIN(srcheight, height);
      x = xdest;
      width = width0;
      while (width > 0) {
         w = MIN(srcwidth, width);
         _mt_draw_pixmap(win, gc, pixmap, 0, 0, x, y, w, h);
         x += w;
         width -= w;
      }
      y += h;
      height -= h;
   }

   _draw_pixmap_not_clipping = 0;
   _unset_clipping(gc);
}


static void _mt_draw_image(MT_WINDOW *win, MT_GC *gc, MT_IMAGE *img, int xsrc, int ysrc, int wsrc, int hsrc, int xdest, int ydest, int width, int height)
{
   GdkPixbuf *pixbuf = (GdkPixbuf *)img;
   GdkPixbuf *pixbuf2 = NULL;

   /* scale if needed: */
   if (wsrc != width || hsrc != height) {
      double scale_x = (double)width / (double)wsrc;
      double scale_y = (double)height / (double)hsrc;

      pixbuf2 = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
      gdk_pixbuf_scale(pixbuf, pixbuf2, 0, 0, width, height, -xsrc * scale_x, -ysrc * scale_y, scale_x, scale_y, GDK_INTERP_NEAREST);
      pixbuf = pixbuf2;
      xsrc = 0;
      ysrc = 0;
   }

   _set_clipping(gc);
   gdk_draw_pixbuf((GdkWindow *)win, ((MT_GTK_GC *)gc)->gc, pixbuf, xsrc, ysrc, xdest, ydest, width, height, GDK_RGB_DITHER_NONE, 0, 0);
   _unset_clipping(gc);

   if (pixbuf2) {
      gdk_pixbuf_unref(pixbuf2);
   }
}


#ifdef HAVE_CAIRO

static cairo_t *_mt_cairo_begin_pixbuf(MT_TOOLKIT *mt, MT_WINDOW *win, MT_RECTANGLE *area, int x, int y, int width, int height)
{
   cairo_t *cr;
   int xoff=0, yoff=0;

   _cairo_buf = _mt_image_new(width, height);
   _cairo_win = win;
   mt_rectangle_set(_cairo_rect, x, y, width, height);

   _cairo_surface = cairo_image_surface_create_for_data(gdk_pixbuf_get_pixels(_cairo_buf), CAIRO_FORMAT_RGB24, width, height, gdk_pixbuf_get_rowstride(_cairo_buf));
   cr = cairo_create(_cairo_surface);
   cairo_translate(cr, -xoff, -yoff);
   return cr;
}


static void _mt_cairo_end_pixbuf(MT_TOOLKIT *mt, cairo_t *cr)
{
   guchar *p, *p0;
   int i,j,rowstride,tmp;
   MT_GC *gc;
   MT_COLOR col;

   cairo_destroy(cr);

   p0 = gdk_pixbuf_get_pixels(_cairo_buf);
   rowstride = gdk_pixbuf_get_rowstride(_cairo_buf);

   /* swap R and B, also set proper alpha: */
   for (i=0; i<_cairo_rect.height; i++) {
      p = p0 + i*rowstride;
      for (j=0; j<_cairo_rect.width; j++) {
         tmp = p[0];
         p[0] = p[2];
         p[2] = tmp;
         p[3] = 255;
         p += 4;
      }
   }

   mt_color_set(col, 0, 0, 0);
   gc = _mt_gc_new_with_foreground(&col);
   gdk_draw_pixbuf(_cairo_win, ((MT_GTK_GC *)gc)->gc, _cairo_buf, 0, 0, _cairo_rect.x, _cairo_rect.y, _cairo_rect.width, _cairo_rect.height, GDK_RGB_DITHER_NONE, 0, 0);
   _mt_gc_destroy(gc);

   cairo_surface_destroy(_cairo_surface);
   _mt_image_destroy(_cairo_buf);
}


static cairo_t *_mt_cairo_begin(MT_TOOLKIT *mt, MT_WINDOW *win, MT_RECTANGLE *area, int x, int y, int width, int height)
{
   GdkWindow *window = (GdkWindow *)win;
   GdkDrawable *drawable;
   cairo_t *cr;
   gint xoff, yoff;
   gint w, h;

   /* revert to rendering to GdkPixbuf (needed for Gecko based applications): */
   if (!window || !GDK_IS_WINDOW(window)) {
      return _mt_cairo_begin_pixbuf(mt, win, area, x, y, width, height);
   }

   gdk_window_get_internal_paint_info(window, &drawable, &xoff, &yoff);

   if (!drawable) {
      return _mt_cairo_begin_pixbuf(mt, win, area, x, y, width, height);
   }

   gdk_drawable_get_size(drawable, &w, &h);
   _cairo_surface = cairo_xlib_surface_create(gdk_x11_drawable_get_xdisplay(drawable),
                                              gdk_x11_drawable_get_xid(drawable),
                                              gdk_x11_visual_get_xvisual(gdk_drawable_get_visual(drawable)),
                                              w, h);
   cr = cairo_create(_cairo_surface);
   
   cairo_translate(cr, -xoff + x, -yoff + y);

   if (area) {
      cairo_rectangle(cr, area->x - x, area->y - y, area->width, area->height);
      cairo_clip(cr);
      cairo_rectangle(cr, 0, 0, width, height);
      cairo_clip(cr);
   }

   return cr;
}


static void _mt_cairo_end(MT_TOOLKIT *mt, cairo_t *cr)
{
   if (_cairo_buf) {
      _mt_cairo_end_pixbuf(mt, cr);
      _cairo_buf = NULL;
      return;
   }

   cairo_destroy(cr);
   cairo_surface_destroy(_cairo_surface);
}

#endif


static MT_WIDGET *_mt_widget_ref(MT_WIDGET *widget)
{
   MT_WIDGET *result;
   
   g_object_ref(G_OBJECT(*widget));
   result = (MT_WIDGET *)malloc(sizeof(MT_WIDGET));
   *result = *widget;
   return result;
}


static void _mt_widget_unref(MT_WIDGET *widget)
{
   g_object_unref(G_OBJECT(*widget));
   free(widget);
}


static MT_WIDGET *_mt_widget_get_parent(MT_WIDGET *widget, MT_WIDGET *result)
{
   if (!result) result = (MT_WIDGET *)malloc(sizeof(MT_WIDGET));
   
   *result = (MT_WIDGET)gtk_widget_get_parent(GTK_WIDGET(*widget));
   return result;
}


static void _destroy_data(GtkWidget *widget, mt_destroy_func destroy_func)
{
   destroy_func(g_hash_table_lookup(widget_data, widget));
   g_hash_table_remove(widget_data, widget);
}


static void _mt_widget_set_data(MT_WIDGET *widget, void *data, mt_destroy_func data_dispose)
{
   if (!widget_data) {
      widget_data = g_hash_table_new(g_direct_hash, g_direct_equal);
   }

   g_hash_table_insert(widget_data, *widget, data);

   if (data_dispose) {
      g_object_connect(*widget, "signal::destroy", _destroy_data, data_dispose, NULL);
   }
}


static void *_mt_widget_get_data(MT_WIDGET *widget)
{
   if (!widget_data) return NULL;
   return g_hash_table_lookup(widget_data, *widget);
}


MT_TOOLKIT metatheme_toolkit = {
   _mt_gc_new_with_foreground,
   _mt_gc_destroy,
   _mt_gc_set_clip_rectangle,
   _mt_gc_get_clip_rectangle,
   _mt_gc_set_foreground,

   _mt_pixmap_new_from_xpm,
   _mt_pixmap_new_from_image,
   _mt_pixmap_destroy,

   _mt_image_new,
   _mt_image_new_from_xpm,
   _mt_image_copy,
   _mt_image_destroy,
   _mt_image_replace_color,
   _mt_image_set_pixel,
   _mt_image_get_pixel,
   _mt_image_fill,
   _mt_image_get_size,

   _mt_draw_point,
   _mt_draw_line,
   _mt_draw_rectangle,
   _mt_draw_polygon,
   _mt_draw_string,
   _mt_draw_pixmap,
   _mt_draw_pixmap_tiled,
   _mt_draw_image,

#if defined(HAVE_CAIRO) && defined(CAIRO_ACCEL)
   _mt_cairo_begin,
   _mt_cairo_end,
#elif defined(HAVE_CAIRO) && !defined(CAIRO_ACCEL)
   _mt_cairo_begin_pixbuf,
   _mt_cairo_end_pixbuf,
#else
   NULL,
   NULL,
#endif

   _mt_widget_ref,
   _mt_widget_unref,
   _mt_widget_get_parent,
   _mt_widget_set_data,
   _mt_widget_get_data
};


static void parse_rc_string(char *defs, char *pattern)
{
   static int dynamic_counter = 0;
   GString *str;

   str = g_string_new(NULL);
   g_string_printf(str, "style \"metatheme-dynamic-%d\" { %s } widget_class \"%s\" style \"metatheme-dynamic-%d\"",
                   dynamic_counter, defs, pattern, dynamic_counter);
   ++dynamic_counter;
   gtk_rc_parse_string(str->str);
   g_string_free(str, TRUE);
}


static void parse_rc_string_int(char *defs, int value, char *pattern)
{
   GString *str;

   str = g_string_new(defs);
   g_string_append_printf(str, " = %d", value);
   parse_rc_string(str->str, pattern);
   g_string_free(str, TRUE);
}


static void parse_rc_string_size(char *defs, int x, int y, char *pattern)
{
   GString *str;

   str = g_string_new(NULL);
   g_string_append_printf(str, defs, "x");
   g_string_append_printf(str, " = %d ", x);
   g_string_append_printf(str, defs, "y");
   g_string_append_printf(str, " = %d", y);
   parse_rc_string(str->str, pattern);
   g_string_free(str, TRUE);
}


#define COL1(type, state, col) \
   g_string_append_printf(str, type "[" state "] = \"#%02x%02x%02x\"\n", col->r, col->g, col->b);

#define COL2(type, col) \
   COL1(type, "NORMAL", col) \
   COL1(type, "PRELIGHT", col) \
   COL1(type, "ACTIVE", col) \
   COL1(type, "SELECTED", col) \
   COL1(type, "INSENSITIVE", col)

static void real_set_color_palette(MT_COLOR_PALETTE *pal)
{
   GString *str;
   MT_COLOR *col;

   str = g_string_new(NULL);

   /* MT_BACKGROUND: */
   col = &(*pal)[MT_BACKGROUND];
   g_string_truncate(str, 0);
   COL2("bg", col);
   parse_rc_string(str->str, "*");

   /* MT_FOREGROUND: */
   col = &(*pal)[MT_FOREGROUND];
   g_string_set_size(str, 0);
   COL2("fg", col);
   parse_rc_string(str->str, "*");

   /* MT_WINDOW_BACKGROUND: */
   col = &(*pal)[MT_WINDOW_BACKGROUND];
   g_string_truncate(str, 0);
   COL2("base", col);
   parse_rc_string(str->str, "*");
   
   /* MT_WINDOW_FOREGROUND: */
   col = &(*pal)[MT_WINDOW_FOREGROUND];
   g_string_truncate(str, 0);
   COL2("text", col);
   parse_rc_string(str->str, "*");

   /* MT_BUTTON_BACKGROUND: */
   col = &(*pal)[MT_BUTTON_BACKGROUND];
   g_string_truncate(str, 0);
   COL2("bg", col);
   parse_rc_string(str->str, "*.GtkButton");
   
   /* MT_BUTTON_FOREGROUND: */
   col = &(*pal)[MT_BUTTON_FOREGROUND];
   g_string_truncate(str, 0);
   COL2("text", col);
   parse_rc_string(str->str, "*.GtkButton");

   /* MT_SELECTED_BACKGROUND: */
   col = &(*pal)[MT_SELECTED_BACKGROUND];
   g_string_truncate(str, 0);
   COL1("base", "SELECTED", col);
   COL1("base", "ACTIVE", col);
   COL1("bg", "SELECTED", col);
   parse_rc_string(str->str, "*");

   g_string_truncate(str, 0);
   COL1("bg", "PRELIGHT", col);
   parse_rc_string(str->str, "*Menu*");

   col = &(*pal)[MT_BACKGROUND];
   g_string_truncate(str, 0);
   COL1("bg", "PRELIGHT", col);
   parse_rc_string(str->str, "*MenuBar*");
   
   /* MT_SELECTED_FOREGROUND: */
   col = &(*pal)[MT_SELECTED_FOREGROUND];
   g_string_truncate(str, 0);
   COL1("text", "SELECTED", col);
   COL1("text", "ACTIVE", col);
   COL1("fg", "SELECTED", col);
   parse_rc_string(str->str, "*");

   g_string_truncate(str, 0);
   COL1("fg", "PRELIGHT", col);
   COL1("text", "PRELIGHT", col);
   parse_rc_string(str->str, "*Menu*");

   col = &(*pal)[MT_FOREGROUND];
   g_string_truncate(str, 0);
   COL1("fg", "PRELIGHT", col);
   parse_rc_string(str->str, "*MenuBar*");

   /* MT_DISABLED_BACKGROUND: */
   col = &(*pal)[MT_DISABLED_BACKGROUND];
   g_string_truncate(str, 0);
   COL1("base", "INSENSITIVE", col);
   parse_rc_string(str->str, "*");
   
   /* MT_DISABLED_FOREGROUND: */
   col = &(*pal)[MT_DISABLED_FOREGROUND];
   g_string_truncate(str, 0);
   COL1("text", "INSENSITIVE", col);
   COL1("fg", "INSENSITIVE", col);
   /* COL1("bg", "INSENSITIVE", col); */
   parse_rc_string(str->str, "*");


   /* GtkOptionMenu: */
   col = &(*pal)[MT_WINDOW_BACKGROUND];
   g_string_truncate(str, 0);
   COL1("bg", "NORMAL", col);
   COL1("bg", "ACTIVE", col);
   COL1("bg", "PRELIGHT", col);
   parse_rc_string(str->str, "*.GtkOptionMenu");

   col = &(*pal)[MT_WINDOW_FOREGROUND];
   g_string_truncate(str, 0);
   COL1("fg", "NORMAL", col);
   COL1("fg", "PRELIGHT", col);
   parse_rc_string(str->str, "*.GtkOptionMenu.*");

   g_string_free(str, TRUE);
}


static void real_set_metric(int metric, int value)
{
   GString *str;
   
   switch (metric) {
      case MT_SCROLLBAR_SIZE:
         parse_rc_string_int("GtkRange::slider-width", value, "*");
         parse_rc_string_int("GtkRange::stepper-size", value, "*");
         break;

      case MT_CHECKBOX_SIZE:         parse_rc_string_int("GtkCheckButton::indicator-size", value, "*"); break;
      case MT_SCALE_SLIDER_SIZE:     parse_rc_string_int("GtkScale::slider-length", value, "*"); break;
      /*
      case MT_STATICBOX_TEXT_OFFSET: defs = "xthickness"; pattern = "*.GtkFrame"; break;
      */
      case MT_NOTEBOOK_TEXT_OFFSET:  parse_rc_string_size("%sthickness", value, value, "*.GtkNotebook"); break;

      case MT_SPLITTER_WIDTH:
         str = g_string_new(NULL);
         g_string_append_printf(str, "GtkHPaned::handle-size = %d\n", value);
         g_string_append_printf(str, "GtkVPaned::handle-size = %d\n", value);
         parse_rc_string(str->str, "*");
         g_string_free(str, TRUE);
         return;
   }
}


static void real_set_metric_size(int metric, int x, int y)
{
   switch (metric) {
      case MT_BUTTON_BORDER:             parse_rc_string_size("%sthickness", x, y, "*.GtkButton"); break;
      case MT_BUTTON_TEXT_OFFSET:        parse_rc_string_size("GtkButton::child-displacement-%s", x, y, "*.GtkButton"); break;
      case MT_BUTTON_TOGGLE_BORDER:      parse_rc_string_size("%sthickness", x, y, "*.GtkToggleButton"); break;
      case MT_BUTTON_TOGGLE_TEXT_OFFSET: parse_rc_string_size("GtkButton::child-displacement-%s", x, y, "*.GtkToggleButton"); break;
      case MT_TOOLBAR_ITEM_BORDER:       parse_rc_string_size("%sthickness", x, y, "*Toolbar*Button"); break;
      case MT_TOOLBAR_ITEM_TEXT_OFFSET:  parse_rc_string_size("GtkButton::child-displacement-%s", x, y, "*Toolbar*Button"); break;
      case MT_MENUBAR_BORDER:            parse_rc_string_size("%sthickness", x, y, "*MenuBar"); break;
      case MT_MENUBAR_ITEM_BORDER:       parse_rc_string_size("%sthickness", x, y, "*MenuBar*MenuItem"); break;
      case MT_MENU_BORDER:               parse_rc_string_size("%sthickness", x, y, "*.GtkMenu"); break;
      case MT_MENU_ITEM_BORDER:          parse_rc_string_size("%sthickness", x, y+1, "*.GtkMenu.Gtk*MenuItem"); break;
      case MT_PROGRESSBAR_BORDER:        parse_rc_string_size("%sthickness", x, y, "*.GtkProgressBar"); break;
   }
}


static void real_set_font(char *face, int size)
{
   GString *str;
   str = g_string_new(NULL);
   g_string_printf(str, "font_name = \"%s %d\"", face, size);
   parse_rc_string(str->str, "*");
   g_string_free(str, TRUE);
}



static void _event_handler(GdkEvent *event, gpointer data)
{
   char *s;

   if (event->type == GDK_CLIENT_EVENT) {
      s = gdk_atom_name(((GdkEventClient *)event)->message_type);
      if (s && !strcmp(s, "_GTK_READ_RCFILES")) {
         mt_default_engine = NULL;
         mt_first_engine = mt_get_engine();
      }
   }

   gtk_main_do_event(event);
}


G_MODULE_EXPORT void theme_init(GTypeModule *module)
{
   /* prevent unloading of this theme: */
   g_type_module_use(module);

   metatheme_toolkit_init(&metatheme_toolkit);

   metatheme_rc_style_register_type (module);
   metatheme_style_register_type (module);

   gdk_event_handler_set(_event_handler, 0, NULL);

   mt_default_engine = NULL;
   mt_first_engine = mt_get_engine();

   /* create window for retrieving messages: */

   comm_window = gtk_window_new(GTK_WINDOW_POPUP);
   gtk_window_set_default_size(GTK_WINDOW(comm_window), 0, 0);
   gtk_widget_set_uposition(comm_window, 32000, 32000);
   gtk_widget_show(comm_window);
}


G_MODULE_EXPORT void theme_exit()
{
   MT_LIBRARY *lib;
   
   gtk_widget_destroy(comm_window);

   mt_gtk2_uninstall_patches();

   lib = mt_default_engine->library;
   mt_default_engine->exit(mt_default_engine);
   metatheme_unload_engine(mt_default_engine);
   metatheme_library_exit(lib);

   gdk_event_handler_set((GdkEventFunc)gtk_main_do_event, 0, NULL);
}


G_MODULE_EXPORT GtkRcStyle *theme_create_rc_style()
{
   void *ptr;

   ptr = GTK_RC_STYLE(g_object_new(METATHEME_TYPE_RC_STYLE, NULL));
   return (GtkRcStyle *)ptr;
}


MT_ENGINE *mt_get_engine()
{
   MT_LIBRARY *mt_library;
   int i;
   const char *tmp;

   if (mt_default_engine) {
      if (mt_first_engine == mt_default_engine) {
         mt_first_engine = NULL;
      }
      else {
         mt_default_engine->refcount++;
      }
      return mt_default_engine;
   }
   
   /* load default theme engine: */

   mt_library = metatheme_library_init(&metatheme_toolkit);
   mt_default_engine = metatheme_load_engine(mt_library, NULL);
   if (!mt_default_engine) {
      metatheme_library_exit(mt_library);
      exit(1);
   }

   metatheme_load_default_settings(mt_default_engine);
   mt_default_engine->init(mt_default_engine);

   real_set_font(mt_default_engine->font, mt_default_engine->font_size);
   for (i=0; i<MT_METRIC_SIZE_MAX; i++) real_set_metric_size(i, mt_default_engine->metric_size[i].x,  mt_default_engine->metric_size[i].y);
   for (i=0; i<MT_METRIC_MAX; i++) real_set_metric(i, mt_default_engine->metric[i]);
   real_set_color_palette(&mt_default_engine->palette);

   tmp = metatheme_get_config_option(metatheme_get_config(mt_library), "swap_buttons", "gtk2");
   mt_swap_buttons = (tmp && !strcmp(tmp, "1"));

   mt_engines = g_list_append(mt_engines, mt_default_engine);

   if (g_list_length(mt_engines) == 1) {
      mt_gtk2_install_patches();
   }

   /* default settings: */

   parse_rc_string("GtkRange::trough-border = 0", "*");
   parse_rc_string("xthickness = 0\nythickness = 0", "*.GtkCList");
   parse_rc_string("xthickness = 0\nythickness = 0", "*.GtkCTree");
   parse_rc_string("xthickness = 1\nythickness = 1", "*.GtkOptionMenu");
   gtk_rc_parse_string("style \"metatheme-tooltips\" { bg[NORMAL] = \"#FFFFE1\" } widget \"gtk-tooltips*\" style \"metatheme-tooltips\"");
   parse_rc_string("GtkMenuItem::horizontal_padding = 0", "*");
   parse_rc_string("GtkMenuItem::toggle_spacing = 6", "*");
   parse_rc_string("GtkMenuItem::arrow_spacing = 2", "*");
   parse_rc_string("GtkCheckMenuItem::indicator_size = 8", "*");
   parse_rc_string("GtkMenu::vertical-padding = 0", "*");
   parse_rc_string("GtkMenu::vertical-offset = 7", "*");
   parse_rc_string("GtkMenu::horizontal-offset = -8", "*");
   parse_rc_string("GtkComboBox::appears-as-list = 0", "*");

   parse_rc_string(
      "GtkNotebook::has_secondary_forward_stepper = 0\n"
      "GtkNotebook::has_secondary_backward_stepper = 1\n"
      "GtkNotebook::has_forward_stepper = 1\n"
      "GtkNotebook::has_backward_stepper = 0\n", "*.GtkNotebook");

   gtk_rc_parse_string("gtk-entry-select-on-focus = 1\n");

   gtk_rc_parse_string(
      "style \"metatheme-main-defaults\" {\n"
      "  GtkButton::default-border = { 0, 0, 0, 0 }\n"
      "  GtkButton::default-outside-border = { 0, 0, 0, 0 }\n"
      /* "  GtkWidget::focus-line-pattern = ""\n" */
      "  GtkWidget::focus-line-width = 0\n"
      "  GtkWidget::focus-padding = 0\n"
      "  GtkWidget::interior-focus = 1\n"
      "  GtkMenuBar::shadow_type = GTK_SHADOW_NONE\n"
      "  GtkMenuBar::internal-padding = 0\n"
      /* "  GtkCheckButton::indicator-size = 20\n" */
      "  GtkPaned::handle-size = 9\n"
      "  GtkToolbar::internal-padding = 0\n"
      "  GtkScrolledWindow::scrollbar-spacing = 0\n"
      "} widget_class \"*\""
      " style \"metatheme-main-defaults\"");

   gtk_rc_parse_string(
      "style \"metatheme-menubaritem-defaults\" {\n"
      "  GtkMenuItem::horizontal_padding = 2\n"
      "} widget_class \"*MenuBar*MenuItem\""
      " style \"metatheme-menubaritem-defaults\"");

   gtk_rc_parse_string(
      "style \"metatheme-scales-defaults\" {\n"
      "  GtkRange::slider-width = 20\n"
      "} widget_class \"*Scale*\""
      " style \"metatheme-scales-defaults\"");

   gtk_rc_parse_string(
      "style \"metatheme-sbar-defaults\" {\n"
      /* "  GtkRange::slider-width = 13\n" */
      /* "  GtkRange::stepper-size = 13\n" */
      "  GtkScrollbar::min-slider-length = 25\n"
      "  GtkScrollbar::has_forward_stepper = 1\n"
      "  GtkScrollbar::has_backward_stepper = 1\n"
      "  GtkScrollbar::has_backward_stepper = 1\n"
      "} widget_class \"*Scrollbar*\""
      " style \"metatheme-sbar-defaults\"");

   return mt_default_engine;
}


void mt_return_engine(MT_ENGINE *engine)
{
   MT_LIBRARY *lib;

   engine->refcount--;

   if (engine->refcount > 0) return;

   mt_engines = g_list_remove(mt_engines, engine);
   if (mt_default_engine == engine) mt_default_engine = NULL;

   lib = engine->library;
   engine->exit(engine);
   metatheme_unload_engine(engine);
   metatheme_library_exit(lib);

   if (g_list_length(mt_engines) == 0) {
      mt_gtk2_uninstall_patches();
   }
}


GtkWidget *mt_get_ancestor(GtkWidget *widget, char *name)
{
   while (widget) {
      if (IS_OBJECT(widget, name)) return widget;
      widget = widget->parent;
   }
   return NULL;
}


void mt_show_ancestors(GtkWidget *widget)
{
   while (widget) {
      fprintf(stderr, "%s > ", GTK_OBJECT_TYPE_NAME(widget));
      widget = widget->parent;
   }
   printf("0\n");
}
