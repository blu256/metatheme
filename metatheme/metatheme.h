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

#ifndef METATHEME_H
#define METATHEME_H

#ifdef HAVE_CAIRO
#include <cairo.h>
#endif

#ifndef NULL
#define NULL (void *)0
#endif

/* widget types: */
enum {
   MT_NONE,

   MT_BASE,              /* background of dialogs & windows */
   MT_WINDOW_BASE,       /* background of entries, etc. */
   
   MT_BORDER_IN,
   MT_BORDER_OUT,
   
   MT_HLINE,
   MT_VLINE,
   
   MT_FOCUS_TAB,
   MT_FOCUS_CHECKBOX,

   MT_RESIZE_GRIP,       /* resize grip in status bar */

   MT_ARROW_UP,
   MT_ARROW_DOWN,
   MT_ARROW_LEFT,
   MT_ARROW_RIGHT,
   
   MT_BUTTON,
   MT_BUTTON_DEFAULT,
   MT_BUTTON_TOGGLE,
   MT_BUTTON_HEADER,
   
   MT_MENU,
   MT_MENU_ITEM,
   MT_MENU_ITEM_CHECK,
   MT_MENU_ITEM_RADIO,
   MT_MENU_ITEM_ARROW,
   MT_MENU_ITEM_SEPARATOR,
   MT_MENU_TEAROFF,

   MT_MENUBAR,
   MT_MENUBAR_ITEM,
   
   MT_TOOLBAR,
   MT_TOOLBAR_ITEM,
   MT_TOOLBAR_ITEM_TOGGLE,
   MT_TOOLBAR_ITEM_DROPDOWN,
   MT_TOOLBAR_SEPARATOR,
   MT_TOOLBAR_HANDLE,

   MT_SCROLLBAR,
   MT_SCROLLBAR_HANDLE,
   MT_SCROLLBAR_ARROW_UP,
   MT_SCROLLBAR_ARROW_DOWN,
   MT_SCROLLBAR_ARROW_LEFT,
   MT_SCROLLBAR_ARROW_RIGHT,

   MT_PROGRESSBAR,
   MT_PROGRESSBAR_SLIDER,

   MT_NOTEBOOK,
   MT_NOTEBOOK_TAB,
   MT_NOTEBOOK_ARROW_LEFT,
   MT_NOTEBOOK_ARROW_RIGHT,
   
   MT_RADIO_BUTTON,
   MT_CHECK_BUTTON,
   MT_CHECK_BOX,
   MT_RADIO_BOX,
   MT_CHECK_CELL,
   MT_RADIO_CELL,

   MT_CHOICE,
   MT_CHOICE_BUTTON,

   MT_SCALE,
   MT_SCALE_HANDLE,

   MT_STATUSBAR_SECTION,
   MT_STATICBOX,

   MT_SPINBUTTON,
   MT_SPINBUTTON_UP,
   MT_SPINBUTTON_DOWN,

   MT_ENTRY,
   MT_ENTRY_BORDER,

   MT_SCROLLED_WINDOW,

   MT_TOOLTIP,

   MT_SPLITTER
};

/* state types: */
enum {
   MT_NORMAL       = 0x0000,   /* normal state */
   MT_HOVER        = 0x0001,   /* mouse is on top of widget */
   MT_ACTIVE       = 0x0002,   /* widget is toggled, downed, etc. */
   MT_DISABLED     = 0x0004,   /* widget is disabled */
   MT_SELECTED     = 0x0008,   /* widget is selected */
   MT_MOUSE_ACTIVE = 0x0010    /* widget is temporary toggled, downed, etc by mouse. */
};

/* metric values: */
enum {
   MT_BORDER,                       /* width of border */
   MT_SCROLLBAR_SIZE,               /* width of scrollbar */
   MT_CHECKBOX_SIZE,                /* size of checkbox including border */
   MT_SCALE_SLIDER_SIZE,            /* length of scale slider */
   MT_STATICBOX_TEXT_OFFSET,        /* offset of text from left */

   MT_MENUBAR_ITEM_SPACING,         /* width of space between items in menubar */
   MT_MENU_SEPARATOR_HEIGHT,        /* height of separator in menu */

