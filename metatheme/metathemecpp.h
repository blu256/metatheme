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

#ifndef METATHEMECPP_H
#define METATHEMECPP_H

#ifndef MT_EXT
   #define MT_EXT_COLOR
   #define MT_EXT_RECT
   #define MT_EXT_GC
   #define MT_EXT_THEMEENGINE
#endif

#include <malloc.h>
#include "metathemeInt.h"

#ifndef MT_EXT
namespace metatheme {
#endif

extern MT_TOOLKIT *mt;


class Color
{
public:
   Color();
   Color(const Color &color);
   Color(int r, int g, int b);
   Color(int r, int g, int b, int a);
   Color(MT_COLOR *color);

   int r() const;
   int g() const;
   int b() const;
   int a() const;

   void set(int r, int g, int b);
   void set(int r, int g, int b, int a);

   void getHsv(int *h, int *s, int *v) const;
   void setHsv(int h, int s, int v);

   Color light(int factor) const;
   Color dark(int factor) const;

   inline MT_COLOR *color() { return &col; }

private:
   MT_COLOR col;

   friend class Palette;
   friend class GC;
   friend class ThemeEngine;

   MT_EXT_COLOR
};


class Palette
{
public:
   Palette();

   void set(int type, int r, int g, int b);
   void set(int type, const Color &color);

   Color get(int type);

private:
   MT_COLOR_PALETTE pal;
};


class Rect
{
public:
   Rect();
   Rect(int x, int y, int width, int height);
   Rect(MT_RECTANGLE *rect);

   int x() const;
   int y() const;
   int width() const;
   int height() const;

   int left() const;
   int top() const;
   int right() const;
   int bottom() const;

   void setX(int x);
   void setY(int y);
   void setWidth(int width);
   void setHeight(int height);
   void setLeft(int x);
   void setTop(int y);
   void setRight(int x);
   void setBottom(int y);
   void translate(int dx, int dy);

private:
   MT_RECTANGLE r;

   MT_EXT_RECT
};


class Image
{
public:
   Image(const Image &image);
   Image(int width, int height);
   Image(char **xpm_data);
   virtual ~Image();

   void replaceColor(int r, int g, int b, int dr, int dg, int db);
   void replaceColor(int r, int g, int b, const Color &color);

   void setPixel(int x, int y, int r, int g, int b);
   void setPixel(int x, int y, const Color &color);

   void fill(int r, int g, int b);
   void fill(const Color &color);

private:
   MT_IMAGE *image;

   friend class Pixmap;
};


class Pixmap
{
public:
   Pixmap(char **xpm_data);
   Pixmap(const Image &image);
   virtual ~Pixmap();

private:
   MT_PIXMAP *pixmap;

   friend class GC;
};


class Window
{
public:
   Window(MT_WINDOW *win);
   Window(const Window &w);

   MT_WINDOW *window();

   void drawString(MT_STRING *str, int x_offset, int y_offset, Color *color) const;

private:
   MT_WINDOW *win;
};


class GC
{
public:
   GC();
   GC(const Color &color);
   virtual ~GC();

   void setColor(int r, int g, int b);
   void setColor(const Color &color);

   //void translate(int x, int y); /* not implemented, yet */

   void begin(const Window &window);
   void end();

   void drawPoint(int x, int y);
   void drawLine(int x1, int y1, int x2, int y2);
   void drawRectangle(bool filled, int x1, int y1, int x2, int y2);
   void drawRectangle(bool filled, const Rect &rect);
   void drawPolygon(bool filled, MT_POINT *points, int npoints);
   void drawPixmap(const Pixmap &pixmap, int xsrc, int ysrc, int xdest, int ydest, int width, int height);
   void drawPixmapTiled(const Pixmap &pixmap, int srcwidth, int srcheight, int xdest, int ydest, int width, int height);
   void drawGradient(int x, int y, int width, int height, const Color &color1, const Color &color2, int orientation);
   void drawGradient(const Rect &rect, const Color &color1, const Color &color2, int orientation);

private:
   MT_GC *gc;
   MT_WINDOW *win;
   MT_COLOR col;
   int dx, dy;

   MT_EXT_GC
};


class Config
{
public:
   Config(const char *fname);
   Config(MT_CONFIG *cfg);
   virtual ~Config();

   const char *get(const char *name, const char *section = 0);
   Color getColor(const char *name, const char *section = 0);

   /*
   void set(const char *name, const char *value);
   void set(const char *name, const char *section, const char *value);

   void remove(const char *name, const char *section = 0);
   */

private:
   MT_CONFIG *cfg;
   bool external;
};


class ThemeEngine
{
public:
   ThemeEngine(MT_ENGINE *engine);
   virtual ~ThemeEngine();

   virtual void realize() = 0;
   virtual void drawWidget(const Window &window, const Rect &area, int type, int state, const Rect &rect, MT_WIDGET_DATA *data) = 0;
   virtual void drawString(const Window &window, int type, int state, MT_STRING *str) = 0;

