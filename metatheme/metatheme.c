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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dlfcn.h>
#include "metathemeInt.h"

#define GRADIENTS_CACHE_SIZE 32

#undef MT_TOOLKIT_HANDLE
#define MT_TOOLKIT_HANDLE mt

struct _MT_LIBRARY {
   MT_TOOLKIT *toolkit;
   MT_CONFIG *cfg;
};

#ifdef HAVE_CAIRO
static cairo_surface_t *_cairo_surface = NULL;
static unsigned char *_cairo_buf = NULL;
static MT_RECTANGLE _cairo_rect;
static MT_WINDOW *_cairo_win;
#endif


MT_LIBRARY *metatheme_library_init(MT_TOOLKIT *toolkit)
{
   MT_LIBRARY *lib;
   char tmp[256];

   lib = (MT_LIBRARY *)malloc(sizeof(MT_LIBRARY));
   lib->toolkit = toolkit;

   snprintf(tmp, 256, "%s/.metatheme/config", getenv("HOME"));
   
   lib->cfg = metatheme_load_config(tmp);
   if (!lib->cfg) {
      fprintf(stderr, "MetaTheme error: Cannot load configuration file (%s).\n", tmp);
      free(lib);
      return NULL;
   }

   return lib;
}


void metatheme_library_exit(MT_LIBRARY *lib)
{
   metatheme_free_config(lib->cfg);
   free(lib);
}


MT_CONFIG *metatheme_get_config(MT_LIBRARY *lib)
{
   return lib->cfg;
}


MT_ENGINE *metatheme_load_engine(MT_LIBRARY *lib, const char *name)
{
   MT_ENGINE *engine;
   MT_CONFIG *themerc;
   char tmp[256];
   const char *option, *option2;

   if (!name) name = metatheme_get_config_option(lib->cfg, "theme", NULL);

   option = metatheme_get_config_option(lib->cfg, "path", NULL);
   if (!option) {
      option = METATHEMEDIR "/themes";
   }

   strcpy(tmp, option);
   strcat(tmp, "/");
   strcat(tmp, name);
   strcat(tmp, "/themerc");

   themerc = metatheme_load_config(tmp);
   if (!themerc) return NULL;

   option2 = metatheme_get_config_option(themerc, "engine", NULL);
   if (!option2) {
      metatheme_free_config(themerc);
      return NULL;
   }

   if (option2[0] == '/') {
      strcpy(tmp, option2);
   }
   else {
      strcpy(tmp, option);
      strcat(tmp, "/");
      strcat(tmp, option2);
   }

   engine = (MT_ENGINE *)malloc(sizeof(MT_ENGINE));
   engine->toolkit = lib->toolkit;
   engine->library = lib;
   engine->refcount = 1;
   engine->name = (char *)strdup(name);
   engine->config = themerc;
   engine->font = 0;
   engine->realized = 0;
   
   engine->dlhandle = dlopen(tmp, RTLD_LAZY);
   if (!engine->dlhandle) goto error;
   
   engine->init = dlsym(engine->dlhandle, "metatheme_init");
   if (!engine->init) goto error;

   engine->exit = dlsym(engine->dlhandle, "metatheme_exit");
   if (!engine->exit) goto error;
   
   engine->realize = dlsym(engine->dlhandle, "metatheme_realize");
   if (!engine->realize) goto error;

   engine->draw_widget = dlsym(engine->dlhandle, "metatheme_draw_widget");
   if (!engine->draw_widget) goto error;

   engine->draw_string = dlsym(engine->dlhandle, "metatheme_draw_string");
   if (!engine->draw_string) goto error;

   return engine;

error:
   fprintf(stderr, "MetaTheme error: %s\n", dlerror());
   if (engine->dlhandle) dlclose(engine->dlhandle);
   free(engine);
   return NULL;
}


void metatheme_unload_engine(MT_ENGINE *engine)
{
   free(engine->name);
   metatheme_free_config(engine->config);
   free(engine->font);
   dlclose(engine->dlhandle);
   free(engine);
}


