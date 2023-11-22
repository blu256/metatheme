/**
 * Copyright (c) 2004 Jirka Zajpt <jzajpt@blueberry.cz>
 *
 * $Id: color_scheme.c 19 2004-07-29 02:45:55Z jzajpt $
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

#include <gtk/gtk.h>
#include <glib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "metathemeInt.h"
#include "main.h"
#include "util.h"
#include "color_scheme.h"


MT_CONFIG*
mt_config_copy( MT_CONFIG* conf )
{
	unsigned int i;
	unsigned int j;
	MT_CONFIG* copy = ( MT_CONFIG* ) malloc( sizeof( MT_CONFIG ) );
	ALLOC_CHECK( copy );

	copy->sections = ( MT_CONFIG_SECTION* ) malloc( sizeof( MT_CONFIG_SECTION ) * conf->count );
	ALLOC_CHECK( copy->sections );
	copy->count = conf->count;
	for ( i = 0 ; i < conf->count ; i++ )
	{
		copy->sections[ i ].name  = g_strdup( conf->sections[ i ].name );
		copy->sections[ i ].count = conf->sections[ i ].count;

		copy->sections[ i ].items = ( char** ) malloc( sizeof( char* ) * ( conf->sections[ i ].count * 2 ) );
		ALLOC_CHECK( copy->sections[ i ].items );
		for ( j = 0 ; j < ( conf->sections[ i ].count * 2 ) ; j++ )
		{
			copy->sections[ i ].items[ j ] = g_strdup( conf->sections[ i ].items[ j ] );
		}
	}

	return copy;
}


/*
 * Create's copy of MTCONFIG_COLOR_SCHEME structure.
 * The returned pointer should be freed using mtconfig_free_color_scheme.
 */
MTCONFIG_COLOR_SCHEME*
copy_color_scheme( MTCONFIG_COLOR_SCHEME* color_scheme )
{
	MTCONFIG_COLOR_SCHEME *copy;

	copy = ( MTCONFIG_COLOR_SCHEME* ) malloc( sizeof( MTCONFIG_COLOR_SCHEME ) );
	ALLOC_CHECK( copy );

	copy->file = g_strdup( color_scheme->file );
	copy->path = g_strdup( color_scheme->path );
	copy->name = g_strdup( color_scheme->name );
	copy->modified = color_scheme->modified;
	copy->removable = color_scheme->removable;
	copy->conf = mt_config_copy( color_scheme->conf );

	return copy;
}


/*
 * Checks whether color scheme configuration has all required options.
 */
gboolean
check_colors_conf( MT_CONFIG* conf )
{
	const char *ptr;

	ptr = metatheme_get_config_option( conf, "name", NULL );
	if ( ptr == NULL ) return FALSE;

	ptr = metatheme_get_config_option( conf, "base_bg", NULL );
	if ( ptr == NULL ) return FALSE;
	ptr = metatheme_get_config_option( conf, "base_fg", NULL );
	if ( ptr == NULL ) return FALSE;

	ptr = metatheme_get_config_option( conf, "window_bg", NULL );
	if ( ptr == NULL ) return FALSE;
	ptr = metatheme_get_config_option( conf, "window_fg", NULL );
	if ( ptr == NULL ) return FALSE;

	ptr = metatheme_get_config_option( conf, "button_bg", NULL );
	if ( ptr == NULL ) return FALSE;
	ptr = metatheme_get_config_option( conf, "button_fg", NULL );
	if ( ptr == NULL ) return FALSE;

	ptr = metatheme_get_config_option( conf, "selected_bg", NULL );
	if ( ptr == NULL ) return FALSE;
	ptr = metatheme_get_config_option( conf, "selected_fg", NULL );
	if ( ptr == NULL ) return FALSE;

	ptr = metatheme_get_config_option( conf, "disabled_bg", NULL );
	if ( ptr == NULL ) return FALSE;
	ptr = metatheme_get_config_option( conf, "disabled_fg", NULL );
	if ( ptr == NULL ) return FALSE;

	return TRUE;
}


/*
 * Saves color scheme pointed by structure color_scheme.
 */
void
save_color_scheme( MTCONFIG_COLOR_SCHEME* color_scheme )
{
	char color_scheme_file_name[ 1024 ];
	snprintf( color_scheme_file_name, 1024, "%s%s",
		color_scheme->path,
		color_scheme->file );
	if ( ! metatheme_save_config( color_scheme->conf, color_scheme_file_name ) )
	{
		/* It's a possible that colors dir doesn't exist, so we
		 * try to create it just for sure.
		 */
		mkdir( metatheme_user_colors_dir, S_IRWXU );

		snprintf( color_scheme_file_name, 1024, "%s%s",
			metatheme_user_colors_dir,
			color_scheme->file );

		metatheme_save_config( color_scheme->conf, color_scheme_file_name );
	}
}


/*
 * Frees color scheme.
 */
void
free_color_scheme( MTCONFIG_COLOR_SCHEME* color_scheme )
{
	if ( color_scheme != NULL )
	{
		mtconfig_free( color_scheme->file );
		mtconfig_free( color_scheme->name );
		mtconfig_free( color_scheme->path );
		if ( color_scheme->conf != NULL )
			metatheme_free_config( color_scheme->conf );
		mtconfig_free( color_scheme );
	}
}