   MT_NOTEBOOK_OVERLAP,             /* number of pixels the tab bar area overlaps the notebook */
   MT_NOTEBOOK_TAB_OVERLAP,         /* number of pixels the tabs should overlap */
   MT_NOTEBOOK_TEXT_OFFSET,         /* offset of text when tab is active */
   MT_NOTEBOOK_ARROW_WIDTH,         /* width of arrow button */
   MT_NOTEBOOK_IS_FILLED,           /* is notebook filled? */

   MT_CHOICE_BUTTON_WIDTH,          /* width of button in choice widget */

   MT_TOOLBAR_ITEM_DROPDOWN_WIDTH,  /* additional width of toolbar item when the button has associated popup menu */

   MT_SPLITTER_WIDTH,               /* width of splitter */

   MT_MENU_TEAROFF_HEIGHT,          /* height of menu tearoff */
   MT_DETACHED_MENU_TEAROFF_HEIGHT, /* height of menu tearoff in detached menu */

   MT_METRIC_MAX
};

/* metric size values: */
enum {
   MT_BUTTON_BORDER,                /* additional space between label and border */
   MT_BUTTON_TEXT_OFFSET,           /* offset of text when button is pressed */
   MT_BUTTON_TOGGLE_BORDER,         /* additional space for border */
   MT_BUTTON_TOGGLE_TEXT_OFFSET,    /* offset of text when toggle button is pressed */
   MT_TOOLBAR_ITEM_TEXT_OFFSET,     /* offset of text and image when toolbar item is pressed */
   MT_TOOLBAR_ITEM_BORDER,          /* additional space for border */
   MT_MENUBAR_BORDER,               /* additional space for border */
   MT_MENUBAR_ITEM_BORDER,          /* additional space for border */
   MT_MENUBAR_ITEM_TEXT_OFFSET,     /* offset of text when menubar item is active */
   MT_MENU_BORDER,                  /* additional space for border */
   MT_MENU_ITEM_BORDER,             /* additional space for border */
   MT_PROGRESSBAR_BORDER,           /* spacing between progressbar and slider */

   MT_METRIC_SIZE_MAX
};

/* color types: */
enum {
   MT_BACKGROUND = 0,      /* windows & dialogs (i.e. black text on grey background) */
   MT_FOREGROUND,

   MT_WINDOW_BACKGROUND,   /* entries, etc. (i.e. black text on white background) */
   MT_WINDOW_FOREGROUND,

   MT_BUTTON_BACKGROUND,   /* buttons */
   MT_BUTTON_FOREGROUND,

   MT_SELECTED_BACKGROUND, /* selected text (i.e. white text on blue background) */
   MT_SELECTED_FOREGROUND,

   MT_DISABLED_BACKGROUND, /* disabled items (i.e. black text on grey background) */
   MT_DISABLED_FOREGROUND,

   MT_BUTTON_LIGHT,        /* lighter than button background */
   MT_BUTTON_MIDLIGHT,     /* between button background and button light */
   MT_BUTTON_DARK,         /* darker than button background */
   MT_BUTTON_MID,          /* between button background and dark */
   MT_BUTTON_SHADOW,       /* very dark color (black) */

   MT_COLOR_MAX
};

/* position types: */
enum {
   MT_POSITION_LEFT,
   MT_POSITION_RIGHT,
   MT_POSITION_TOP,
   MT_POSITION_BOTTOM
};

/* orientation types: */
enum {
   MT_HORIZONTAL,
   MT_VERTICAL
};

/* widget flags: */
enum {
   MT_SCROLLBAR_UNSCROLLABLE       = 0x0001,
   MT_SCROLLBAR_SUBPAGE_ACTIVE     = 0x0002,
   MT_SCROLLBAR_ADDPAGE_ACTIVE     = 0x0004,

   MT_NOTEBOOK_FIRST_VISIBLE_TAB   = 0x0001, /* for drawing left ending of tabbar (usually the first visible tab, in QT first tab) */
   MT_NOTEBOOK_FIRST_TAB           = 0x0002,
   MT_NOTEBOOK_LAST_TAB            = 0x0004,

   MT_SPINBUTTON_HOVER             = 0x0001,

   MT_TOOLBAR_ITEM_DROPDOWN_ACTIVE = 0x0001,

   MT_MENU_DETACHED                = 0x0001, /* for MT_MENU_TEAROFF */

   MT_DRAW_MASK                    = 0x0001, /* for MT_CHECK_BOX and MT_RADIO_BOX */

   MT_PROGRESSBAR_REVERSED         = 0x0001
};

#define BIT_IS_SET(a, b) ((a & b) == b)

