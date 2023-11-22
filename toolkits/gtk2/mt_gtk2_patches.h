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

#ifndef MT_GTK2_PATCHES_H
#define MT_GTK2_PATCHES_H

void mt_gtk2_install_patches();
void mt_gtk2_uninstall_patches();

int mt_is_range_slider_pressed(GtkWidget *widget);
int mt_is_range_page_pressed(GtkWidget *widget);
void mt_range_get_slider_position(GtkWidget *widget, int *pos, int *size);

#endif /* MT_GTK2_PATCHES_H */