   Color palette(int type);
   void setPalette(int type, const Color& color);

   void setMetric(int type, int value);
   void setMetric(int type, int x, int y);

   Config getConfig();

protected:
   MT_ENGINE *engine;

   MT_EXT_THEMEENGINE
};



/********************************/
/*** END OF INTERFACE SECTION ***/
/********************************/

#undef MT_TOOLKIT_HANDLE
#define MT_TOOLKIT_HANDLE mt

#define THEME_ENGINE(klass) \
MT_TOOLKIT *MT_EXT::mt; \
\
extern "C" void metatheme_init(MT_ENGINE *engine, MT_TOOLKIT *toolkit) \
{ \
   mt = engine->toolkit; \
   engine->data = new klass(engine); \
} \
\
extern "C" void metatheme_realize(MT_ENGINE *engine) \
{ \
   mt = engine->toolkit; \
   ((ThemeEngine *)engine->data)->realize(); \
} \
\
extern "C" void metatheme_exit(MT_ENGINE *engine) \
{ \
   mt = engine->toolkit; \
   delete ((ThemeEngine *)engine->data); \
} \
\
extern "C" void metatheme_draw_widget(MT_ENGINE *engine, MT_WINDOW *win, MT_RECTANGLE *area, int type, int state, int x, int y, int width, int height, MT_WIDGET_DATA *data) \
{ \
   mt = engine->toolkit; \
   ((ThemeEngine *)engine->data)->drawWidget(Window(win), area? Rect(area) : Rect(0, 0, -1, -1), type, state, Rect(x, y, width, height), data); \
} \
\
extern "C" void metatheme_draw_string(MT_ENGINE *engine, MT_WINDOW *win, int type, int state, MT_STRING *str) \
{ \
   mt = engine->toolkit; \
   ((ThemeEngine *)engine->data)->drawString(Window(win), type, state, str); \
}

inline Color::Color() { mt_color_set(col, 0, 0, 0); }
inline Color::Color(const Color &color) { col = color.col; }
inline Color::Color(int r, int g, int b) { mt_color_set(col, r, g, b); }
inline Color::Color(int r, int g, int b, int a) { mt_color_set2(col, r, g, b, a); }
inline Color::Color(MT_COLOR *color) { col = *color; }
inline int Color::r() const { return col.r; }
inline int Color::g() const { return col.g; }
inline int Color::b() const { return col.b; }
inline int Color::a() const { return col.a; }
inline void Color::set(int r, int g, int b) { mt_color_set(col, r, g, b); }
inline void Color::set(int r, int g, int b, int a) { mt_color_set2(col, r, g, b, a); }

inline void Color::getHsv(int *h, int *s, int *v) const
{
   mt_color_get_hsv((MT_COLOR *)&col, h, s, v);
}

inline void Color::setHsv(int h, int s, int v)
{
   mt_color_set_hsv(&col, h, s, v);
}

inline Color Color::light(int factor) const
{
   MT_COLOR result;
   mt_color_light(&result, (MT_COLOR *)&col, factor);
   return Color(&result);
}

inline Color Color::dark(int factor) const
{
   MT_COLOR result;
   mt_color_dark(&result, (MT_COLOR *)&col, factor);
   return Color(&result);
}

inline Palette::Palette() {}
inline void Palette::set(int type, int r, int g, int b) { mt_color_set(pal[type], r, g, b); }
inline void Palette::set(int type, const Color &color) { pal[type] = color.col; }
inline Color Palette::get(int type) { return Color(&pal[type]); }

inline Rect::Rect() { mt_rectangle_set(r, 0, 0, 0, 0); }
inline Rect::Rect(int x, int y, int width, int height) { mt_rectangle_set(r, x, y, width, height); }
inline Rect::Rect(MT_RECTANGLE *rect) { r = *rect; }
inline int Rect::x() const { return r.x; }
inline int Rect::y() const { return r.y; }
inline int Rect::width() const { return r.width; }
inline int Rect::height() const { return r.height; }
inline int Rect::left() const { return r.x; }
inline int Rect::top() const { return r.y; }
inline int Rect::right() const { return r.x + r.width - 1; }
inline int Rect::bottom() const { return r.y + r.height - 1; }
inline void Rect::setX(int x) { r.x = x; }
inline void Rect::setY(int y) { r.y = y; }
inline void Rect::setWidth(int width) { r.width = width; }
inline void Rect::setHeight(int height) { r.height = height; }
inline void Rect::setLeft(int x) { r.width += (r.x - x); r.x = x; }
inline void Rect::setTop(int y) { r.height += (r.y - y); r.y = y; }
inline void Rect::setRight(int x) { r.width += (x - right()); }
inline void Rect::setBottom(int y) { r.height += (y - bottom()); }
inline void Rect::translate(int dx, int dy) { r.x += dx; r.y += dy; }

inline Image::Image(const Image &image) { this->image = mt_image_copy(image.image); }
inline Image::Image(int width, int height) { image = mt_image_new(width, height); }
inline Image::Image(char **xpm_data) { image = mt_image_new_from_xpm(xpm_data); }
inline Image::~Image() { mt_image_destroy(image); }

inline Pixmap::Pixmap(char **xpm_data) { pixmap = mt_pixmap_new_from_xpm(xpm_data); }
inline Pixmap::Pixmap(const Image &image) { pixmap = mt_pixmap_new_from_image(image.image); }
inline Pixmap::~Pixmap() { mt_pixmap_destroy(pixmap); }

inline Window::Window(MT_WINDOW *win) { this->win = win; }
inline Window::Window(const Window &w) { win = w.win; }
inline MT_WINDOW *Window::window() { return win; }

inline void Window::drawString(MT_STRING *str, int x_offset, int y_offset, Color *color) const
{
   mt_draw_string(win, str, x_offset, y_offset, (color)? color->color() : NULL);
}

inline GC::GC(): win(NULL), dx(0), dy(0)
{
   MT_COLOR col;
   gc = mt_gc_new_with_foreground(&col);
}

inline GC::GC(const Color &color): win(NULL), dx(0), dy(0)
{
   gc = mt_gc_new_with_foreground((MT_COLOR *)&color.col);
}

inline GC::~GC() { mt_gc_destroy(gc); }
inline void GC::setColor(int r, int g, int b) { mt_color_set(col, r, g, b); mt_gc_set_foreground(gc, &col); }
inline void GC::setColor(const Color &color) { col = color.col; mt_gc_set_foreground(gc, &col); }
//inline void GC::translate(int x, int y) { }
inline void GC::begin(const Window &window) { win = ((Window)window).window(); }
inline void GC::end() { win = NULL; }
inline void GC::drawPoint(int x, int y) { mt_draw_point(win, gc, x, y); }
inline void GC::drawLine(int x1, int y1, int x2, int y2) { mt_draw_line(win, gc, x1, y1, x2, y2); }
inline void GC::drawRectangle(bool filled, int x1, int y1, int x2, int y2) { mt_draw_rectangle(win, gc, filled, x1, y1, x2, y2); }
inline void GC::drawRectangle(bool filled, const Rect &rect) { mt_draw_rectangle(win, gc, filled, rect.x(), rect.y(), rect.right(), rect.bottom()); }
inline void GC::drawPolygon(bool filled, MT_POINT *points, int npoints) { mt_draw_polygon(win, gc, filled, points, npoints); }
inline void GC::drawPixmap(const Pixmap &pixmap, int xsrc, int ysrc, int xdest, int ydest, int width, int height) { mt_draw_pixmap(win, gc, pixmap.pixmap, xsrc, ysrc, xdest, ydest, width, height); }
inline void GC::drawPixmapTiled(const Pixmap &pixmap, int srcwidth, int srcheight, int xdest, int ydest, int width, int height) { mt_draw_pixmap_tiled(win, gc, pixmap.pixmap, srcwidth, srcheight, xdest, ydest, width, height); }
inline void GC::drawGradient(int x, int y, int width, int height, const Color &color1, const Color &color2, int orientation) { mt_draw_gradient(win, gc, x, y, width, height, (MT_COLOR *)&color1.col, (MT_COLOR *)&color2.col, orientation); }
inline void GC::drawGradient(const Rect &rect, const Color &color1, const Color &color2, int orientation) { mt_draw_gradient(win, gc, rect.x(), rect.y(), rect.width(), rect.height(), (MT_COLOR *)&color1.col, (MT_COLOR *)&color2.col, orientation); }

inline Config::Config(const char *fname) { cfg = mt_load_config(fname); external = false; }
inline Config::Config(MT_CONFIG *cfg) { this->cfg = cfg; external = true; }
inline Config::~Config() { if (!external) mt_free_config(cfg); }
inline const char *Config::get(const char *name, const char *section) { return mt_get_config_option(cfg, name, section); }

inline Color Config::getColor(const char *name, const char *section)
{
   MT_COLOR c;
   mt_color_set_from_config(cfg, &c, name, section);
   return Color(&c);
}

inline ThemeEngine::ThemeEngine(MT_ENGINE *engine) { this->engine = engine; }
inline ThemeEngine::~ThemeEngine() {}
inline Color ThemeEngine::palette(int type) { return Color(&engine->palette[type]); }
inline void ThemeEngine::setPalette(int type, const Color& color) { engine->palette[type] = color.col; }
inline void ThemeEngine::setMetric(int type, int value) { mt_set_metric(engine, type, value); }
inline void ThemeEngine::setMetric(int type, int x, int y) { mt_set_metric_size(engine, type, x, y); }
inline Config ThemeEngine::getConfig() { return Config(engine->config); }

#ifndef MT_EXT
} /* end of namespace metatheme */

using namespace metatheme;
#endif

#endif /* METATHEMECPP_H */