void metatheme_load_default_settings(MT_ENGINE *engine)
{
   char tmp[256];
   const char *colors_name;
   MT_CONFIG *colors;
   int i,size,found;
   MT_LIBRARY *lib;

   lib = engine->library;

   colors_name = metatheme_get_config_option(lib->cfg, "colors", NULL);
   if (!colors_name) return;
   
   snprintf(tmp, 256, "%s/.metatheme/colors/%s", getenv("HOME"), colors_name);
   colors = metatheme_load_config(tmp);
   if (!colors) {
      snprintf(tmp, 256, "%s/colors/%s", METATHEMEDIR, colors_name);
      colors = metatheme_load_config(tmp);
      if (!colors) return;
   }

   metatheme_color_set_from_config(colors, &(engine->palette[MT_BACKGROUND]),          "base_bg",     NULL);
   metatheme_color_set_from_config(colors, &(engine->palette[MT_FOREGROUND]),          "base_fg",     NULL);
   metatheme_color_set_from_config(colors, &(engine->palette[MT_WINDOW_BACKGROUND]),   "window_bg",   NULL);
   metatheme_color_set_from_config(colors, &(engine->palette[MT_WINDOW_FOREGROUND]),   "window_fg",   NULL);
   metatheme_color_set_from_config(colors, &(engine->palette[MT_BUTTON_BACKGROUND]),   "button_bg",   NULL);
   metatheme_color_set_from_config(colors, &(engine->palette[MT_BUTTON_FOREGROUND]),   "button_fg",   NULL);
   metatheme_color_set_from_config(colors, &(engine->palette[MT_SELECTED_BACKGROUND]), "selected_bg", NULL);
   metatheme_color_set_from_config(colors, &(engine->palette[MT_SELECTED_FOREGROUND]), "selected_fg", NULL);
   metatheme_color_set_from_config(colors, &(engine->palette[MT_DISABLED_BACKGROUND]), "disabled_bg", NULL);
   metatheme_color_set_from_config(colors, &(engine->palette[MT_DISABLED_FOREGROUND]), "disabled_fg", NULL);

   metatheme_free_config(colors);

   snprintf(tmp, 256, "%s", metatheme_get_config_option(lib->cfg, "font", NULL));
   found = 0;
   for (i=0; i<strlen(tmp); i++) {
      if (tmp[i] == ' ' && tmp[i+1] >= '0' && tmp[i+1] <= '9') {
         tmp[i] = 0;
         found = 1;
         break;
      }
   }

   if (found) {
      if (sscanf(tmp+i+1, "%d", &size) == 1) {
         engine->font = (char *)strdup(tmp);
         engine->font_size = size;
      }
      else {
         found = 0;
      }
   }
   
   if (!found) {
      engine->font = (char *)strdup("Helvetica");
      engine->font_size = 10;
   }
}


static char *trim(char *s)
{
   int len;
   
   while (isspace(*s)) s++;

   len = strlen(s);
   while (isspace(s[len-1])) s[(len--)-1] = 0;
   
   return s;
}


MT_CONFIG *metatheme_load_config(const char *fname)
{
   MT_CONFIG *cfg;
   MT_CONFIG_SECTION *section;
   FILE *f;
   char buf[1024];
   char *s, *tmp;

   f = fopen(fname, "r");
   if (!f) return NULL;

   cfg = (MT_CONFIG *)malloc(sizeof(MT_CONFIG));

   cfg->count = 1;
   cfg->sections = (MT_CONFIG_SECTION *)malloc(sizeof(MT_CONFIG_SECTION));
   
   section = &cfg->sections[0];
   section->name = NULL; /* default section */
   section->count = 0;
   section->items = NULL;

   while (fgets(buf, 1024, f)) {
      s = trim((char *)&buf);

      /* comments and blank lines: */
      if (*s == '#' || *s == 0) continue;

      /* new section: */
      if (strlen(s) > 2 && *s == '[' && *(s + strlen(s) - 1) == ']') {
         s++;
         s[strlen(s)-1] = 0;
         
         ++cfg->count;
         cfg->sections = (MT_CONFIG_SECTION *)realloc(cfg->sections, sizeof(MT_CONFIG_SECTION) * cfg->count);
         section = &cfg->sections[cfg->count-1];
         section->name = (char *)strdup(s);
         section->count = 0;
         section->items = NULL;
         continue;
      }
      
      tmp = (char *)strchr(s, '=');
      if (!tmp) continue;

      *(tmp++) = 0;

      s = trim(s);
      tmp = trim(tmp);

      ++section->count;
      section->items = (char **)realloc(section->items, sizeof(char *) * section->count * 2);
      section->items[(section->count-1)*2] = (char *)strdup(s);
      section->items[(section->count-1)*2+1] = (char *)strdup(tmp);
   }

   fclose(f);

   return cfg;
}


