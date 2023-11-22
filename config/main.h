/**
 * Copyright (c) 2004 Jirka Zajpt <jzajpt@blueberry.cz>
 *
 * $Id: main.h 19 2004-07-29 02:45:55Z jzajpt $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program  if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _METATHEME_CONFIG_MAIN__H
#define _METATHEME_CONFIG_MAIN__H

#include <sys/types.h>

typedef struct
{
	char*    name;
	char*    engine;
	char*    description;
	char*    author;
	char*    directory;
	char*    config;
} MTCONFIG_THEME_INFO;

#include "color_scheme.h"
#include "settings.h"

extern char     metatheme_user_dir[ 1024 ];
extern char     metatheme_user_config_file[ 1024 ];
extern char     metatheme_user_colors_dir[ 1024 ];
extern gboolean is_embed;
extern pid_t    parent_pid;

extern GtkWidget*                main_window;
extern GtkSizeGroup*             vsizegroup;
extern GList*                    list_themes_info;
extern GList*                    list_color_schemes;


int  create_window( void );


#endif /* _METATHEME_CONFIG_MAIN__H */
