/**
 * Copyright (c) 2004 Jirka Zajpt <jzajpt@blueberry.cz>
 *
 * $Id: color_scheme.h 19 2004-07-29 02:45:55Z jzajpt $
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

#ifndef _METATHEME_CONFIG_COLOR_SCHEME__H
#define _METATHEME_CONFIG_COLOR_SCHEME__H


typedef struct
{
	char*      file;
	char*      path;
	char*      name;
	gboolean   modified;
	gboolean   removable;
	MT_CONFIG* conf;
} MTCONFIG_COLOR_SCHEME;


MTCONFIG_COLOR_SCHEME* copy_color_scheme( MTCONFIG_COLOR_SCHEME* color_scheme );
void                   save_color_scheme( MTCONFIG_COLOR_SCHEME* color_scheme );
void                   free_color_scheme( MTCONFIG_COLOR_SCHEME* color_scheme );
gboolean               check_colors_conf( MT_CONFIG* conf );


#endif /* _METATHEME_CONFIG_COLOR_SCHEME__H */
