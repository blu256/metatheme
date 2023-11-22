/**
 * Copyright (c) 2004 Jirka Zajpt <jzajpt@blueberry.cz>
 *
 * $Id: settings.c 19 2004-07-29 02:45:55Z jzajpt $
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

#include <glib.h>
#include <gtk/gtk.h>
#include "metathemeInt.h"
#include "color_scheme.h"
#include "settings.h"


/* Current settings which user has set. */
MTCONFIG_CURRENT_SETTINGS current_settings;


void settings_initialize( void )
{
	current_settings.application_font     = NULL;
	current_settings.theme                = NULL;
	current_settings.color_scheme         = NULL;
	current_settings.current_color_scheme = 0;
	current_settings.current_widget       = 0;
	current_settings.current_theme        = 0;
	current_settings.watch                = FALSE;
	current_settings.notify_func          = NULL;
}


void settings_destruct( void )
{
	free( current_settings.application_font );
	free( current_settings.theme );
	free_color_scheme( current_settings.color_scheme );
	current_settings.application_font = NULL;
	current_settings.theme            = NULL;
	current_settings.color_scheme     = NULL;
}


void settings_enable_watch( void( *func )( void ) )
{
	current_settings.watch = TRUE;
	current_settings.notify_func = func;
}


void settings_set_application_font( const char* name )
{
	free( current_settings.application_font );
	current_settings.application_font = g_strdup( name );
	if ( current_settings.watch == TRUE )
		current_settings.notify_func();
}


void settings_set_theme( const char* name )
{
	free( current_settings.theme );
	current_settings.theme = g_strdup( name );
}


void settings_set_color_scheme( MTCONFIG_COLOR_SCHEME* color_scheme )
{
	free( current_settings.color_scheme );
	current_settings.color_scheme = copy_color_scheme( color_scheme );
}


void settings_set_current_color_scheme( int i )
{
	current_settings.current_color_scheme = i;
	if ( current_settings.watch == TRUE )
		current_settings.notify_func();
}


void settings_set_current_widget( int i )
{
	current_settings.current_widget = i;
}


void settings_set_current_theme( int i )
{
	current_settings.current_theme = i;
	if ( current_settings.watch == TRUE )
		current_settings.notify_func();
}
