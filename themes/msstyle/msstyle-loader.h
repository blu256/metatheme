/**
 * This file is part of MetaTheme.
 * Copyright (c) 2005 Petr Jelinek <pjmodos@centrum.cz>
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

#ifndef _msstyle_loader_h_
#define _msstyle_loader_h_

#include "metatheme.h"
#include "metathemeInt.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "msstyle-ini.h"

typedef struct ms_theme_info_s {
	char *defaultini;
	char *name;
	char *author;
	char *copyright;
	char *website;
} ms_theme_info_t;

typedef struct ms_theme_s {
   size_t pedatasize;
   int themefd;
   char *pedata;

   GHashTable *bitmaps;
   GHashTable *ini_files;
   ms_theme_info_t *ms_theme_info;
} ms_theme_t;

ms_theme_t *ms_open_theme(const char *filename);
void ms_close_theme(ms_theme_t *ms);
MT_IMAGE *ms_load_widget(ms_theme_t *ms, MT_ENGINE *engine, char *name, int index);
INI_FILE *ms_load_ini(ms_theme_t *ms, const char *name);

typedef struct bitmap_entry_s {
	MT_IMAGE *image;
	char *PointerToRawData;
	size_t sizeOfRawData;
} bitmap_entry;

#endif
