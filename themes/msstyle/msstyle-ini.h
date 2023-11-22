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

#ifndef MSSTYLE_INI_H 
#define MSSTYLE_INI_H

#include <glib.h>

typedef struct {
   GHashTable *keys;
} INI_SECTION;

typedef struct {
   char *start;
   int length;
   GHashTable *sections;
   GPtrArray *sorted;
} INI_FILE;

GHashTable *msstyle_ini_files_new();
INI_FILE *msstyle_ini_get(GHashTable *ini_files, const char *name);
char *msstyle_ini_get_option(INI_FILE *ini, char *section, char *name);
char *msstyle_ini_get_class_option(INI_FILE *ini, char *class1, char *class2, char *state, char *name);

#endif /* MSSTYLE_INI_H */
