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

#ifndef MT_GTK2_H
#define MT_GTK2_H

#include <gtk/gtk.h>
#include <string.h>
#include "metathemeInt.h"

#include "mt_gtk2_style.h"

#define USE_AUTO_CLIPPING
/* #define DEBUG */

#ifdef DEBUG
#define DEBUG_MSG(s, p...) fprintf(stderr, s, ##p)
#else
#define DEBUG_MSG(s, p...)
#endif

#define GTK_FULL_VERSION ((gtk_major_version << 16) + (gtk_minor_version << 8) + gtk_micro_version)

#define DETAIL(xx) ((detail) && (!strcmp(xx, detail)))
#define IS_OBJECT(widget, name) (!strcmp(GTK_OBJECT_TYPE_NAME(widget), name))

#define PARENT(widget) ((widget)->parent)
#define PARENT2(widget) ((widget)->parent->parent)
#define PARENT3(widget) ((widget)->parent->parent->parent)

#define HAS_PARENT(widget) ((widget) && PARENT(widget))
#define HAS_PARENT2(widget) ((widget) && PARENT(widget) && PARENT2(widget))
#define HAS_PARENT3(widget) ((widget) && PARENT(widget) && PARENT2(widget) && PARENT3(widget))

extern MetaThemeStyle *mt_style;
extern MT_ENGINE *mt_default_engine;
extern int mt_swap_buttons;
extern MT_RECTANGLE mt_clip_rect;
extern MT_TOOLKIT metatheme_toolkit;

typedef struct {
   GtkStyleClass* parent_class;
   GtkStyle* style;
   GtkStateType state_type;
   gboolean use_text;
	GdkRectangle* area;
   GtkWidget* widget;
	const gchar* detail;
   gint x; gint y;
	PangoLayout* layout;
   MT_COLOR *color;
} MT_GTK_STRING;

typedef struct {
   GdkGC *gc;
   MT_RECTANGLE clip;
} MT_GTK_GC;

MT_ENGINE *mt_get_engine();
void mt_return_engine(MT_ENGINE *engine);

GtkWidget *mt_get_ancestor(GtkWidget *widget, char *name);
void mt_show_ancestors(GtkWidget *widget);

#endif /* MT_GTK2_H */
