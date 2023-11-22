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

#include "mt_gtk2.h"

static void metatheme_rc_style_init (MetaThemeRCStyle * style);
static void metatheme_rc_style_class_init (MetaThemeRCStyleClass * klass);

static void metatheme_rc_style_merge (GtkRcStyle * dest, GtkRcStyle * src);
static GtkStyle *metatheme_rc_style_create_style (GtkRcStyle * rc_style);

static GtkRcStyleClass * parent_class;
GType metatheme_type_rc_style = 0;


void metatheme_rc_style_register_type(GTypeModule *module)
{
   static const GTypeInfo object_info = {
      sizeof(MetaThemeRCStyleClass),
      (GBaseInitFunc)NULL,
      (GBaseFinalizeFunc)NULL,
      (GClassInitFunc)metatheme_rc_style_class_init,
      NULL,
      NULL,
      sizeof(MetaThemeRCStyle),
      0,
      (GInstanceInitFunc)metatheme_rc_style_init,
   };

   metatheme_type_rc_style = g_type_module_register_type(module, GTK_TYPE_RC_STYLE, "MetaThemeRCStyle", &object_info, 0);
}


static void metatheme_rc_style_init(MetaThemeRCStyle *style)
{
   MetaThemeRCStyle *rc = METATHEME_RC_STYLE(style);

   rc->style = NULL;
}


static void metatheme_rc_style_class_init(MetaThemeRCStyleClass *klass)
{
   GtkRcStyleClass *rc_style_class = GTK_RC_STYLE_CLASS(klass);

   parent_class = g_type_class_peek_parent(klass);

   rc_style_class->merge = metatheme_rc_style_merge;
   rc_style_class->create_style = metatheme_rc_style_create_style;
}


static void metatheme_rc_style_merge(GtkRcStyle *dest, GtkRcStyle *src)
{
   parent_class->merge(dest, src);
}


static GtkStyle *metatheme_rc_style_create_style(GtkRcStyle *rc_style)
{
   return g_object_new(METATHEME_TYPE_STYLE, NULL);
}
