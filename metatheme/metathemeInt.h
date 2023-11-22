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

#ifndef METATHEMEINT_H
#define METATHEMEINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "metatheme.h"


struct _MT_ENGINE {
   MT_TOOLKIT *toolkit;
   MT_LIBRARY *library;
   int refcount;

   char *name;
   void *data;
   MT_CONFIG *config;

   MT_COLOR_PALETTE palette;
   int metric[MT_METRIC_MAX];
   MT_POINT metric_size[MT_METRIC_SIZE_MAX];
   char *font;
   int font_size;

   void *dlhandle;
   int realized; /* this is optional and used only in some toolkit backends in their own way */

   void (*init)(MT_ENGINE *engine);
   void (*exit)(MT_ENGINE *engine);
   void (*realize)(MT_ENGINE *engine);
   void (*draw_widget)(MT_ENGINE *engine, MT_WINDOW *win, MT_RECTANGLE *area, int type, int state, int x, int y, int width, int height, MT_WIDGET_DATA *data);
   void (*draw_string)(MT_ENGINE *engine, MT_WINDOW *win, int type, int state, MT_STRING *str);
};

void metatheme_toolkit_init(MT_TOOLKIT *toolkit);

MT_LIBRARY *metatheme_library_init(MT_TOOLKIT *toolkit);
void metatheme_library_exit(MT_LIBRARY *lib);
MT_CONFIG *metatheme_get_config(MT_LIBRARY *lib);

MT_ENGINE *metatheme_load_engine(MT_LIBRARY *lib, const char *name);
void metatheme_unload_engine(MT_ENGINE *engine);
void metatheme_load_default_settings(MT_ENGINE *engine);

MT_CONFIG *metatheme_load_config(const char *fname);
int metatheme_save_config(MT_CONFIG *cfg, const char *fname);
void metatheme_free_config(MT_CONFIG *cfg);
const char *metatheme_get_config_option(MT_CONFIG *cfg, const char *name, const char *section);
void metatheme_color_set_from_config(MT_CONFIG *cfg, MT_COLOR *col, const char *name, const char *section);
void metatheme_set_config_option(MT_CONFIG *cfg, const char *name, const char *section, const char *value);

void metatheme_refresh();

#ifdef __cplusplus
}
#endif

#endif /* METATHEMEINT_H */