#define mt_color_set(c, red, green, blue) \
   mt_color_set2(c, red, green, blue, 255)

#define mt_color_set2(c, red, green, blue, alpha) \
   c.r = red; \
   c.g = green; \
   c.b = blue; \
   c.a = alpha;

#define mt_rectangle_set(r, _x, _y, _w, _h) \
   { \
      r.x = _x; \
      r.y = _y; \
      r.width = _w; \
      r.height = _h; \
   }

#define mt_is_widget(widget) ((widget) && *(widget))

typedef void MT_GC;
typedef void MT_WINDOW;
typedef void MT_STRING;
typedef void MT_PIXMAP;
typedef void MT_IMAGE;

typedef void *MT_WIDGET;
typedef void (*mt_destroy_func)(void *data);

typedef struct {
   int x;
   int y;
} MT_POINT;

typedef struct {
   int x;
   int y;
   int width;
   int height;
} MT_RECTANGLE;

typedef struct {
   int r;
   int g;
   int b;
   int a;
} MT_COLOR;

typedef struct {
   MT_WIDGET widget;
   int flags;
   int gap_x, gap_width;
   int gap_position;
   int orientation;
   MT_COLOR background_color;
   int handle_position, groove_size;
} MT_WIDGET_DATA;

typedef MT_COLOR MT_COLOR_PALETTE[MT_COLOR_MAX];

typedef struct {
   char *name;
   int count;
   char **items;
} MT_CONFIG_SECTION;

typedef struct {
   int count;
   MT_CONFIG_SECTION *sections;
} MT_CONFIG;

typedef void (*dummy_func)();

struct _MT_LIBRARY;
struct _MT_ENGINE;

typedef struct _MT_LIBRARY MT_LIBRARY;
typedef struct _MT_ENGINE MT_ENGINE;

typedef struct _MT_TOOLKIT {

   /* toolkit functions: */

   MT_GC *(*gc_new_with_foreground)(MT_COLOR *col);
   void (*gc_destroy)(MT_GC *gc);
   void (*gc_set_clip_rectangle)(MT_RECTANGLE *rect, ...);
   MT_RECTANGLE *(*gc_get_clip_rectangle)(MT_GC *gc);
   void (*gc_set_foreground)(MT_GC *gc, MT_COLOR *col);

   MT_PIXMAP *(*pixmap_new_from_xpm)(char **xpm_data);
   MT_PIXMAP *(*pixmap_new_from_image)(MT_IMAGE *img);
   void (*pixmap_destroy)(MT_PIXMAP *pixmap);

   MT_IMAGE *(*image_new)(int width, int height);
   MT_IMAGE *(*image_new_from_xpm)(char **xpm_data);
   MT_IMAGE *(*image_copy)(MT_IMAGE *img);
   void (*image_destroy)(MT_IMAGE *image);
   void (*image_replace_color)(MT_IMAGE *img, int r, int g, int b, MT_COLOR *col);
   void (*image_set_pixel)(MT_IMAGE *img, int x, int y, MT_COLOR *col);
   void (*image_get_pixel)(MT_IMAGE *img, int x, int y, MT_COLOR *col);
   void (*image_fill)(MT_IMAGE *img, MT_COLOR *col);
   void (*image_get_size)(MT_IMAGE *img, int *width, int *height);

   void (*draw_point)(MT_WINDOW *win, MT_GC *gc, int x, int y);
   void (*draw_line)(MT_WINDOW *win, MT_GC *gc, int x1, int y1, int x2, int y2);
   void (*draw_rectangle)(MT_WINDOW *win, MT_GC *gc, int filled, int x1, int y1, int x2, int y2);
   void (*draw_polygon)(MT_WINDOW *win, MT_GC *gc, int filled, MT_POINT *points, int npoints);
   void (*draw_string)(MT_WINDOW *win, MT_STRING *str, int x_offset, int y_offset, MT_COLOR *color);
   void (*draw_pixmap)(MT_WINDOW *win, MT_GC *gc, MT_PIXMAP *pixmap, int xsrc, int ysrc, int xdest, int ydest, int width, int height);
   void (*draw_pixmap_tiled)(MT_WINDOW *win, MT_GC *gc, MT_PIXMAP *pixmap, int srcwidth, int srcheight, int xdest, int ydest, int width, int height);
   void (*draw_image)(MT_WINDOW *win, MT_GC *gc, MT_IMAGE *img, int xsrc, int ysrc, int wsrc, int hsrc, int xdest, int ydest, int width, int height);

#ifdef HAVE_CAIRO
   cairo_t *(*cairo_begin)(struct _MT_TOOLKIT *mt, MT_WINDOW *win, MT_RECTANGLE *area, int x, int y, int width, int height);
   void (*cairo_end)(struct _MT_TOOLKIT *mt, cairo_t *cr);
#else
   dummy_func cairo_dummy[2];
#endif

   MT_WIDGET *(*widget_ref)(MT_WIDGET *widget);
   void (*widget_unref)(MT_WIDGET *widget);
   MT_WIDGET *(*widget_get_parent)(MT_WIDGET *widget, MT_WIDGET *result);
   void (*widget_set_data)(MT_WIDGET *widget, void *data, mt_destroy_func data_dispose);
   void *(*widget_get_data)(MT_WIDGET *widget);

   dummy_func dummy[16]; /* for binary compatibility */

   /* core functions: */
   
   MT_ENGINE *(*load_engine)(MT_LIBRARY *lib, const char *name);
   void (*unload_engine)(MT_ENGINE *engine);
   
   MT_CONFIG *(*get_config)(MT_LIBRARY *lib);
   MT_CONFIG *(*load_config)(const char *fname);
   void (*free_config)(MT_CONFIG *cfg);
   const char *(*get_config_option)(MT_CONFIG *cfg, const char *name, const char *section);
   void (*color_set_from_config)(MT_CONFIG *cfg, MT_COLOR *col, const char *name, const char *section);

   void (*draw_gradient)(struct _MT_TOOLKIT *mt, MT_WINDOW *win, MT_GC *gc, int x, int y, int width, int height, MT_COLOR *color1, MT_COLOR *color2, int orientation);

   void (*color_get_hsv)(MT_COLOR *col, int *h, int *s, int *v);
   void (*color_set_hsv)(MT_COLOR *result, int h, int s, int v);
   void (*color_light)(MT_COLOR *result, MT_COLOR *col, int factor);
   void (*color_dark)(MT_COLOR *result, MT_COLOR *col, int factor);

} MT_TOOLKIT;