int metatheme_save_config(MT_CONFIG *cfg, const char *fname)
{
   int i,j;
   MT_CONFIG_SECTION *sec;
   FILE *f;

   f = fopen(fname, "wb");
   if (!f) return 0;

   for (i=0; i<cfg->count; i++) {
      sec = &cfg->sections[i];
      if (sec->name) {
         fprintf(f, "\n[%s]\n", sec->name);
      }

      for (j=0; j<sec->count; j++) {
         fprintf(f, "%s = %s\n", sec->items[j*2], sec->items[j*2+1]);
      }
   }

   fclose(f);
   return 1;
}


void metatheme_free_config(MT_CONFIG *cfg)
{
   int i;
   MT_CONFIG_SECTION *section;

   for (i=0; i<cfg->count; i++) {
      section = &cfg->sections[i];
      free(section->name);
      free(section->items);
   }
   free(cfg->sections);
   free(cfg);
}


const char *metatheme_get_config_option(MT_CONFIG *cfg, const char *name, const char *section)
{
   int i,j;
   MT_CONFIG_SECTION *sec;

   for (i=0; i<cfg->count; i++) {
      sec = &cfg->sections[i];
      if ((section == NULL && sec->name == NULL) || (section != NULL && sec->name != NULL && !strcmp(section, sec->name))) {
         for (j=0; j<sec->count; j++) {
            if (!strcmp(sec->items[j*2], name)) {
               return sec->items[j*2+1];
            }
         }
      }
   }

   return NULL;
}


static void parse_color(MT_COLOR *dest, const char *str)
{
   if (!str) return;

   sscanf(str, "%d,%d,%d", &dest->r, &dest->g, &dest->b);
   dest->a = 255;
}


void metatheme_color_set_from_config(MT_CONFIG *cfg, MT_COLOR *col, const char *name, const char *section)
{
   parse_color(col, metatheme_get_config_option(cfg, name, section));
}


void metatheme_set_config_option(MT_CONFIG *cfg, const char *name, const char *section, const char *value)
{
   int i,j;
   MT_CONFIG_SECTION *sec;

   for (i=0; i<cfg->count; i++) {
      sec = &cfg->sections[i];
      if ((section == NULL && sec->name == NULL) || (section != NULL && sec->name != NULL && !strcmp(section, sec->name))) {
         for (j=0; j<sec->count; j++) {
            if (!strcmp(sec->items[j*2], name)) {
               free(sec->items[j*2+1]);
               sec->items[j*2+1] = (char *)strdup(value);
               return;
            }
         }
         goto add;
      }
   }

   ++cfg->count;
   cfg->sections = (MT_CONFIG_SECTION *)realloc(cfg->sections, sizeof(MT_CONFIG_SECTION) * cfg->count);
   sec = &cfg->sections[cfg->count-1];
   sec->name = (char *)strdup(section);
   sec->count = 0;
   sec->items = NULL;

add:
   ++sec->count;
   sec->items = (char **)realloc(sec->items, sizeof(char *) * sec->count * 2);
   sec->items[(sec->count-1)*2] = (char *)strdup(name);
   sec->items[(sec->count-1)*2+1] = (char *)strdup(value);
}


static MT_PIXMAP *create_gradient(MT_TOOLKIT *mt, int width, int height, MT_COLOR *ca, MT_COLOR *cb, int orientation)
{
   unsigned int r, g, b;
   unsigned int rdiff, gdiff, bdiff;
   unsigned int rd, gd, bd;
   int x, y;

   MT_IMAGE *img;
   MT_PIXMAP *pixmap;
   MT_COLOR col;

   if (width < 1) width = 1;
   if (height < 1) height = 1;

   img = mt_image_new(width, height);

   r = ca->r << 16;
   g = ca->g << 16;
   b = ca->b << 16;

   rdiff = cb->r - ca->r;
   gdiff = cb->g - ca->g;
   bdiff = cb->b - ca->b;

   col.a = 255;

   if (orientation == MT_VERTICAL) {
      rd = ((1<<16) / height) * rdiff;
      gd = ((1<<16) / height) * gdiff;
      bd = ((1<<16) / height) * bdiff;

      for (y=0; y<height; y++) {
         col.r = (r += rd) >> 16;
         col.g = (g += gd) >> 16;
         col.b = (b += bd) >> 16;
         for (x=0; x<width; x++) {
            mt_image_set_pixel(img, x, y, &col);
         }
      }
   }
   else {
      rd = ((1<<16) / width) * rdiff;
      gd = ((1<<16) / width) * gdiff;
      bd = ((1<<16) / width) * bdiff;

      for (x=0; x<width; x++) {
         col.r = (r += rd) >> 16;
         col.g = (g += gd) >> 16;
         col.b = (b += bd) >> 16;
         for (y=0; y<height; y++) {
            mt_image_set_pixel(img, x, y, &col);
         }
      }
   }

   pixmap = mt_pixmap_new_from_image(img);
   mt_image_destroy(img);
   return pixmap;
}


