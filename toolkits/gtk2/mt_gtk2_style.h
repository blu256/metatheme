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

#ifndef MT_GTK2_STYLE_H
#define MT_GTK2_STYLE_H

#include "metatheme.h"

typedef struct _MetaThemeStyle MetaThemeStyle;
typedef struct _MetaThemeStyleClass MetaThemeStyleClass;
typedef struct _MetaThemeRCStyle MetaThemeRCStyle;
typedef struct _MetaThemeRCStyleClass MetaThemeRCStyleClass;

extern GType metatheme_type_style;
extern GType metatheme_type_rc_style;

#define METATHEME_TYPE_STYLE                 metatheme_type_style
#define METATHEME_STYLE(object)              (G_TYPE_CHECK_INSTANCE_CAST ((object), METATHEME_TYPE_STYLE, MetaThemeStyle))
#define METATHEME_STYLE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), METATHEME_TYPE_STYLE, MetaThemeStyleClass))
#define METATHEME_IS_STYLE(object)           (G_TYPE_CHECK_INSTANCE_TYPE ((object), METATHEME_TYPE_STYLE))
#define METATHEME_IS_STYLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), METATHEME_TYPE_STYLE))
#define METATHEME_STYLE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), METATHEME_TYPE_STYLE, MetaThemeStyleClass))

#define METATHEME_TYPE_RC_STYLE              metatheme_type_rc_style
#define METATHEME_RC_STYLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), METATHEME_TYPE_RC_STYLE, MetaThemeRCStyle))
#define METATHEME_RC_STYLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), METATHEME_TYPE_RC_STYLE, MetaThemeRCStyleClass))
#define METATHEME_IS_RC_STYLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), METATHEME_TYPE_RC_STYLE))
#define METATHEME_IS_RC_STYLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), METATHEME_TYPE_RC_STYLE))
#define METATHEME_RC_STYLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), METATHEME_TYPE_RC_STYLE, MetaThemeRCStyleClass))

struct _MetaThemeStyle {
   GtkStyle parent_instance;

   MT_ENGINE *engine;
   int realized;
};

struct _MetaThemeStyleClass {
   GtkStyleClass parent_class;
};

struct _MetaThemeRCStyle {
   GtkRcStyle parent_instance;

   GtkStyle *style;
};

struct _MetaThemeRCStyleClass {
   GtkRcStyleClass parent_class;
};

void metatheme_style_register_type(GTypeModule *module);
void metatheme_rc_style_register_type(GTypeModule *module);

#endif /* MT_GTK2_STYLE_H */