#define MT_TOOLKIT_HANDLE engine->toolkit

#define mt_get_config(lib) (MT_TOOLKIT_HANDLE->get_config)(lib)
#define mt_load_config(fname) (MT_TOOLKIT_HANDLE->load_config)(fname)
#define mt_free_config(cfg) (MT_TOOLKIT_HANDLE->free_config)(cfg)
#define mt_get_config_option(cfg, name, section) (MT_TOOLKIT_HANDLE->get_config_option)(cfg, name, section)
#define mt_color_set_from_config(cfg, col, name, section) (MT_TOOLKIT_HANDLE->color_set_from_config)(cfg, col, name, section)

#define mt_color_get_hsv(col, h, s, v) (MT_TOOLKIT_HANDLE->color_get_hsv)(col, h, s, v)
#define mt_color_set_hsv(result, h, s, v) (MT_TOOLKIT_HANDLE->color_set_hsv)(result, h, s, v)
#define mt_color_light(result, col, factor) (MT_TOOLKIT_HANDLE->color_light)(result, col, factor)
#define mt_color_dark(result, col, factor) (MT_TOOLKIT_HANDLE->color_dark)(result, col, factor)

#define mt_gc_new_with_foreground(col) (MT_TOOLKIT_HANDLE->gc_new_with_foreground)(col)
#define mt_gc_destroy(gc) (MT_TOOLKIT_HANDLE->gc_destroy)(gc)
#define mt_gc_set_clip_rectangle(rect, p...) (MT_TOOLKIT_HANDLE->gc_set_clip_rectangle)(rect, p)
#define mt_gc_get_clip_rectangle(gc) (MT_TOOLKIT_HANDLE->gc_get_clip_rectangle)(gc)
#define mt_gc_set_foreground(gc, col) (MT_TOOLKIT_HANDLE->gc_set_foreground)(gc, col)

#define mt_pixmap_new_from_xpm(xpm_data) (MT_TOOLKIT_HANDLE->pixmap_new_from_xpm)(xpm_data)
#define mt_pixmap_new_from_image(img) (MT_TOOLKIT_HANDLE->pixmap_new_from_image)(img)
#define mt_pixmap_destroy(pixmap) (MT_TOOLKIT_HANDLE->pixmap_destroy)(pixmap)