static MT_PIXMAP *get_gradient(MT_TOOLKIT *mt, int width, int height, MT_COLOR *ca, MT_COLOR *cb, int orientation)
{
   static struct {
      MT_PIXMAP *pixmap;
      int w,h,orientation;
      MT_COLOR ca, cb;
      int count;
   } gradients[GRADIENTS_CACHE_SIZE];
   static int initialized = 0;
   
   int i, j, lc;

   if (!initialized) {
      initialized = 1;
      for (i=0; i<GRADIENTS_CACHE_SIZE; i++) {
         gradients[i].w = -1;
         gradients[i].count = 0;
      }
   }

   j = -1;
   lc = 0x7FFFFFFF;
   for (i=0; i<GRADIENTS_CACHE_SIZE; i++) {
      if (gradients[i].w == width && gradients[i].h == height && gradients[i].orientation == orientation &&
         gradients[i].ca.r == ca->r && gradients[i].ca.g == ca->g && gradients[i].ca.b == ca->b &&
         gradients[i].cb.r == cb->r && gradients[i].cb.g == cb->g && gradients[i].cb.b == cb->b) {
         gradients[i].count++;
         return gradients[i].pixmap;
      }

      if (gradients[i].count < lc) {
         j = i;
         lc = gradients[i].count;
      }

      gradients[i].count--;
      if (gradients[i].count < 0) gradients[i].count = 0;
   }

   if (gradients[j].w != -1) mt_pixmap_destroy(gradients[j].pixmap);

   gradients[j].w = width;
   gradients[j].h = height;
   gradients[j].orientation = orientation;
   gradients[j].ca.r = ca->r;
   gradients[j].ca.g = ca->g;
   gradients[j].ca.b = ca->b;
   gradients[j].cb.r = cb->r;
   gradients[j].cb.g = cb->g;
   gradients[j].cb.b = cb->b;
   gradients[j].count = 0xFFFF;
   gradients[j].pixmap = create_gradient(mt, width, height, ca, cb, orientation);
   return gradients[j].pixmap;
}


static void _mt_draw_gradient(MT_TOOLKIT *mt, MT_WINDOW *win, MT_GC *gc, int x, int y, int width, int height, MT_COLOR *color1, MT_COLOR *color2, int orientation)
{
   MT_PIXMAP *tmp;
   
   if (orientation == MT_VERTICAL) {
      if (width < 8) goto simple;
      tmp = get_gradient(mt, 32, height, color1, color2, orientation);
      mt_draw_pixmap_tiled(win, gc, tmp, 32, height, x, y, width, height);
   }
   else {
      if (height < 8) goto simple;
      tmp = get_gradient(mt, width, 32, color1, color2, orientation);
      mt_draw_pixmap_tiled(win, gc, tmp, width, 32, x, y, width, height);
   }
   return;

simple:
   tmp = get_gradient(mt, width, height, color1, color2, orientation);
   mt_draw_pixmap(win, gc, tmp, 0, 0, x, y, width, height);
}


static void _mt_color_get_hsv(MT_COLOR *col, int *h, int *s, int *v)
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


static void _mt_color_set_hsv(MT_COLOR *result, int h, int s, int v)
{
   int r=v, g=v, b=v;
   unsigned int f, p, q, t;

   if (h < -1 || s > 255 || v > 255) return;

   if (s == 0 || h == -1) { /* achromatic case */
      /* Ignore */
   }
   else { /* chromatic case */
      if ((unsigned int)h >= 360) {
         h %= 360;
      }

      f = h%60;
      h /= 60;
      p = (unsigned int)(2*v*(255-s)+255)/510;
      if (h & 1) {
         q = (unsigned int)(2*v*(15300-s*f)+15300)/30600;
         switch (h) {
            case 1: r=(int)q; g=(int)v, b=(int)p; break;
            case 3: r=(int)p; g=(int)q, b=(int)v; break;
            case 5: r=(int)v; g=(int)p, b=(int)q; break;
         }
      }
      else {
         t = (unsigned int)(2*v*(15300-(s*(60-f)))+15300)/30600;
         switch (h) {
            case 0: r=(int)v; g=(int)t, b=(int)p; break;
            case 2: r=(int)p; g=(int)v, b=(int)t; break;
            case 4: r=(int)t; g=(int)p, b=(int)v; break;
         }
      }
   }

   result->r = r;
   result->g = g;
   result->b = b;
   result->a = 255;
}