#define mt_image_new(width, height) (MT_TOOLKIT_HANDLE->image_new)(width, height)
#define mt_image_new_from_xpm(xpm_data) (MT_TOOLKIT_HANDLE->image_new_from_xpm)(xpm_data)
#define mt_image_copy(img) (MT_TOOLKIT_HANDLE->image_copy)(img)
#define mt_image_destroy(image) (MT_TOOLKIT_HANDLE->image_destroy)(image)
#define mt_image_replace_color(img, r, g, b, col) (MT_TOOLKIT_HANDLE->image_replace_color)(img, r, g, b, col)
#define mt_image_set_pixel(img, x, y, col) (MT_TOOLKIT_HANDLE->image_set_pixel)(img, x, y, col)
#define mt_image_get_pixel(img, x, y, col) (MT_TOOLKIT_HANDLE->image_get_pixel)(img, x, y, col)
#define mt_image_fill(img, col) (MT_TOOLKIT_HANDLE->image_fill)(img, col)
#define mt_image_get_size(img, width, height) (MT_TOOLKIT_HANDLE->image_get_size)(img, width, height)

#define mt_set_metric(engine, type, value) (engine)->metric[type] = value;
#define mt_set_metric_size(engine, type, _x, _y) \
   { \
      (engine)->metric_size[type].x = _x; \
      (engine)->metric_size[type].y = _y; \
   }

#define mt_draw_point(win, gc, x, y) (MT_TOOLKIT_HANDLE->draw_point)(win, gc, x, y)
#define mt_draw_line(win, gc, x1, y1, x2, y2) (MT_TOOLKIT_HANDLE->draw_line)(win, gc, x1, y1, x2, y2)
#define mt_draw_rectangle(win, gc, filled, x1, y1, x2, y2) (MT_TOOLKIT_HANDLE->draw_rectangle)(win, gc, filled, x1, y1, x2, y2)
#define mt_draw_polygon(win, gc, filled, points, npoints) (MT_TOOLKIT_HANDLE->draw_polygon)(win, gc, filled, points, npoints)
#define mt_draw_string(win, str, x_offset, y_offset, color) (MT_TOOLKIT_HANDLE->draw_string)(win, str, x_offset, y_offset, color)
#define mt_draw_pixmap(win, gc, pixmap, xsrc, ysrc, xdest, ydest, width, height) (MT_TOOLKIT_HANDLE->draw_pixmap)(win, gc, pixmap, xsrc, ysrc, xdest, ydest, width, height)
#define mt_draw_pixmap_tiled(win, gc, pixmap, srcwidth, srcheight, xdest, ydest, width, height) (MT_TOOLKIT_HANDLE->draw_pixmap_tiled)(win, gc, pixmap, srcwidth, srcheight, xdest, ydest, width, height)
#define mt_draw_gradient(win, gc, x, y, width, height, color1, color2, orientation) (MT_TOOLKIT_HANDLE->draw_gradient)(MT_TOOLKIT_HANDLE, win, gc, x, y, width, height, color1, color2, orientation)
#define mt_draw_image(win, gc, img, xsrc, ysrc, wsrc, hsrc, xdest, ydest, width, height) (MT_TOOLKIT_HANDLE->draw_image)(win, gc, img, xsrc, ysrc, wsrc, hsrc, xdest, ydest, width, height)

#ifdef HAVE_CAIRO
#define mt_cairo_begin(win, area, x, y, width, height) (MT_TOOLKIT_HANDLE->cairo_begin)(MT_TOOLKIT_HANDLE, win, area, x, y, width, height)
#define mt_cairo_end(cr) (MT_TOOLKIT_HANDLE->cairo_end)(MT_TOOLKIT_HANDLE, cr)
#endif

#define mt_widget_ref(widget) (MT_TOOLKIT_HANDLE->widget_ref)(widget)
#define mt_widget_unref(widget) (MT_TOOLKIT_HANDLE->widget_unref)(widget)
#define mt_widget_get_parent(widget, result) (MT_TOOLKIT_HANDLE->widget_get_parent)(widget, result)
#define mt_widget_set_data(widget, data, data_dispose) (MT_TOOLKIT_HANDLE->widget_set_data)(widget, data, data_dispose)
#define mt_widget_get_data(widget) (MT_TOOLKIT_HANDLE->widget_get_data)(widget)

#endif /* METATHEME_H */