static void _mt_color_dark(MT_COLOR *result, MT_COLOR *col, int factor);

static void _mt_color_light(MT_COLOR *result, MT_COLOR *col, int factor)
{
   int h, s, v;

   if (factor <= 0) { /* invalid lightness factor */
      *result = *col;
      return;
   }
   else if (factor < 100) {
      /* makes color darker: */
      _mt_color_dark(result, col, 10000/factor);
      return;
   }

   _mt_color_get_hsv(col, &h, &s, &v);
   v = (factor*v)/100;
   if (v > 255) { /* overflow */
      s -= v-255; /* adjust saturation */
      if (s < 0) s = 0;
      v = 255;
   }

   _mt_color_set_hsv(result, h, s, v);
}


static void _mt_color_dark(MT_COLOR *result, MT_COLOR *col, int factor)
{
   int h, s, v;

   if (factor <= 0) {
      /* invalid darkness factor */
   	*result = *col;
      return;
   }
   else if (factor < 100) {
      /* makes color lighter: */
   	_mt_color_light(result, col, 10000/factor);
      return;
   }
   
   _mt_color_get_hsv(col, &h, &s, &v);
   v = (v*100)/factor;
   _mt_color_set_hsv(result, h, s, v);
}


#ifdef HAVE_CAIRO

static cairo_t *_mt_cairo_begin(MT_TOOLKIT *mt, MT_WINDOW *win, MT_RECTANGLE *area, int x, int y, int width, int height)
{
   cairo_t *cr;

   _cairo_buf = (unsigned char *)malloc(4 * width * height);
   _cairo_win = win;
   mt_rectangle_set(_cairo_rect, x, y, width, height);

   _cairo_surface = cairo_image_surface_create_for_data(_cairo_buf, CAIRO_FORMAT_RGB24, width, height, 4*width);
   cr = cairo_create(_cairo_surface);
   return cr;
}


static void _mt_cairo_end(MT_TOOLKIT *mt, cairo_t *cr)
{
   MT_GC *gc;
   int i,j;
   MT_COLOR col;

   cairo_destroy(cr);

   mt_color_set(col, 0, 0, 0);
   gc = mt_gc_new_with_foreground(&col);
   for (i=0; i<_cairo_rect.height; i++) {
      for (j=0; j<_cairo_rect.width; j++) {
         col.r = _cairo_buf[4*(_cairo_rect.width*i+j) + 2];
         col.g = _cairo_buf[4*(_cairo_rect.width*i+j) + 1];
         col.b = _cairo_buf[4*(_cairo_rect.width*i+j) + 0];
         mt_gc_set_foreground(gc, &col);
         mt_draw_point(_cairo_win, gc, _cairo_rect.x+j, _cairo_rect.y+i);
      }
   }
   mt_gc_destroy(gc);

   cairo_surface_destroy(_cairo_surface);
   free(_cairo_buf);
}

#endif


void metatheme_toolkit_init(MT_TOOLKIT *toolkit)
{
   toolkit->load_engine = metatheme_load_engine;
   toolkit->unload_engine = metatheme_unload_engine;

   toolkit->get_config = metatheme_get_config;
   toolkit->load_config = metatheme_load_config;
   toolkit->free_config = metatheme_free_config;
   toolkit->get_config_option = metatheme_get_config_option;
   toolkit->color_set_from_config = metatheme_color_set_from_config;

   toolkit->draw_gradient = _mt_draw_gradient;

   toolkit->color_get_hsv = _mt_color_get_hsv;
   toolkit->color_set_hsv = _mt_color_set_hsv;
   toolkit->color_light = _mt_color_light;
   toolkit->color_dark = _mt_color_dark;

#ifdef HAVE_CAIRO
   /* revert to slow method when toolkit doesn't have support for Cairo: */
   if (!toolkit->cairo_begin) {
      toolkit->cairo_begin = _mt_cairo_begin;
      toolkit->cairo_end = _mt_cairo_end;
   }
#endif
}
